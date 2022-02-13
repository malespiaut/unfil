# unfil

Polished fork of James Park-Watt's [constructor-fil](https://github.com/jimmypw/constructor-fil), who made it using the [game-file-format wiki](https://github.com/shlainn/game-file-formats/wiki/Constructor-.FIL-files).

This tool extract the content of a FIL file in the current directory.

## Building

This project makes use of [Michael Crawford's GenericMakefile](https://github.com/mbcrawfo/GenericMakefile), and is written in plain C. You only have to type `make` to build the projet.

```
$ make
Creating directories
Beginning release build
Compiling: main.c -> build/release/main.o
	 Compile time: 00:00:00
Linking: bin/release/unfil
	 Link time: 00:00:00
Making symlink: unfil -> bin/release/unfil
Total build time: 00:00:01
```

## Usage

```
Usage: ./unfil [OPTION] [FILE]

Options:
  -l    list files without extracting them

NOTE: this tool extracts the content of the FIL file in the current directory.
```
