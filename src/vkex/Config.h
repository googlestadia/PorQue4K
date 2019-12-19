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

#ifndef __VKEX_CONFIG_H__
#define __VKEX_CONFIG_H__

#if defined(VKEX_WIN32)
# define VC_EXTRALEAN
# define WIN32_LEAN_AND_MEAN 
# define NOMINMAX
#endif

#include "vkex/VkexLoader.h"
#include "vkex/VkexLoaderHelper.h"
#include "spirv_reflect.h"
#include "vk_mem_alloc.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <vector>

#include "vkex/FileSystem.h"
#include "vkex/Forward.h"
#include "vkex/Log.h"
#include "vkex/ConfigMath.h"
#include "vkex/Util.h"

#define VKEX_MINIMUM_REQUIRED_VULKAN_VERSION  VK_MAKE_VERSION(1, 1, 0)

namespace vkex {

#define VKEX_FOUR_CC(C0, C1, C2, C3) \
  ((int)(C0 & 0xFF) <<  0) |         \
  ((int)(C1 & 0xFF) <<  8) |         \
  ((int)(C2 & 0xFF) << 16) |         \
  ((int)(C3 & 0xFF) << 24)

// =================================================================================================
// Enums
// =================================================================================================

/** @enum ComponentType
 *
 */
enum class ComponentType {
  UNDEFINED = 0,
  INT8,
  UINT8,
  INT16,
  UINT16,
  INT32,
  UINT32,
  INT64,
  UINT64,
  FLOAT16,
  FLOAT32,
  FLOAT64,
  PACKED,
  COMPRESSED,
};

// =================================================================================================
// Types
// =================================================================================================

/** @struct InvalidValue
 *
 */
template <typename T> struct InvalidValue {};

template <> struct InvalidValue<VkResult> {
  static const VkResult Value = static_cast<VkResult>(0x7FFFFFFF);
};

template <> struct InvalidValue<VkImageType> {
  static const VkImageType Value = static_cast<VkImageType>(~0);
};

template <> struct InvalidValue<VkImageViewType> {
  static const VkImageViewType Value = static_cast<VkImageViewType>(~0);
};

template <> struct InvalidValue<VkImageAspectFlags> {
  static const VkImageAspectFlags Value = static_cast<VkImageAspectFlags>(~0);
};

template <> struct InvalidValue<VkIndexType> {
  static const VkIndexType Value = static_cast<VkIndexType>(~0);
};

template <> struct InvalidValue<VkDescriptorType> {
  static const VkDescriptorType Value = static_cast<VkDescriptorType>(~0);
};

// =================================================================================================
// Result
// =================================================================================================

/** @class Result
 *
 */
class Result {
public:
  enum Value {
    Success                                             =  0,
    Undefined                                           =  0x7FFFFFFF,

    ErrorFailed                                         = -1,
    ErrorAllocationFailed                               = -2,
    ErrorUnexpectedNullPointer                          = -3,
    ErrorOutOfRange                                     = -4,

    ErrorInstanceAlreadyExists                          = -100,
    ErrorInstanceNotAllocated                           = -102,
    ErrorInvalidInstanceAddress                         = -101,

    ErrorVkexRequiresAtLeastVulkan11                    = -200,

    ErrorFunctionSetNotLoaded                           = -1000,
    ErrorInstanceProcNotFound                           = -1001,
    ErrorDeviceProcNotFound                             = -1002,
    ErrorInstanceLayerNotFound                          = -1003,
    ErrorInstanceExtensionNotFound                      = -1004,
    ErrorDeviceExtensionNotFound                        = -1005,
    ErrorPhysicalDevicesNotFound                        = -1006,
    ErrorSupportedQueueSlotNotFound                     = -1007,

    ErrorInvalidQueueCount                              = -1100,
    ErrorInvalidQueuePriorityCount                      = -1101,
    ErrorInvalidSurfaceFormatCount                      = -1102,
    ErrorInvalidQueueFamilyIndex                        = -1103,
    ErrorInvalidSwapchainImageIndex                     = -1104,
    ErrorInvalidBufferUsage                             = -1105,
    ErrorInvalidDestinationImageLayout                  = -1106,
    ErrorInvalidDescriptorBinding                       = -1107,
    ErrorInvalidDescriptorType                          = -1108,

    ErrorInvalidQueueObject                             = -1120,

    ErrorBufferSizeMustBeGreaterThanZero                = -1150,
    ErrorImageDiemsionsMustBeGreaterThanZero            = -1151,
    ErrorConstantBufferSizeMustBeGreaterThanZero        = -1152,
    ErrorStorageBufferSizeMustBeGreaterThanZero         = -1153,
    ErrorIndexBufferSizeMustBeGreaterThanZero           = -1154,
    ErrorVertexBufferSizeMustBeGreaterThanZero          = -1155,
    ErrorIndirectBufferSizeMustBeGreaterThanZero        = -1156,

    ErrorResourceIsNull                                 = -1170,
    ErrorResourceIsNotHostVisible                       = -1171,
    ErrorResouceSizeIsInsufficient                      = -1172,

