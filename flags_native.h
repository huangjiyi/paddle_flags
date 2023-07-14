// Copyright (c) 2023 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <string>

#include <iostream>
#define LOG(t) std::cout << t << std::endl
#define LOG_VAR(var) std::cout << #var << ": " << var << std::endl

#if defined(_WIN32)
#define PHI_EXPORT_FLAG __declspec(dllexport)
#define PHI_IMPORT_FLAG __declspec(dllimport)
#else
#define PHI_EXPORT_FLAG
#define PHI_IMPORT_FLAG
#endif  // _WIN32

namespace phi {
void ParseCommandLineFlags(int* argc, char*** argv);
}  // namespace phi

// ----------------------------DECLARE FLAGS----------------------------
#define PHI_DECLARE_FLAG(type, name)          \
  namespace flag_##type {                     \
    extern PHI_IMPORT_FLAG type FLAGS_##name; \
  }                                           \
  using flag_##type::FLAGS_##name

#define PHI_DECLARE_bool(name) PHI_DECLARE_FLAG(bool, name)
#define PHI_DECLARE_int32(name) PHI_DECLARE_FLAG(int32_t, name)
#define PHI_DECLARE_uint32(name) PHI_DECLARE_FLAG(uint32_t, name)
#define PHI_DECLARE_int64(name) PHI_DECLARE_FLAG(int64_t, name)
#define PHI_DECLARE_uint64(name) PHI_DECLARE_FLAG(uint64_t, name)
#define PHI_DECLARE_double(name) PHI_DECLARE_FLAG(double, name)
#define PHI_DECLARE_string(name) PHI_DECLARE_FLAG(std::string, name)

namespace phi {
class FlagRegisterer {
public:
  template <typename T>
  FlagRegisterer(std::string name,
                 std::string help,
                 std::string file,
                 T* current_value,
                 T* default_value);
};
}  // namespace phi

// ----------------------------DEFINE FLAGS----------------------------
#define PHI_DEFINE_FLAG(type, name, default_value, help_string)              \
  namespace phi {                                                            \
  namespace flag_##type {                                                    \
    static type FLAGS_##name##_default = default_value;                      \
    PHI_EXPORT_FLAG type FLAGS_##name = FLAGS_##name##_default;              \
    /* Register FLAG */                                                      \
    static phi::FlagRegisterer Flag##name##_FlagRegisterer(                  \
      #name, help_string, __FILE__, &FLAGS_##name, &FLAGS_##name##_default); \
  }                                                                          \
  }                                                                          \
  using phi::flag_##type::FLAGS_##name

#define PHI_DEFINE_bool(name, val, txt) \
  PHI_DEFINE_FLAG(bool, name, val, txt)
#define PHI_DEFINE_int32(name, val, txt) \
  PHI_DEFINE_FLAG(int32_t, name, val, txt)
#define PHI_DEFINE_uint32(name, val, txt) \
  PHI_DEFINE_FLAG(uint32_t, name, val, txt)
#define PHI_DEFINE_int64(name, val, txt) \
  PHI_DEFINE_FLAG(int64_t, name, val, txt)
#define PHI_DEFINE_uint64(name, val, txt) \
  PHI_DEFINE_FLAG(uint64_t, name, val, txt)
#define PHI_DEFINE_double(name, val, txt) \
  PHI_DEFINE_FLAG(double, name, val, txt)
#define PHI_DEFINE_string(name, val, txt) \
  PHI_DEFINE_FLAG(std::string, name, val, txt)
