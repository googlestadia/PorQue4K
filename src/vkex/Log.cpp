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

#include <vkex/Log.h>

namespace vkex {

// =================================================================================================
// Log
// =================================================================================================
static std::unique_ptr<Log> s_logger;

Log::Log()
{
}

Log::~Log()
{
}

void Log::Initialize(const std::string& file_path)
{
  if (s_logger) {
    return;
  }
  s_logger = std::unique_ptr<Log>(new CombinedLog(file_path));

  if (!file_path.empty()) {
    std::stringstream ss;
    ss << "Opened log file: " << file_path << std::endl;
#if defined(VKEX_WIN32)
    OutputDebugStringA(ss.str().c_str());
#elif defined(VKEX_LINUX)
    std::cout << ss.str();
#endif
  }
}

Log* Log::Get()
{
  if (!s_logger) {
    s_logger = std::unique_ptr<Log>(new CombinedLog("vkex.log"));
  }
  return s_logger.get();
}

// =================================================================================================
// ConsoleLog
// =================================================================================================
ConsoleLog::ConsoleLog()
{
}

ConsoleLog::~ConsoleLog()
{
}

#if defined(VKEX_WIN32)
void ConsoleLog::Write(const std::string& msg)
{
  OutputDebugStringA(msg.c_str());
}
#elif defined(VKEX_LINUX)
void ConsoleLog::Write(const std::string& msg)
{
  std::cout << msg;
}
#endif

#if defined(VKEX_WIN32)
void ConsoleLog::FlushOutput()
{
}
#elif defined(VKEX_LINUX)
void ConsoleLog::FlushOutput()
{
  std::cout << std::flush;
}
#endif

// =================================================================================================
// FileLog
// =================================================================================================
FileLog::FileLog(const std::string& file_path)
  : m_file_path(file_path)
{
  m_file = std::ofstream(m_file_path.c_str());
}

FileLog::~FileLog()
{
  if (!m_file.is_open()) {
    return;
  }
  m_file.flush();
  m_file.close();
}

void FileLog::Write(const std::string& msg)
{
  if (!m_file.is_open()) {
    return;
  }
  m_file << msg;
}

void FileLog::FlushOutput()
{
  if (!m_file.is_open()) {
    return;
  }
  m_file.flush();
}

// =================================================================================================
// CombinedLog
// =================================================================================================
CombinedLog::CombinedLog(const std::string& file_path)
{
  m_console_log = std::make_unique<ConsoleLog>();
  m_file_log = std::make_unique<FileLog>(file_path);
}

CombinedLog::~CombinedLog()
{
}

void CombinedLog::Write(const std::string& msg)
{
  m_console_log->Write(msg);
  m_file_log->Write(msg);
}


void CombinedLog::FlushOutput()
{
  m_console_log->FlushOutput();
  m_file_log->FlushOutput();
}

} // namespace vkex
