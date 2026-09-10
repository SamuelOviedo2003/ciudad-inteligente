// Mock del LCD: guarda lo escrito en un framebuffer de filas x 40 columnas y
// cuenta cuantas veces un texto se pasa del ancho real (20) para detectar
// desbordes de linea.
#pragma once
#include "Arduino.h"

class LiquidCrystal_I2C : public Print {
 public:
  int cols, rows, col = 0, row = 0;
  char fb[4][41];
  int overflows = 0;
  std::vector<std::string> overflow_samples;
  int clears = 0;

  LiquidCrystal_I2C(uint8_t, int c, int r) : cols(c), rows(r) { clear(); clears = 0; }
  void init() {}
  void backlight() {}
  void clear() {
    for (int r = 0; r < 4; r++) { memset(fb[r], ' ', 40); fb[r][40] = 0; }
    col = row = 0;
    clears++;
  }
  void setCursor(int c, int r) { col = c; row = r; }
  size_t write(uint8_t ch) override {
    if (row < 4 && col < 40) fb[row][col] = (char)ch;
    col++;
    if (col == cols + 1) {  // primer caracter fuera del ancho visible
      overflows++;
      if (overflow_samples.size() < 20) overflow_samples.push_back(std::string("fila ") + std::to_string(row) + ": '" + std::string(fb[row], 40) + "'");
    }
    return 1;
  }
  using Print::write;
  std::string line(int r) const { std::string s(fb[r], cols); while (!s.empty() && s.back() == ' ') s.pop_back(); return s; }
};
