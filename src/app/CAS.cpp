/*
 Copyright 2020 Google Inc.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

#include <cmath>
#include <stdint.h>
#define A_CPU 1

#include "ffx_a.h"
#include "ffx_cas.h"

#include "AppCore.h"

void VkexInfoApp::UpdateCASConstants(const VkExtent2D &srcExtent,
                                     const VkExtent2D &dstExtent,
                                     const float sharpness,
                                     CASUpscalingConstants &constants) {
  varAU4(const0);
  varAU4(const1);
  CasSetup(const0, const1, sharpness, AF1_(srcExtent.width),
           AF1_(srcExtent.height), AF1_(dstExtent.width),
           AF1_(dstExtent.height));
  constants.data.const0.r = const0[0];
  constants.data.const0.g = const0[1];
  constants.data.const0.b = const0[2];
  constants.data.const0.a = const0[3];
  constants.data.const1.r = const1[0];
  constants.data.const1.g = const1[1];
  constants.data.const1.b = const1[2];
  constants.data.const1.a = const1[3];
  constants.data.srcWidth = srcExtent.width;
  constants.data.srcHeight = srcExtent.height;
  constants.data.dstWidth = dstExtent.width;
  constants.data.dstHeight = dstExtent.height;
  constants.data.sharpness = sharpness;
}
