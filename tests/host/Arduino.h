#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <array>
using std::size_t;
struct __FlashStringHelper {};
#define F(x) reinterpret_cast<const __FlashStringHelper *>(x)
constexpr int LOW=0,HIGH=1,INPUT_PULLUP=2,OUTPUT=1;
inline uint32_t nowMs=0;
inline std::array<int,70> levels{}, modes{}, outputs{};
inline char physicalKey=0;
inline uint32_t millis() { return nowMs; }
inline void pinMode(int p,int m) { modes[p]=m; }
inline void digitalWrite(int p,int v) { outputs[p]=v; }
inline int digitalRead(int p) {
  const char keys[4][4]={{'1','2','3','A'},{'4','5','6','B'},{'7','8','9','C'},{'*','0','#','D'}};
  if(p>=32 && p<=35) for(int r=0;r<4;++r)
    if(modes[28+r]==OUTPUT && outputs[28+r]==LOW && physicalKey==keys[r][p-32]) return LOW;
  return levels[p];
}
inline void tone(int,int) {}
inline void noTone(int) {}
struct SerialMock {
  std::string log;
  void begin(int) {}
  void print(const __FlashStringHelper *s) { log+=reinterpret_cast<const char*>(s); }
  void println(const __FlashStringHelper *s) { print(s);log+='\n'; }
};
inline SerialMock Serial1;
