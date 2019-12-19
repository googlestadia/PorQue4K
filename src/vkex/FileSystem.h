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

#ifndef __VKEX_FILE_SYSTEM_H__
#define __VKEX_FILE_SYSTEM_H__

#ifndef __cplusplus
 #error "C++ is required"
#endif

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#if defined(__linux__)
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
#endif

namespace vkex {
namespace fs {

enum {
  PATH_DOES_NOT_EXIST = -1,
  PATH_IS_NOT_FILE    = -2,
  OPEN_FILE_FAILED    = -3,
};

class path {
public:
  path() {}

  path(const std::string& s) { 
    append(s, true); 
    update_cache();
  }

  path(const char* s) { 
    append(s, true); 
    update_cache();
  }

  path(const path& p) { 
    m_cached = p.m_cached;
    m_dirty = p.m_dirty;
    m_has_root = p.m_has_root; 
    m_parts = p.m_parts; 
  }

  virtual ~path() {}

  operator bool() const { 
    update_cache(); 
    return m_cached.empty() ? false : true; 
  }

  operator std::string() const { 
    return str(); 
  }

  operator const char*() const {
    return c_str();
  }

  bool  operator==(const path& rhs) const { 
    return str() == rhs.str(); 
  }

  bool  operator!=(const path& rhs) const { 
    return str() != rhs.str(); 
  }

  path& operator=(const path& rhs) { 
    if (this != &rhs) { 
      m_cached = rhs.m_cached;
      m_dirty = rhs.m_dirty; 
      m_has_root = rhs.m_has_root; 
      m_parts = rhs.m_parts; 
    } 
    return *this; 
  }

  path& operator=(const std::string& rhs) { 
    append(rhs, true); 
    return *this; 
  }

  path& operator=(const char* rhs) { 
    append(rhs, true); 
    return *this; 
  }

  path& operator/=(const path& rhs) { 
    append(rhs.str()); 
    return *this; 
  }

  path& operator/=(const std::string& rhs) { 
    append(rhs); 
    return *this; 
  }

  path& operator/=(const char* rhs) { 
    append(rhs); 
    return *this; 
  }

  path operator/(const path& rhs) const { 
    path r = *this;
    r.append(rhs.str()); 
    return r; 
  }

  path operator/(const std::string& rhs) const { 
    path r = *this; 
    r.append(rhs); 
    return r; 
  }

  path operator/(const char* rhs) const { 
    path r = *this; 
    r.append(rhs); 
    return r; 
  }

  path operator+(const char* rhs) const {
    path r = path(std::string(this->c_str()) + rhs);
    return r;
  }

  bool empty() const {
    return m_parts.empty();
  }

  size_t part_count() const {
    return m_parts.size();
  }

  const std::string&  str() const { 
    update_cache(); 
    return m_cached; 
  }

  const char* c_str() const { 
    update_cache(); 
    return m_cached.c_str(); 
  }

  bool  is_root() const { 
    update_cache(); 
    return ((m_cached.size() == 1) && (m_cached[0] == '/')) || 
           ((m_cached.size() == 2) && (m_cached[1] == ':'));
  }

  fs::path parent() const { 
    fs::path r = *this; 
    if ((! r.is_root()) && (! r.m_parts.empty())) { 
      r.m_parts.pop_back(); 
    }
    r.m_dirty = true; 
    r.update_cache();
    return r; 
  }
  
  /*! @fn extension 

   @return Returns the extension start from, and including, the last period.

   file.ext       - ".ext" is returned
   file.other.ext - ".ext" is returned

  */
  fs::path extension() const {
    fs::path ext;
    if (!m_parts.empty()) {
      const std::string& s = m_parts.back();
      std::string::size_type pos = s.rfind('.');
      if (pos != std::string::npos) {        
        std::string::size_type len = s.length();
        std::string s_ext = s.substr(pos, len - pos);
        ext = fs::path(s_ext);
      }
    }
    return ext;
  }


  /*! @fn full_extension 

   @return Returns the extension start from, and including, the last period.

   file.ext       - ".ext" is returned
   file.other.ext - "other.ext" is returned

  */
  fs::path full_extension() const {
    fs::path ext;
    if (!m_parts.empty()) {
      const std::string& s = m_parts.back();
      std::string::size_type pos = s.find('.');
      if (pos != std::string::npos) {        
        std::string::size_type len = s.length();
        std::string s_ext = s.substr(pos, len - pos);
        ext = fs::path(s_ext);
      }
    }
    return ext;
  }
  
