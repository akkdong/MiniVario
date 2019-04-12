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



typedef struct _PrefItem
{
	int type;
	const char * title;
	int value;
	
} PrefItem;


#endif // __PREFITEM_H__
