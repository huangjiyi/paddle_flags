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

PD_DECLARE_string(name);
PD_DEFINE_int32(count, 10, "test int type flag...");

int main(int argc, char* argv[]) {
  LOG(argv[0]);
  LOG("main function start...");
  phi::SetUsageMessage("test ...");
  phi::ParseCommandLineFlags(&argc, &argv);
  LOG_VAR(FLAGS_name);

  string a = "asd";
  try {
    int a_int = std::stoi(a);
  } catch (std::exception& e) {
    LOG(e.what());
  }

  constexpr char asda[] = "asdf";

  phi::PrintAllFlags();

  return 0;
}