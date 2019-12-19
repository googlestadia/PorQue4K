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

#ifndef __VKEX_UTIL_H__
#define __VKEX_UTIL_H__

#include <type_traits>

namespace vkex {

template <typename T>
T RoundUp(T value, T multiple)
{
  static_assert(
    std::is_integral<T>::value,
    "T must be an integral type"
  );

  assert(multiple && ((multiple & (multiple - 1)) == 0));
  return (value + multiple - 1) & ~(multiple - 1);
}

inline uint32_t CountFlagBits(uint32_t value)
{
  uint32_t count = 0;
  while (value > 0) {
    count += (value & 1) ? 1 : 0;
    value >>= 1;
  }
  return count;
}

inline uint32_t GetHighestBitIndex(uint32_t value) 
{
  uint32_t index = 0;
  while (value > 0) {
    index +=  1;
    value >>= 1;
  }
  return index;
}

} // namespace vkex

#endif // __VKEX_UTIL_H__