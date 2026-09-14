#pragma once
#include <array>
#include <stdexcept>
struct EEPROMMock {
  std::array<uint8_t,4096> data;
  int cutAfter=-1,writes=0;
  EEPROMMock() { data.fill(0xFF); }
  uint8_t read(int address) { return data.at(address); }
  void update(int address,uint8_t value) {
    if(data.at(address)==value) return;
    if(cutAfter>=0 && writes==cutAfter) throw std::runtime_error("power cut");
    data.at(address)=value; ++writes;
  }
};
inline EEPROMMock EEPROM;
