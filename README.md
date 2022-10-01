# CatJudge

This is migrated from [NJUST-FishTeam/OnlineJudgeCore](https://github.com/NJUST-FishTeam/OnlineJudgeCore/tree/cat).

## Build

> **Prerequisite**
>
> It works only on the **Linux**.
>
> Install latest [cmake](https://cmake.org/), C/C++ compiler ([gcc](https://gcc.gnu.org/) or [clang](https://clang.llvm.org/)), and [python 2/3](https://www.python.org/).

```bash
# Clone submodule testlib
$ git submodule update --init --recursive

# Configure release project
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release -B ./build -G "Unix Makefiles"

# Build project
$ cmake --build ./build --config Debug --target all -j 18

# Run testcases (optional)
$ cd ./build
$ sudo ctest --verbose
```

## Usage

Refer to [core](./core/README.md).

## License

MIT License © 2022 [XLor](https://github.com/yjl9903)
