Read this in other language: [Русский](readme.ru.md)

## Description

**RccExtended** is a compiler and **decompiler** of binary Qt resources (files with the `.rcc` extension).

It allows you to edit Qt application resources without having access to the source code.

This utility is based on the standard Qt resource compiler, with an added resource unpacking feature: command line keys `--reverse` or `-r`.

Editing workflow:

  - unpack (decompile) binary Qt resources (using this utility);
  - edit the unpacked files (.png, .xml, .txt, etc.) with third-party tools;
  - recompile the edited files back into binary Qt resources (using this utility).

To unpack resources, follow these steps:

  - navigate to the folder containing the resources;
  - run the utility with the `--reverse` or `-r` key.

*Optionally, you can specify a filename pattern or a path to a specific file to unpack (see examples below).*

When launched, the utility performs the following actions:

  - scans the current directory for files with the `.rcc` extension;
  - unpacks all found resource files (each into its own folder);
  - generates `.qrc` project files and a `rcc-make.bat` script (or `rcc-make.sh` on Linux) for recompiling resources into binary format;
  - outputs to the console and logs information about the unpacking process.

## Examples

Unpack all resource files with the `*.rcc` extension in the current directory:
```
    cd path-to-my-qt-resources
    rcc --reverse
```

Unpack all resource files with the `*.qbtheme` extension in the current directory:
```
    cd path-to-my-qt-resources
    rcc --reverse *.qbtheme
```

Unpack a specific file:
```
    rcc -r example.rcc
```

## Compilation

On Windows 10:
  
  - install [MSYS2](https://www.msys2.org/) environment
  - install [gcc](https://packages.msys2.org/packages/mingw-w64-x86_64-gcc), [qt5-static](https://packages.msys2.org/base/mingw-w64-qt5-static) and [libzstd](https://packages.msys2.org/base/mingw-w64-zstd) packages: `pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-qt5-static mingw-w64-x86_64-zstd`
  - open a `mingw64` shell and navigate to the `rccextended/src` folder
  - run the command `qmake rcc.pro && make`

On Ubuntu 24.04 LTS:

  - install C++ compiler, Qt5 and libzstd packages: `sudo apt install build-essential qtbase5-dev qtbase5-dev-tools qt5-qmake qtchooser libzstd-dev`
  - navigate to the `rccextended/src` folder
  - run the command: `qmake rcc.pro && make`
  - move the compiled binary to `~/.local/bin` using: `mv -v rcc ~/.local/bin/`

## Installation

The program is fully portable, requires no installation, and consists of a single file: `rcc.exe`.

For convenience, it is recommended to place the program in the folder `C:\Program Files\RccExtended\` and add this path to the system PATH environment variable. 
