![Build Status](https://github.com/Dead-tech/dead-lang/actions/workflows/windows.yml/badge.svg)
![Build Status](https://github.com/Dead-tech/dead-lang/actions/workflows/linux.yml/badge.svg)
![Build Status](https://github.com/Dead-tech/dead-lang/actions/workflows/macos.yml/badge.svg)

# Dead-lang

**_This project is still in active development, use it at your own risk!_**

Programming Language that compiles to the VM Bytecode.

## Language

**_DISCLAIMER: The language itself hasn't been developed yet!_**

## Vm

There's a stack virtual machine that has its own bytecode, that for now implements these instructions:

* `push <number||dquoted string>`
* `print <varname?>` (optional, if not specified prints the element at the top of the stack)
* `set <varname> <string||number>`
* `jump <labelname>`
* `jumpne <varname> <comparator> <labelname>`
* `dec <varname>`
* `inc <varname>`
* `add <varname1> <varname2>` == `var2 += var1` (for integers)
* `add <varname1> <varname2> <varname3>` == `var3 = var 1 + var 2` (for integers)
* `concat <varname1> <varname2>` == `var2 += var1` (for strings)
* `concat <varname1> <varname2> <varname3>` == `var3 = var 1 + var 2` (for strings)
* `mov <varname1> <varname2>` == `var2 = var1` (for integers and strings)
* `ret`
* `pop`
* `swap`
* `nop`
* `halt`

For more information about the instructions and their usage checkout the [examples folder](examples/) in the repo.

## Dependencies

This project currently has zero dependencies other than the standard library.

## Building

### Visual Studio 2019 on Windows

In order to build the project in VS2019 on Windows, you need to have the CMake Projects Extensions. Once you have that
installed you can open the project by selecting the root [CMakeLists.txt file](CMakeLists.txt), and then build from
there like any other project in VS2019.

### CMake Visual Studio Generator on Windows

```terminal
$ cmake -S. -B <build folder> -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Debug||Release
$ cmake --build <build folder> --config Debug||Release
```

### CMake on Windows

```terminal
$ cmake -S. -B <build foldeR> -DCMAKE_BUILD_TYPE=Debug||Release
$ cmake --build <build folder> --config Debug||Release
```

### CMake on MacOS

```terminal
$ cmake -S. -B <build folder> -DCMAKE_BUILD_TYPE=Debug||Release
$ cmake --build <build folder> --config Debug||Release
```

### CMake on Linux

```terminal
$ cmake -S. -B <build folder> -DCMAKE_BUILD_TYPE=Debug||Release
$ cmake --build <build folder> --config Debug||Release
```

## Usage

To use the stack virtual machine (the only thing implemented so far) it's as simple as:

```terminal
$ path/to/executable/dead-lang <filename>
```
