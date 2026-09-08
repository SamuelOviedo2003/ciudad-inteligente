// Iluminacion en lazo abierto
#define PIN_ESTIMULO 2
#define PIN_RESPUESTA A0

bool tg = false;
int estimulo = 0;
int respuesta  = 0;

unsigned long cont  = 0;
unsigned long tiempo = 0;

void setup_timer2() { //set timer2 interrupt at 1kHz
  cli();

  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 1khz increments
  //OCR2A = 249;// = (16e6) / (1000*64) - 1 (must be <256)
  OCR2A = 249;// = (16e6) / (250*256) - 1 (must be <256)
  //OCR2A = 155;// = (16e6) / (100*1024) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Prescalers https://microcontrollerslab.com/arduino-timer-interrupts-tutorial/
  // CS = 001 for 1
  // CS = 010 for 8
  // CS = 011 for 32
  // CS = 100 for 64
  // CS = 101 for 128
  // CS = 110 for 256
  // CS = 111 for 1024
  TCCR2B |= (1 << CS22);
  TCCR2B |= (1 << CS21);
  // TCCR2B |= (1 << CS20);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);

  sei();//allow interrupts
}

ISR(TIMER2_COMPA_vect) {
  timed_loop();
}

void setup() {
  setup_timer2();
  pinMode(PIN_ESTIMULO, OUTPUT);
  Serial.begin(115200);
}

void timed_loop() {
  tiempo = micros();
  cont++ ;
  if (cont >= 250) {
    tg = !tg;
    if (tg) { estimulo = 255; }
    else    { estimulo = 0;   }
    cont = 0;
  }
  analogWrite(PIN_ESTIMULO, estimulo);
  Serial.print(tiempo);
  Serial.print(",");
  Serial.print(estimulo);
  Serial.print(",");
  Serial.println(respuesta);
}

void loop() {
  respuesta = analogRead(PIN_RESPUESTA);
}
