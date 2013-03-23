/* 
 * File:   sram_defs.h
 * Author: Hans
 *
 * Created on March 23, 2013, 8:00 PM
 */

#ifndef SRAM_DEFS_H
#define	SRAM_DEFS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "types.h"

    typedef struct meIsLarge_s
    {
        UI08_t data[80];
        UI32_t d ;
    } meIsLarge_t;

#define SRAM_TABLE(SRAM_MEM_VARIABLE, SRAM_MEM_ARRAY) \
    SRAM_MEM_VARIABLE   (UI32_t, myVariable) \
    SRAM_MEM_VARIABLE(meIsLarge_t, myLarge) \
    SRAM_MEM_ARRAY      (UI32_t, myArray, 128) \

#include "sram.h"

#ifdef	__cplusplus
}
#endif

#endif	/* SRAM_DEFS_H */
