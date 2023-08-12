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

PD_DEFINE_int32(env_int32, 0, "test flag from env ..");

PD_DECLARE_string(name);
PD_DEFINE_int32(count, 10, "test int type flag...");

using namespace paddle::flags;

int main(int argc, char* argv[]) {
  SetUsageMessage("test ...");
  ParseCommandLineFlags(&argc, &argv);

  // phi::PrintAllFlagHelp();
  SetFlagsFromEnv({"env_int32", "env_int64"}, false);
  
  PrintAllFlagValue();

  return 0;
}