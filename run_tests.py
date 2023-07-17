import argparse
import os
import re
import subprocess

_TEST_DIR   = "tests"
_BUILD_CMD  = ["make", "all"]
_CLEAN_CMD  = ["make", "clean"]
_MAIN_FILE  = "src/albatross.cpp"
_BIN        = "build/albatross"

_STAGE_FLAGS = [
    "COMPILE_STAGE_LEXER", 
    "COMPILE_STAGE_PARSER",
    "COMPILE_STAGE_SYMBOL_RESOLVER",
    "COMPILE_STAGE_TYPE_CHECKER"
]

# test dir, #defined stage flags, valid return codes on failure
_COMPAT_TEST_CONFIGS = [
    ("tests/lexer-tests",    _STAGE_FLAGS[:1], [201]),
    ("tests/parser-tests",   _STAGE_FLAGS[:2], [202]),
    ("tests/semantic-tests", _STAGE_FLAGS[:4], [203, 204]),
]


# #undef every flag in _MAIN_FILE
def undef_flags():
    sed_flags = []

    for flag in _STAGE_FLAGS:
        sed_flags.append("-e")
        sed_flags.append(f"s/#define {flag}/#undef {flag}/g")

    result = subprocess.run(["sed"] + sed_flags + [_MAIN_FILE], 
                            capture_output=True)

    with open(_MAIN_FILE, "w") as main_file:
        main_file.write(result.stdout.decode("utf-8"))


def define_flags(flags):
    # Undef every flag in _MAIN_FILE
    sed_flags = []

    for flag in flags:
        sed_flags.append("-e")
        sed_flags.append(f"s/#undef {flag}/#define {flag}/g")

    result = subprocess.run(["sed"] + sed_flags + [_MAIN_FILE], 
                            capture_output=True)

    with open(_MAIN_FILE, "w") as main_file:
        main_file.write(result.stdout.decode("utf-8"))


def main():

    # Save the original copy of our main file
    subprocess.run(["cp", _MAIN_FILE, _MAIN_FILE + ".original"])
    undef_flags()

    for test_dir, flags, fail_errcodes in _COMPAT_TEST_CONFIGS:
        # Compile the binary w/ given flags for this test group.
        define_flags(flags)

        print("Compiling binary for", test_dir)

        subprocess.run(_CLEAN_CMD, check=True, capture_output=True)
        subprocess.run(_BUILD_CMD, check=True, capture_output=True)

        for test_subgroup in sorted(os.listdir(test_dir)):
            subgroup_path = f"{test_dir}/{test_subgroup}/"
            print(subgroup_path)

            input_files = sorted(os.listdir(subgroup_path))

            n_group_tests = len([s 
                                 for s in input_files 
                                 if s.split(".")[1] == "albatross"])
            n_passed = 0

            for input_file in input_files:

                # Skip anything that isn't a source file
                if input_file.split(".")[1] != "albatross":
                    continue

                input_path = subgroup_path + input_file
                should_fail = input_file[:4] == "fail"

                result = subprocess.run([_BIN, input_path], capture_output=True)

                try:
                    result.check_returncode()
                    n_passed += 1

                except subprocess.CalledProcessError:
                    if should_fail:
                        n_passed += 1
                    else:
                        print("\033[1;31m FAILED:\033[0m", input_path)
                        # print(result.stdout.decode("utf-8"))
                        # print(result.stderr.decode("utf-8"))
    subprocess.run(["mv", _MAIN_FILE + ".original", _MAIN_FILE])

if __name__ == "__main__":
    main()