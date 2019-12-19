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

#ifndef __VKEX_TIMER_H__
#define __VKEX_TIMER_H__

//
// -----------------------------------------------------------------------------
// Usage
// -----------------------------------------------------------------------------
// In one C/C++ file that #includes this file, do this:
//   #define VKEX_TIMER_IMPLEMENTATION
//   #include "Timer.h"
//
// C++ class requires implementation in C++ file.
//
// Use VKEX_TIMER_DISABLE_NULL_POINTER_CHECK to disable null pointer checks
// for function parameters.
// 
// -----------------------------------------------------------------------------
// Linux 
// -----------------------------------------------------------------------------
// clock_gettime and clock_getres defaults to CLOCK_MONOTONIC_RAW for clk_id
// parameter. Use VKEX_FORCE_MONOTONIC to force CLOCK_MONOTONIC.
//

#if !(defined(VKEX_LINUX) || defined(VKEX_WIN32))
#  if defined(__linux__)
#    define VKEX_LINUX
#  elif defined(WIN32)
#    define VKEX_WIN32
#  else
#    error "Uh, oh! Looks like this is an unsupported platform!"
#  endif
#endif

#include <assert.h>
#include <stdint.h>

#if defined(VKEX_LINUX)
#  include <time.h>
#  include <unistd.h>
#endif

#if defined(VKEX_WIN32)
#  define VC_EXTRALEAN
#  define WIN32_LEAN_AND_MEAN
#  include <Windows.h>
#endif

#define VKEX_TIMER_SECONDS_TO_NANOS  1000000000
#define VKEX_TIMER_MILLIS_TO_NANOS   1000000
#define VKEX_TIMER_MICROS_TO_NANOS   1000
#define VKEX_TIMER_NANOS_TO_SECONDS  0.000000001
#define VKEX_TIMER_NANOS_TO_MILLIS   0.000001
#define VKEX_TIMER_NANOS_TO_MICROS   0.001

// Select which clock to use
#if defined(VKEX_TIMER_FORCE_MONOTONIC)
#  define VKEX_TIMER_CLK_ID CLOCK_MONOTONIC
#else
#  define VKEX_TIMER_CLK_ID CLOCK_MONOTONIC_RAW
#endif

enum {
  VKEX_TIMER_SUCCESS                 =  0,
  VKEX_TIMER_ERROR_NULL_POINTER      = -1,
  VKEX_TIMER_ERROR_INITIALIZE_FAILED = -2,
  VKEX_TIMER_ERROR_TIMESTAMP_FAILED  = -3,
  VKEX_TIMER_ERROR_SLEEP_FAILED      = -4,
  VKEX_TIMER_ERROR_CORRUPTED_DATA    = -5,
};

typedef struct vkex_timer {
  bool      initialized;
  uint64_t  start_timestamp;
  uint64_t  stop_timestamp;
} vkex_timer;

int vkex_timer_timestamp(uint64_t* p_timestamp);
int vkex_timer_timestamp_to_seconds(uint64_t timestamp, double* p_seconds);
int vkex_timer_timestamp_to_millis(uint64_t timestamp, double* p_millis);
int vkex_timer_timestamp_to_micros(uint64_t timestamp, double* p_micros);
int vkex_timer_timestamp_to_nanos(uint64_t timestamp, double* p_nanos);

int vkex_timer_start(struct vkex_timer* p_timer);
int vkex_timer_stop(struct vkex_timer* p_timer);
int vkex_timer_start_timestamp(const struct vkex_timer* p_timer, uint64_t* p_timestamp);
int vkex_timer_stop_timestamp(const struct vkex_timer* p_timer, uint64_t* p_timestamp);
int vkex_timer_seconds(const struct vkex_timer* p_timer, double* p_seconds);
int vkex_timer_millis(const struct vkex_timer* p_timer, double* p_millis);
int vkex_timer_micros(const struct vkex_timer* p_timer, double* p_micros);
int vkex_timer_nanos(const struct vkex_timer* p_timer, double* p_nanos);

