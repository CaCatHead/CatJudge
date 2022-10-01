# CatJudge

[![CI](https://github.com/XLoJ/CatJudge/actions/workflows/ci.yml/badge.svg)](https://github.com/XLoJ/CatJudge/actions/workflows/ci.yml)

A simple sandbox program used for competitive programming contest migrated from [NJUST-FishTeam/OnlineJudgeCore](https://github.com/NJUST-FishTeam/OnlineJudgeCore/tree/cat).

## Build

> **Prerequisite**
>
> It works only on the **Linux**.
>
> Install latest [cmake](https://cmake.org/), C/C++ compiler ([gcc](https://gcc.gnu.org/) or [clang](https://clang.llvm.org/)), and [python 2/3](https://www.python.org/).

Before building this project, you should first modify `CMakeLists.txt` to config the default checker (since we should not bring break changes to original system).

We recommend you use the checker [lcmp](https://github.com/MikeMirzayanov/testlib/blob/7fd543d7e6ae36a04bb382c5ebb4eee254362c6a/checkers/lcmp.cpp) (It diffs the content of each line, and ignores extra whitespaces).

```cmake
# CMakeLists.txt
set(DEFAULT_CHECKER <the absolute path of your default checker>)

# For example:
# set(DEFAULT_CHECKER /usr/bin/lcmp)
```

Then, follow these commands.

```bash
# Clone submodule testlib
$ git submodule update --init --recursive

# Configure release project
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release -B ./build -G "Unix Makefiles"

# Build project
$ cmake --build ./build --config Debug --target all -j 18

# You may need copy the default checker to the expected directory (optional)
# It depends on your previous configuration
# $ cp ./build/lcmp /usr/bin/lcmp

# Add testlib.h to the include path
$ cp ./testlib/testlib.h /usr/local/include/testlib.h

# Run testcases (optional)
$ cd ./build
$ sudo ctest --verbose
```

## Usage

Command line options:

+ `-d <dir>`: Run directory
+ `-l <language>`: Code Language (one of `c`, `cpp`, `java`)
+ `-t <time>`: Time limit (unit: ms, default: 1000 ms)
+ `-m <memory>`: Memory limit (unit: KB, default: 65536 KB)
+ `-s <checker>`: Binary checker path (optional)

Example:

```bash
$ sudo ./build/main  -d ./test -l cpp -t 1000 -m 65535 -s ./build/lcmp
```

Run the program at the run directory `./test`, using at most 1000 ms time and at most 65536 KB memory, and the checker is located at `./build/lcmp`.

### Conventions

Before running this judge sandbox program, you should prepare the run directory with the following files.

+ A compiled program `a.out` (for `c` and `cpp`) or `Main.java` (for `java`).
+ Testcase input file: `in.in`.
+ Testcase answer file: `out.out`.

An example client is located at `./core/test/run.py` used for unit test.

### Checker

You should follow the conventions of testlib checker. More details see [MikeMirzayanov/testlib](https://github.com/MikeMirzayanov/testlib/tree/7fd543d7e6ae36a04bb382c5ebb4eee254362c6a#checker), [Checker - OI Wiki](https://oi-wiki.org/tools/testlib/checker/), and some common examples at [testlib/checkers](https://github.com/MikeMirzayanov/testlib/tree/7fd543d7e6ae36a04bb382c5ebb4eee254362c6a/checkers).

The following code is a simple checker used for the well-known problem "A + B".

> **Note**: You should copy the header `testlib.h` to the include path of your computer (or judge server).

```cpp
#include "testlib.h"

int main(int argc, char * argv[]) {
  setName("compares two signed integers");
  registerTestlibCmd(argc, argv);
  int ja = ans.readInt();
  int pa = ouf.readInt();
  if (ja != pa)
    quitf(_wa, "expected %d, found %d", ja, pa);
  quitf(_ok, "answer is %d", ja);
}
```

## License

MIT License © 2022 [XLor](https://github.com/yjl9903)
