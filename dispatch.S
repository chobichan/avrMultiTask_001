/* ----------------------------------------
 not preempt multi task service call routines.
  dispatcher for AVR

  Copyright (c) 2020 hamayan (hamayan.contact@gmail.com).
  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Created 2020 by hamayan (hamayan.contact@gmail.com)
---------------------------------------- */

/* --------------------------------------------------------------- */
/*  dispatcher for Atmel AVR                                       */
/*  Refer to AVR dispatcher from project HOS                       */
/*                                                                 */
/*  Hyper Operating System V4  μITRON4.0 specification Real-Time OS */
/*   Processor abstraction component (Atmel ATmega)                */
/*    Context control                                              */
/*                                                                 */
/*                         Copyright (C) 1998-2002 by Project HOS  */
/*                         http://sourceforge.jp/projects/hos/     */
/* --------------------------------------------------------------- */
#define __SFR_OFFSET 0
#include <avr/io.h>

__SREG__ = 0x3f     ;status register
__SP_H__ = 0x3e     ;SP high
__SP_L__ = 0x3d     ;SP low
__tmp_reg__ = 0     ;r0
__zero_reg__ = 1    ;r1: always zero

;
;AVR GCC Destruction Register : r0,r18-r27(X register),r30-r31(Z register)
;AVR GCC non-destructive registers : r2-r17,r28-r29(Y register)
;Always set the r1 register to 0 when returning from a function  r1=0
;AVR GCC first argument : r25:r24
;AVR GCC second argument : r23:r22
;AVR GCC third argument : r21:r20
;AVR GCC forth argument : r19:r18
;AVR GCC return values : r25:r24

  .text

/************************************************
  disable interrupt
*************************************************/
	.global	disableInterrupt
	.func	disableInterrupt
disableInterrupt:
	cli  /* disable interrupt */
	ret
	.endfunc

/************************************************
  enable interrupt
*************************************************/
	.global	enableInterrupt
	.func	enableInterrupt
enableInterrupt:
	sei			/* enable interrupt */
	ret
	.endfunc

/************************************************
 Load context and start task
 void sta_ctx(
 void *exe ) // Execution context save destination
 r25: r24: Stack pointer storage source address
************************************************/
  .global  sta_ctx
  .func    sta_ctx
sta_ctx:
  movw  XL,r24  ; Get the save address of the context to be expanded
  ld  r0, X+  ; Get lower stack pointer from save source
  out __SP_L__,r0  ; Expand the lower part of the stack pointer below the stack pointer of the IO register
  ld  r0,X  ; Get stack pointer from save source
  out __SP_H__,r0  ; Expand the upper part of the stack pointer to the upper part of the stack pointer of the IO register

  pop r29   ; Expand the upper part of the non-destructive Y register
  pop r28   ; Expand the lower part of the non-destructive Y register

  pop r17  ; Expand non-destructive registers
  pop r16  ; Expand non-destructive registers
  pop r15  ; Expand non-destructive registers
  pop r14  ; Expand non-destructive registers
  pop r13  ; Expand non-destructive registers
  pop r12  ; Expand non-destructive registers
  pop r11  ; Expand non-destructive registers
  pop r10  ; Expand non-destructive registers
  pop r9  ; Expand non-destructive registers
  pop r8  ; Expand non-destructive registers
  pop r7  ; Expand non-destructive registers
  pop r6  ; Expand non-destructive registers
  pop r5  ; Expand non-destructive registers
  pop r4  ; Expand non-destructive registers
  pop r3  ; Expand non-destructive registers
  pop r2  ; Expand non-destructive registers
  pop r0  ; Do not assign the status register

  ret
  .endfunc


/************************************************
 Switching the execution context
 void swi_ctx(
 void *pre, // Current context destination
 void *post) // The context to switch to
 r25: r24: Stack pointer storage address
 r23: r22: Stack pointer storage source address
************************************************/
  .global  swi_ctx
  .func    swi_ctx
swi_ctx:
  in    r0,__SREG__ ; Save status register
  cli     ; Disable interrupt once
  push  r0    ; Save status register
  push  r2  ; Saving non-destructive registers
  push  r3  ; Saving non-destructive registers
  push  r4  ; Saving non-destructive registers
  push  r5  ; Saving non-destructive registers
  push  r6  ; Saving non-destructive registers
  push  r7  ; Saving non-destructive registers
  push  r8  ; Saving non-destructive registers
  push  r9  ; Saving non-destructive registers
  push  r10  ; Saving non-destructive registers
  push  r11  ; Saving non-destructive registers
  push  r12  ; Saving non-destructive registers
  push  r13  ; Saving non-destructive registers
  push  r14  ; Saving non-destructive registers
  push  r15  ; Saving non-destructive registers
  push  r16  ; Saving non-destructive registers
  push  r17  ; Saving non-destructive registers

  push  r28   ; Save the lower part of the non-destructive Y register
  push  r29   ; Stores the upper part of the non-destructive Y register

  movw  XL,r24  ; Get the save address of the context to be saved
  in  r0,__SP_L__  ; Read the lower part of the IO register stack pointer
  st  X+,r0  ; Store lower part of stack pointer of IO register
  in  r0,__SP_H__ ; Read the upper part of the IO register stack pointer
  st  X,r0  ; Store upper part of stack pointer of IO register

  movw  XL,r22  ; Get the save address of the context to be expanded
  ld  r0, X+  ; Get lower stack pointer from save source
  out __SP_L__,r0  ; Expand the lower part of the stack pointer below the stack pointer of the IO register
  ld  r0,X  ; Get stack pointer from save source
  out __SP_H__,r0  ; Expand the upper part of the stack pointer to the upper part of the stack pointer of the IO register

  pop r29   ; Expand the upper part of the non-destructive Y register
  pop r28   ; Expand the lower part of the non-destructive Y register

  pop r17  ; Expand non-destructive registers
  pop r16  ; Expand non-destructive registers
  pop r15  ; Expand non-destructive registers
  pop r14  ; Expand non-destructive registers
  pop r13  ; Expand non-destructive registers
  pop r12  ; Expand non-destructive registers
  pop r11  ; Expand non-destructive registers
  pop r10  ; Expand non-destructive registers
  pop r9  ; Expand non-destructive registers
  pop r8  ; Expand non-destructive registers
  pop r7  ; Expand non-destructive registers
  pop r6  ; Expand non-destructive registers
  pop r5  ; Expand non-destructive registers
  pop r4  ; Expand non-destructive registers
  pop r3  ; Expand non-destructive registers
  pop r2  ; Expand non-destructive registers
  pop r0  ; Return status register
  out __SREG__,r0  ; Return status register

  ret  ; On return, the PC saved in the stack is expanded
  .endfunc

/* -------------------------------------------------------------------------*/
/*  Copyright (C) 1998-2002 by Project HOS                                  */
/* -------------------------------------------------------------------------*/
