#!/bin/sh
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
SRC_DIR=${SCRIPT_DIR}
SPV_DIR=${SCRIPT_DIR}/../../assets/shaders

SELF_INC_DIR=${SCRIPT_DIR}
SELF_INC_DIR=$(realpath --relative-to=${SCRIPT_DIR} ${SELF_INC_DIR})
VKEX_INC_DIR=$(realpath --relative-to=${SCRIPT_DIR} ${SRC_DIR}/..)

echo ${VKEX_INC_DIR}

HLSL_FILES=(draw_buffer_load_no_transform.hlsl draw_mtex_load_no_transform.hlsl draw_tex_load_no_transform.hlsl draw_tex_read_no_transform.hlsl\
 draw_mtex_read_no_transform.hlsl draw_deferred_no_transform.hlsl draw_deferred_no_transform.hlsl draw_deferred_no_transform.hlsl draw_red_no_transform.hlsl\
 draw_vertex_no_transform.hlsl draw_vertex.hlsl draw_texture.hlsl draw_alu_pbr_no_transform.hlsl)
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

HLSL_COMPUTE_FILES=(fill_buffer.hlsl compute_launch_rate.hlsl)
for src_file in "${HLSL_COMPUTE_FILES[@]}"
do
  echo -e "\nCompiling ${src_file}"
  base_name=$(basename -s .hlsl ${src_file})
  hlsl_file=${SRC_DIR}/${src_file}
  cs_spv=${SPV_DIR}/${base_name}.cs.spv
  cmd="dxc -spirv -T cs_6_0 -E csmain -fvk-use-dx-layout -Fo ${cs_spv} -I ${SELF_INC_DIR} -I ${VKEX_INC_DIR} ${hlsl_file}"
  echo ${cmd}
  eval ${cmd}
done

read -p "Press enter to continue" nothing