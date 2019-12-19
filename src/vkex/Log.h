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

#ifndef __VKEX_LOG_H__
#define __VKEX_LOG_H__

#if defined(VKEX_WIN32)
 #define VC_EXTRALEAN
 #define WIN32_LEAN_AND_MEAN
 #include <Windows.h>
#endif

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

namespace vkex {

/** @class Log
*
*/
class Log {
public:
  Log();
  virtual ~Log();

  static void Initialize(const std::string& file_path);
  static Log* Get();

  void Flush() {
    Write(m_stream.str());
    FlushOutput();
    m_stream.str(std::string());
    m_stream.clear();
  }

  template <typename T>
  Log& operator<<(const T& value) {
    m_stream << value;
    return *this;
  }

  Log& operator<<(std::ostream&(*manip)(std::ostream&)) {
    if (manip == (std::basic_ostream<char>&(*)(std::basic_ostream<char>&))&std::endl) {
      m_stream << std::endl;
      Flush();
    }
    return *this;
  }

protected:
  virtual void Write(const std::string& msg) = 0;
  virtual void FlushOutput() = 0;

private:
  std::stringstream m_stream;
};

/** @class ConsoleLog
 *
 */
class ConsoleLog : public Log {
public:
  ConsoleLog();
  virtual ~ConsoleLog();

protected:
  virtual void Write(const std::string& msg);
  virtual void FlushOutput();
  friend class CombinedLog;
};

/** @class File
*
*/
class FileLog : public Log {
public:
  FileLog(const std::string& file_path);
  virtual ~FileLog();

protected:
  virtual void Write(const std::string& msg);
  virtual void FlushOutput();
  friend class CombinedLog;

private:
  std::string   m_file_path;
  std::ofstream m_file;
};

/** @class CombinedLog
*
*/
class CombinedLog : public Log {
public:
  CombinedLog(const std::string& file_path);
  virtual ~CombinedLog();

protected:
  virtual void Write(const std::string& msg);
  virtual void FlushOutput();

private:
  std::unique_ptr<ConsoleLog> m_console_log;
  std::unique_ptr<FileLog>    m_file_log;
};

} // namespace vkex

#define VKEX_LOG_RAW(MSG) \
  (*vkex::Log::Get()) << MSG << std::endl

#define VKEX_LOG_INFO(MSG) \
  (*vkex::Log::Get()) << MSG << std::endl

#define VKEX_LOG_WARN(MSG) \
  (*vkex::Log::Get()) << "[WARNING] " << MSG << std::endl

#define VKEX_LOG_DEBUG(MSG) \
  (*vkex::Log::Get()) << "[DEBUG] " << MSG << std::endl

#define VKEX_LOG_ERROR(MSG) \
  (*vkex::Log::Get()) << "[ERROR] " << MSG << std::endl

#define VKEX_LOG_FATAL(MSG) \
  (*vkex::Log::Get()) << "[FATAL ERROR] " << MSG << std::endl

/*
#define VKEX_LOG_INFO(MSG) \
  (*vkex::Log::Get()) << "[VKEX_INFO] " << MSG << std::endl

#define VKEX_LOG_WARN(MSG) \
  (*vkex::Log::Get()) << "[VKEX_WARN] " << MSG << std::endl

#define VKEX_LOG_DEBUG(MSG) \
  (*vkex::Log::Get()) << "[VKEX_DEBUG] " << MSG << std::endl

#define VKEX_LOG_ERROR(MSG) \
  (*vkex::Log::Get()) << "[VKEX_ERROR] " << MSG << std::endl

#define VKEX_LOG_FATAL(MSG) \
  (*vkex::Log::Get()) << "[VKEX_FATAL] " << MSG << std::endl
*/

#endif // __VKEX_LOG_H__
