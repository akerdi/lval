# Lval

Lval provides minimalistic language interpretation support for Lisp.

Recommand using CMake to add `Lval` as a submodule.

[Supported Lisp Basic Script Usage](./compiler/foo.cpp), more usage please read `lib.lsp`.

## Usage

Force lval to be a library:

`CMake Option: LVAL_LIB_MODE`

    ON: Library Mode
    OFF: Test Mode

Add Lval as submodule to your project:

    git submodule add git@github.com:akerdi/lval.git lval --recursive

you can refer to  [CMakeLists.txt.demo](./CMakeLists.txt.demo)

Start:

```cpp
// foo.cpp
#include <lval.h>
Lval::quick_start();
```

use `lib.lsp`:

```lisp
> load "lib.lsp"
```

## Problem

When add Lval as a submodule, error(`/lval/lval.h:6:10: fatal error: 'compiler.h' file not found`) will happen, below is the right way to treat:

CMakeLists.txt add submodule(compiler) directory:

    include_directories("${PROJECT_SOURCE_DIR}/lval/compiler")

## Goal

- [x] Library
- [x] String/Load file
- [x] Order/Compare/If
- [x] Function
- [x] Env
- [x] Q-Express
- [x] S-Express
- [x] AST
- [x] REPL
- [x] init

## TODO

- [ ] aoto gc instead lval_delete
- [x] lval to be library
- [x] Lval.cell use deque instead

## Contact me
Feel free to contact me if you have any trouble on this project:

Create an issue.
Send mail to me, tianxiaoxin001gmail.com

## Ref

[original - BuildYourOwnLisp](https://github.com/orangeduck/BuildYourOwnLisp)

[gcc-buildyourownlisp](https://github.com/akerdi/buildyourownlisp)

[typescript-jslispy](https://github.com/akerdi/jslispy)
