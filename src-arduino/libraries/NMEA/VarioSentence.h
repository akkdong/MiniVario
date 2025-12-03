// VarioSentence.h
//

#ifndef __VARIOSENTENCE_H__
#define __VARIOSENTENCE_H__

#include <Arduino.h>

#include "DeviceDefines.h"
#include "Digit.h"


/////////////////////////////////////////////////////////////////////////////
// interface IVarioSentence

class IVarioSentence
{
public:
	virtual void		begin(float height, float vel, float temp, float bat);
	virtual void		begin(float height, float vel, float temp, float prs, float bat);
	
	virtual int			available();
	virtual int			read();
};


/////////////////////////////////////////////////////////////////////////////
// class LK8Sentence

class LK8Sentence : public IVarioSentence
{
public:
	virtual void		begin(float height, float vel, float temp, float bat);
	virtual void		begin(float height, float vel, float temp, float prs, float bat);
	
	virtual int			available();
	virtual int			read();
	
private:
	float				altimeter;
	float				vario;
	float				temperature;
	float				voltage;
	
	Digit 				valueDigit;
	HexDigit 			parityDigit;
	
	unsigned char		tagPos;
	unsigned char		parity;
	
	static const char *	LK8Tag;
};


/////////////////////////////////////////////////////////////////////////////
// class LxNavSentence

class LxNavSentence : public IVarioSentence
{
public:
	virtual void		begin(float height, float vel, float temp, float bat);
	virtual void		begin(float height, float vel, float temp, float prs, float bat);
	
	virtual int			available();
	virtual int			read();
	
private:
	float				vario;
	
	Digit 				valueDigit;
	HexDigit 			parityDigit;
	
	unsigned char		tagPos;
	unsigned char		parity;
	
	static const char *	LxNavTag;
};


/////////////////////////////////////////////////////////////////////////////
// class VarioSentence

class VarioSentence
{
public:
	VarioSentence(char type);
	
public:
	void				begin(float height, float vel, float temp, float bat);
	void				begin(float height, float vel, float temp, float prs, float bat);
	
	int					available();	
	int					read();
	
	void				use(char type)	{ sentenceType = type; }
	
	int					checkInterval();
	
private:
	IVarioSentence *	varioSentence;
	
	LK8Sentence			LK8;
	LxNavSentence		LxNav;

	char				sentenceType;
	unsigned long		lastTick;
};

#endif // __VARIOSENTENCE_H__
