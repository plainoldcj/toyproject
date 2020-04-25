//
//  Renderer.m
//  macos_client
//
//  Created by Christian Jäger on 30.3.2020.
//  Copyright © 2020 Christian Jäger. All rights reserved.
//

#import <simd/simd.h>
#import <ModelIO/ModelIO.h>

#import "Renderer.h"

// Include header shared between C code here, which executes Metal API commands, and .metal files
#import "ShaderTypes.h"

#include <universal/game_api.h>
#include <universal/graphics.h>
#include <universal/vertex.h>

#define GFX_MAX_BUFFER_COUNT    128
#define GFX_MAX_TEXTURE_COUNT   256

struct GfxBuffer
{
    id<MTLBuffer>   buffer;
    uint16_t        generation;
    uint16_t        nextFree;
};

struct GfxTexture
{
    id<MTLTexture>  texture;
    uint16_t        generation;
    uint16_t        nextFree;
    
    uint16_t        width;
    uint16_t        height;
    uint8_t         bytesPerPixel;
};

extern struct GameApi* g_gameApi;

static const NSUInteger kMaxBuffersInFlight = 3;

static const size_t kAlignedUniformsSize = (sizeof(Uniforms) & ~0xFF) + 0x100;

@implementation Renderer
{
    dispatch_semaphore_t _inFlightSemaphore;
    id <MTLDevice> _device;
    id <MTLCommandQueue> _commandQueue;

    id <MTLBuffer> _dynamicUniformBuffer;
    id <MTLRenderPipelineState> _pipelineState;
    id <MTLDepthStencilState> _depthState;
    id <MTLTexture> _colorMap;
    MTLVertexDescriptor *_mtlVertexDescriptor;

    uint32_t _uniformBufferOffset;

    uint8_t _uniformBufferIndex;

    void* _uniformBufferAddress;

    matrix_float4x4 _projectionMatrix;
    matrix_float4x4 _modelViewMatrix;

    float _rotation;

    MTKMesh *_mesh;
    
    // cj
    struct GfxBuffer    _buffers[GFX_MAX_BUFFER_COUNT];
    uint16_t            _firstFreeBuffer;
    
    struct GfxTexture   _textures[GFX_MAX_TEXTURE_COUNT];
    uint16_t            _firstFreeTexture;
    
    id<MTLRenderCommandEncoder> _renderEncoder;
}

static hgbuffer_t Graphics_CreateBuffer(void* ins, void* data, uint32_t size)
{
    Renderer* renderer = (__bridge Renderer*)ins;
    return [renderer createBufferWithData:data andSize:size];
}

static void Graphics_DestroyBuffer(void* ins, hgbuffer_t hgbuffer)
{
    Renderer* renderer = (__bridge Renderer*)ins;
    return [renderer destroyBuffer:hgbuffer];
}

static void Graphics_SetBufferData(void* ins, hgbuffer_t hgbuffer, void* data, uint32_t size)
{
    Renderer* renderer = (__bridge Renderer*)ins;
    return [renderer setBufferData:hgbuffer data:data size:size];
}

static hgtex_t Graphics_CreateTexture(void* ins, uint16_t width, uint16_t height, uint16_t format)
{
    Renderer* renderer = (__bridge Renderer*)ins;
    return [renderer createTextureWithWidth: width height: height format: format];
}

static void Graphics_SetTextureData(void* ins, hgtex_t hgtex, void* pixelData)
{
    Renderer* renderer = (__bridge Renderer*)ins;
    [renderer setTextureData:hgtex pixelData:pixelData];
}

static void Graphics_BindTexture(void* ins, hgtex_t tex)
{
    Renderer* renderer = (__bridge Renderer*)ins;
    return [renderer bindTexture:tex];
}

static void Graphics_SetUniforms(void* ins, struct GfxUniforms* uniforms)
{
    Renderer* renderer = (__bridge Renderer*)ins;
    return [renderer setUniforms:uniforms];
}

static void Graphics_SetUniformBuffer(void* ins, hgbuffer_t buffer, uint32_t offset)
{
    Renderer* renderer = (__bridge Renderer*)ins;
    return [renderer setUniformBuffer:buffer offset:offset];
}

static void Graphics_DrawPrimitives(void* ins, hgbuffer_t hgbuffer, uint16_t first, uint16_t count)
{
    Renderer* renderer = (__bridge Renderer*)ins;
    return [renderer drawPrimitives:hgbuffer first:first count:count];
}

