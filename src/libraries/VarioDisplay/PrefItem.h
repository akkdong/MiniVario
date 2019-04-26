// PrefItem.h
//

#ifndef __PREFITEM_H__
#define __PREFITEM_H__

#include <Arduino.h>


////////////////////////////////////////////////////////////////////////////////////////////////
// 

typedef enum _PrefItem_Style
{
	PrefItem_Title,
	PrefItem_Seperator,
	
} PrefItem_Style;



class PrefItem
{
public:
	PrefItem() : type(0), title(0), value(0) {}

	int16_t getHeight() { return 0; }

protected:
	int type;
	const char * title;
	int value;
	
};


#endif // __PREFITEM_H__
