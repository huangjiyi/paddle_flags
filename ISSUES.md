#### `--help` 相关实现

本来计划 `--help` 实现打印 `main()` 所在文件中定义的 flag，然后设计一个接口打印所有 flag，但是实现的时候因为要进行一个文件匹配，好像没有办法在 `flags.cc` 中获取获取 `main()` 函数的文件地址，只能，看了一下 gflags 的实现源码，发现他是从 `argv` 中获取可执行文件名 `program_name`，然后在 `FlagRegistry` 中搜索与 `program_name` 相同的源文件，这样做的前提是 `program_name` 与 `main()` 所在的源文件名称要相同，但是这是不确定的

解决方法：

想要在代码中获取 `main()` 函数的文件地址需要在 `main()` 函数中使用 `__FILE__` 宏，然后需要通过参数传递的方式传给 `ParseCommandLineFlags`，但是这对用户不友好，还有一种方式是将 `ParseCommandLineFlags` 设计成宏，暂时抛弃打印当前文件中定义的 flag，实现打印所有 flag