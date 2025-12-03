// HardwareTimer.h
//

#ifndef __HARDWARETIMER_H__
#define __HARDWARETIMER_H__

#include <Arduino.h>
#include <esp32-hal.h>
#include <freertos/FreeRTOS.h>


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

typedef void (* TimerProcType)(void *);
	
typedef enum 
{
	TIMER_1,
	TIMER_2,
	TIMER_3,
	TIMER_4,
	TIMER_COUNT
} TimerType;

	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class HardwareTimer

class HardwareTimer
{
public:
	HardwareTimer(TimerType type);

public:
	int 							begin(uint16_t divider, bool countUp);
	void 							end();

	void 							setConfig(uint32_t config);
	uint32_t 					getConfig();

	void 							attachInterrupt(TimerProcType procPtr, bool edge);
	void							attachInterrupt(bool edge);
	void 							detachInterrupt();

	void 							start();
	void 							stop();
	void 							restart();
	void 							write(uint64_t val);
	void 							setDivider(uint16_t divider);
	void 							setCountUp(bool countUp);
	void 							setAutoReload(bool autoreload);

	bool 						started();
	uint64_t 					read();
	uint64_t 					readMicros();
	double 						readSeconds();
	uint16_t 					getDivider();
	bool 						getCountUp();
	bool 						getAutoReload();

	void 							alarmEnable();
	void 							alarmDisable();
	void 							alarmWrite(uint64_t interruptAt, bool autoreload);

	bool 						alarmEnabled();
	uint64_t 					alarmRead();
	uint64_t 					alarmReadMicros();
	double 						alarmReadSeconds();
	
public:	
	virtual void				timerCallback() {
		static int i = 0;
		digitalWrite(19, i < 4);
		i += 1;
		if (i > 10)
			i = 0;
	}
	
protected:
	static void IRAM_ATTR	_TimerISR(void * args);
	static void						_APB_Changed(void * arg, apb_change_ev_t ev_type, uint32_t old_apb, uint32_t new_apb);
	
protected:
	 uint8_t						mType;
	 uint8_t						mGroup;	// 0, 1
	 uint8_t						mTimer;	// 0, 1
	 portMUX_TYPE			mLock;
	 
	 static void *				mTimerRegisters[TIMER_COUNT];
	 static TimerProcType	mTimerHandlers[TIMER_COUNT];
};

#endif // __HARDWARETIMER_H__
