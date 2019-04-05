// HardwareTimer.cpp
//

#include "HardwareTimer.h"

#include "esp32-hal-timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/xtensa_api.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"
#include "soc/timer_group_struct.h"
#include "soc/dport_reg.h"
#include "esp_attr.h"
#include "esp_intr.h"

typedef struct 
{
    union 
	{
        struct 
		{
            uint32_t reserved0:   10;
            uint32_t alarm_en:     1;             /*When set  alarm is enabled*/
            uint32_t level_int_en: 1;             /*When set  level type interrupt will be generated during alarm*/
            uint32_t edge_int_en:  1;             /*When set  edge type interrupt will be generated during alarm*/
            uint32_t divider:     16;             /*Timer clock (T0/1_clk) pre-scale value.*/
            uint32_t autoreload:   1;             /*When set  timer 0/1 auto-reload at alarming is enabled*/
            uint32_t increase:     1;             /*When set  timer 0/1 time-base counter increment. When cleared timer 0 time-base counter decrement.*/
            uint32_t enable:       1;             /*When set  timer 0/1 time-base counter is enabled*/
        };
        uint32_t val;
    } config;
	
    uint32_t cnt_low;                             /*Register to store timer 0/1 time-base counter current value lower 32 bits.*/
    uint32_t cnt_high;                            /*Register to store timer 0 time-base counter current value higher 32 bits.*/
    uint32_t update;                              /*Write any value will trigger a timer 0 time-base counter value update (timer 0 current value will be stored in registers above)*/
    uint32_t alarm_low;                           /*Timer 0 time-base counter value lower 32 bits that will trigger the alarm*/
    uint32_t alarm_high;                          /*Timer 0 time-base counter value higher 32 bits that will trigger the alarm*/
    uint32_t load_low;                            /*Lower 32 bits of the value that will load into timer 0 time-base counter*/
    uint32_t load_high;                           /*higher 32 bits of the value that will load into timer 0 time-base counter*/
    uint32_t reload;                              /*Write any value will trigger timer 0 time-base counter reload*/
} hw_timer_reg_t;

typedef void (* TimerProcType)(void *);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class HardwareTimer static properties

void * HardwareTimer::mTimerRegisters[TIMER_COUNT] = 
{
	(void *)(DR_REG_TIMERGROUP0_BASE + 0x0000),
	(void *)(DR_REG_TIMERGROUP0_BASE + 0x0024),
	(void *)(DR_REG_TIMERGROUP0_BASE + 0x1000),
	(void *)(DR_REG_TIMERGROUP0_BASE + 0x1024),
};

TimerProcType HardwareTimer::mTimerHandlers[TIMER_COUNT] = { NULL, NULL, NULL, NULL };



///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class HardwareTimer

HardwareTimer::HardwareTimer(TimerType type) : mType(type), mLock(portMUX_INITIALIZER_UNLOCKED)
{
	switch (type)
	{
	case TIMER_1 :
		mGroup = 0;
		mTimer = 0;
		break;
	case TIMER_2 :
		mGroup = 0;
		mTimer = 1;
		break;
	case TIMER_3 :
		mGroup = 1;
		mTimer = 0;
		break;
	case TIMER_4 :
		mGroup = 1;
		mTimer = 1;
		break;
	}
}

void IRAM_ATTR HardwareTimer::_TimerISR(void * args)
{
	HardwareTimer * pThis = (HardwareTimer *)args;
	uint32_t status = 0;
	
	if (pThis->mGroup)
	{
		status = TIMERG1.int_st_timers.val & (1 << pThis->mTimer);
		TIMERG1.int_clr_timers.val = (1 << pThis->mTimer);
	}
	else
	{
		status = TIMERG0.int_st_timers.val & (1 << pThis->mTimer);
		TIMERG0.int_clr_timers.val = (1 << pThis->mTimer);
	}
	
	//restart the timers that should autoreload
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[pThis->mType];
	if(status && dev->config.autoreload)
		dev->config.alarm_en = 1;
	
	//call callbacks
	if(status && HardwareTimer::mTimerHandlers[pThis->mType])
	{
		TimerProcType procPtr = HardwareTimer::mTimerHandlers[pThis->mType];
		
		if (procPtr == (TimerProcType)-1)
			pThis->timerCallback();
		else
			procPtr(pThis);
	}
}

uint64_t HardwareTimer::read()
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    dev->update = 1;
    uint64_t h = dev->cnt_high;
    uint64_t l = dev->cnt_low;
	
    return (h << 32) | l;
}

uint64_t HardwareTimer::alarmRead()
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    uint64_t h = dev->alarm_high;
    uint64_t l = dev->alarm_low;
	
    return (h << 32) | l;
}

void HardwareTimer::write(uint64_t val)
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    dev->load_high = (uint32_t) (val >> 32);
    dev->load_low = (uint32_t) (val);
    dev->reload = 1;
}

void HardwareTimer::alarmWrite(uint64_t alarm_value, bool autoreload)
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    dev->alarm_high = (uint32_t) (alarm_value >> 32);
    dev->alarm_low = (uint32_t) alarm_value;
    dev->config.autoreload = autoreload;
}

void HardwareTimer::setConfig(uint32_t config)
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    dev->config.val = config;
}

uint32_t HardwareTimer::getConfig()
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    return dev->config.val;
}

void HardwareTimer::setCountUp(bool countUp)
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    dev->config.increase = countUp;
}

bool HardwareTimer::getCountUp()
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    return dev->config.increase;
}

void HardwareTimer::setAutoReload(bool autoreload)
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    dev->config.autoreload = autoreload;
}

bool HardwareTimer::getAutoReload()
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    return dev->config.autoreload;
}

