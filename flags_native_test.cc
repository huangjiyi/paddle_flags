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

#include "flags_native.h"

#include <iostream>

#define LOG(t) std::cout << t << std::endl
#define LOG_VAR(var) std::cout << #var << ": " << var << std::endl

namespace phi {
namespace test {

}
}

PHI_DEFINE_int32(name, 16, "test help string...");

int main(int argc, char* argv[]) {
  LOG("main start");
  LOG_VAR(FLAGS_name);

  return 0;
}