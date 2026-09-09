// Mock minimo de la API de Arduino para compilar los .ino del proyecto en el
// PC y simularlos con tiempo virtual. Solo cubre lo que usan nivel_bajo.ino y
// nivel_medio.ino.
#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <deque>
#include <vector>
#include <functional>

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define DEC 10
typedef uint8_t byte;
typedef bool boolean;

namespace sim {
extern unsigned long now_ms;
extern int pin_mode[64];
extern int pin_level[64];
extern int analog_value[64];
extern std::deque<char> serial_in;
extern std::string serial_out;
extern std::function<void(int, int)> on_digital_write;
}  // namespace sim

unsigned long millis();
unsigned long micros();
void delay(unsigned long ms);
void pinMode(int pin, int mode);
void digitalWrite(int pin, int val);
int digitalRead(int pin);
int analogRead(int pin);

class String {
  std::string s;

 public:
  String() {}
  String(const char* c) : s(c ? c : "") {}
  String(const std::string& c) : s(c) {}
  String(char c) : s(1, c) {}
  String(int v) : s(std::to_string(v)) {}
  String(unsigned int v) : s(std::to_string(v)) {}
  String(long v) : s(std::to_string(v)) {}
  String(unsigned long v) : s(std::to_string(v)) {}
  String(double v, unsigned char d = 2) { char b[48]; snprintf(b, sizeof b, "%.*f", d, v); s = b; }
  const char* c_str() const { return s.c_str(); }
  unsigned int length() const { return (unsigned int)s.size(); }
  String& operator+=(char c) { s += c; return *this; }
  String& operator+=(const char* c) { s += c; return *this; }
  String& operator+=(const String& o) { s += o.s; return *this; }
  String& operator+=(int v) { s += std::to_string(v); return *this; }
  String& operator+=(unsigned int v) { s += std::to_string(v); return *this; }
  String& operator+=(long v) { s += std::to_string(v); return *this; }
  String& operator+=(unsigned long v) { s += std::to_string(v); return *this; }
  String& operator+=(double v) { char b[48]; snprintf(b, sizeof b, "%.2f", v); s += b; return *this; }
  template <typename T> friend String operator+(String a, T b) { a += b; return a; }
  String& operator=(const char* c) { s = c; return *this; }
  bool operator==(const char* c) const { return s == c; }
  bool operator==(const String& o) const { return s == o.s; }
  bool operator!=(const char* c) const { return s != c; }
  bool startsWith(const char* p) const { return s.rfind(p, 0) == 0; }
  int indexOf(const char* p) const { auto i = s.find(p); return i == std::string::npos ? -1 : (int)i; }
  String substring(unsigned int from) const { return from >= s.size() ? String() : String(s.substr(from)); }
  long toInt() const { return atol(s.c_str()); }
  void trim() {
    size_t a = s.find_first_not_of(" \t\r\n");
    size_t b = s.find_last_not_of(" \t\r\n");
    s = (a == std::string::npos) ? "" : s.substr(a, b - a + 1);
  }
  void remove(unsigned int idx) { if (idx < s.size()) s.erase(idx); }
  char operator[](unsigned int i) const { return s[i]; }
};

class Print {
 public:
  virtual size_t write(uint8_t c) = 0;
  size_t write(const char* str) { size_t n = 0; while (*str) n += write((uint8_t)*str++); return n; }
  size_t print(const char* str) { return write(str); }
  size_t print(char c) { return write((uint8_t)c); }
  size_t print(unsigned char v, int = DEC) { return print((unsigned long)v); }
  size_t print(int v, int = DEC) { return print((long)v); }
  size_t print(unsigned int v, int = DEC) { return print((unsigned long)v); }
  size_t print(long v, int = DEC) { char b[32]; snprintf(b, sizeof b, "%ld", v); return write(b); }
  size_t print(unsigned long v, int = DEC) { char b[32]; snprintf(b, sizeof b, "%lu", v); return write(b); }
  size_t print(double v, int digits = 2) { char b[48]; snprintf(b, sizeof b, "%.*f", digits, v); return write(b); }
  size_t print(const String& s) { return write(s.c_str()); }
  template <typename T> size_t println(T v) { size_t n = print(v); return n + write("\r\n"); }
  template <typename T> size_t println(T v, int d) { size_t n = print(v, d); return n + write("\r\n"); }
  size_t println() { return write("\r\n"); }
};

class HWSerialMock : public Print {
 public:
  void begin(unsigned long) {}
  void setTxTimeoutMs(uint32_t) {}  // existe solo en HWCDC (CDCOnBoot=cdc); aqui es un no-op
  int available() { return (int)sim::serial_in.size(); }
  int read() { if (sim::serial_in.empty()) return -1; char c = sim::serial_in.front(); sim::serial_in.pop_front(); return (unsigned char)c; }
  size_t write(uint8_t c) override { sim::serial_out += (char)c; return 1; }
  using Print::write;
};
extern HWSerialMock Serial;