    ErrorVertexShaderLoadFailed                         = -1181,
    ErrorHullShaderLoadFailed                           = -1182,
    ErrorDomainShaderLoadFailed                         = -1183,
    ErrorGeometryShaderLoadFailed                       = -1184,
    ErrorPixelShaderLoadFailed                          = -1185,
    ErrorComputeShaderLoadFailed                        = -1186,

    ErrorCommandBufferCountMustNotBeZero                = -1200,
    ErrorCommandBufferCountMustBeOne                    = -1201,
    ErrorDescriptorSetLayoutsMustBeMoreThanZero         = -1202,
    ErrorDescriptorSetLayoutsMustBeOne                  = -1203,
    ErrorDescriptorSetNumberNotFound                    = -1204,
    ErrorDuplicatetDescriptorBinding                    = -1205,
    ErrorPipelineMissingRequiredShaderStage             = -1206,

    ErrorVulkanFunctionFailed                           = -1300,
    ErrorSpirvReflectionError                           = -1301,
    ErrorImGuiInitializeFailed                          = -1302,

    // Application
    ErrorArgsParseFailed                                = -4000,
    ErrorInvalidApplicationMode                         = -4001,

    // Window
    ErrorInvalidWindowWidth                             = -5000,
    ErrorInvalidWindowHeight                            = -5001,
    ErrorGlfwInitializeFailed                           = -5002,
    ErrorGlfwWindowCreateFailed                         = -5003,
    ErrorWindowEventsAlreadyRegistered                  = -5004,

    // File system
    ErrorPathDoesNotExist                               = -10000,
    ErrorPathIsNotFile                                  = -10001,
    ErrorOpenFileFailed                                 = -10002,

    // Image error
    ErrorImageLoadFailed                                = -20000,
    ErrorImageInfoFailed                                = -20001,
    ErrorImageStorageSizeInsufficient                   = -20002,
    ErrorImageWriteFailed                               = -20003,
  };

  Result() {}

  Result(Result::Value value)
    : m_value(value) {}

  explicit Result(VkResult value)
    : m_value(ErrorVulkanFunctionFailed), m_vk_result(value)
  {
    m_flags.bits.vulkan = true;
  }

  explicit Result(SpvReflectResult value)
    : m_value(ErrorSpirvReflectionError), m_spv_reflect_result(value)
  {
    m_flags.bits.spirv_reflection = true;
  }

  ~Result() {}

  operator bool() const {
    bool result = IsSuccess();
    return result;
  }

  bool operator==(const Result& rhs) const {
    bool result = (m_value == rhs.m_value);
    return result;
  }

  bool operator!=(const Result& rhs) const {
    bool result = (m_value != rhs.m_value);
    return result;
  }

  bool operator==(const Result::Value rhs) const {
    bool result = (m_value == rhs);
    return result;
  }

  bool operator!=(const Result::Value rhs) const {
    bool result = (m_value != rhs);
    return result;
  }

  bool IsSuccess() const {
    bool result = (m_value == Result::Value::Success);
    return result;
  }

  bool IsWarning() const {
    bool result = (m_value > Result::Value::Success);
    return result;
  }

  bool IsError() const {
    bool result = (m_value < Result::Value::Success);
    return result;
  }

  Result::Value GetValue() const {
    return m_value;
  }

  VkResult GetVkResult() const {
    return m_vk_result;
  }

  friend std::ostream& operator<<(
    std::ostream& os,
    const Result& obj
  )
  {
    os << obj.m_value;
    return os;
  }

private:
  union Flags {
    struct {
      bool    vulkan           : 1;
      bool    spirv_reflection : 1;
    } bits;
    uint32_t  flags;
  };

