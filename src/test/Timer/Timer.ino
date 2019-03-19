volatile int interruptCounter;
int totalInterruptCounter;
volatile int on;
 
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
 
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  on = 1 - on;
  portEXIT_CRITICAL_ISR(&timerMux);
  
  digitalWrite(21, on ? HIGH : LOW);
}
 
void setup() {
 
  Serial.begin(115200);
 
  timer = timerBegin(0, 80, true); // ESP32 Counter: 80 MHz, Prescaler: 80 --> 1MHz timer
  timerAttachInterrupt(timer, onTimer, true);
  timerAlarmWrite(timer, 1000000 / 100, true); // 100Hz -> alarm every 10milliseconds
  timerAlarmEnable(timer);
  
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);
}
 
void loop() {
 
  if (interruptCounter > 100) {
 
    portENTER_CRITICAL(&timerMux);
    interruptCounter = 0;
    portEXIT_CRITICAL(&timerMux);
 
    totalInterruptCounter++;
 
    Serial.print("An interrupt as occurred. Total number: ");
    Serial.println(totalInterruptCounter);
  }
}