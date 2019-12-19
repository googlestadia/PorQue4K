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

#ifndef __VKEX_CPU_RESOURCE_H__
#define __VKEX_CPU_RESOURCE_H__

#include <vkex/Config.h>
#include <vkex/VulkanUtil.h>

namespace vkex {

// =================================================================================================
// ElementDataStore
// =================================================================================================

/** @fn ElementDataStore
 *
 */ 
class ElementDataStore {
public:
  ElementDataStore();

  ~ElementDataStore();

  bool            IsEmpty() const;
  size_t          GetElementCount() const;
  size_t          GetDataSize() const;
  uint8_t*        GetData();
  const uint8_t*  GetData() const;

  template <typename T>
  const T* GetData() const {
    const T* p_data = reinterpret_cast<const T*>(m_data.data());
    return p_data;
  }

  template <typename T>
  T* GetData() {
    T* p_data = reinterpret_cast<T*>(m_data.data());
    return p_data;
  }

  template <typename T>
  void AppendData(const T& src_data) {
    size_t src_data_size = sizeof(T);
    size_t current_size = m_data.size();
    size_t new_size = current_size + src_data_size;
    m_data.resize(new_size);
    const uint8_t* src = reinterpret_cast<const uint8_t*>(&src_data);
    uint8_t* dst = reinterpret_cast<uint8_t*>(&m_data[current_size]);
    std::memcpy(dst, src, src_data_size);
  }

  template <typename T>
  void CopyData(const std::initializer_list<T>& src_data) {
    m_element_count = src_data.size();
    size_t size = m_element_count * sizeof(T);
    m_data.resize(size);
    if (size > 0) {
      typename std::initializer_list<T>::const_iterator src = src_data.begin();
      T* dst = reinterpret_cast<T*>(m_data.data());
      std::memcpy(dst, src, size);
    }
  }

  template <typename T>
  void CopyData(const std::vector<T>& src_data) {
    m_element_count = src_data.size();
    size_t size = m_element_count * sizeof(T);
    m_data.resize(size);
    if (size > 0) {
      const T* src = src_data.data();
      T* dst = reinterpret_cast<T*>(m_data.data());
      std::memcpy(dst, src, size);
    }
  }

protected:
  size_t                m_element_count = 0;
  std::vector<uint8_t>  m_data;
};

// =================================================================================================
// ConstantBufferData
// =================================================================================================
template <typename DataT>
struct ConstantBufferData {
  using data_type = DataT;
  static const size_t size = sizeof(data_type);
  data_type data = {};
};

// =================================================================================================
// VertexBufferData
// =================================================================================================

/** @fn VertexBufferData
 *
 */ 
class VertexBufferData : public ElementDataStore {
public:
  VertexBufferData();

  VertexBufferData(const vkex::VertexBindingDescription& vertex_description) 
    : m_vertex_description(vertex_description)
  {
  }

  template <typename T>
  VertexBufferData(const std::initializer_list<T>& obj) 
  {
    CopyData(obj);
  }

  template <typename T>
  VertexBufferData(const vkex::VertexBindingDescription& vertex_description, const std::initializer_list<T>& vertex_data)
    : m_vertex_description(vertex_description)
  {
    CopyData(vertex_data);
  }

  template <typename T>
  VertexBufferData(const vkex::VertexBindingDescription& vertex_description, const std::vector<T>& vertex_data)
    : m_vertex_description(vertex_description)
  {
    CopyData(vertex_data);
  }

  ~VertexBufferData();

  const vkex::VertexBindingDescription& GetVertexBindingDescription() const;
  void                                  SetVertexBindingDescription(const vkex::VertexBindingDescription& vertex_description);      

private:
  vkex::VertexBindingDescription  m_vertex_description;
};

// =================================================================================================
// IndexBufferData
// =================================================================================================

/** @fn IndexBufferData
 *
 */ 
class IndexBufferData : public ElementDataStore {
public:
  IndexBufferData();

  template <typename T>
  IndexBufferData(const std::initializer_list<T>& obj) 
  {
    size_t index_size = sizeof(T);
    if (index_size == 2) {
      m_index_type = VK_INDEX_TYPE_UINT16;
    }
    else if (index_size == 4) {
      m_index_type = VK_INDEX_TYPE_UINT32;
    }

    if (IsValid()) {
      CopyData(obj);
    }
  }

  template <typename T>
  IndexBufferData(const std::vector<T>& obj) 
  {
    size_t index_size = sizeof(T);
    if (index_size == 2) {
      m_index_type = VK_INDEX_TYPE_UINT16;
    }
    else if (index_size == 4) {
      m_index_type = VK_INDEX_TYPE_UINT32;
    }

    if (IsValid()) {
      CopyData(obj);
    }
  }

  ~IndexBufferData();

  bool          IsValid() const;
  VkIndexType   GetIndexType() const;

private:
  VkIndexType m_index_type = InvalidValue<VkIndexType>::Value;
};

} // namespace vkex

#endif // __VKEX_CPU_RESOURCE_H__
