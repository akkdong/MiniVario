/* DAC Cosine Generator Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <Arduino.h>
#include "SineGenerator.h"


#include "soc/rtc_io_reg.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"
#include "soc/rtc.h"

#include "driver/dac.h"


/* Declare global sine waveform parameters
 * so they may be then accessed and changed from debugger
 * over an JTAG interface
 */
/*
int clk_8m_div = 0;      // RTC 8M clock divider (division is by clk_8m_div+1, i.e. 0 means 8MHz frequency)
int frequency_step = 8;  // Frequency step for CW generator
int scale = 1;           // 50% of the full scale
int offset;              // leave it default / 0 = no any offset
int invert = 2;          // invert MSB to get sine waveform
*/

/*
 * Enable cosine waveform generator on a DAC channel
 */
void dac_cosine_enable(dac_channel_t channel)
{
    // Enable tone generator common to both channels
    SET_PERI_REG_MASK(SENS_SAR_DAC_CTRL1_REG, SENS_SW_TONE_EN);
    switch(channel) {
        case DAC_CHANNEL_1:
            // Enable / connect tone tone generator on / to this channel
            SET_PERI_REG_MASK(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_CW_EN1_M);
            // Invert MSB, otherwise part of waveform will have inverted
            SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_INV1, 2, SENS_DAC_INV1_S);
            break;
        case DAC_CHANNEL_2:
            SET_PERI_REG_MASK(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_CW_EN2_M);
            SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_INV2, 2, SENS_DAC_INV2_S);
            break;
        default :
			//printf("Channel %d\n", channel);
			break;
    }
}

/*
 *
 */
void dac_cosine_disable(dac_channel_t channel)
{
    switch(channel) {
        case DAC_CHANNEL_1:
            // Disable / disconnect tone tone generator on / to this channel
            CLEAR_PERI_REG_MASK(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_CW_EN1_M);
            break;
        case DAC_CHANNEL_2:
            CLEAR_PERI_REG_MASK(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_CW_EN2_M);
            break;
        default :
			 break;
    }
	
    // Disable tone generator common to both channels
    CLEAR_PERI_REG_MASK(SENS_SAR_DAC_CTRL1_REG, SENS_SW_TONE_EN);
}


/*
 * Set frequency of internal CW generator common to both DAC channels
 *
 * clk_8m_div: 0b000 - 0b111
 * frequency_step: range 0x0001 - 0xFFFF
 *
 */
void dac_frequency_set(int clk_8m_div, int frequency_step)
{
    REG_SET_FIELD(RTC_CNTL_CLK_CONF_REG, RTC_CNTL_CK8M_DIV_SEL, clk_8m_div);
    SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL1_REG, SENS_SW_FSTEP, frequency_step, SENS_SW_FSTEP_S);
}


/*
 * Scale output of a DAC channel using two bit pattern:
 *
 * - 00: no scale
 * - 01: scale to 1/2
 * - 10: scale to 1/4
 * - 11: scale to 1/8
 *
 */
void dac_scale_set(dac_channel_t channel, int scale)
{
    switch(channel) {
        case DAC_CHANNEL_1:
            SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_SCALE1, scale, SENS_DAC_SCALE1_S);
            break;
        case DAC_CHANNEL_2:
            SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_SCALE2, scale, SENS_DAC_SCALE2_S);
            break;
        default :
           printf("Channel %d\n", channel);
    }
}


/*
 * Offset output of a DAC channel
 *
 * Range 0x00 - 0xFF
 *
 */
void dac_offset_set(dac_channel_t channel, int offset)
{
    switch(channel) {
        case DAC_CHANNEL_1:
            SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_DC1, offset, SENS_DAC_DC1_S);
            break;
        case DAC_CHANNEL_2:
            SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_DC2, offset, SENS_DAC_DC2_S);
            break;
        default :
           //printf("Channel %d\n", channel);
					 break;
    }
}


/*
 * Invert output pattern of a DAC channel
 *
 * - 00: does not invert any bits,
 * - 01: inverts all bits,
 * - 10: inverts MSB,
 * - 11: inverts all bits except for MSB
 *
 */
void dac_invert_set(dac_channel_t channel, int invert)
{
    switch(channel) {
        case DAC_CHANNEL_1:
            SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_INV1, invert, SENS_DAC_INV1_S);
            break;
        case DAC_CHANNEL_2:
            SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_INV2, invert, SENS_DAC_INV2_S);
            break;
        default :
           //printf("Channel %d\n", channel);
					 break;
    }
}

#if 0
/*
 * Generate a sine waveform on both DAC channels:
 *
 * DAC_CHANNEL_1 - GPIO25
 * DAC_CHANNEL_2 - GPIO26
 *
 * Connect scope to both GPIO25 and GPIO26
 * to observe the waveform changes
 * in response to the parameter change
*/
void app_main()
{
    dac_cosine_enable(DAC_CHANNEL_1);
    dac_cosine_enable(DAC_CHANNEL_2);

    dac_output_enable(DAC_CHANNEL_1);
    dac_output_enable(DAC_CHANNEL_2);
		

	// frequency setting is common to both channels
	dac_frequency_set(clk_8m_div, frequency_step);

	/* Tune parameters of channel 2 only
	* to see and compare changes against channel 1
	*/
	dac_scale_set(DAC_CHANNEL_2, scale);
	dac_offset_set(DAC_CHANNEL_2, offset);
	dac_invert_set(DAC_CHANNEL_2, invert);

	float frequency = RTC_FAST_CLK_FREQ_APPROX / (1 + clk_8m_div) * (float) frequency_step / 65536;
	//printf("clk_8m_div: %d, frequency step: %d, frequency: %.0f Hz\n", clk_8m_div, frequency_step, frequency);
	//printf("DAC2 scale: %d, offset %d, invert: %d\n", scale, offset, invert);
}
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

