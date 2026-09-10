// Mock de Preferences (NVS del ESP32): guarda los pares clave/valor en un
// mapa en memoria (sim::prefs) que los tests pueden inspeccionar o borrar.
#pragma once
#include "Arduino.h"
#include <algorithm>

class Preferences {
 public:
  bool begin(const char*, bool = false) { return true; }
  void end() {}
  bool clear() { sim::prefs.clear(); return true; }
  bool isKey(const char* k) { return sim::prefs.count(k) > 0; }
  size_t getBytesLength(const char* k) { auto it = sim::prefs.find(k); return it == sim::prefs.end() ? 0 : it->second.size(); }
  size_t getBytes(const char* k, void* buf, size_t len) {
    auto it = sim::prefs.find(k);
    if (it == sim::prefs.end()) return 0;
    size_t n = std::min(len, it->second.size());
    memcpy(buf, it->second.data(), n);
    return n;
  }
  size_t putBytes(const char* k, const void* buf, size_t len) {
    sim::prefs[k].assign((const uint8_t*)buf, (const uint8_t*)buf + len);
    return len;
  }
  size_t putUInt(const char* k, uint32_t v) { return putBytes(k, &v, 4); }
  uint32_t getUInt(const char* k, uint32_t def = 0) { uint32_t v = def; getBytes(k, &v, 4); return v; }
};
