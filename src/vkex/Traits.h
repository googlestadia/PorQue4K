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

#ifndef __VKEX_TRAITS_H__
#define __VKEX_TRAITS_H__

#include <vkex/Config.h>

namespace vkex {

/** @class IObjectStorageFunctions
 *
 */
class IObjectStorageFunctions {
protected:
  /** @fn CreateObject
   *
   */
  template <
    typename IObjectT,
    typename SetParentMemberFnT,
    typename ParentT,
    typename CreateInfoT,
    typename HandleT = typename std::add_pointer<IObjectT>::type,
    typename UniquePtrT = std::unique_ptr<IObjectT>
  >
  vkex::Result CreateObject(
    const CreateInfoT&            create_info, 
    const VkAllocationCallbacks*  p_allocator,
    std::vector<UniquePtrT>&      storage,
    SetParentMemberFnT            p_set_parent_member_fn,
    ParentT                       parent,  
    HandleT*                      p_object
  )
  {
    // Allocate object
    UniquePtrT obj = std::make_unique<IObjectT>();
    if (!obj) {
      return vkex::Result::ErrorAllocationFailed;
    }
    // Set parent
    HandleT raw_obj = obj.get();
    (raw_obj->*p_set_parent_member_fn)(parent);
    // Internal create
    vkex::Result vkex_result = obj->InternalCreate(create_info, p_allocator);
    if (!vkex_result) {
      obj.reset();
      return vkex_result;
    }
    // Grab object pointer
    *p_object = obj.get();
    // Store object
    storage.push_back(std::move(obj));
    // Success
    return vkex::Result::Success;
  }

  /** @fn DestroyObject
   *
   */
  template <
    typename IObjectT,
    typename HandleT = typename std::add_pointer<IObjectT>::type,
    typename UniquePtrT = std::unique_ptr<IObjectT>
  >
  vkex::Result DestroyObject(
    std::vector<UniquePtrT>&      storage,
    HandleT                       object, 
    const VkAllocationCallbacks*  p_allocator
  )
  {
    auto it = std::find_if(
      std::begin(storage),
      std::end(storage),
      [object](const UniquePtrT& elem) -> bool {
        return elem.get() == object; });

    // Exit if object isn't found
    if (it == std::end(storage)) {
      return vkex::Result::Success;
    }

    // Move object to take ownership
    UniquePtrT obj = std::move(*it);
    
    // Erase/remove elements that are null
    storage.erase(
      std::remove_if(
        std::begin(storage), 
        std::end(storage),
        [](const UniquePtrT& elem) -> bool {
          return elem.get() == nullptr ? true : false; }),
      std::end(storage));

    vkex::Result vkex_result = obj->InternalDestroy(p_allocator);
    if (!vkex_result) {
      return vkex_result;
    }

    return vkex::Result::Success;
  }

  /** @fn DestroyAllObjects
   *
   */
  template <
    typename IObjectT,
    typename UniquePtrT = std::unique_ptr<IObjectT>
  >
  vkex::Result DestroyAllObjects(
    std::vector<UniquePtrT>&      storage,
    const VkAllocationCallbacks*  p_allocator
  )
  {
    for (auto& obj : storage) {
      vkex::Result vkex_result = obj->InternalDestroy(p_allocator);
      if (!vkex_result) {
        return vkex_result;
      }
    }
    storage.clear();

    return vkex::Result::Success;
  }
};

/** @class IDeviceObject
 *
 */
class IDeviceObject {
public:
  vkex::Device GetDevice() const {
    return m_device;
  }
protected:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  void SetDevice(vkex::Device device) {
    m_device = device;
  }

protected:
  vkex::Device  m_device = nullptr;
};

} // namespace vkex

#endif // __VKEX_TRAITS_H__