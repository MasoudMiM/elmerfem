/******************************************************************************
 *
 *       ELMER, A Computational Fluid Dynamics Program.
 *
 *       Copyright 1st April 1995 - , Center for Scientific Computing,
 *                                    Finland.
 *
 *       All rights reserved. No part of this program may be used,
 *       reproduced or transmitted in any form or by any means
 *       without the written permission of CSC.
 *
 ******************************************************************************/

/*******************************************************************************
 *
 *     Error/signal trapping for MATC.
 *
 *******************************************************************************
 *
 *                     Author:       Juha Ruokolainen
 *
 *                    Address: Center for Scientific Computing
 *                                Tietotie 6, P.O. BOX 405
 *                                  02101 Espoo, Finland
 *                                  Tel. +358 0 457 2723
 *                                Telefax: +358 0 457 2302
 *                              EMail: Juha.Ruokolainen@csc.fi
 *
 *                       Date: 30 May 1996
 *
 *                Modified by:
 *
 *       Date of modification:
 *
 ******************************************************************************/

/*
 * $Id: error.c,v 1.2 1998/08/01 12:34:34 jpr Exp $ 
 *
 * $Log: error.c,v $
 * Revision 1.2  1998/08/01 12:34:34  jpr
 *
 * Added Id, started Log.
 * 
 *
 */

#include "matc.h"
#include "str.h"

void sig_trap()
/*======================================================================
?  Interrupt or floating point exeption. Free all memory allocated 
|  after last call to doread, give an error message and
|  longjump back to doread.
&  longjmp, mem_free_all
~  doread
^=====================================================================*/
{
  fprintf( math_out, "^C\n" );

#if 0
  signal(SIGINT, sig_trap);
  signal(SIGFPE, sig_trap);
#endif

  mem_free_all();

  longjmp(*jmpbuf, 2);
}

void error(char *fmt,void *p1, void *p2)
/*======================================================================
?  An error is detected by the program. Free all memory allocated 
|  after last call to doread, give an error message and
|  longjump back to doread.
&  longjmp, fprintf, mem_free_all
~  doread
^=====================================================================*/
{
  PrintOut( "MATC ERROR: " );
  PrintOut( fmt,p1,p2 );

  mem_free_all();

  longjmp(*jmpbuf, 2);
}

