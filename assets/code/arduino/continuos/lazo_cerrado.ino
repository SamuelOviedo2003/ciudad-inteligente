// Iluminacion en lazo cerrado
#define PIN_ESTIMULO 2
#define PIN_RESPUESTA A0

int estimulo = 0;
int respuesta  = 0;

float ref = 500;
float error = 0;
float errorSum = 0;
float KP = 2.5;
float KI = 10;
float dt = 0.004;

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
  // Prescalers
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
  // put your setup code here, to run once:

  setup_timer2();
  pinMode(PIN_ESTIMULO, OUTPUT);

  Serial.begin(115200);

}

void timed_loop() {

  error = ref - in;
  errorSum = errorSum + error * dt;
  errorSum = constrain(errorSum, -255 / KI, 255 / KI);

  estimulo = int(KP * error + KI * errorSum);
  estimulo = constrain(estimulo, 0, 255);

  analogWrite(PIN_ESTIMULO, estimulo);
  Serial.print(ref);
  Serial.print(",");
  Serial.print(estimulo);
  Serial.print(",");
  Serial.print(respuesta);
  Serial.println();
}

void loop() {
  respuesta = analogRead(PIN_RESPUESTA);
}
