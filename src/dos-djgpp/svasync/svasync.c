/*
 * SVAsync 0.10
 * Copyright (c) 1996 Samuel Vincent, 7337 Carioca Ct, Rohnert Park, Ca 94928
 * SVAsync 0.10b
 * Bugfixes 10-27-96 by Anton Helm, Steinerstr. 10, A-4400 Steyr, Austria
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <dpmi.h>
#include <pc.h>
#include <go32.h>
#include <sys/farptr.h>
#include <sys/movedata.h>
#include <conio.h>

#include "svasync.h"

extern unsigned short __djgpp_ds_alias;
extern void SVAsyncProtISR(void);

#define Ctrl8259_0 0x020                      /* 8259 port */
#define Ctrl8259_1 0x021                      /* 8259 port (Masks) */
#define BufSize 32768                         /* Buffer Size */

  /* Globals to be set in SVAsyncInit() */

static unsigned char VectorNum;               /* Vector Number */
static unsigned char EnableIRQ;               /* Mask to enable 8259 IRQ */
static unsigned char DisableIRQ;              /* Mask to disable 8259 IRQ */
static _go32_dpmi_seginfo ProtVector;         /* Old Protmode Vector */
static _go32_dpmi_seginfo info;               /* New Protmode Vector */

  /* Register Addresses for the UART */
static unsigned short Port;                   /* Port Base Address */

unsigned short THR;                           /* Transmitter Holding Register */
unsigned short RDR;                           /* Reciever Data Register */
unsigned short BRDL;                          /* Baud Rate Divisor, Low byte */
unsigned short BRDH;                          /* Baud Rate Divisor, High Byte */
unsigned short IER;                           /* Interupt Enable Register */
unsigned short IIR;                           /* Interupt Identification Register */
unsigned short FCR;                           /* FIFO Control Register */
unsigned short LCR;                           /* Line Control Register */
unsigned short MCR;                           /* Modem Control Register */
unsigned short LSR;                           /* Line Status Register */
unsigned short MSR;                           /* Modem Status Register */
unsigned short SCR;                           /* SCR Register */

  /* Buffer Data */
unsigned volatile char RecBuffer[BufSize] = { 0 };
unsigned volatile int RecHead, RecTail;

static unsigned char SVAsyncStatus=0;

static void lock_interrupt_memory(void);
static void unlock_interrupt_memory(void);


/**************************************
*  This will empty the recieve buffer *
***************************************/
void SVAsyncClear(void)
  {

  disable();
  RecHead = 0;
  RecTail = 0;
  enable();

  return;

  } /* End SVAsyncClear() */


/********************************************************
*  This initalizes the serial port and installs the ISR *
*  CommPort maps out to the following:                  *
*        0  =  COM1                                     *
*        1  =  COM2                                     *
*        2  =  COM3                                     *
*        3  =  COM4                                     *
*********************************************************/
int SVAsyncInit(unsigned int CommPort)
  {

  /**** Set various things according to com port number */

  if(!CommPort)               /* com 1 */
    {
    Port = 0x03F8;
    VectorNum = 0x0C;
    EnableIRQ = 0xEF;
    DisableIRQ = 0x10;
    }
  else
    {
    if(CommPort == 1)         /* com 2 */
      {
      Port = 0x02F8;
      VectorNum = 0x0B;
      EnableIRQ = 0xF7;
      DisableIRQ = 0x08;
      }
    else
      {
      if(CommPort == 2)       /* com 3 */
        {
        Port = 0x03E8;
        VectorNum = 0x0C;
        EnableIRQ = 0xEF;
        DisableIRQ = 0x10;
        }
      else                    /* com 4 */
        {
        Port = 0x02E8;
        VectorNum = 0x0B;
        EnableIRQ = 0xF7;
        DisableIRQ = 0x08;
        }
      }
    }

  /**** Compute Register locations */
  THR   = Port;
  RDR   = Port;
  BRDL  = Port;
  BRDH  = 1 + Port;
  IER   = 1 + Port;
  IIR   = 2 + Port;
  FCR   = 2 + Port;
  LCR   = 3 + Port;
  MCR   = 4 + Port;
  LSR   = 5 + Port;
  MSR   = 6 + Port;
  SCR   = 7 + Port;


  /***** Initalize Buffer */
  SVAsyncClear();
  
  lock_interrupt_memory();
  atexit(unlock_interrupt_memory);

  /***** Set bit 3 in MCR to 0 */
  outportb(MCR, (inportb(MCR) & 0xF7));

  /*** Save and reassign interrupt vectors */
  _go32_dpmi_get_protected_mode_interrupt_vector(VectorNum, &ProtVector);
  
  info.pm_offset = (int) SVAsyncProtISR;
  info.pm_selector = _my_cs();
  _go32_dpmi_set_protected_mode_interrupt_vector(VectorNum, &info);
  
  atexit(SVAsyncStop);

  /***** Enable 8259 interrupt (IRQ) line for this async adapter */
  outportb(Ctrl8259_1, (inportb(Ctrl8259_1) & EnableIRQ));

  /***** Enable 8250 Interrupt-on-data-ready */
  outportb(LCR, (inportb(LCR) & 0x7F));
  
  outportb(IER, 0);

  if(inportb(IER))
    {
    SVAsyncStatus = 0;
    return 1;
    }

  outportb(IER, 0x01);

  /***** Clear 8250 Status and data registers */

    {
    unsigned char temp;

    do
      {
      temp=inportb(RDR);
      temp=inportb(LSR);
      temp=inportb(MSR);
      temp=inportb(IIR);
      }
    while(!(temp & 1));

    }

  /***** Set Bit 3 of MCR -- Enable interupts */
  outportb(MCR, (inportb(MCR) | 0x08));

  SVAsyncStatus = 1;

  /***** Clear Buffer Just in case */
  SVAsyncClear();

  return 0;

  } /* End SVAsyncInit() */