void HardwareTimer::setDivider(uint16_t divider) // 2 to 65536
{
    if(!divider)
	{
        divider = 0xFFFF;
    }
	else if(divider == 1)
	{
        divider = 2;
    }
	
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
    int timer_en = dev->config.enable;
    dev->config.enable = 0;
    dev->config.divider = divider;
    dev->config.enable = timer_en;
}

uint16_t HardwareTimer::getDivider()
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    return dev->config.divider;
}

void HardwareTimer::start()
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    dev->config.enable = 1;
}

void HardwareTimer::stop()
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    dev->config.enable = 0;
}

void HardwareTimer::restart()
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    dev->config.enable = 0;
    dev->config.enable = 1;
}

bool HardwareTimer::started()
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    return dev->config.enable;
}

void HardwareTimer::alarmEnable()
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    dev->config.alarm_en = 1;
}

void HardwareTimer::alarmDisable()
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    dev->config.alarm_en = 0;
}

bool HardwareTimer::alarmEnabled()
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    return dev->config.alarm_en;
}

void HardwareTimer::_APB_Changed(void * arg, apb_change_ev_t ev_type, uint32_t old_apb, uint32_t new_apb)
{
	HardwareTimer * pThis = (HardwareTimer *)arg;
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[pThis->mType];
	
    if(ev_type == APB_BEFORE_CHANGE)
	{
        dev->config.enable = 0;
    }
	else
	{
        old_apb /= 1000000;
        new_apb /= 1000000;
		
        dev->config.divider = (new_apb * dev->config.divider) / old_apb;
        dev->config.enable = 1;
    }
}

int HardwareTimer::begin(uint16_t divider, bool countUp)
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
	
    if(mGroup) 
	{
        DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_TIMERGROUP1_CLK_EN);
        DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_TIMERGROUP1_RST);
        TIMERG1.int_ena.val &= ~BIT(mTimer);
    } 
	else 
	{
        DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_TIMERGROUP_CLK_EN);
        DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_TIMERGROUP_RST);
        TIMERG0.int_ena.val &= ~BIT(mTimer);
    }
	
    dev->config.enable = 0;
    setDivider(divider);
    setCountUp(countUp);
    setAutoReload(false);
    attachInterrupt(NULL, false);
    write(0);

    dev->config.enable = 1;
    addApbChangeCallback(this, _APB_Changed);
	
    return mType;
}

void HardwareTimer::end()
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
    dev->config.enable = 0;
	
    attachInterrupt(NULL, false);
    removeApbChangeCallback(this, _APB_Changed);
}

void HardwareTimer::attachInterrupt(TimerProcType procPtr, bool edge)
{
	hw_timer_reg_t * dev = (hw_timer_reg_t *)HardwareTimer::mTimerRegisters[mType];
    static bool initialized = false;
    static intr_handle_t intr_handle = NULL;
	
    if(intr_handle)
        esp_intr_disable(intr_handle);
	
    if(procPtr == NULL)
	{
        dev->config.level_int_en = 0;
        dev->config.edge_int_en = 0;
        dev->config.alarm_en = 0;
		
//		if(mType & 2) // TIMER_3, TIMER_4
		if (mGroup)
            TIMERG1.int_ena.val &= ~BIT(mTimer);
		else 
            TIMERG0.int_ena.val &= ~BIT(mTimer);
		
        mTimerHandlers[mType] = NULL;
    } 
	else 
	{
        mTimerHandlers[mType] = procPtr;
		
        dev->config.level_int_en = edge ? 0 : 1;//When set, an alarm will generate a level type interrupt.
        dev->config.edge_int_en = edge ? 1 : 0;//When set, an alarm will generate an edge type interrupt.
		
        int intr_source = 0;
        if(! edge)
		{
            if(mGroup)
                intr_source = ETS_TG1_T0_LEVEL_INTR_SOURCE + mTimer;
			else 
                intr_source = ETS_TG0_T0_LEVEL_INTR_SOURCE + mTimer;
        } 
		else 
		{
            if(mGroup)
                intr_source = ETS_TG1_T0_EDGE_INTR_SOURCE + mTimer;
			else 
                intr_source = ETS_TG0_T0_EDGE_INTR_SOURCE + mTimer;
        }
		
        if(! initialized)
		{
            initialized = true;
            esp_intr_alloc(intr_source, (int)(ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_EDGE), _TimerISR, this, &intr_handle);
        } 
		else 
		{
            intr_matrix_set(esp_intr_get_cpu(intr_handle), intr_source, esp_intr_get_intno(intr_handle));
        }
		
        if(mGroup)
            TIMERG1.int_ena.val |= BIT(mTimer);
		else 
            TIMERG0.int_ena.val |= BIT(mTimer);
    }
	
    if(intr_handle)
        esp_intr_enable(intr_handle);
}

void HardwareTimer::attachInterrupt(bool edge)
{
	attachInterrupt((TimerProcType)-1, edge);
}

void HardwareTimer::detachInterrupt()
{
    attachInterrupt(NULL, false);
}

uint64_t HardwareTimer::readMicros()
{
    uint64_t timer_val = read();
    uint16_t div = getDivider();
	
    return timer_val * div / (getApbFrequency() / 1000000);
}

double HardwareTimer::readSeconds()
{
    uint64_t timer_val = read();
    uint16_t div = getDivider();
	
    return (double)timer_val * div / getApbFrequency();
}

uint64_t HardwareTimer::alarmReadMicros()
{
    uint64_t timer_val = alarmRead();
    uint16_t div = getDivider();
	
    return timer_val * div / (getApbFrequency() / 1000000);
}

double HardwareTimer::alarmReadSeconds()
{
    uint64_t timer_val = alarmRead();
    uint16_t div = getDivider();
	
    return (double)timer_val * div / getApbFrequency();
}
