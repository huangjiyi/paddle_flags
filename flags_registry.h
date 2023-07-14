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

#include <map>
#include <string>

#include "flags_native.h"

namespace phi {

enum class FlagType : uint8_t {
  BOOL = 0,
  INT32 = 1,
  UINT32 = 2,
  INT64 = 3,
  UINT64 = 4,
  DOUBLE = 5,
  STRING = 6,
  UNDEFINED = 7,
};

template <typename T>
struct FlagTypeTraits {
  static constexpr FlagType Type = FlagType::UNDEFINED;
};

#define DEFINE_FLAG_TYPE_TRAITS(type, flag_type) \
  template <>                                    \
  struct FlagTypeTraits<type> {                  \
    static constexpr FlagType Type = flag_type;  \
  }

DEFINE_FLAG_TYPE_TRAITS(bool, FlagType::BOOL);
DEFINE_FLAG_TYPE_TRAITS(int32_t, FlagType::INT32);
DEFINE_FLAG_TYPE_TRAITS(uint32_t, FlagType::UINT32);
DEFINE_FLAG_TYPE_TRAITS(int64_t, FlagType::INT64);
DEFINE_FLAG_TYPE_TRAITS(uint64_t, FlagType::UINT64);
DEFINE_FLAG_TYPE_TRAITS(double, FlagType::DOUBLE);
DEFINE_FLAG_TYPE_TRAITS(std::string, FlagType::STRING);

#undef DEFINE_FLAG_TYPE_TRAITS

class Flag {
public:
  Flag(std::string name,
       std::string help,
       std::string file,
       FlagType type,
       void* current_val,
       void* default_val)
    : name_(name),
      help_(help),
      file_(file),
      type_(type),
      current_val_(current_val),
      default_val_(default_val) {
  }
  ~Flag() = default;

private:
  friend class FlagRegistry;

  const std::string name_;  // flag name
  const std::string help_;  // help message
  const std::string file_;  // file name where the flag is defined
  const FlagType type_;     // flag value type
  void* current_val_;       // current flag value
  void* default_val_;       // default flag value
};

class FlagRegistry {
public:
  static FlagRegistry* Instance() {
    static FlagRegistry* global_registry_ = new FlagRegistry();
    return global_registry_;
  }

  void RegisterFlag(Flag* flag);

private:
  FlagRegistry() = default;

  std::map<std::string, Flag*> flags_;
};

void FlagRegistry::RegisterFlag(Flag* flag) {
  LOG("INFO: register flag \"" + flag->name_ + "\" which defined in file: " + flag->file_);
  // Defining a flag with the same name and type twice will raise a compile
  // error. While defining a flag with the same name and different type will
  // not (in different namespaces), but this is not allowed. So we check 
  // the flag name is whether registered here.
  auto iter = flags_.find(flag->name_);
  if (iter != flags_.end()) {
    LOG("ERROR: flag \"" + flag->name_ + "\" has been defined in " + iter->second->file_);
  } else {
    flags_[flag->name_] = flag;
  }
}

template <typename T>
FlagRegisterer::FlagRegisterer(std::string name,
                               std::string help,
                               std::string file,
                               T* current_value,
                               T* default_value) {
  FlagType type = FlagTypeTraits<T>::Type;
  Flag* flag = new Flag(name, help, file, type, current_value, default_value);
  FlagRegistry::Instance()->RegisterFlag(flag);
}

// Instantiate FlagRegisterer for supported types.
#define INSTANTIATE_FLAG_REGISTERER(type)                 \
  template FlagRegisterer::FlagRegisterer(                \
    std::string name, std::string help, std::string file, \
    type* current_value, type* default_value)

INSTANTIATE_FLAG_REGISTERER(bool);
INSTANTIATE_FLAG_REGISTERER(int32_t);
INSTANTIATE_FLAG_REGISTERER(uint32_t);
INSTANTIATE_FLAG_REGISTERER(int64_t);
INSTANTIATE_FLAG_REGISTERER(uint64_t);
INSTANTIATE_FLAG_REGISTERER(double);
INSTANTIATE_FLAG_REGISTERER(std::string);

#undef INSTANTIATE_FLAG_REGISTERER

}  // namespace phi