/*******************************************************
*  This uninstalls the ISR and resets the serial port. *
********************************************************/
void SVAsyncStop(void)
  {

  if(!SVAsyncStatus)
    return;

  SVAsyncStatus = 0;

  /***** Mask (disable) 8259 IRQ Interrupt */
  outportb(Ctrl8259_1, (inportb(Ctrl8259_1) | DisableIRQ));

  /***** Disable 8250 interrupt */
  outportb(LCR, (inportb(LCR) & 0x7F));
  outportb(IER, 0);

  /***** Set bit 3 in MCR to 0 */
  outportb(MCR, (inportb(MCR) & 0xF7));

  /***** Interrupts are disabled.  Restore saved interrupt vector. */
  _go32_dpmi_set_protected_mode_interrupt_vector(VectorNum, &ProtVector);

  return;

  } /* End SVAsyncStop() */


/*************************************
*  Gets a byte from the input buffer *
**************************************/
unsigned char SVAsyncIn(void)
  {
  unsigned char retvalue;

  if(RecTail == RecHead)
    {
    return 0;
    }
  
  disable();

  retvalue = RecBuffer[RecTail++];

  if(RecTail >= BufSize)
    RecTail = 0;

  enable();

  return retvalue;

  } /* End SVAsynchIn() */


/***************************************
*  Gets a binary from the input buffer *
***************************************/
int SVAsyncInBin(unsigned char *BinIn)
  {

  if(RecTail == RecHead)
    {
    return 0;
    }
  
  if(!BinIn)
    {
    return 0;
    }

  disable();

  *BinIn = RecBuffer[RecTail++];

  if(RecTail >= BufSize)
    RecTail = 0;

  enable();

  return 1;

  } /* End SVAsyncInBin() */


/**************************************************
*  This simply outputs a byte to the serial port. *
***************************************************/
void SVAsyncOut(unsigned char CharOut)
  {

  while(~inportb(LSR) & 0x20);

  outportb(THR, CharOut);

  return;

  } /* End SVAsyncOut() */


/****************************************************************************
*  Sets communication parameters                                            *
*  Baud = 150, 300, 600, 1200, 2400, 4800, 9600, 19200, 28800, 38400, 57600 *
*  Control = The value to place in the LCR                                  *
*****************************************************************************/

void SVAsyncSet(unsigned int Baud, unsigned int Control)
  {
  int divisor;
  unsigned char divlow, divhigh;

  if(!Baud)
    return;
  
  divisor = 115200 / Baud;

  disable();
  
  outportb(LCR, Control | 0x80); /* Set Port Toggle to BRDL/BRDH registers */
  divlow = divisor & 0x000000ff;
  divhigh = (divisor >> 8) & 0x000000ff;
  outportb(BRDL, divlow);     /* Set Baud Rate */
  outportb(BRDH, divhigh);

  outportb(LCR, Control & 0x007F);          /* Set LCR and Port Toggle */

  enable();

  return;

  } /* End SVAsyncSet() */


/*************************************************
*  Returns the # of characters in input buffer   *
*  waiting to be read.                           *
**************************************************/
int SVAsyncInStat(void)
  {
  int retvalue;

  if(RecHead >= RecTail)
    retvalue = RecHead - RecTail;
  else
    retvalue = (RecHead - RecTail) + BufSize;

  return retvalue;

  } /* End SvaSyncInStat() */


/***************************************************************
*  Returns the # of characters in output buffer.               *
*  This will only be useful once interrupt driven transmitting *
*  is implemented.                                             *
*  Now, it returns 1 if it cannot instantly send a character   *
*  and 0 otherwise.                                            *
****************************************************************/
int SVAsyncOutStat(void)
  {

  if(~inportb(LSR) & 0x20)
    return 1;
  else
    return 0;

  } /* End SVAsyncOutStat() */