SineGenerator::SineGenerator()
{
	mMode = USE_DIFFERENTIAL;

	mFrequency = 0;
	mFreqStep	= 0;
}

int SineGenerator::mClkDivider = 0;

void SineGenerator::begin(MODE mode, SCALE scale, uint8_t offset, int freq)
{
	//
	mMode = mode;
	
	//
	switch (mMode)
	{
	case USE_CHANNEL_1 :
		dac_cosine_enable(DAC_CHANNEL_1);
		break;
	case USE_CHANNEL_2 :
		dac_cosine_enable(DAC_CHANNEL_2);
		break;
	case USE_DIFFERENTIAL :
		dac_cosine_enable(DAC_CHANNEL_1);	
		dac_cosine_enable(DAC_CHANNEL_2);	
		dac_invert_set(DAC_CHANNEL_2, 3);
		break;
	}
	
	//
	setScale(scale);
	setOffset(offset);
	setFrequency(freq);

	//
	switch (mMode)
	{
	case USE_CHANNEL_1 :
		dac_output_enable(DAC_CHANNEL_1);
		break;
	case USE_CHANNEL_2 :
		dac_output_enable(DAC_CHANNEL_2);
		break;
	case USE_DIFFERENTIAL :
		dac_output_enable(DAC_CHANNEL_1);
		dac_output_enable(DAC_CHANNEL_2);
		break;
	}	
}

void SineGenerator::end()
{
	setFrequency(0);
	
	switch (mMode)
	{
	case USE_CHANNEL_1 :
		dac_output_disable(DAC_CHANNEL_1);
		dac_cosine_disable(DAC_CHANNEL_1);
		break;
	case USE_CHANNEL_2 :
		dac_output_disable(DAC_CHANNEL_2);
		dac_cosine_disable(DAC_CHANNEL_2);
		break;
	case USE_DIFFERENTIAL :
		dac_output_disable(DAC_CHANNEL_1);
		dac_output_disable(DAC_CHANNEL_2);
		
		dac_cosine_disable(DAC_CHANNEL_1);
		dac_cosine_disable(DAC_CHANNEL_2);
		break;
	}
}

void SineGenerator::setScale(SCALE scale)
{
	int _scale = 0;

	switch (scale)
	{
	case SCALE_FULL 	: _scale = 0; break;
	case SCALE_HALF 	: _scale = 1; break;
	case SCALE_QUATER 	: _scale = 2; break;
	default 			: return;
	}
	
	switch (mMode)
	{
	case USE_CHANNEL_1 :
		dac_scale_set(DAC_CHANNEL_1, _scale);
		break;
	case USE_CHANNEL_2 :
		dac_scale_set(DAC_CHANNEL_2, _scale);
		break;
	case USE_DIFFERENTIAL :
		dac_scale_set(DAC_CHANNEL_1, _scale);
		dac_scale_set(DAC_CHANNEL_2, _scale);
		break;
	}
}

void SineGenerator::setOffset(uint8_t offset)
{
	switch (mMode)
	{
	case USE_CHANNEL_1 :
		dac_offset_set(DAC_CHANNEL_1, offset);
		break;
	case USE_CHANNEL_2 :
		dac_offset_set(DAC_CHANNEL_2, offset);
		break;
	case USE_DIFFERENTIAL :
		dac_offset_set(DAC_CHANNEL_1, offset);
		dac_offset_set(DAC_CHANNEL_2, offset);
		break;
	}
}

void SineGenerator::setFrequency(int freq)
{
	if (mFrequency == freq)
		return;

	mFrequency = freq;
	mFreqStep = calcFreqencyStep(freq);
	
	dac_frequency_set(mClkDivider, mFreqStep);
}

uint16_t SineGenerator::calcFreqencyStep(int frequency)
{
	return (uint16_t)((float)frequency * (1 + mClkDivider) / RTC_FAST_CLK_FREQ_APPROX * 65536.0);
}
	

	
void __dacWrite(uint8_t pin, uint8_t value)
{
    if(pin < 25 || pin > 26){
        return;//not dac pin
    }
    pinMode(pin, ANALOG);
    uint8_t channel = pin - 25;


    //Disable Tone
    //CLEAR_PERI_REG_MASK(SENS_SAR_DAC_CTRL1_REG, SENS_SW_TONE_EN);

    if (channel) {
        //Disable Channel Tone
        CLEAR_PERI_REG_MASK(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_CW_EN2_M);
        //Set the Dac value
        SET_PERI_REG_BITS(RTC_IO_PAD_DAC2_REG, RTC_IO_PDAC2_DAC, value, RTC_IO_PDAC2_DAC_S);   //dac_output
        //Channel output enable
        SET_PERI_REG_MASK(RTC_IO_PAD_DAC2_REG, RTC_IO_PDAC2_XPD_DAC | RTC_IO_PDAC2_DAC_XPD_FORCE);
    } else {
        //Disable Channel Tone
        CLEAR_PERI_REG_MASK(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_CW_EN1_M);
        //Set the Dac value
        SET_PERI_REG_BITS(RTC_IO_PAD_DAC1_REG, RTC_IO_PDAC1_DAC, value, RTC_IO_PDAC1_DAC_S);   //dac_output
        //Channel output enable
        SET_PERI_REG_MASK(RTC_IO_PAD_DAC1_REG, RTC_IO_PDAC1_XPD_DAC | RTC_IO_PDAC1_DAC_XPD_FORCE);
    }
}
