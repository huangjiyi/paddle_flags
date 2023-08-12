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

PD_DEFINE_bool(bool_flag, false, "test bool type flag...");
PD_DEFINE_int32(count, 10, "test int type flag...");
PD_DEFINE_uint32(uint32_flag, 10, "test uint32 type flag...");
PD_DEFINE_int64(int64_flag, 10, "test int64 type flag...");
PD_DEFINE_uint64(uint64_flag, 10, "test uint64 type flag...");
PD_DEFINE_double(double_flag, 10.0, "test double type flag...");

PD_DECLARE_string(name);

PD_DEFINE_int32(env_int32, 0, "test flag from env ..");

using namespace paddle::flags;

int main(int argc, char* argv[]) {
  ParseCommandLineFlags(&argc, &argv);

  // SetFlagsFromEnv({"env_int32", "int32_flag", "asd"}, false);
  
  // PrintAllFlagHelp();
  
  PrintAllFlagValue();

  return 0;
}