/**********************************
*  Sets various handshaking lines *
*  Returns nothing.               *
***********************************/
void SVAsyncHand(unsigned int Hand)
  {

  outportb(MCR, Hand | 0x08);  /* Keep interrupt enable ON */

  return;

  } /* End SVAsyncHand() */


unsigned int SVAsyncStat(void)
  {

  return 0;

  } /* End SVAsyncStat() */


/*

;-----------------------------------------------------------------------------
;       SVAsyncStat                                       Returns Async/Modem status
;-----------------------------------------------------------------------------
;       unsigned        SVAsyncStat( void)
;
;       MSR is returned in the high byte, LSR in the low byte
;-----------------------------------------------------------------------------
PROC    _SVAsyncStat
  push    bp
  mov     bp, sp

  mov     dx, [MSR]
  in      al, dx
  mov     cl, al
  mov     dx, [LSR]
  in      al, dx                  ; LSR in low byte
  mov     ah, cl                  ; MSR in high byte

  pop     bp
  ret
ENDP    _SVAsyncStat


  END

*/


/**********************
 * Internal function  *
 **********************/
static void lock_interrupt_memory(void)
  {
  int errval;
  __dpmi_meminfo info;
  unsigned long address;
  
  __dpmi_get_segment_base_address(_my_ds(), &address);

  info.address = (int) address + (int) &RDR;
  info.size = sizeof(RDR);
  errval = __dpmi_lock_linear_region(&info);
  if(errval == -1)
    printf("Error in locking memory (1)\n!");

  info.address = (int) address + (int) &LSR;
  info.size = sizeof(LSR);
  errval = __dpmi_lock_linear_region(&info);
  if(errval == -1)
    printf("Error in locking memory (2)\n!");

  info.address = (int) address + (int) &RecHead;
  info.size = sizeof(RecHead);
  errval = __dpmi_lock_linear_region(&info);
  if(errval == -1)
    printf("Error in locking memory (3)\n!");

  info.address = (int) address + (int) &RecBuffer;
  info.size = sizeof(RecBuffer);
  errval = __dpmi_lock_linear_region(&info);
  if(errval == -1)
    printf("Error in locking memory (4)\n!");

  info.address = (int) address + (int) RecBuffer;
  info.size = BufSize;
  errval = __dpmi_lock_linear_region(&info);
  if(errval == -1)
    printf("Error in locking memory (5)\n!");
  
  __dpmi_get_segment_base_address(_my_cs(), &address);
  
  info.address = (int) address + (int) SVAsyncProtISR;
  info.size = 4096; /* 4096 bytes is probably overkill. */
  errval = __dpmi_lock_linear_region(&info);
  if(errval == -1)
    printf("Error in locking memory (6)\n!");

  return;

  } /* End lock_interrupt_memory() */


/**********************
 * Internal function  *
 **********************/
static void unlock_interrupt_memory(void)
  {
  __dpmi_meminfo info;
  unsigned long address;
  
  __dpmi_get_segment_base_address(_my_ds(), &address);

  info.address = (int) address + (int) &RDR;
  info.size = sizeof(RDR);
  __dpmi_unlock_linear_region(&info);

  info.address = (int) address + (int) &LSR;
  info.size = sizeof(LSR);
  __dpmi_unlock_linear_region(&info);

  info.address = (int) address + (int) &RecHead;
  info.size = sizeof(RecHead);
  __dpmi_unlock_linear_region(&info);

  info.address = (int) address + (int) &RecBuffer;
  info.size = sizeof(RecBuffer);
  __dpmi_unlock_linear_region(&info);

  info.address = (int) address + (int) RecBuffer;
  info.size = BufSize;
  __dpmi_unlock_linear_region(&info);
  
  __dpmi_get_segment_base_address(_my_cs(), &address);
  
  info.address = (int) address + (int) SVAsyncProtISR;
  info.size = 4096; /* probably overkill */
  __dpmi_unlock_linear_region(&info);

  return;

  } /* End unlock_interrupt_memory() */


/**************************************************
 * Detects UART type, enables FIFO if it exists.  *
 * Returns 0 if 16550 UART and FIFO enabled.      *
 * Returns 1 if 16450 UART.                       *
 * Returns 2 if less than 16450 UART.             *
 **************************************************/

int SVAsyncFifoInit(void)
  {

  outportb(SCR, 0x55);

  if(inportb(SCR) != 0x55)
    return 2;

  outportb(FCR, 0x0f);

  if((inportb(IIR)&0xC0) != 0xC0)
    return 1;

  outportb(FCR, 1 | 2 | 64); /* 8 byte trigger level on receive. */

  return 0;

  } /* End SVAsyncFifoInit() */

