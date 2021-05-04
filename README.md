# Hyrise (Codename OpossumDB)

*Have a look at our [contributor guidelines](CONTRIBUTING.md)*

## Dependencies
You can install the dependencies on your own or use the install_dependencies.sh script (**recommended**) which installs all of the therein listed dependencies and submodules.
The install script was tested under macOS Big Sur and Ubuntu 20.04 (apt-get).

See [dependencies.md](dependencies.md) for a detailed list of dependencies to use with `brew install` or `apt-get install`, depending on your platform. As compilers, we generally use the most recent version of gcc and clang.
Older versions may work, but are neither tested nor supported.

## Building and Tooling
It is highly recommended to perform out-of-source builds, i.e., creating a separate directory for the build.
Advisable names for this directory would be `cmake-build-{debug,release}`, depending on the build type.
Within this directory call `cmake ..` to configure the build.
Subsequent calls to CMake, e.g., when adding files to the build will not be necessary, the generated Makefiles will take care of that.

### Compiler choice
CMake will default to your system's default compiler.
To use a different one, call like `cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..` in a clean build directory.

### Build
Simply call `make -j*`, where `*` denotes the number of threads to use.

Usually debug binaries are created.
To configure a build directory for a release build make sure it is empty and call CMake like `cmake -DCMAKE_BUILD_TYPE=Release`

### Lint
`./scripts/lint.sh` (Google's cpplint is used.)

### Format
`./scripts/format.sh` (clang-format is used.)

### Test
Calling `make hyriseTest` from the build directory builds all available tests.

### Coverage
After building `hyriseCoverage`, `./scripts/coverage.sh <build dir>` will print a summary to the command line and create detailed html reports at ./coverage/index.html

*Supports only clang on MacOS and only gcc on linux*


### Address/UndefinedBehavior Sanitizers
`cmake -DENABLE_ADDR_UB_SANITIZATION=ON` will generate Makefiles with AddressSanitizer and Undefined Behavior options.
Compile and run them as normal - if any issues are detected, they will be printed to the console.
It will fail on the first detected error and will print a summary.
To convert addresses to actual source code locations, make sure llvm-symbolizer is installed (included in the llvm package) and is available in `$PATH`.
To specify a custom location for the symbolizer, set `$ASAN_SYMBOLIZER_PATH` to the path of the executable.
This seems to work out of the box on macOS - If not, make sure to have llvm installed.
The binary can be executed with `LSAN_OPTIONS=suppressions=asan-ignore.txt ./<YourBuildDirectory>/hyriseTest`.

`cmake -DENABLE_THREAD_SANITIZATION=ON` will work as above but with the ThreadSanitizer. Some sanitizers are mutually exclusive, which is why we use two configurations for this.


## Naming convention for gtest macros:

TEST(ModuleNameClassNameTest, TestName), e.g., TEST(OperatorsGetTableTest, RowCount)
same for fixtures Test_F()

If you want to test a single module, class or test you have to execute the test binary and use the `gtest_filter` option:

- Testing the storage module: `./build/test --gtest_filter="Storage*"`
- Testing the table class: `./build/test --gtest_filter="StorageTableTest*"`
- Testing the RowCount test: `./build/test --gtest_filter="StorageTableTest.RowCount"`

## Maintainers

- Jan Kossmann
- Marcel Weisgut
- Martin Boissier


Contact: firstname.lastname@hpi.de