int vkex_timer_sleep_seconds(double seconds);
int vkex_timer_sleep_millis(double millis);
int vkex_timer_sleep_micros(double micros);
int vkex_timer_sleep_nanos(double nanos);

// =============================================================================
// C++ 
// =============================================================================
#if defined(__cplusplus)
namespace vkex {

class Timer {
public:
  Timer() {}
  ~Timer() {}

  static int      Timestamp(uint64_t* p_timestamp);
  static uint64_t Timestamp();
  static int      TimestampToSeconds(uint64_t timestamp, double* p_seconds);
  static double   TimestampToSeconds(uint64_t timestamp);
  static int      TimestampToMillis(uint64_t timestamp, double* p_millis);
  static double   TimestampToMillis(uint64_t timestamp);
  static int      TimestampToMicros(uint64_t timestamp, double* p_micros);
  static double   TimestampToMicros(uint64_t timestamp);
  static int      TimestampToNanos(uint64_t timestamp, double* p_nanos);
  static double   TimestampToNanos(uint64_t timestamp);

  int Start() {
    int result = vkex_timer_start(&m_timer);
    return result;
  }

  int Stop() {
    int result = vkex_timer_stop(&m_timer);
    return result;
  }

  int StartTimestamp(uint64_t* p_timestamp) const {
    int result = vkex_timer_start_timestamp(&m_timer, p_timestamp);
    return result;
  }

  uint64_t StartTimestamp() const {
    uint64_t timestamp = 0;
    int result = vkex_timer_start_timestamp(&m_timer, &timestamp);
    return timestamp;
  }

  int StopTimestamp(uint64_t* p_timestamp) const {
    int result = vkex_timer_stop_timestamp(&m_timer, p_timestamp);
    return result;
  }

  uint64_t StopTimestamp() const {
    uint64_t timestamp = 0;
    int result = vkex_timer_stop_timestamp(&m_timer, &timestamp);
    return timestamp;
  }

  int Seconds(double* p_seconds) const {
    int result = vkex_timer_seconds(&m_timer, p_seconds);
    return result;
  }

  double Seconds() const {
    double seconds = 0;
    int result = vkex_timer_seconds(&m_timer, &seconds);
    assert(result == VKEX_TIMER_SUCCESS);
    return seconds;
  }

  int Millis(double* p_millis) const {
    int result = vkex_timer_millis(&m_timer, p_millis);
    return result;
  }

  double Millis() const {
    double millis = 0;
    int result = vkex_timer_millis(&m_timer, &millis);
    assert(result == VKEX_TIMER_SUCCESS);
    return millis;
  }

  int Micros(double* p_micros) const {
    int result = vkex_timer_micros(&m_timer, p_micros);
    return result;
  }

  double Micros() const {
    double micros = 0;
    int result = vkex_timer_micros(&m_timer, &micros);
    assert(result == VKEX_TIMER_SUCCESS);
    return micros;
  }

  int Nanos(double* p_nanos) const {
    int result = vkex_timer_nanos(&m_timer, p_nanos);
    return result;
  }

  double Nanos() const {
    double nanos = 0;
    int result = vkex_timer_nanos(&m_timer, &nanos);
    assert(result == VKEX_TIMER_SUCCESS);
    return nanos;
  }

  static int SleepSeconds(double seconds);
  static int SleepMillis(double millis);
  static int SleepMicros(double micros);
  static int SleepNanos(double nanos);

private:
  vkex_timer m_timer;
};

} // namespace vkex
#endif

// =============================================================================
// Implementation
// =============================================================================
#if defined(VKEX_TIMER_IMPLEMENTATION)
#include <math.h>
#include <string.h>

