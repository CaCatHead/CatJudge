# CatJudge

This is migrated from [NJUST-FishTeam/OnlineJudgeCore](https://github.com/NJUST-FishTeam/OnlineJudgeCore/tree/cat).

## Build

> **Prerequisite**
>
> Install latest [cmake](https://cmake.org/), C/C++ compiler ([gcc](https://gcc.gnu.org/) or [clang](https://clang.llvm.org/)), and [python](https://www.python.org/).

```bash
# Configure project
$ cmake -B ./build -G "Unix Makefiles"

# Build project
$ cmake --build ./build --config Debug --target all -j 18

# Run testcases (optional)
$ cd ./build
$ sudo ctest --verbose
```

## Usage

Refer to [core](./core/README.md).

## License

MIT License © 2021 [XLor](https://github.com/yjl9903)