#include "Arduino.h"

namespace sim {
unsigned long now_ms = 0;
int pin_mode[64] = {0};
int pin_level[64] = {0};
int analog_value[64] = {0};
std::deque<char> serial_in;
std::string serial_out;
std::function<void(int, int)> on_digital_write;
}  // namespace sim

HWSerialMock Serial;

unsigned long millis() { return sim::now_ms; }
unsigned long micros() { return sim::now_ms * 1000; }
void delay(unsigned long ms) { sim::now_ms += ms; }
void pinMode(int pin, int mode) {
  sim::pin_mode[pin] = mode;
  if (mode == INPUT_PULLUP) sim::pin_level[pin] = HIGH;
}
void digitalWrite(int pin, int val) {
  sim::pin_level[pin] = val ? HIGH : LOW;
  if (sim::on_digital_write) sim::on_digital_write(pin, val);
}
int digitalRead(int pin) { return sim::pin_level[pin]; }
int analogRead(int pin) { return sim::analog_value[pin]; }
