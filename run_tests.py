import argparse
import os
import re
import subprocess

_TEST_DIR    = "tests"
_BUILD_CMD   = ["make", "all", "-j8"]
_CLEAN_CMD   = ["make", "clean"]
_STAGE_FILE  = "src/compiler_stages.h"
_BIN         = "build/albatross"

_STAGE_FLAGS = [
    "COMPILE_STAGE_LEXER", 
    "COMPILE_STAGE_PARSER",
    "COMPILE_STAGE_SYMBOL_RESOLVER",
    "COMPILE_STAGE_TYPE_CHECKER"
]

_DEFAULT_COMPILER_STAGES_FILE_CONTENTS = """// DO NOT EDIT MANUALLY. 
// This file is completely overwritten by run_tests.py when compiling different 
// stages of Albatross.\n
""" + "\n".join([f"#define {flag}" for flag in _STAGE_FLAGS])

# test dir, #defined stage flags, valid return codes on failure
_COMPAT_TEST_CONFIGS = [
    ("tests/lexer-tests",    _STAGE_FLAGS[:1], [201]),
    ("tests/parser-tests",   _STAGE_FLAGS[:2], [202]),
    ("tests/semantic-tests", _STAGE_FLAGS[:4], [203, 204]),
]

def define_flags(flags):

    lines = [f"#define {flag}\n" for flag in flags]

    with open(_STAGE_FILE, "w") as stage_file:
        stage_file.writelines(lines)


_RJUST_COLUMN = 70
def main():

    for test_dir, flags, fail_errcodes in _COMPAT_TEST_CONFIGS:
        # Compile the binary w/ given flags for this test group.
        define_flags(flags)

        print("Compiling binary for", test_dir)

        subprocess.run(_CLEAN_CMD, check=True, capture_output=True)
        subprocess.run(_BUILD_CMD, check=True, capture_output=True)

        for test_subgroup in sorted(os.listdir(test_dir)):
            subgroup_path = f"{test_dir}/{test_subgroup}/"

            input_files = sorted(os.listdir(subgroup_path))

            n_passed = 0

            failed_inputs = []
            expected_outputs = {}

            for name in input_files:
                test_name, extension = name.split(".")

                if extension != "albatross":
                    continue
                
                expected_outputs[test_name + ".albatross"] = None

                # If this test is supposed to pass, it *should* have a 
                # corresponding .expected output file.
                if test_name[:4] == "pass":
                    expected_outputs[name] = test_name + ".expected"

            for input_file, output_file in expected_outputs.items():

                # Skip anything that isn't a source file
                if input_file.split(".")[1] != "albatross":
                    continue

                input_path = subgroup_path + input_file
                should_fail = input_file[:4] == "fail"

                result = subprocess.run([_BIN, input_path], capture_output=True)

                try:
                    result.check_returncode()

                    if output_file is not None:
                        # Compare output
                        cmd = f"diff {subgroup_path + output_file} <({_BIN} {subgroup_path + input_file})"
                        subprocess.check_call(cmd, shell=True, executable="/bin/bash")

                    n_passed += 1

                except subprocess.CalledProcessError:
                    if should_fail and result.returncode in fail_errcodes:
                        n_passed += 1
                    else:
                        stderr = result.stderr.decode("utf-8")
                        failed_inputs.append((input_path, stderr))

                        # print("\033[1;31m FAILED:\033[0m", input_path)
                        # print(result.stdout.decode("utf-8"))
                        # print(result.stderr.decode("utf-8"))
            

            # output_line = f"\033[1m {subgroup_path}"
            # output_line += f"{n_passed}/{n_group_tests}\033[0m".rjust(_RJUST_COLUMN - len(output_line))
            # output_line += "\033[1;31m FAIL\033[0m" if n_passed != n_group_tests else "\033[1;32m PASS\033[0m"
            # print(output_line)

            for input_path, stderr in failed_inputs:
                print(f"  {input_path} \n  stderr: {stderr}")

    with open(_STAGE_FILE, "w") as stage_file:
        stage_file.write(_DEFAULT_COMPILER_STAGES_FILE_CONTENTS)

if __name__ == "__main__":
    main()