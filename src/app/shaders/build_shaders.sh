#!/bin/sh

#
# Copyright 2020 Google Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
SRC_DIR=${SCRIPT_DIR}
SPV_DIR=${SCRIPT_DIR}/../../../assets/shaders

SELF_INC_DIR=${SCRIPT_DIR}
SELF_INC_DIR=$(realpath --relative-to=${SCRIPT_DIR} ${SELF_INC_DIR})
VKEX_INC_DIR=$(realpath --relative-to=${SCRIPT_DIR} ${SRC_DIR}/..)
CAS_INC_DIR=$(realpath --relative-to=${SCRIPT_DIR} ${SRC_DIR}/../../../third_party/FidelityFX/FFX_CAS/ffx-cas-headers)

echo ${VKEX_INC_DIR}

HLSL_FILES=(draw_cb.hlsl draw_standard.hlsl)
for src_file in "${HLSL_FILES[@]}"
do
  echo -e "\nCompiling ${src_file}"
  base_name=$(basename -s .hlsl ${src_file})
  hlsl_file=${SRC_DIR}/${src_file}
  vs_spv=${SPV_DIR}/${base_name}.vs.spv
  ps_spv=${SPV_DIR}/${base_name}.ps.spv
  cmd="dxc -spirv -T vs_6_0 -E vsmain -fvk-use-dx-layout -Fo ${vs_spv} -I ${SELF_INC_DIR} -I ${VKEX_INC_DIR} ${hlsl_file}"
  echo ${cmd}
  eval ${cmd}
  cmd="dxc -spirv -T ps_6_0 -E psmain -fvk-use-dx-layout -Fo ${ps_spv} -I ${SELF_INC_DIR} -I ${VKEX_INC_DIR} ${hlsl_file}"
  echo ${cmd}
  eval ${cmd} 
done

HLSL_COMPUTE_FILES=(cas.hlsl checkerboard_upscale.hlsl copy_texture.hlsl image_delta.hlsl)
for src_file in "${HLSL_COMPUTE_FILES[@]}"
do
  echo -e "\nCompiling ${src_file}"
  base_name=$(basename -s .hlsl ${src_file})
  hlsl_file=${SRC_DIR}/${src_file}
  cs_spv=${SPV_DIR}/${base_name}.cs.spv
  cmd="dxc -spirv -T cs_6_0 -E csmain -fvk-use-dx-layout -Fo ${cs_spv} -I ${SELF_INC_DIR} -I ${VKEX_INC_DIR} -I ${CAS_INC_DIR} ${hlsl_file}"
  echo ${cmd}
  eval ${cmd}
done

read -p "Press enter to continue" nothing