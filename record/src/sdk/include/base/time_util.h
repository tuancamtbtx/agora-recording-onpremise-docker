#pragma once

#include <cstdint>
#include <signal.h>
#include <time.h>
#include <map>

namespace agora {
namespace base {
uint64_t now_us();
uint64_t now_ms();
uint32_t now_seconds();
uint32_t now_ts();

uint64_t tick_us();
uint64_t tick_ms();
uint32_t tick_seconds();
uint64_t tick_ms_by_avsync_type(int avsync_mode, int avsync_type, uint64_t new_ms,  uint64_t old_ms = 0);
void Time2UTCStr_s(char *buffer, int len);
int64_t tick_ms_to_utc_ms(uint64_t tick_ms);
void Time2UTSStr(char *buffer, int len);
void Time2UTCStr(char *buffer, int len);
void Time2UTCStrWithSlash(char *buffer, int len);
void Time2UTCStrWithSlash_ns(char *buffer, int len);
int64_t utc_now_ms();
void tick_ms_to_utc_str(uint64_t ms, char* buffer, int len);
void utc_ms_to_utc_str(int64_t utc_ms, char* buffer, int len);
void Time2LocalStr_hms(char *buffer, int len);

typedef void* TimerHandle;

struct TimerHandler {
  void(*handler) (void*);
  void* arg;
};

class TimerManager {
  public:
    TimerManager();
    ~TimerManager();

  public:
    static TimerHandle getOneshotTimer(uint32_t ms, void(*handler)(void*), void* arg);
    static TimerHandle getIntervalTimer(uint32_t expire_ms, uint32_t interval, void(*handler)(void*), void* arg);
    static void releaseTimer(TimerHandle handle);

  private:
    static std::map<TimerHandle, TimerHandler*> timers_;
};
}
}