-(hgbuffer_t)createBufferWithData:(void*)data andSize:(uint32_t)size;
{
    if(_firstFreeBuffer == (uint16_t)~0)
    {
        // TODO(cj): Error reporting.
        hgbuffer_t invalid = { 0, 0 };
        return invalid;
    }
    
    hgbuffer_t handle;
    
    handle.index = _firstFreeBuffer;
    
    struct GfxBuffer* buffer = &_buffers[_firstFreeBuffer];
    _firstFreeBuffer = buffer->nextFree;
    
    handle.generation = ++buffer->generation;
    
    buffer->buffer = [_device newBufferWithBytes:data length:size options:MTLResourceStorageModeShared];
    
    return handle;
}

// TODO(cj): Move this method.
-(struct GfxBuffer*)resolveBufferHandle:(hgbuffer_t)hgbuffer;
{
    assert(hgbuffer.index < GFX_MAX_BUFFER_COUNT);
    
    struct GfxBuffer* const buffer = &_buffers[hgbuffer.index];
    
    assert(hgbuffer.generation == buffer->generation);
    
    return buffer;
}

-(void)destroyBuffer:(hgbuffer_t)hgbuffer;
{
    struct GfxBuffer* buffer = [self resolveBufferHandle:hgbuffer];
    
    // TODO(cj): How to destroy the buffer and what is ARC?
    // [buffer->buffer release];
    
    ++buffer->generation;
    
    buffer->nextFree = _firstFreeBuffer;
    _firstFreeBuffer = hgbuffer.index;
}

-(void)setBufferData:(hgbuffer_t)hgbuffer data:(void*)data size:(uint32_t) size;
{
    struct GfxBuffer* buffer = [self resolveBufferHandle:hgbuffer];
    memcpy(buffer->buffer.contents, data, size);
}

-(hgtex_t)createTextureWithWidth:(uint16_t) width height:(uint16_t)height format:(uint16_t)format;
{
    if(_firstFreeTexture == (uint16_t)~0)
    {
        // TODO(cj): Error reporting.
        hgtex_t invalid = { 0, 0 };
        return invalid;
    }
    
    MTLPixelFormat internalFormat = MTLPixelFormatInvalid;
    uint8_t bytesPerPixel;
    switch(format)
    {
        case GfxPixelFormat_RGBA8:
            internalFormat = MTLPixelFormatRGBA8Unorm;
            bytesPerPixel = 4;
            break;
    }
    
    if(internalFormat == MTLPixelFormatInvalid)
    {
        // TODO(cj): Error reporting.
        hgtex_t invalid = { 0, 0 };
        return invalid;
    }
    
    hgtex_t handle;
    
    handle.index = _firstFreeTexture;
    
    struct GfxTexture* tex = &_textures[_firstFreeTexture];
    _firstFreeTexture = tex->nextFree;
    
    handle.generation = ++tex->generation;
    
    MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];

    desc.width = width;
    desc.height = height;
    desc.pixelFormat = internalFormat;
    
    tex->texture = [_device newTextureWithDescriptor:desc];
    
    tex->width = width;
    tex->height = height;
    tex->bytesPerPixel = 4;
    
    return handle;
}

// TODO(cj): Move this method.
-(struct GfxTexture*)resolveTextureHandle:(hgtex_t)hgtex;
{
    assert(hgtex.index < GFX_MAX_TEXTURE_COUNT);
    
    struct GfxTexture* const tex = &_textures[hgtex.index];
    
    assert(hgtex.generation == tex->generation);
    
    return tex;
}

-(void)setTextureData:(hgtex_t) hgtex pixelData:(void*) pixelData;
{
    struct GfxTexture* tex = [self resolveTextureHandle:hgtex];
    
    MTLRegion region =
    {
        { 0, 0, 0 },
        { tex->width, tex->height, 1 }
    };
    
    NSUInteger bytesPerRow = tex->width * tex->bytesPerPixel;
    
    [tex->texture replaceRegion:region mipmapLevel:0 withBytes:pixelData bytesPerRow:bytesPerRow];
}

-(void)bindTexture:(hgtex_t) hgtex;
{
    struct GfxTexture* tex = [self resolveTextureHandle:hgtex];
    
    [_renderEncoder setFragmentTexture:tex->texture
                              atIndex:TextureIndexColor];
}

