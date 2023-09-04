// FixedLenDigit.h
//

#ifndef __FIXEDLENDIGIT_H__
#define __FIXEDLENDIGIT_H__

/////////////////////////////////////////////////////////////////////////////
// class FixedLenDigit

class FixedLenDigit
{
public:
	FixedLenDigit();
	
public:
	int32_t			begin(int32_t v, int32_t n);
//	int32_t			begin(float v, int32_t n);
	
	int32_t			available();
	int32_t			read();

private:
	int32_t			powi(int32_t base, int32_t exp);
	
private:
	int32_t			value;
	int32_t			pos;
	int32_t			length;
	
	static int32_t	powTable[];
};


#endif // __FIXEDLENDIGIT_H__
