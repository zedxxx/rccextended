Read this in other languages: *[Русский](readme.ru.md)*

----

## Description

RccExtended - compiler and **decompiler** for binary Qt resources (files with the .rcc extension).

The utility allows you to edit the resources of Qt programs without having their sources.

Editing algorithm:
  
  - unpack (decompile) binary Qt resources (using this utility)
  - edit unpacked files (.png, .xml, etc.) by third-party tools
  - compile of edited files back into binary Qt resources (using this utility)

This utility is based on the standard Qt resource compiler, which has added the function 
decompilation of resources (command line switch `--reverse`).

To unpack resources, you need to do 2 things:

  - go to the folder with resources
  - run the utility with the `--reverse` key
  
Once launched, the utility performs the following actions:
  
  - scans working directory looking for `.rcc` files
  - unpacks all found resource files (each to a separate folder)
  - generates project files `.qrc` and `rcc-make.bat` / `rcc-make.sh` script for reverse compilation of resources into a binary form
  - outputs to the console and logs information about the progress of unpacking 

Usage example:
```
    cd /Path/To/My/QtResources
    rcc --reverse
```

----

## How to build from sources?

Windows: 

  - install [MSYS2](https://www.msys2.org/wiki/MSYS2-installation/) environment
  - install `qt5-static` package
  - navigate to the `rccextended/src` folder
  - compile: `qmake rcc.pro && make`

Ubuntu:

  - install Qt5 packages: `sudo apt install qt5-default qtdeclarative5-dev`
  - navigate to the `rccextended/src` folder
  - compile: `qmake rcc.pro && make`
  - move compiled binary to *.local/bin*: `mv -v rcc ~/.local/bin/`