  Flags               m_flags     = {};
  Value               m_value     = Value::ErrorFailed;
  union {
    VkResult          m_vk_result = VK_SUCCESS;
    SpvReflectResult  m_spv_reflect_result;
  };
};

// =================================================================================================
// Utility Functions
// =================================================================================================

/** @fn CountU32
 *
 */
template <typename T>
uint32_t CountU32(const std::vector<T>& v)
{
  return static_cast<uint32_t>(v.size());
}

/** @fn DataPtr
 *
 */
template <typename T>
const T* DataPtr(const std::vector<T>& v)
{
  return v.empty() ? nullptr : v.data();
}

/** @fn DataPtr
 *
 */
inline const char* DataPtr(const std::string& s)
{
  return s.empty() ? nullptr : s.c_str();
}

/** @fn GetCStrings
 *
 */
inline std::vector<const char*> GetCStrings(const std::vector<std::string>& v)
{
  std::vector<const char*> c_strs;
  for (const std::string& s : v) {
    c_strs.push_back(s.c_str());
  }
  return c_strs;
}

/** @fn Find
 *
 */
template <typename T>
typename std::vector<T>::const_iterator Find(const std::vector<T>& container, const T& value)
{
  auto it = std::find(std::begin(container), std::end(container), value);
  return it;
}

/** @fn FindIf
 *
 */
template <typename T, typename UnaryPredicate>
typename std::vector<T>::iterator FindIf(std::vector<T>& container, UnaryPredicate pred)
{
  auto it = std::find_if(std::begin(container), std::end(container), pred);
  return it;
}

/** @fn FindIf
 *
 */
template <typename T, typename UnaryPredicate>
typename std::vector<T>::const_iterator FindIf(const std::vector<T>& container, UnaryPredicate pred)
{
  auto it = std::find_if(std::begin(container), std::end(container), pred);
  return it;
}

/** @fn Contains
 *
 */
template <typename T>
bool Contains(const std::vector<T>& container, const T& value)
{
  auto it = Find(container, value);
  bool found = (it != std::end(container));
  return found;
}

/** @fn ContainsIf
 *
 */
template <typename T, typename UnaryPredicate>
bool ContainsIf(const std::vector<T>& container, UnaryPredicate pred)
{
  auto it = FindIf(container, pred);
  bool found = (it != std::end(container));
  return found;
}

// =================================================================================================
// Macros
// =================================================================================================
#define VKEX_ASSERT(COND)                     \
  if (!(COND)) {                              \
    VKEX_LOG_RAW("\n*** FATAL ERROR ***");    \
    VKEX_LOG_FATAL("Type : VKEX ASSERT");     \
    VKEX_LOG_FATAL("Cond : " << #COND);       \
    VKEX_LOG_FATAL("File : " << __FILE__);    \
    VKEX_LOG_FATAL("Line : " << __LINE__);    \
    assert(false);                            \
  }

#define VKEX_ASSERT_MSG(COND, MSG)            \
  if (!(COND)) {                              \
    VKEX_LOG_RAW("\n*** FATAL ERROR ***");    \
    VKEX_LOG_FATAL("Type : VKEX ASSERT");     \
    VKEX_LOG_FATAL("Cond : " << #COND);       \
    VKEX_LOG_FATAL("Msg  : " << MSG);         \
    VKEX_LOG_FATAL("File : " << __FILE__);    \
    VKEX_LOG_FATAL("Line : " << __LINE__);    \
    assert(false);                            \
  }

#define VKEX_ASSERT_EXTRA(COND, MSG, EXTRA)   \
  if (!(COND)) {                              \
    VKEX_LOG_RAW("\n*** FATAL ERROR ***");    \
    VKEX_LOG_FATAL("Type : VKEX ASSERT");     \
    VKEX_LOG_FATAL("Msg  : " << MSG);         \
    VKEX_LOG_FATAL("File : " << __FILE__);    \
    VKEX_LOG_FATAL("Line : " << __LINE__);    \
    VKEX_LOG_RAW(EXTRA);                      \
    assert(false);                            \
  }

#define VKEX_CALL(FN_CALL)                                        \
  {                                                               \
    vkex::Result vkex_result_from_fn_call = FN_CALL;              \
    if (!vkex_result_from_fn_call) {                              \
      VKEX_LOG_RAW("\n*** FATAL ERROR ***");                      \
      VKEX_LOG_FATAL("Type     : VKEX FUNCTION FAILED");          \
      VKEX_LOG_FATAL("Function : " << #FN_CALL);                  \
      VKEX_LOG_FATAL("Return   : " << vkex_result_from_fn_call);  \
      VKEX_LOG_FATAL("File     : " << __FILE__);                  \
      VKEX_LOG_FATAL("Line     : " << __LINE__);                  \
      assert(false);                                              \
    }                                                             \
  }

#define VKEX_RESULT_CALL(RES, FN_CALL)                  \
  RES = FN_CALL;                                        \
  if (!RES) {                                           \
    VKEX_LOG_RAW("\n*** FATAL ERROR ***");              \
    VKEX_LOG_FATAL("Type     : VKEX FUNCTION FAILED");  \
    VKEX_LOG_FATAL("Function : " << #FN_CALL);          \
    VKEX_LOG_FATAL("Return   : " << RES);               \
    VKEX_LOG_FATAL("File     : " << __FILE__);          \
    VKEX_LOG_FATAL("Line     : " << __LINE__);          \
    assert(false);                                      \
  }

#define VKEX_VULKAN_RESULT_CALL(RES, FN_CALL)             \
  RES = FN_CALL;                                          \
  if (RES != VK_SUCCESS) {                                \
    VKEX_LOG_RAW("\n*** FATAL ERROR ***");                \
    VKEX_LOG_FATAL("Type     : VULKAN FUNCTION FAILED");  \
    VKEX_LOG_FATAL("Function : " << #FN_CALL);            \
    VKEX_LOG_FATAL("Return   : " << vkex::ToString(RES)); \
    VKEX_LOG_FATAL("File     : " << __FILE__);            \
    VKEX_LOG_FATAL("Line     : " << __LINE__);            \
    assert(false);                                        \
  }

} // namespace vkex

#endif // __VKEX_CONFIG_H__
