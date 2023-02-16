# app

## cmake

### cmake识别USE_MY_COMPILER

首先在 - config.h.in 设置: `#cmakedefine USE_MY_COMPILER` 或者 `#define USE_MY_COMPILER`.

接着 - CMakeLists.txt 中引入定义:

```txt
// 试cmake支持定义
option(USE_MY_COMPILER
    "Use provided compiler implementation" ON)
// 生成配置文件
configure_file(
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_SOURCE_DIR}/config.h"
)
```

之后就可以使用了: `if (DEFINED USE_MY_COMPILER)`.

### 引入子库

```txt
if (DEFINED USE_MY_COMPILER)
    // 引入头文件
    include_directories("${PROKECT_SOURCE_DIR}/compiler")
    // 包括子目录文件夹
    add_subdirectory(compiler)
    set(EXTRA_LIBS ${EXTRA_LIBS} Compiler)
endif()

// 链接库
target_link_libraries(app ${EXTRA_LIBS})
```