#ifndef MACRO_DEFINITIONS_H
#define MACRO_DEFINITIONS_H

// #include <stdint.h>  // 引入标准整数类型定义，例如 uintptr_t

// --------------- 内存对齐相关宏 ---------------

// 将 `a` 向上取整到 `sz` 的整数倍
#define ROUNDUP(a, sz)      ((((uintptr_t)a) + (sz) - 1) & ~((sz) - 1))

// 将 `a` 向下取整到 `sz` 的整数倍
#define ROUNDDOWN(a, sz)    ((((uintptr_t)a)) & ~((sz) - 1))

// 计算数组 `arr` 的元素个数
#define LENGTH(arr)         (sizeof(arr) / sizeof((arr)[0]))

// 定义一个地址区间 `[st, ed)`
#define RANGE(st, ed)       (Area) { .start = (void *)(st), .end = (void *)(ed) }

// 判断指针 `ptr` 是否在 `area` 指定的范围内
#define IN_RANGE(ptr, area) ((area).start <= (ptr) && (ptr) < (area).end)

// --------------- 预处理字符串操作 ---------------

// 将参数 `s` 转换为字符串
#define STRINGIFY(s)        #s
#define TOSTRING(s)         STRINGIFY(s)

// 连接两个标识符 `x` 和 `y`，形成一个新的标识符
#define _CONCAT(x, y)       x ## y
#define CONCAT(x, y)        _CONCAT(x, y)

// --------------- I/O 操作 ---------------

// 逐字符输出字符串 `s`，调用 `putch()` 函数输出
#define putstr(s) \
  ({ for (const char *p = s; *p; p++) putch(*p); })

// 读取 I/O 寄存器 `reg` 的值
#define io_read(reg) \
  ({ reg##_T __io_param; \
    ioe_read(reg, &__io_param); \
    __io_param; })

// 向 I/O 寄存器 `reg` 写入值
#define io_write(reg, ...) \
  ({ reg##_T __io_param = (reg##_T) { __VA_ARGS__ }; \
    ioe_write(reg, &__io_param); })

// --------------- 编译时检查 ---------------

// 静态断言（编译时检查条件 `const_cond` 是否成立，失败时报错）
#define static_assert(const_cond) \
  static char CONCAT(_static_assert_, __LINE__) [(const_cond) ? 1 : -1] __attribute__((unused))

// --------------- 断言与异常处理 ---------------

// 如果 `cond` 为真，输出错误信息 `s` 并终止程序
#define panic_on(cond, s) \
  ({ if (cond) { \
      putstr("AM Panic: "); putstr(s); \
      putstr(" @ " __FILE__ ":" TOSTRING(__LINE__) "  \n"); \
      halt(1); \
    } })

// 触发 panic，输出错误信息 `s` 并终止程序
#define panic(s) panic_on(1, s)

#endif // MACRO_DEFINITIONS_H
