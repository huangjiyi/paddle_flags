## 实现 Paddle flags 机制

| 版本 | 作者      | 时间      |
| ---- | --------- | -------- |
| V1.0 | huangjiyi | 2023.7.23 |

## 一、概要

### 1. 相关背景

目前 Paddle 已基本完成 PHI 算子库的独立编译 ([PR#53735](https://github.com/PaddlePaddle/Paddle/pull/52991))，在实现这个目标的过程中出现过一个问题：phi 中用到 gflags 第三方库的 Flag 定义宏在 phi 编译成动态链接库后无法在 windows 上暴露 Flag 符号，当时的做法是在 phi 下重写 Flag 定义宏 (底层仍然依赖 gflags 第三方库)，使其能够在 windows 上暴露 Flag 符号 ([PR#52991](https://github.com/PaddlePaddle/Paddle/pull/52991))

目前还存在 gflags 第三方库相关的另外一个问题，由于 Paddle 依然依赖了 gflags 库，外部用户同时使用 paddle C++ 库和 gflags 库时，会出现以下错误：

``` bash
ERROR: something wrong with flag 'flagfile' in file '/Paddle/third_party/gflags/src/gflags.cc'.  One possibility: file '/Paddle/third_party/gflags/src/gflags.cc' is being linked both statically and dynamically into this executable.
```

这个错误是因为在 gflags 的源文件 `gflags.cc` 中，会注册一些 Flag，比如 `flagfile`：

``` C++
DEFINE_string(flagfile,   "", "load flags from file");
```

因为 Paddle 依赖了 gflags，所以 `libpaddle.so` 中会注册 `flagfile`，然后外部用户如果再依赖 gflags，会重复注册 `flagfile` 导致报错，`gflags.cc` 中的报错相关代码：

``` C++
void FlagRegistry::RegisterFlag(CommandLineFlag* flag) {
  Lock();
  pair<FlagIterator, bool> ins =
    flags_.insert(pair<const char*, CommandLineFlag*>(flag->name(), flag));
  if (ins.second == false) {   // means the name was already in the map
    if (strcmp(ins.first->second->filename(), flag->filename()) != 0) {
      ReportError(DIE, "ERROR: flag '%s' was defined more than once "
                  "(in files '%s' and '%s').\n",
                  flag->name(),
                  ins.first->second->filename(),
                  flag->filename());
    } else {
      ReportError(DIE, "ERROR: something wrong with flag '%s' in file '%s'.  "
                  "One possibility: file '%s' is being linked both statically "
                  "and dynamically into this executable.\n",
                  flag->name(),
                  flag->filename(), flag->filename());
    }
  }
  // Also add to the flags_by_ptr_ map.
  flags_by_ptr_[flag->current_->value_buffer_] = flag;
  Unlock();
}
```

为了解决上述问题，计划移除 Paddle 对 gflags 第三方库的依赖，在 Paddle 下实现一套 flags 相关机制。

### 2. 功能目标





