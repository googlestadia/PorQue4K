/*
 Copyright 2018-2019 Google Inc.
 
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

#ifndef __VKEX_TO_STRING_H__
#define __VKEX_TO_STRING_H__

#include <vkex/Config.h>

#include <iomanip>
#include <type_traits>

namespace vkex {

std::string ToString(VkResult vk_result);

std::string ToStringShort(VkPhysicalDeviceType value);
std::string ToString(VkPhysicalDeviceType value);

std::string ToStringShort(VkFormat value);
std::string ToString(VkFormat value);

std::string ToStringShort(VkColorSpaceKHR value);
std::string ToString(VkColorSpaceKHR value);

std::string ToStringShort(VkPresentModeKHR value);
std::string ToString(VkPresentModeKHR value);

template <typename T>
std::string ToHexString(T* ptr, bool pad_zero = true, bool upper_case_alpha = true, bool prefix = true)
{
  std::stringstream ss;
  ss << std::setfill('0');
  if (sizeof(ptr) == 8) {
    if (pad_zero) {
      ss << std::setw(16);
    }
    ss << std::hex << reinterpret_cast<uintptr_t>(ptr);
  }
  else if (sizeof(ptr) == 4) {
    if (pad_zero) {
      ss << std::setw(8);
    }
    ss << std::hex << reinterpret_cast<uintptr_t>(ptr);
  }
  std::string s = ss.str();
  if (upper_case_alpha) {
    for (auto& c : s) {
      c = toupper(c);
    }
  }
  if (prefix) {
    s = "0x" + s;
  }
  return s;
}

template <typename T>
std::string ToHexString(T value, bool pad_zero = true, bool upper_case_alpha = true, bool prefix = true)
{
  bool is_integer = std::numeric_limits<T>::is_integer;
  assert(is_integer && "T isn't an integer!");
  std::stringstream ss;
  ss << std::setfill('0');
  if (sizeof(value) == 8) {
    if (pad_zero) {
      ss << std::setw(16);
    }
    ss << std::hex << value;
  }
  else if (sizeof(value) == 4) {
    if (pad_zero) {
      ss << std::setw(8);
    }
    ss << std::hex << value;
  }
  else if (sizeof(value) == 2) {
    if (pad_zero) {
      ss << std::setw(4);
    }
    ss << std::hex << value;
  }
  else if (sizeof(value) == 1) {
    if (pad_zero) {
      ss << std::setw(2);
    }
    ss << std::hex << static_cast<uint32_t>(value);
  }
  std::string s = ss.str();
  if (upper_case_alpha) {
    for (auto& c : s) {
      c = toupper(c);
    }
  }
  if (prefix) {
    s = "0x" + s;
  }
  return s;
}

} // namespace vkex

#endif // __VKEX_TO_STRING_H__