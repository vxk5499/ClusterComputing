#ifndef __SLAVE_PROCESS_H__
#define __SLAVE_PROCESS_H__

#include "RayTrace.h"

void slaveMain( ConfigData *data );
void slaveVertical(ConfigData *data, float* pixels);
void slaveCyclic(ConfigData *data);
void slaveBlock(ConfigData *data);



#endif