  friend std::ostream& operator<<(std::ostream& os, const fs::path& obj) {
    os << obj.str();
    return os;
  }

private:
  void append(std::string s, bool reset = false) {
    if (s.empty()) { 
      return; 
    }

    if ((! reset) && (s[0] == '/')) {
      throw std::runtime_error("cannot append path that contains root");
    }

    // Change all '\' to '/'
    if (s.find('\\') != std::string::npos) {
      std::transform(s.begin(), 
                     s.end(), 
                     s.begin(),
                     [](typename std::string::value_type c)
                         { return (c == '\\') ? '/' : c; });
    }

    // Collapse repeating '/' to a single '/'
    while (s.find("//") != std::string::npos) {
      auto new_end = std::unique(s.begin(), 
                                 s.end(), 
                                 [](typename std::string::value_type lhs, 
                                    typename std::string::value_type rhs) 
                                        { return (lhs == rhs) && (lhs == '/'); });
      s.erase(new_end, s.end());
    }

    // Clear everything if this is a reset
    if (reset) {
      m_cached.clear(); 
      m_dirty = false; 
      m_has_root = false;  
      m_parts.clear(); 
    }

    // Split string into components
    m_has_root = (! s.empty()) && ((s[0] == '/') || ((s.size() > 1) && (s[1] == ':')));
    if (s.find('/') != std::string::npos) {
      std::istringstream is(s);
      while (std::getline(is, s, '/')) { 
        m_parts.push_back(s); 
      }
    } else {
      m_parts.push_back(s);
    }
    // Mark dirty
    m_dirty = true;
  }

  void update_cache() const {
    if (! m_dirty) { 
      return; 
    }

    m_cached.clear();
    
    if (m_has_root && (! ((m_parts[0].size() > 1) && (m_parts[0][1] == ':')))) { 
        m_cached = "/"; 
    }

    if (! m_parts.empty()) {
      auto iter = m_parts.begin();
      m_cached = *(iter++);
      while (iter != m_parts.end()) {
        m_cached += '/';
        m_cached += *(iter++);
      }
    }

    m_dirty = false;
  }

private:
  mutable std::string       m_cached;
  mutable bool              m_dirty = false;
  mutable bool              m_has_root = false;
  std::vector<std::string>  m_parts;
};

inline bool exists(const fs::path& p) {
  struct stat info = {};
  return 0 == stat(p.c_str(), &info);
}

inline bool is_file(const fs::path& p) {
  struct stat info = {};
  if (0 != stat(p.c_str(), &info)) { return false;}
  return S_IFREG == (info.st_mode & S_IFREG);
}

inline bool is_directory(const fs::path& p) {
  struct stat info = {};
  if (0 != stat(p.c_str(), &info)) { return false;}
  return S_IFDIR == (info.st_mode & S_IFDIR);
}

/*! @fn file_size

   @return Returns file size if 'p' exists and is readable.
           Returns negative error code on error.

 */
inline int64_t file_size(const fs::path& p) {
  if (!exists(p)) {
    return fs::PATH_DOES_NOT_EXIST;
  }
  if (!is_file(p)) {
    return fs::PATH_IS_NOT_FILE;
  }
  std::ifstream is(p.c_str());
  if (!is.is_open()) {
    return fs::OPEN_FILE_FAILED;
  }
  is.seekg(0, std::ios::end);
  int64_t size = static_cast<int64_t>(is.tellg());
  is.close();
  return size;
}

/*! @fn load_file

   @return Returns array with data loaded from 'p'.
           Returns empty array if load fails.

 */
inline std::vector<uint8_t> load_file(const fs::path& p)
{
  std::vector<uint8_t> data;
  if (exists(p) && is_file(p)) {
    std::ifstream is(p.c_str(), std::ios::binary);
    if (is.is_open()) {      
      is.seekg(0, std::ios::end);
      size_t size = is.tellg();
      if (size > 0) {
        data.resize(size);
        is.seekg(0, std::ios::beg);
        is.read(reinterpret_cast<char*>(data.data()), data.size());
      }
      is.close();
    }
  }
  return data;
}

} // namespace fs
} // namespace vkex

#endif // __VKEX_FILE_SYSTEM_H__
