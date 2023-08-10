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

#include "flags.h"

#include <assert.h>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <mutex>
#include <fstream>

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

class Flag {
public:
  Flag(std::string name,
       std::string description,
       std::string file,
       FlagType type,
       const void* default_value,
       void* value)
    : name_(name),
      description_(description),
      file_(file),
      type_(type),
      default_value_(default_value),
      value_(value) {
  }
  ~Flag() = default;

  // Summary: --name_: type_, description_ (default: default_value_)
  std::string Summary() const;

  void SetValueFromString(const std::string& value);

private:
  friend class FlagRegistry;

  const std::string name_;
  const std::string description_;
  const std::string file_;
  const FlagType type_;
  const void* default_value_;
  void* value_;
};

class FlagRegistry {
public:
  static FlagRegistry* Instance() {
    static FlagRegistry* global_registry_ = new FlagRegistry();
    return global_registry_;
  }

  void RegisterFlag(Flag* flag);

  void SetFlagValue(const std::string& name, const std::string& value);

  bool HasFlag(const std::string& name) const;

  void PrintAllFlags(std::ostream& os) const;

private:
  FlagRegistry() = default;

  std::map<std::string, Flag*> flags_;

  struct FlagCompare {
    bool operator()(const Flag* flag1, const Flag* flag2) const {
      return flag1->name_ < flag2->name_;
    }
  };

  std::map<std::string, std::set<Flag*, FlagCompare>> flags_by_file_;

  std::mutex mutex_;
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

template <typename T>
FlagRegisterer::FlagRegisterer(std::string name,
                               std::string help,
                               std::string file,
                               const T* default_value,
                               T* value) {
  FlagType type = FlagTypeTraits<T>::Type;
  Flag* flag = new Flag(name, help, file, type, default_value, value);
  FlagRegistry::Instance()->RegisterFlag(flag);
}

// Instantiate FlagRegisterer for supported types.
#define INSTANTIATE_FLAG_REGISTERER(type)  \
  template FlagRegisterer::FlagRegisterer( \
    std::string name, std::string help, std::string file, const type* default_value, type* value)

INSTANTIATE_FLAG_REGISTERER(bool);
INSTANTIATE_FLAG_REGISTERER(int32_t);
INSTANTIATE_FLAG_REGISTERER(uint32_t);
INSTANTIATE_FLAG_REGISTERER(int64_t);
INSTANTIATE_FLAG_REGISTERER(uint64_t);
INSTANTIATE_FLAG_REGISTERER(double);
INSTANTIATE_FLAG_REGISTERER(std::string);

#undef INSTANTIATE_FLAG_REGISTERER

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
    std::lock_guard<std::mutex> lock(mutex_);
    flags_[flag->name_] = flag;
    flags_by_file_[flag->file_].insert(flag);
  }
}

void FlagRegistry::SetFlagValue(const std::string& name, const std::string& value) {
  std::lock_guard<std::mutex> lock(mutex_);
  flags_[name]->SetValueFromString(value);
}

bool FlagRegistry::HasFlag(const std::string& name) const {
  return flags_.find(name) != flags_.end();
}

void FlagRegistry::PrintAllFlags(std::ostream& os) const {
  for (const auto& iter : flags_by_file_) {
    os << std::endl
       << "Flags defined in " << iter.first << ":" << std::endl;
    for (const auto& flag : iter.second) {
      os << "  " << flag->Summary() << std::endl;
    }
  }
  os << std::endl;
}

std::string Flag::Summary() const {
  std::string type_string, value_string;
  switch (type_) {
  case FlagType::BOOL: {
    type_string = "bool";
    const bool* val = static_cast<const bool*>(default_value_);
    value_string = *val ? "true" : "false";
    break;
  }
  case FlagType::INT32: {
    type_string = "int32";
    const int32_t* val = static_cast<const int32_t*>(default_value_);
    value_string = std::to_string(*val);
    break;
  }
  case FlagType::UINT32: {
    type_string = "uint32";
    const uint32_t* val = static_cast<const uint32_t*>(default_value_);
    value_string = std::to_string(*val);
    break;
  }
  case FlagType::INT64: {
    type_string = "int64";
    const int64_t* val = static_cast<const int64_t*>(default_value_);
    value_string = std::to_string(*val);
    break;
  }
  case FlagType::UINT64: {
    type_string = "uint64";
    const uint64_t* val = static_cast<const uint64_t*>(default_value_);
    value_string = std::to_string(*val);
    break;
  }
  case FlagType::DOUBLE: {
    type_string = "double";
    const double* val = static_cast<const double*>(default_value_);
    value_string = std::to_string(*val);
    break;
  }
  case FlagType::STRING: {
    type_string = "string";
    const std::string* val = static_cast<const std::string*>(default_value_);
    value_string = *val;
    break;
  }
  default:
    assert(false);
  }
  std::string summary = "--" + name_ + ": " + type_string + ", " + description_ + " (default: " + value_string + ")";
  return summary;
}

