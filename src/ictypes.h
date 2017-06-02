/**
* Модуль определения дополнительных типов
* @file
*/


#if !defined( __ICTYPES_H )
#define __ICTYPES_H

typedef int                 BOOL;
typedef unsigned long       DWORD;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef NONE
#define NONE    -1
#endif

#ifndef ERROR
#define ERROR   0
#endif

#ifndef OK
#define OK      +1
#endif

#define MAX_PATH          260

typedef DWORD COLORREF;


#endif /*__ICTYPES_H*/

