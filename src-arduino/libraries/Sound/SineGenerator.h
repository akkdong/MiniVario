// SineGenerator.h
//

#ifndef __SINEGENERATOR_H__
#define _SINEGENERATOR_H__


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class SineGenerator
{
public: 
	SineGenerator();
	
	enum CHANNEL {
		CHANNEL_1,
		CHANNEL_2
	};
	
	enum MODE {
		USE_CHANNEL_1,
		USE_CHANNEL_2,
		USE_DIFFERENTIAL
	};
	
	enum SCALE {
		SCALE_FULL,
		SCALE_HALF,
		SCALE_QUATER
	};
	
public:
	void						begin(MODE mode, SCALE scale = SCALE_FULL, uint8_t offset = 0, int freq = 0 /* C4*/);
	void 						end();
	
	void						setScale(SCALE scale);
	void						setOffset(uint8_t offset);
	void 						setFrequency(int freq); // 0 to turn-off(mute)
	
	void						setTone(uint16_t hz, uint8_t volume = 100) { setFrequency(volume > 0 ? hz : 0); }
	
protected:
//	void						enable(CHANNEL channel);	
//	void						setFrequency();

	uint16_t					calcFreqencyStep(int frequency);
	
protected:
	//
	MODE 						mMode;
	
	//
	int							mFrequency;
	uint16_t					mFreqStep;

	static int					mClkDivider;
};


#endif // _SINEGENERATOR_H__