static matrix_float4x4 loadMatrix4v(float* src)
{
    simd_float4 row0 = simd_make_float4(src[0], src[1], src[2], src[3]);
    simd_float4 row1 = simd_make_float4(src[4], src[5], src[6], src[7]);
    simd_float4 row2 = simd_make_float4(src[8], src[9], src[10], src[11]);
    simd_float4 row3 = simd_make_float4(src[12], src[13], src[14], src[15]);
    
    return simd_matrix(row0, row1, row2, row3);
}

-(void)setUniforms:(struct GfxUniforms*)uniforms;
{
    _projectionMatrix = loadMatrix4v(uniforms->projection);
    _modelViewMatrix = loadMatrix4v(uniforms->modelView);
}

-(void)setUniformBuffer:(hgbuffer_t)hgbuffer offset:(uint32_t)offset;
{
    struct GfxBuffer* buffer = [self resolveBufferHandle:hgbuffer];
    
    [_renderEncoder setVertexBuffer:buffer->buffer
                            offset:offset
                           atIndex:BufferIndexUniforms];

    [_renderEncoder setFragmentBuffer:buffer->buffer
                              offset:offset
                             atIndex:BufferIndexUniforms];
}

-(void)drawPrimitives:(hgbuffer_t)hgbuffer first:(uint16_t)first count:(uint16_t)count;
{
    struct GfxBuffer* const buffer = [self resolveBufferHandle:hgbuffer];
    
    [_renderEncoder setCullMode:MTLCullModeNone];
    [_renderEncoder setVertexBuffer:buffer->buffer offset:0 atIndex:BufferIndexVertexAttributes];
    [_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:first vertexCount:count];
}

-(nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view;
{
    self = [super init];
    if(self)
    {
        _device = view.device;
        _inFlightSemaphore = dispatch_semaphore_create(kMaxBuffersInFlight);
        [self _loadMetalWithView:view];
        [self _loadAssets];
        
        // Init matrices.
        _projectionMatrix = matrix_identity_float4x4;
        _modelViewMatrix = matrix_identity_float4x4;
        
        // Init buffers.
        uint16_t bufferIndex = 0;
        while(bufferIndex < GFX_MAX_BUFFER_COUNT - 1)
        {
            struct GfxBuffer* buffer = &_buffers[bufferIndex];
            buffer->nextFree = ++bufferIndex;
        }
        _buffers[bufferIndex].nextFree = (uint16_t)~0u;
        _firstFreeBuffer = 0;
        
        // Init textures.
        uint16_t textureIndex = 0;
        while(textureIndex < GFX_MAX_TEXTURE_COUNT - 1)
        {
            struct GfxTexture* tex = &_textures[textureIndex];
            tex->nextFree = ++textureIndex;
        }
        _textures[textureIndex].nextFree = (uint16_t)~0u;
        _firstFreeTexture = 0;
    }

    return self;
}

-(void)getGraphics:(nonnull struct Graphics*)graphics;
{
    memset(graphics, 0, sizeof(struct Graphics));
    
    graphics->createBuffer = &Graphics_CreateBuffer;
    graphics->destroyBuffer = &Graphics_DestroyBuffer;
    graphics->setBufferData = &Graphics_SetBufferData;
    
    graphics->createTexture = &Graphics_CreateTexture;
    graphics->setTextureData = &Graphics_SetTextureData;
    graphics->bindTexture = &Graphics_BindTexture;
    graphics->destroyBuffer = &Graphics_DestroyBuffer;
    
    graphics->setUniforms = &Graphics_SetUniforms;
    graphics->setUniformBuffer = &Graphics_SetUniformBuffer;
    graphics->drawPrimitives = &Graphics_DrawPrimitives;
    
    graphics->ins = (__bridge void *)(self);
}

- (void)_loadMetalWithView:(nonnull MTKView *)view;
{
    /// Load Metal state objects and initalize renderer dependent view properties

    view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    view.sampleCount = 1;

    _mtlVertexDescriptor = [[MTLVertexDescriptor alloc] init];

    _mtlVertexDescriptor.attributes[VertexAttributePosition].format = MTLVertexFormatFloat3;
    _mtlVertexDescriptor.attributes[VertexAttributePosition].offset = 0;
    _mtlVertexDescriptor.attributes[VertexAttributePosition].bufferIndex = BufferIndexVertexAttributes;

    _mtlVertexDescriptor.attributes[VertexAttributeTexcoord].format = MTLVertexFormatFloat2;
    _mtlVertexDescriptor.attributes[VertexAttributeTexcoord].offset = offsetof(struct Vertex, texCoord); // TODO(cj): Move Vertex definition to universal.
    _mtlVertexDescriptor.attributes[VertexAttributeTexcoord].bufferIndex = BufferIndexVertexAttributes;

    _mtlVertexDescriptor.layouts[BufferIndexVertexAttributes].stride = sizeof(struct Vertex);
    _mtlVertexDescriptor.layouts[BufferIndexVertexAttributes].stepRate = 1;
    _mtlVertexDescriptor.layouts[BufferIndexVertexAttributes].stepFunction = MTLVertexStepFunctionPerVertex;

    id<MTLLibrary> defaultLibrary = [_device newDefaultLibrary];

    id <MTLFunction> vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];

    id <MTLFunction> fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentShader"];

    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"MyPipeline";
    pipelineStateDescriptor.sampleCount = view.sampleCount;
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;
    pipelineStateDescriptor.vertexDescriptor = _mtlVertexDescriptor;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
    pipelineStateDescriptor.depthAttachmentPixelFormat = view.depthStencilPixelFormat;
    pipelineStateDescriptor.stencilAttachmentPixelFormat = view.depthStencilPixelFormat;

    NSError *error = NULL;
    _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
    if (!_pipelineState)
    {
        NSLog(@"Failed to created pipeline state, error %@", error);
    }

    MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
    // TODO(cj): enable depth test again?
    depthStateDesc.depthWriteEnabled = NO;
    _depthState = [_device newDepthStencilStateWithDescriptor:depthStateDesc];

    NSUInteger uniformBufferSize = kAlignedUniformsSize * kMaxBuffersInFlight;

    _dynamicUniformBuffer = [_device newBufferWithLength:uniformBufferSize
                                                 options:MTLResourceStorageModeShared];

    _dynamicUniformBuffer.label = @"UniformBuffer";

    _commandQueue = [_device newCommandQueue];
}

