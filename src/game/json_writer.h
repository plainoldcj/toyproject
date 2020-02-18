#pragma once

#include <stdbool.h>
#include <stdint.h>

struct ReflectedType;

bool WriteJson(
	const struct ReflectedType* type,
	void* object,
	char* buffer,
	uint32_t bufferSize,
	const char* debugName);
