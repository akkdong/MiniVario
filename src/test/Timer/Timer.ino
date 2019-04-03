// Timer.ino
//

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

class TimerExample
{
public:
	TimerExample();
	
public:
	void								begin();
	
public:
	static void IRAM_ATTR 	TimerProc();
	static void 					TaskProc(void * param);
	
	static TimerExample *	mActive;
	
public:
	hw_timer_t *					mTimer;
	SemaphoreHandle_t		mSemaphore;
	portMUX_TYPE				mMux;
};

TimerExample * TimerExample::mActive = NULL;

TimerExample::TimerExample()
	: mTimer(NULL)
	, mSemaphore(xSemaphoreCreateBinary())
	, mMux(portMUX_INITIALIZER_UNLOCKED)
{
}

void TimerExample::begin()
{
	//
	mActive = this;
	
	//
	digitalWrite(19, HIGH);
	digitalWrite(23, LOW);
	pinMode(19, OUTPUT);
	pinMode(23, OUTPUT);
	
	//
	xTaskCreatePinnedToCore(
		TaskProc,  
		"TaskBaro",     // A name just for humans
		1024,   // This stack size can be checked & adjusted by reading the Stack Highwater
		NULL,  
		2,    // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		NULL ,  
		ARDUINO_RUNNING_CORE);
	
	//
	mTimer = timerBegin(0, 80, true); // ESP32 Counter: 80 MHz, Prescaler: 80 --> 1MHz timer
	timerAttachInterrupt(mTimer, TimerProc, true);
	timerAlarmWrite(mTimer, 1000000 / 100, true); // 118Hz -> alarm every 8.4746 milliseconds  :  the measure period need to be greater than 8.22 ms
	timerAlarmEnable(mTimer);
}

void IRAM_ATTR TimerExample::TimerProc() 
{
	if (TimerExample::mActive != NULL)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(TimerExample::mActive->mSemaphore, &xHigherPriorityTaskWoken);
		if( xHigherPriorityTaskWoken == pdTRUE) 
			portYIELD_FROM_ISR(); // this wakes up task immediately instead of on next FreeRTOS tick
	}
}

void TimerExample::TaskProc(void * param) 
{
	if (! TimerExample::mActive)
	{
		vTaskDelete(NULL);
		return;
	}
	
	int toggle = 1;
	
	digitalWrite(19, toggle);
	digitalWrite(23, ! toggle);
	
	while (1)
	{
		//
		xSemaphoreTake(TimerExample::mActive->mSemaphore, portMAX_DELAY);
		
		//
		toggle = 1 -  toggle;
		
		digitalWrite(19, toggle);
		digitalWrite(23, ! toggle);		
	}
}

TimerExample _TimerExample;

void setup()
{
	_TimerExample.begin();
}

void loop()
{
	// nop
}


#if 0 // OLD_EXAMPLE
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
#endif