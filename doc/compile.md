# How to compile?

To make mruby/c sample programs, just `make` in top directory. `Makefile` generates mruby/c VM library `libmrubyc.a` and mruby/c executables.

## libmrubyc.a

`libmrubyc.a` is generated in `/src` directory. `libmrubyc.a` contains the mruby/c VM.


## mruby/c executables

Three mruby/c executables are generated in `/sample_c` directory.
These executables introduce several approaches to running the mruby/c VM.

- `sample_include`<br>
The mruby bytecode is internalized into the executable as a C array.
This is an example of the simplest procedure to start the mruby/c VM.
- `sample_no_scheduler`<br>
This is an example of taking a single mruby bytecode (`.mrb` file) as an argument and executing it.
- `sample_scheduler`<br>
This is also an example of taking a single mruby bytecode (`.mrb` file) as an argument and executing it.
Generates a task that executes bytecode.
- `sample_concurrent`<br>
This is an example that takes multiple mruby bytecodes as arguments and executes them concurrently.
- `sample_myclass`<br>
This is an example of defining mruby/c classes and methods in C.

## Auto-generated files

Several header files under `src/` are auto-generated from the source code and
are not tracked by git under normal circumstances. They are committed to the
repository by CI (GitHub Actions) on every push to `master` using
`git add --force`.

### Generating locally

Run `make autogen` from the top directory:

```
make autogen
```

This regenerates all `src/_autogen_*.h` files. If `src/UnicodeData.txt` is not
present, `_autogen_unicode_case.h` is downloaded automatically from the Unicode
Consortium. You can also supply the file explicitly:

```
make autogen UNICODE_DATA=/path/to/UnicodeData.txt
```

Alternatively, place `UnicodeData.txt` in the `src/` directory before running
`make autogen`.

### Clean including auto-generated files

```
make clean_all
```

This removes build artifacts as well as all `src/_autogen_*.h` files.

## How to run your mruby bytecode

First, mruby bytecode is generated from the mruby source code(`.rb` file). The mruby compiler(`mrbc`) is required for bytecode generation.

```
mrbc your_program.rb
```

Now you get the mruby bytecode(`your_program.mrb`).
This bytecode is executed in the `sample_scheduler` program.

```
sample_scheduler your_program.mrb
```