// =============================================================================
// vkex_timer_timestamp
// =============================================================================
#if defined(VKEX_LINUX)
int vkex_timer_timestamp(uint64_t* p_timestamp)
{
  assert(p_timestamp != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_timestamp == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  // Read 
  struct timespec tp;
  int result = clock_gettime(VKEX_TIMER_CLK_ID, &tp);
  assert(result == 0);
  if (result != 0) {
    return VKEX_TIMER_ERROR_TIMESTAMP_FAILED;
  }

  // Convert seconds to nanoseconds
  uint64_t timestamp = (uint64_t)tp.tv_sec * 
    (uint64_t)VKEX_TIMER_SECONDS_TO_NANOS;
  // Add nanoseconds
  timestamp += (uint64_t)tp.tv_nsec;

  *p_timestamp = timestamp;

  return VKEX_TIMER_SUCCESS;
}
#elif defined(VKEX_WIN32)
static bool   s_initialized_nanos_per_count = false;
static double s_nanos_per_count = 0.0;
int vkex_timer_timestamp(uint64_t* p_timestamp)
{
  assert(p_timestamp != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_timestamp == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  if (!s_initialized_nanos_per_count) {
    LARGE_INTEGER frequency;
    BOOL result = QueryPerformanceFrequency(&frequency);
    assert(result != FALSE);
    if (result == FALSE) {
      return VKEX_TIMER_ERROR_TIMESTAMP_FAILED;
    }
    s_nanos_per_count = 1.0e9 / (double)frequency.QuadPart;
    s_initialized_nanos_per_count = true;
  }
  
  // Read
  LARGE_INTEGER counter;
  BOOL result = QueryPerformanceCounter(&counter);
  assert(result != FALSE);
  if (result == FALSE) {
    return VKEX_TIMER_ERROR_TIMESTAMP_FAILED;
  }

  //
  // QPC: https://msdn.microsoft.com/en-us/library/ms644904(v=VS.85).aspx
  // 
  // According to the QPC link above, QueryPerformanceCounter returns a
  // timestamp that's < 1us. 
  //
  double timestamp = (double)counter.QuadPart * s_nanos_per_count;

  *p_timestamp = static_cast<uint64_t>(timestamp);

  return VKEX_TIMER_SUCCESS;
}
#endif

// =============================================================================
// vkex_timer_timestamp_to_seconds
// =============================================================================
int vkex_timer_timestamp_to_seconds(uint64_t timestamp, double* p_seconds)
{
  assert(p_seconds != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_seconds == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  double seconds = (double)timestamp * (double)VKEX_TIMER_NANOS_TO_SECONDS;
  *p_seconds = seconds;

  return VKEX_TIMER_SUCCESS;
}

// =============================================================================
// vkex_timer_timestamp_to_millis
// =============================================================================
int vkex_timer_timestamp_to_millis(uint64_t timestamp, double* p_millis)
{
  assert(p_millis != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_millis == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  double millis = (double)timestamp * (double)VKEX_TIMER_NANOS_TO_MILLIS;
  *p_millis = millis;

  return VKEX_TIMER_SUCCESS;
}

// =============================================================================
// vkex_timer_timestamp_to_micros
// =============================================================================
int vkex_timer_timestamp_to_micros(uint64_t timestamp, double* p_micros)
{
  assert(p_micros != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_micros == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  double micros = (double)timestamp * (double)VKEX_TIMER_NANOS_TO_MICROS;
  *p_micros = micros;

  return VKEX_TIMER_SUCCESS;
}

// =============================================================================
// vkex_timer_timestamp_to_nanos
// =============================================================================
int vkex_timer_timestamp_to_nanos(uint64_t timestamp, double* p_nanos)
{
  assert(p_nanos != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_nanos == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  double nanos = (double)timestamp;
  *p_nanos = nanos;

  return VKEX_TIMER_SUCCESS;
}

// =============================================================================
// vkex_timer_start
// =============================================================================
int vkex_timer_start(struct vkex_timer* p_timer)
{
  assert(p_timer != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_timer == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  memset(p_timer, 0, sizeof(*p_timer));
  
  int result = vkex_timer_timestamp(&p_timer->start_timestamp);
  if (result != VKEX_TIMER_SUCCESS) {
    return result;
  }

  return VKEX_TIMER_SUCCESS;
}

// =============================================================================
// vkex_timer_stop
// =============================================================================
int vkex_timer_stop(struct vkex_timer* p_timer)
{
  assert(p_timer != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_timer == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  if (p_timer->stop_timestamp != 0) {
    return VKEX_TIMER_ERROR_CORRUPTED_DATA;
  }

  int result = vkex_timer_timestamp(&p_timer->stop_timestamp);
  if (result != VKEX_TIMER_SUCCESS) {
    return result;
  }

  return VKEX_TIMER_SUCCESS;
}



// =============================================================================
// vkex_timer_start_timestamp
// =============================================================================
int vkex_timer_start_timestamp(const struct vkex_timer* p_timer, uint64_t* p_timestamp)
{
  assert(p_timer != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_timer == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  assert(p_timestamp != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_timestamp == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  *p_timestamp = p_timer->start_timestamp;

  return VKEX_TIMER_SUCCESS;
}

// =============================================================================
// vkex_timer_stop_timestamp
// =============================================================================
int vkex_timer_stop_timestamp(const struct vkex_timer* p_timer, uint64_t* p_timestamp)
{
  assert(p_timer != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_timer == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  assert(p_timestamp != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_timestamp == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  *p_timestamp = p_timer->stop_timestamp;

  return VKEX_TIMER_SUCCESS;
}

// =============================================================================
// vkex_timer_seconds
// =============================================================================
int vkex_timer_seconds(const struct vkex_timer* p_timer, double* p_seconds)
{
  assert(p_timer != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_timer == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  assert(p_seconds != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_seconds == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  uint64_t diff = 0;
  if (p_timer->stop_timestamp == 0) {
    uint64_t timestamp;
    int result = vkex_timer_timestamp(&timestamp);
    if (result != VKEX_TIMER_SUCCESS) {
      return result;
    }
    diff = timestamp - p_timer->start_timestamp; 
  }
  else {
    diff = p_timer->stop_timestamp - p_timer->start_timestamp; 
  }

  int result = vkex_timer_timestamp_to_seconds(diff, p_seconds);
  if (result != VKEX_TIMER_SUCCESS) {
    return result;
  }

  return VKEX_TIMER_SUCCESS;
}

// =============================================================================
// vkex_timer_millis
// =============================================================================
int vkex_timer_millis(const struct vkex_timer* p_timer, double* p_millis)
{
  assert(p_timer != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_timer == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  assert(p_millis != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_millis == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  uint64_t diff = 0;
  if (p_timer->stop_timestamp == 0) {
    uint64_t timestamp;
    int result = vkex_timer_timestamp(&timestamp);
    if (result != VKEX_TIMER_SUCCESS) {
      return result;
    }
    diff = timestamp - p_timer->start_timestamp; 
  }
  else {
    diff = p_timer->stop_timestamp - p_timer->start_timestamp; 
  }

  int result = vkex_timer_timestamp_to_millis(diff, p_millis);
  if (result != VKEX_TIMER_SUCCESS) {
    return result;
  }

  return VKEX_TIMER_SUCCESS;
}

// =============================================================================
// vkex_timer_micros
// =============================================================================
int vkex_timer_micros(const struct vkex_timer* p_timer, double* p_micros)
{
  assert(p_timer != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_timer == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  assert(p_micros != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_micros == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  uint64_t diff = 0;
  if (p_timer->stop_timestamp == 0) {
    uint64_t timestamp;
    int result = vkex_timer_timestamp(&timestamp);
    if (result != VKEX_TIMER_SUCCESS) {
      return result;
    }
    diff = timestamp - p_timer->start_timestamp; 
  }
  else {
    diff = p_timer->stop_timestamp - p_timer->start_timestamp; 
  }

  int result = vkex_timer_timestamp_to_micros(diff, p_micros);
  if (result != VKEX_TIMER_SUCCESS) {
    return result;
  }

  return VKEX_TIMER_SUCCESS;
}

// =============================================================================
// vkex_timer_nanos
// =============================================================================
int vkex_timer_nanos(const struct vkex_timer* p_timer, double* p_nanos)
{
  assert(p_timer != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_timer == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  assert(p_nanos != NULL);
#if !defined(VKEX_TIMER_DISABLE_NULL_POINTER_CHECK)
  if (p_nanos == NULL) {
    return VKEX_TIMER_ERROR_NULL_POINTER;
  }
#endif

  uint64_t diff = 0;
  if (p_timer->stop_timestamp == 0) {
    uint64_t timestamp;
    int result = vkex_timer_timestamp(&timestamp);
    if (result != VKEX_TIMER_SUCCESS) {
      return result;
    }
    diff = timestamp - p_timer->start_timestamp; 
  }
  else {
    diff = p_timer->stop_timestamp - p_timer->start_timestamp; 
  }

  int result = vkex_timer_timestamp_to_nanos(diff, p_nanos);
  if (result != VKEX_TIMER_SUCCESS) {
    return result;
  }

  return VKEX_TIMER_SUCCESS;
}

// =============================================================================
// vkex_timer_sleep_seconds
// =============================================================================
#if defined(VKEX_LINUX)
int vkex_timer_sleep_seconds(double seconds)
{ 
  double nanos = seconds * (double)VKEX_TIMER_SECONDS_TO_NANOS;
  double secs  = floor(nanos * (double)VKEX_TIMER_NANOS_TO_SECONDS);

  struct timespec req;
  req.tv_sec  = (time_t)secs;
  req.tv_nsec = (long)(nanos - (secs * (double)VKEX_TIMER_SECONDS_TO_NANOS)); 
  
  int result = nanosleep(&req, NULL);
  assert(result == 0);
  if (result != 0) {
    return VKEX_TIMER_ERROR_SLEEP_FAILED;
  }

  return VKEX_TIMER_SUCCESS;
}
#elif defined(VKEX_WIN32)
int vkex_timer_sleep_seconds(double seconds)
{
  return VKEX_TIMER_ERROR_SLEEP_FAILED;
}
#endif

// =============================================================================
// vkex_timer_sleep_millis
// =============================================================================
#if defined(VKEX_LINUX)
int vkex_timer_sleep_millis(double millis)
{ 
  double nanos = millis * (double)VKEX_TIMER_MILLIS_TO_NANOS;
  double secs  = floor(nanos * (double)VKEX_TIMER_NANOS_TO_SECONDS);

  struct timespec req;
  req.tv_sec  = (time_t)secs;
  req.tv_nsec = (long)(nanos - (secs * (double)VKEX_TIMER_SECONDS_TO_NANOS)); 
  
  int result = nanosleep(&req, NULL);
  assert(result == 0);
  if (result != 0) {
    return VKEX_TIMER_ERROR_SLEEP_FAILED;
  }

  return VKEX_TIMER_SUCCESS;
}
#elif defined(VKEX_WIN32)
int vkex_timer_sleep_millis(double millis)
{
  return VKEX_TIMER_ERROR_SLEEP_FAILED;
}
#endif

// =============================================================================
// vkex_timer_sleep_micros
// =============================================================================
#if defined(VKEX_LINUX)
int vkex_timer_sleep_micros(double micros)
{ 
  double nanos = micros * (double)VKEX_TIMER_MICROS_TO_NANOS;
  double secs  = floor(nanos * (double)VKEX_TIMER_NANOS_TO_SECONDS);

  struct timespec req;
  req.tv_sec  = (time_t)secs;
  req.tv_nsec = (long)(nanos - (secs * (double)VKEX_TIMER_SECONDS_TO_NANOS)); 
  
  int result = nanosleep(&req, NULL);
  assert(result == 0);
  if (result != 0) {
    return VKEX_TIMER_ERROR_SLEEP_FAILED;
  }

  return VKEX_TIMER_SUCCESS;
}
#elif defined(VKEX_WIN32)
int vkex_timer_sleep_micros(double micros)
{
  return VKEX_TIMER_ERROR_SLEEP_FAILED;
}
#endif

// =============================================================================
// vkex_timer_sleep_nanos
// =============================================================================
#if defined(VKEX_LINUX)
int vkex_timer_sleep_nanos(double nanos)
{ 
  double secs  = floor(nanos * (double)VKEX_TIMER_NANOS_TO_SECONDS);

  struct timespec req;
  req.tv_sec  = (time_t)secs;
  req.tv_nsec = (long)(nanos - (secs * (double)VKEX_TIMER_SECONDS_TO_NANOS)); 
  
  int result = nanosleep(&req, NULL);
  assert(result == 0);
  if (result != 0) {
    return VKEX_TIMER_ERROR_SLEEP_FAILED;
  }

  return VKEX_TIMER_SUCCESS;
}
#elif defined(VKEX_WIN32)
int vkex_timer_sleep_nanos(double nanos)
{
  return VKEX_TIMER_ERROR_SLEEP_FAILED;
}
#endif

// =============================================================================
// C++ 
// =============================================================================
#if defined(__cplusplus)

namespace vkex {

int Timer::Timestamp(uint64_t* p_timestamp)
{
  int result = vkex_timer_timestamp(p_timestamp);
  return result;
}

uint64_t Timer::Timestamp()
{
  uint64_t timestamp = 0;
  int result = vkex_timer_timestamp(&timestamp);
  assert(result == VKEX_TIMER_SUCCESS);
  return timestamp;
}

int Timer::TimestampToSeconds(uint64_t timestamp, double* p_seconds)
{
  int result = vkex_timer_timestamp_to_seconds(timestamp, p_seconds);
  return result;
}

double Timer::TimestampToSeconds(uint64_t timestamp)
{
  double seconds = 0;
  int result = vkex_timer_timestamp_to_seconds(timestamp, &seconds);
  return seconds;
}

int Timer::TimestampToMillis(uint64_t timestamp, double* p_millis)
{
  int result = vkex_timer_timestamp_to_millis(timestamp, p_millis);
  return result;
}

double Timer::TimestampToMillis(uint64_t timestamp)
{
  double millis = 0;
  int result = vkex_timer_timestamp_to_millis(timestamp, &millis);
  return millis;
}

int Timer::TimestampToMicros(uint64_t timestamp, double* p_micros)
{
  int result = vkex_timer_timestamp_to_micros(timestamp, p_micros);
  return result;
}

double Timer::TimestampToMicros(uint64_t timestamp)
{
  double micros = 0;
  int result = vkex_timer_timestamp_to_micros(timestamp, &micros);
  return micros;
}

int Timer::TimestampToNanos(uint64_t timestamp, double* p_nanos)
{
  int result = vkex_timer_timestamp_to_nanos(timestamp, p_nanos);
  return result;
}

double Timer::TimestampToNanos(uint64_t timestamp)
{
  double nanos = 0;
  int result = vkex_timer_timestamp_to_nanos(timestamp, &nanos);
  return nanos;
}

int Timer::SleepSeconds(double seconds)
{
  int result = vkex_timer_sleep_seconds(seconds);
  return result;
}

int Timer::SleepMillis(double millis)
{
  int result = vkex_timer_sleep_millis(millis);
  return result;
}

int Timer::SleepMicros(double micros)
{
  int result = vkex_timer_sleep_micros(micros);
  return result;
}

int Timer::SleepNanos(double nanos)
{
  int result = vkex_timer_sleep_nanos(nanos);
  return result;
}

} // namespace vkex

#endif // defined(__cplusplus)
#endif // defined(VKEX_TIMER_IMPLEMENTATION)

#endif // __VKEX_TIMER_H__
