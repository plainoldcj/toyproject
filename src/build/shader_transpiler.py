#!/usr/bin/python3

import argparse
import os
import subprocess
import sys

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

def main():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument("--verbose", action="store_true")
    arg_parser.add_argument("--continueonerror", action="store_true")

    args = arg_parser.parse_args()

    success, project_root = find_project_root()
    if not success:
        print("Unable to find project root directory.")
        quit()

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

            spv_file_path = os.path.join(shader_out_dir, filename + ".spv")

            make_parent_dirs(spv_file_path);

            proc = subprocess.run([glslang_validator, abs_path,
                "-V",
                "-o", spv_file_path],
                text=True, capture_output=True)

            reported_output = False

            if not is_cooked(spv_file_path, abs_path):
                print("Generating SPIR-V from GLSL failed for shader '{}':".format(rel_path))

                print(proc.stdout)
                print(proc.stderr)
                reported_output = True

                if args.continueonerror:
                    continue
                else:
                    sys.exit(-1)

            if args.verbose and not reported_output:
                print(proc.stdout)
                print(proc.stderr)

            msl_file_path = os.path.join(shader_out_dir, filename + ".msl")

            make_parent_dirs(spv_file_path);

            entry_name, entry_type = shader_ext_to_entry[ext]

            proc = subprocess.run([spirv_cross, spv_file_path,
                "--msl",
                "--output", msl_file_path,
                "--rename-entry-point", "main", entry_name, entry_type],
                text = True, capture_output=True)

            reported_output = False

            if not is_cooked(msl_file_path, spv_file_path):
                print("Generating MSL from SPIR-V failed for shader '{}':".format(rel_path))

                print(proc.stdout)
                print(proc.stderr)
                reported_output = True

                if args.continueonerror:
                    continue
                else:
                    sys.exit(-1)

            if args.verbose and not reported_output:
                print(proc.stdout)
                print(proc.stderr)



    sys.exit(0)
# ----------------------------------------------------------------

if __name__ == "__main__":
    main()
