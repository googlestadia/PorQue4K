/*
 Copyright 2018 Google Inc.
 
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

#ifndef __DEBUG_MARKER_H__
#define __DEBUG_MARKER_H__

#include <glm/glm.hpp>
#include "VkexLoader.h"

#define SCOPE1(cmd, name) vkex::internal::DebugMarkerHelper temp_##__LINE__(cmd, name);
#define SCOPE2(cmd, name, color) vkex::internal::DebugMarkerHelper temp_##__LINE__(cmd, name, color);
#define BEGIN1(cmd, name) vkex::internal::DebugMarkerBegin(cmd, name);
#define BEGIN2(cmd, name, color) vkex::internal::DebugMarkerBegin(cmd, name, color);
#define GET_MACRO(_1, _2, _3, NAME, ...) NAME

#if defined(VKEX_WIN32)
// No op these for Windows
#define DEBUG_MARKER_SCOPE
#define DEBUG_MARKER_BEGIN
#define DEBUG_MARKER_END
#else
// Use as follows: DEBUG_MARKER_SCOPE(name) or DEBUG_MARKER_SCOPE(name, color).
// Note: Make sure scope exists within vkBeginCommandBuffer/vkEndCommandBuffer!
#define DEBUG_MARKER_SCOPE(...) GET_MACRO(__VA_ARGS__, SCOPE2, SCOPE1)(__VA_ARGS__)

// Use as follows: DEBUG_MARKER_BEGIN(name) or DEBUG_MARKER_BEGIN(name, color).
#define DEBUG_MARKER_BEGIN(...) GET_MACRO(__VA_ARGS__, BEGIN2, BEGIN1)(__VA_ARGS__)

#define DEBUG_MARKER_END(cmd) vkex::internal::DebugMarkerEnd(cmd);

#endif

namespace vkex {

// =================================================================================================
// Internal functions/classes. Use above DEBUG_MARKER_* macros! 
// =================================================================================================
namespace internal {

inline void DebugMarkerBegin(VkCommandBuffer cmd_buffer, const char *marker_name, 
                              const glm::vec4 &color = glm::vec4(1,1,1,1)) {
  VkDebugMarkerMarkerInfoEXT markerInfo = {};
  markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
  memcpy(markerInfo.color, &color[0], sizeof(float) * 4);
  markerInfo.pMarkerName = marker_name;
  if (vkex::CmdDebugMarkerBeginEXT != nullptr)
    vkex::CmdDebugMarkerBeginEXT(cmd_buffer, &markerInfo);
}

inline void DebugMarkerEnd(VkCommandBuffer cmd_buffer) {
  if (vkex::CmdDebugMarkerEndEXT != nullptr)
    vkex::CmdDebugMarkerEndEXT(cmd_buffer);
}

// =================================================================================================
// DebugMarkerHelper
// =================================================================================================
class DebugMarkerHelper {
public:
  DebugMarkerHelper(VkCommandBuffer cmd_buffer, const char *marker_name, 
                    const glm::vec4 &color = glm::vec4(1,1,1,1)) {
    m_cmd_buffer = cmd_buffer;
    DebugMarkerBegin(cmd_buffer, marker_name, color);
  }
  ~DebugMarkerHelper() {
    DebugMarkerEnd(m_cmd_buffer);
  }

private:
  VkCommandBuffer m_cmd_buffer;
};

} // namespace internal
} // namespace vkex

#endif // __DEBUG_MARKER_H__