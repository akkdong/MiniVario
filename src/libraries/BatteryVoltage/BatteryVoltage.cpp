// BatteryVoltage.cpp
//

#include "BatteryVoltage.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"


#define ADC_MEASURE_INTERVAL		(50)
#define ADC_SAMPLES					(1000/50)
#define DEFAULT_VREF    			(3600)


///////////////////////////////////////////////////////////////////////////
//

static esp_adc_cal_characteristics_t * 	adcChars = NULL;
static const adc_channel_t 				channel = ADC_CHANNEL_0;     // GPIO34 if ADC1, GPIO14 if ADC2
static const adc_atten_t 				atten = ADC_ATTEN_DB_11;
static const adc_unit_t 				unit = ADC_UNIT_1;



///////////////////////////////////////////////////////////////////////////
// class BatteryVoltage



int BatteryVoltage::begin()
{
	//
	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten((adc1_channel_t)channel, atten);
	
	
    adcChars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adcChars);
	
	lastTick = millis();
	measVoltage = 0.0;
	
	measData = 0;
	sampleCount = 0;
	
	for (int i = 0; i < ADC_SAMPLES; i++)
		updateVoltage();
	
	return 0;
}

int BatteryVoltage::update()
{
	if ((millis() - lastTick) > ADC_MEASURE_INTERVAL)
	{
		lastTick = millis();
		
		return updateVoltage();
	}
	
	return 0;
}

int BatteryVoltage::updateVoltage()
{
	measData += adc1_get_raw((adc1_channel_t)channel);
	sampleCount += 1;
	
	if (sampleCount >= ADC_SAMPLES)
	{
		// convert adc_reading to voltage in mV
		measVoltage = esp_adc_cal_raw_to_voltage(measData / sampleCount, adcChars);
		// need to convert from measured voltage to actual voltage, ex: 3.3 -> 5
		// ...
		
		measData = 0.0f;
		sampleCount = 0;
		
		return 1;
	}
	
	return 0;
}
