## 实现 Paddle flags 机制

| 版本 | 作者      | 时间      |
| ---- | --------- | -------- |
| V1.0 | huangjiyi | 2023.7.23 |

## 一、概要

### 1. 相关背景

目前 Paddle 已基本完成 PHI 算子库的独立编译 ([PR#53735](https://github.com/PaddlePaddle/Paddle/pull/53735))，在实现这个目标的过程中出现过一个问题：phi 中用到 gflags 第三方库的 Flag 定义宏在 phi 编译成动态链接库后无法在 windows 上暴露 Flag 符号，当时的做法是在 phi 下重写 Flag 定义宏 (底层仍然依赖 gflags 第三方库)，使其能够在 windows 上暴露 Flag 符号 ([PR#52991](https://github.com/PaddlePaddle/Paddle/pull/52991))

但是目前还存在 gflags 第三方库相关的另外一个问题：由于 Paddle 依然依赖了 gflags 库，外部用户同时使用 paddle C++ 库和 gflags 库时，会出现以下错误：

``` bash
ERROR: something wrong with flag 'flagfile' in file '/Paddle/third_party/gflags/src/gflags.cc'.  One possibility: file '/Paddle/third_party/gflags/src/gflags.cc' is being linked both statically and dynamically into this executable.
```

这个错误是因为在 gflags 的源文件 `gflags.cc` 中，会注册一些 Flag，比如 `flagfile`：

``` C++
DEFINE_string(flagfile,   "", "load flags from file");
```

因为 Paddle 依赖了 gflags，所以 `libpaddle.so` 中也会注册 `flagfile`，然后外部用户如果再依赖 gflags，会重复注册 `flagfile` 导致报错，`gflags.cc` 中的报错相关代码：

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

为了解决上述问题，计划移除 Paddle 对 gflags 第三方库的依赖，在 Paddle 下实现一套独立的 flags 相关机制。

### 2. 功能目标

在 Paddle 下实现一套独立的 flags 相关机制，包括：

- 多种类型（bool, int32, uint32, int64, uint64, double, string）的 Flag 定义和声明宏
- 命令行参数解析，即根据命令行参数对已定义的 Flag 的 value 进行更新
- 其他 Paddle 用到的 Flag 相关操作
- 待后续补充 ...

细节要求：新实现的 flags 相关机制提供的接口与现有的接口尽可能保持一致，从而降低替换成本

待后续 Paddle 下独立的 flags 相关机制初步实现完善后，暂时将 Paddle 现有的依赖第三方库的 flags 机制保留，实现能够通过编译选项以及宏控制，选择使用哪个版本 flags 机制（需要两个版本的接口一致）

### 3. 意义

完善 Paddle 下的 flags 机制，提高框架开发者开发体验以及用户使用体验

## 二、飞桨现状

Paddle 目前在 `paddle/phi/core/flags.h` 中对 gflags 中的 Flag 注册宏 `DEFINE_<type>` 和声明宏 `DEFINE_<type>` 进行了重写，重写的代码和 gflags 的实现基本一致，只是修改了一些接口名字和命名空间，同时添加了支持 Windows 下的 Flag 符号暴露，但 Paddle 目前的 Flag 注册宏和声明宏底层依然依赖的是 gflags 的代码

### Paddle 中现有的 flags 用法

在 Paddle 中现有的 flags 用法主要是 Flag 注册和声明宏，以及一些 gflags 的接口：

1. flags 机制中使用的接口是 Flag 注册和声明宏：`(PHI_)?(DEFINE|DECLARE)_<type>`，Paddle 最多的用法：
   - `DEFINE_<type>(name,val, txt)` 用于定义目标类型的 FLAG，相当于定义一个全局变量 `FLAGS_name`，约 200+ 处用法
   - `DECLARE_<type>(name)` 用于声明 FLAG，相当于 `extern` 用法，约 300+ 处用法
2. `gflags::ParseCommandLineFlags(int* argc, char*** argv, bool remove_flags)`：用于解析运行时命令行输入的标志，大部分在测试文件中使用，约 20+ 处用法
3. `gflags::(GetCommandLineOption|SetCommandLineOption|AllowCommandLineReparsing|<Type>FromEnv)`：其他一些用法较少的 gflags 接口：
   - `bool GetCommandLineOption(const char* name, std::string* OUTPUT)`：用于获取 FLAG 的值，1 处用法
   - `std::string SetCommandLineOption (const char* name, const char* value)`：将 `value` 赋值给 `FLAGS_name`，2 处用法
   - `void AllowCommandLineReparsing()`：允许命令行重新解析，1 处用法

## 三、业内方案调研

### gflags

ref: https://gflags.github.io/gflags/



### pytorch