- (void)_loadAssets
{
    /// Load assets into metal objects

    NSError *error;

    MTKMeshBufferAllocator *metalAllocator = [[MTKMeshBufferAllocator alloc]
                                              initWithDevice: _device];

    MDLMesh *mdlMesh = [MDLMesh newBoxWithDimensions:(vector_float3){4, 4, 4}
                                            segments:(vector_uint3){2, 2, 2}
                                        geometryType:MDLGeometryTypeTriangles
                                       inwardNormals:NO
                                           allocator:metalAllocator];

    MDLVertexDescriptor *mdlVertexDescriptor =
    MTKModelIOVertexDescriptorFromMetal(_mtlVertexDescriptor);

    mdlVertexDescriptor.attributes[VertexAttributePosition].name  = MDLVertexAttributePosition;
    mdlVertexDescriptor.attributes[VertexAttributeTexcoord].name  = MDLVertexAttributeTextureCoordinate;

    mdlMesh.vertexDescriptor = mdlVertexDescriptor;

    _mesh = [[MTKMesh alloc] initWithMesh:mdlMesh
                                   device:_device
                                    error:&error];

    if(!_mesh || error)
    {
        NSLog(@"Error creating MetalKit mesh %@", error.localizedDescription);
    }

    MTKTextureLoader* textureLoader = [[MTKTextureLoader alloc] initWithDevice:_device];

    NSDictionary *textureLoaderOptions =
    @{
      MTKTextureLoaderOptionTextureUsage       : @(MTLTextureUsageShaderRead),
      MTKTextureLoaderOptionTextureStorageMode : @(MTLStorageModePrivate)
      };

    _colorMap = [textureLoader newTextureWithName:@"ColorMap"
                                      scaleFactor:1.0
                                           bundle:nil
                                          options:textureLoaderOptions
                                            error:&error];

    if(!_colorMap || error)
    {
        NSLog(@"Error creating texture %@", error.localizedDescription);
    }
}

- (void)_updateDynamicBufferState
{
    /// Update the state of our uniform buffers before rendering

    _uniformBufferIndex = (_uniformBufferIndex + 1) % kMaxBuffersInFlight;

    _uniformBufferOffset = kAlignedUniformsSize * _uniformBufferIndex;

    _uniformBufferAddress = ((uint8_t*)_dynamicUniformBuffer.contents) + _uniformBufferOffset;
}

