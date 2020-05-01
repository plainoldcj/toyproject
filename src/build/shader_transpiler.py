#!/usr/bin/python3

import argparse
import os
import subprocess
import sys

class InputFile:
    pass
# ----------------------------------------------------------------

class OutputFile:
    pass
# ----------------------------------------------------------------

class CookingFailed(Exception):
    def __init__(self, input_file, output_file, output):
        self.input_file = input_file
        self.output_file = output_file
        self.output = output
# ----------------------------------------------------------------

def find_project_root():
    current_dir = "."

    for i in range(10):
        root, dirs, file_paths = next(os.walk(current_dir))
        if ".gitignore" in file_paths and ".git" in dirs:
            return (True, os.path.abspath(root))
        current_dir += "/.."

    return (False,"")
# ----------------------------------------------------------------

def is_cooked(cooked, source):
    try:
        return os.stat(cooked).st_mtime >= os.stat(source).st_mtime
    except:
        return False
# ----------------------------------------------------------------

def make_parent_dirs(file_path):
    try:
        os.makedirs(os.path.split(file_path)[0])
    except FileExistsError:
        pass
# ----------------------------------------------------------------

def expand_proc_arg(x, new_input, new_output):
    if isinstance(x, InputFile):
        return new_input
    elif isinstance(x, OutputFile):
        return new_output
    return x
# ----------------------------------------------------------------

def cook(proc_args, input_file_path, output_file_path):
    make_parent_dirs(output_file_path);

    expanded_proc_args = [ expand_proc_arg(x, input_file_path, output_file_path) for x in proc_args]

    proc = subprocess.run(expanded_proc_args,
        text=True,
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT) # Combine both streams

    output = proc.stdout

    if not is_cooked(output_file_path, input_file_path):
        raise CookingFailed(input_file_path, output_file_path, output)

    return output
# ----------------------------------------------------------------

def main():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument("--verbose", action="store_true")
    arg_parser.add_argument("--continueonerror", action="store_true")

    args = arg_parser.parse_args()

    success, project_root = find_project_root()
    if not success:
        print("Unable to find project root directory.")
        sys.exit(-1)

    if args.verbose:
        print("Project root directory: '{}'".format(project_root))

    glslang_validator = os.path.join(project_root, "build/bin/glslangValidator")
    spirv_cross = os.path.join(project_root, "build/bin/spirv-cross")

    glsl_dir = os.path.join(project_root, "assets/shaders")
    shader_out_dir = os.path.join(project_root, "cooked/shaders")

    shader_ext_to_entry = {
            ".vert": ( "vsmain", "vert" ),
            ".frag": ( "psmain", "frag" )
    }

    for root, _, filenames in os.walk(glsl_dir):
        for filename in filenames:
            ext = os.path.splitext(filename)[1]
            if ext not in shader_ext_to_entry:
                continue

            abs_path = os.path.join(root, filename)
            rel_path = abs_path[len(glsl_dir) + 1:] # Path relative to glsl_dir

            if args.verbose:
                print("Processing shader '{}'".format(rel_path))

            try:
                spv_file_path = os.path.join(shader_out_dir, filename + ".spv")

                output = cook([glslang_validator, InputFile(),
                    "-V",
                    "-o", OutputFile()],
                    abs_path,
                    spv_file_path)

                if args.verbose:
                    print(output)

                msl_file_path = os.path.join(shader_out_dir, filename + ".msl")

                entry_name, entry_type = shader_ext_to_entry[ext]

                output = cook([spirv_cross, InputFile(),
                    "--msl",
                    "--output", OutputFile(),
                    "--rename-entry-point", "main", entry_name, entry_type],
                    spv_file_path, msl_file_path)

                if args.verbose:
                    print(output)

            except CookingFailed as e:
                print("Cooking of file '{}' from file '{}' failed:".format(
                    os.path.relpath(e.input_file, project_root),
                    os.path.relpath(e.output_file, project_root)))
                print(e.output)

                if args.continueonerror:
                    continue
                else:
                    sys.exit(-1)


    sys.exit(0)
# ----------------------------------------------------------------

if __name__ == "__main__":
    main()
