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
present, it is downloaded automatically from the Unicode Consortium and used
(via `generate_unicode_case_tables.rb`) to generate `_autogen_unicode_case.h`.
You can also supply the file explicitly:

```
make autogen UNICODE_DATA=/path/to/UnicodeData.txt
```

Alternatively, place `UnicodeData.txt` in the `src/` directory before running
`make autogen`.

### Upgrading the Unicode version

`_autogen_unicode_case.h` is generated from a pinned version of
`UnicodeData.txt`. The pinned version and its SHA256 checksum are defined in
`src/Makefile` as `UNICODE_VERSION` and `UNICODE_DATA_SHA256`.

To upgrade to a new Unicode version:

1. Find the new version number at https://www.unicode.org/versions/
2. Download the new `UnicodeData.txt` and compute its SHA256:
   ```
   curl -fsSL https://www.unicode.org/Public/NEW_VERSION/ucd/UnicodeData.txt \
     -o /tmp/UnicodeData.txt
   sha256sum /tmp/UnicodeData.txt   # Linux
   shasum -a 256 /tmp/UnicodeData.txt   # macOS
   ```
3. Update `UNICODE_VERSION` and `UNICODE_DATA_SHA256` in `src/Makefile`
4. Regenerate the header:
   ```
   make autogen UNICODE_DATA=/tmp/UnicodeData.txt
   ```
5. Review the diff of `src/_autogen_unicode_case.h` and commit

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
