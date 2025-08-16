// Macro for killing denormalled numbers
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// Based on IS_DENORMAL macro by Jon Watte
// This code is public domain

#ifndef _denormals_
#define _denormals_

#include <cstring>

// #define undenormalise(sample) if(((*(unsigned int*)&sample)&0x7f800000)==0) sample=0.0f

#define undenormalise(sample) do {          \
    unsigned int tmp;                        \
    memcpy(&tmp, &(sample), sizeof(float));  \
    if((tmp & 0x7f800000) == 0) sample=0.0f; \
} while(0)

#endif//_denormals_

//ends
