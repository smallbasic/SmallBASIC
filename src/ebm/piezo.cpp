// -*- c-file-style: "java" -*-
// $Id: piezo.cpp,v 1.2 2004-04-12 00:21:41 zeeb90au Exp $
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2003 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
/*                  _.-_:\
//                 /      \
//                 \_.--*_/
//                       v
*/
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <stddef.h>
#include <ebm_object.h>
#include <string.h>
#include <stdio.h>
#include <rs232.h>
#include <cplusplus.h>
#include <time.h>

C_LINKAGE_BEGIN
#include <piezo.h>
#include <sigma.h>
#include <pen.h>

int ncalls=0;

// this file is only used with for debugging. see Makefile for details
void piezo_play_1tone(struct piezo_tone t, int flags) {
    // do nothing: debug mode only
}

//int ebo_first_xobject (ebo_enumerator_t * e) {
//    // do nothing: debug mode only
//    return -2;
//}
//int Sigma_GetAvailableMem(void);

int Sigma_GetPhysicalMemSize(void) {
    return 0;
}

U16	BAT_GetVoltage(void) {
    return 0;
}

U8	BAT_GetPercent(void) {
    return 0;
}

C_LINKAGE_END


RS232_ERRORCODE  RS232_Close(RS232_PORTID port) {
    return RS232_NO_ERROR;
}

U32 RS232_Open(RS232_PORTID port, RS232_BAUDRATE baud, U16 rx_buff_size) {
    return RS232_NO_ERROR;
}

RS232_ERRORCODE RS232_GetBytes(RS232_PORTID port, 
                               U8 *p_ch, 
                               U16 n, 
                               U16 *bytes_read,
                               U32 timeout) {

    if (++ncalls >= 20) {
        const char* msg = "hello there, how are you?";
        strcpy((char*)p_ch, msg);
        *bytes_read = strlen(msg);
        ncalls = 0;
        return RS232_NO_ERROR;
    } else {
        return RS232_RX_BUFF_EMPTY;
    }
}

RS232_ERRORCODE RS232_SendBytes(RS232_PORTID port, U8 *buffer, U16 n) {
    char t[250];
    strncpy(t, (char*)buffer, n);
    t[n]=0;
    printf("buffer=%s len=%d\n", t,n);
    return RS232_NO_ERROR;    
}

