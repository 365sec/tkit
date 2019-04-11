#ifndef CY_DEBUG_H
#define CY_DEBUG_H
#include"ProductInfo.h"


#ifndef  CY_CHECKKEY
void cy_outputdebugstring(const char* fomat,...);
#else
#define  cy_outputdebugstring(fomat,...)  
#endif

#endif