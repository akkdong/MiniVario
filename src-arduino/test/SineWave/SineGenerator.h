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
	void						begin(MODE mode, SCALE scale = SCALE_FULL, uint8_t offset = 0, int freq = 262 /* C4*/);
	void 						end();
	
	void						setScale(SCALE scale);
	void						setOffset(uint8_t offset);
	void 						setFrequency(int freq); // 0 to turn-off(mute)
	
protected:
//	void						enable(CHANNEL channel);	
//	void						setFrequency();

	uint16_t					calcFreqencyStep(int frequency);
	
protected:
	//
	MODE 					mMode;
	
//	int						mFrequency;
//	SCALE					mScale;
//	uint8_t					mOffset;
	
	//
	uint16_t					mFreqStep;
};


#endif // _SINEGENERATOR_H__