void Flag::SetValueFromString(const std::string& value) {
  switch (type_) {
  case FlagType::BOOL: {
    bool* val = static_cast<bool*>(value_);
    if (value == "true" || value == "True" || value == "TRUE" || value == "1") {
      *val = true;
    } else if (value == "false" || value == "False" || value == "FALSE" || value == "0") {
      *val = false;
    } else {
      std::cout << "ERROR: Value: \"" + value + "\" is invalid for bool flag \"" + name_
                << "\", please use [true, True, TRUE, 1] or [false, False, FALSE, 0].";
    }
    break;
  }
  case FlagType::INT32: {
    int32_t* val = static_cast<int32_t*>(value_);
    *val = std::stoi(value);
    break;
  }
  case FlagType::UINT32: {
    uint32_t* val = static_cast<uint32_t*>(value_);
    *val = std::stoul(value);
    break;
  }
  case FlagType::INT64: {
    int64_t* val = static_cast<int64_t*>(value_);
    *val = std::stoll(value);
    break;
  }
  case FlagType::UINT64: {
    uint64_t* val = static_cast<uint64_t*>(value_);
    *val = std::stoull(value);
    break;
  }
  case FlagType::DOUBLE: {
    double* val = static_cast<double*>(value_);
    *val = std::stod(value);
    break;
  }
  case FlagType::STRING: {
    std::string* val = static_cast<std::string*>(value_);
    *val = value;
    break;
  }
  default: {
    std::cout << "ERROR: flag \"" + name_ + "\" has undefined type.";
    break;
  }
  }
}

void PrintAllFlags(bool to_file, const std::string& file_path) {
  if (to_file) {
    std::ofstream fout(file_path);
    FlagRegistry::Instance()->PrintAllFlags(fout);
  } else {
    FlagRegistry::Instance()->PrintAllFlags(std::cout);
  }
}

static std::string program_usage = "";

void SetUsageMessage(const std::string& usage) {
  program_usage = usage;
}

void ParseCommandLineFlags(int* pargc, char*** pargv) {
  std::cout << "Start parse commandline flags." << std::endl;
  // Pre-process arguments
  assert(*pargc > 0);
  size_t argv_num = *pargc - 1;
  std::vector<std::string> argvs(*pargv + 1, *pargv + *pargc);

  // Parse arguments
  for (int i = 0; i < argv_num; i++) {
    const std::string& argv = argvs[i];
    std::cout << std::endl;
    LOG_VAR(argv);

    // Ignore the argvs that not start with "--"
    if (argv.size() < 2 || argv[0] != '-' || argv[1] != '-') {
      std::cout << "ERROR: Invalid commandline argument: \"" << argv << "\""
                << ", it should match the format: "
                << "\"--help\", \"--name=value\" or \"--name value\"."
                << std::endl;
      continue;
    }
    // If argv is "--help", print all flags help message and exit
    if (argv == "--help") {
      std::cout << "Usage: " << program_usage << std::endl;
      FlagRegistry::Instance()->PrintAllFlags(std::cout);
      exit(1);
    }

    // Parse arg name and value
    string name, value;
    size_t split_pos = argv.find('=');
    if (split_pos == string::npos) {
      // The argv format is "--name value", get the value from next argv.
      name = argv.substr(2);
      ++i;
      if (i == argv_num || (argvs[i][0] == '-' and argvs[i][1] == '-')) {
        std::cout << "ERROR: expected flag value of commandline argument "
                  << "\"" << argv << "\" but found none." << std::endl;
        continue;
      } else {
        value = argvs[i];
      }
    } else {
      // The argv format is "--name=value"
      name = argv.substr(2, split_pos - 2);
      value = argv.substr(split_pos + 1);
    }
    LOG_VAR(name);
    LOG_VAR(value);

    // check if the flag is registered
    FlagRegistry* registry_ = FlagRegistry::Instance();
    if (!registry_->HasFlag(name)) {
      std::cout << "Error: undefined flag named "
                << "\"" << name << "\". " << std::endl;
      continue;
    } else {
      // set the flag value
      registry_->SetFlagValue(name, value);
    }
  }
}

}  // namespace phi