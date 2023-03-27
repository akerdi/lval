# CPPLispy

Using cpp implete lisp.

## Usage

force lval to be a library:

`CMake Option: LVAL_LIB_MODE`

    ON: Library Mode
    OFF: Test Mode

```cpp
#include <lval.h>
Lval::quick_start();
```

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