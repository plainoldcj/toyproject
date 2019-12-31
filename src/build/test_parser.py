import argparse
import pathlib
import re

def write_unit_tests(File, unit_tests):
	preamble="""\
// This is an automatically generated file.
// Any content may be overwritten.

#include <stdio.h>

"""

	File.write(preamble)

# Prototypes.
	for filename in unit_tests:
		File.write(f"// {filename}\n")
		for func in unit_tests[filename]:
			File.write(f"void {func}(void);\n")
		File.write("\n")

	File.write("void RunAllUnitTests(void)\n{\n")

# Call tests.
	for filename in unit_tests:
		for func in unit_tests[filename]:
			File.write(f'printf("%s:%s\\n", "{filename}", "{func}");\n')
			File.write(f'{func}();\n')
			File.write("\n")
		File.write("\n")

	File.write("}\n") # End of function RunAllUnitTests

def find_unit_tests(filepaths):
	regex = re.compile("\s*UNIT_TEST\((\w+)\)")
	all_tests = {}

	root_path = pathlib.Path('.').resolve()

	for filepath in filepaths:
		relative_path = filepath.resolve().relative_to(root_path)

		tests_in_file = []
		File = open(filepath.resolve(), "r")
		for line in File:
			match = regex.match(line)
			if match:
				tests_in_file.append(match.group(1))

		all_tests[str(relative_path)] = tests_in_file

	return all_tests

def main():
	parser = argparse.ArgumentParser()
	parser.add_argument('outfile')
	parser.add_argument('mod_dir')

	args = parser.parse_args()

# Find all files in module directory.
	mod_dir = pathlib.Path('.').resolve().joinpath(args.mod_dir)
	test_filenames = [ f for f in mod_dir.iterdir() if "_tests.c" in f.name ]

	unit_tests = find_unit_tests(test_filenames)
	print(unit_tests)

	outfile = pathlib.Path('.').resolve().joinpath(args.outfile)
	File = open(outfile, "w")
	write_unit_tests(File, unit_tests)

if __name__ == "__main__":
	main()