- (void)_updateGameState
{
    /// Update any game state before encoding renderint commands to our drawable

    Uniforms * uniforms = (Uniforms*)_uniformBufferAddress;

    uniforms->projectionMatrix = _projectionMatrix;
    uniforms->modelViewMatrix = _modelViewMatrix;

    _rotation += .01;
}

- (void)drawInMTKView:(nonnull MTKView *)view
{
    /// Per frame updates here
    
    g_gameApi->tick();

    dispatch_semaphore_wait(_inFlightSemaphore, DISPATCH_TIME_FOREVER);

    id <MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    commandBuffer.label = @"MyCommand";

    __block dispatch_semaphore_t block_sema = _inFlightSemaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer)
     {
         dispatch_semaphore_signal(block_sema);
     }];

    [self _updateDynamicBufferState];

    [self _updateGameState];

    /// Delay getting the currentRenderPassDescriptor until we absolutely need it to avoid
    ///   holding onto the drawable and blocking the display pipeline any longer than necessary
    MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;

    if(renderPassDescriptor != nil) {

        /// Final pass rendering code here

        id <MTLRenderCommandEncoder> renderEncoder =
        [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        renderEncoder.label = @"MyRenderEncoder";

        [renderEncoder pushDebugGroup:@"DrawBox"];

        [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
        [renderEncoder setCullMode:MTLCullModeBack];
        [renderEncoder setRenderPipelineState:_pipelineState];
        [renderEncoder setDepthStencilState:_depthState];

        [renderEncoder setVertexBuffer:_dynamicUniformBuffer
                                offset:_uniformBufferOffset
                               atIndex:BufferIndexUniforms];

        [renderEncoder setFragmentBuffer:_dynamicUniformBuffer
                                  offset:_uniformBufferOffset
                                 atIndex:BufferIndexUniforms];

        for (NSUInteger bufferIndex = 0; bufferIndex < _mesh.vertexBuffers.count; bufferIndex++)
        {
            MTKMeshBuffer *vertexBuffer = _mesh.vertexBuffers[bufferIndex];
            if((NSNull*)vertexBuffer != [NSNull null])
            {
                [renderEncoder setVertexBuffer:vertexBuffer.buffer
                                        offset:vertexBuffer.offset
                                       atIndex:bufferIndex];
            }
        }

        [renderEncoder setFragmentTexture:_colorMap
                                  atIndex:TextureIndexColor];

        /*
        for(MTKSubmesh *submesh in _mesh.submeshes)
        {
            [renderEncoder drawIndexedPrimitives:submesh.primitiveType
                                      indexCount:submesh.indexCount
                                       indexType:submesh.indexType
                                     indexBuffer:submesh.indexBuffer.buffer
                               indexBufferOffset:submesh.indexBuffer.offset];
        }
         */

        [renderEncoder popDebugGroup];
        
        _renderEncoder = renderEncoder;
        
        g_gameApi->draw();
        
        _renderEncoder = nil;

        [renderEncoder endEncoding];

        [commandBuffer presentDrawable:view.currentDrawable];
    }

    [commandBuffer commit];
    
    [commandBuffer waitUntilCompleted]; // TODO(cj) Stalls!
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    /// Respond to drawable size or orientation changes here
    g_gameApi->resize((uint16_t)size.width, (uint16_t)size.height);
}

#pragma mark Matrix Math Utilities

matrix_float4x4 matrix4x4_translation(float tx, float ty, float tz)
{
    return (matrix_float4x4) {{
        { 1,   0,  0,  0 },
        { 0,   1,  0,  0 },
        { 0,   0,  1,  0 },
        { tx, ty, tz,  1 }
    }};
}

static matrix_float4x4 matrix4x4_rotation(float radians, vector_float3 axis)
{
    axis = vector_normalize(axis);
    float ct = cosf(radians);
    float st = sinf(radians);
    float ci = 1 - ct;
    float x = axis.x, y = axis.y, z = axis.z;

    return (matrix_float4x4) {{
        { ct + x * x * ci,     y * x * ci + z * st, z * x * ci - y * st, 0},
        { x * y * ci - z * st,     ct + y * y * ci, z * y * ci + x * st, 0},
        { x * z * ci + y * st, y * z * ci - x * st,     ct + z * z * ci, 0},
        {                   0,                   0,                   0, 1}
    }};
}

@end
