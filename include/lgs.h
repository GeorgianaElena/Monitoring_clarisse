#ifndef _LGS_H_
#define _LGS_H_

#include <evpath.h>


#define PORT 12345
#define MAX_APPLN_NAME 50


//int initLGS(CManager cm);
int addSource(CManager cm, char *name, EVstone stone, EVaction action);
//int addSink(CManager cm, char *name, EVstone stone, char *filterspec);
int addSink(CManager cm, char *name, EVstone stone, char *filter_name, char *filterspec);

#endif //_LGS_H_
