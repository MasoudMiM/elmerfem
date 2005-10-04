/******************************************************************************
! *
! *       ELMER, A Computational Fluid Dynamics Program.
! *
! *       Copyright 1st April 1995 - , Center for Scientific Computing,
! *                                    Finland.
! *
! *       All rights reserved. No part of this program may be used,
! *       reproduced or transmitted in any form or by any means
! *       without the written permission of CSC.
! *
! *****************************************************************************/
 
/*******************************************************************************
! *
! *  Utilities for dynamic loading of user functions, and other operating
! *  system interfaces.
! *
! ******************************************************************************
! *
! *                     Author:       Juha Ruokolainen
! *
! *                    Address: Center for Scientific Computing
! *                                Tietotie 6, P.O. BOX 405
! *                                  02
! *                                  Tel. +358 0 457 2723
! *                                Telefax: +358 0 457 2302
! *                              EMail: Juha.Ruokolainen@csc.fi
! *
! *                       Date: 02 Jun 1997
! *
! *                Modified by:
! *
! *       Date of modification:
! *
! *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* #include <elmer/matc.h> maybe in the future */

/* eg. FC_CHAR_PTR and FC_FUNC is defined here */
#include "../config.h"

#if defined(WIN32) | defined(MINGW32)
#  include <direct.h>
#  include <windows.h>
#else
#include <strings.h>
#  include <dlfcn.h>
#endif

#define MAX_NAME_LEN 128

#ifdef SGI64
void corename_()
{
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/prctl.h>

 prctl( PR_COREPID,0,0 );
}
#endif

/* pc needs more bits on 64bit arch  */
#ifdef ARCH_32_BITS
#define f_ptr int *
#else 
#define f_ptr long int *
#endif

/*--------------------------------------------------------------------------
  This routine will create a directory given name of the directory.
  -------------------------------------------------------------------------*/
void STDCALLBULL FC_FUNC(makedirectory,MAKEDIRECTORY) 
     ( FC_CHAR_PTR(Name,len) )
{
#if defined(WIN32) || defined(MINGW32)
    if ( _mkdir( Name ) != 0 ) {
#else
    if ( mkdir( Name, 0700 ) != 0 ) {
      chmod( Name, 0700 );
#endif
    }
}

/*--------------------------------------------------------------------------
  This routine execute a operating system command.
  -------------------------------------------------------------------------*/
void STDCALLBULL FC_FUNC(systemc,SYSTEMC) ( FC_CHAR_PTR(str,l1) )
{
   system( str );
}

/*--------------------------------------------------------------------------
  This routine will return value of a environment variable to a
  given string variable.
  -------------------------------------------------------------------------*/
void STDCALLBULL FC_FUNC(envir,ENVIR) ( FC_CHAR_PTR(Name,l1), FC_CHAR_PTR(Value,l2), int *len )
{
    if ( getenv( Name ) ) {
      strncpy( Value,(char *)getenv(Name), MAX_NAME_LEN );
      *len = strlen( Value );
    } else {
      *len = 0;
      *Value = '\0';
    }
}

/*--------------------------------------------------------------------------
  Internal: convert function names into to fortran mangled form for dynamical
  loading
  ---------------------------------------------------------------------------*/
static void fortranMangle(char *orig, char *mangled)
{
  int uscore, i;
  
  if(ELMER_LINKTYP == 1 || ELMER_LINKTYP == 3 || ELMER_LINKTYP == 4)
  {
    for( i=0 ; i<strlen(orig) ; i++ ) /* to lower case */
    {
      if ( orig[i] >= 'A'  && orig[i] <= 'Z' ) 
	orig[i] += 'a' - 'A';
    }
  }
  if(ELMER_LINKTYP == 2)
  {
    for( i=0; i<strlen(orig); i++ ) /* to upper case */
    {
      if ( orig[i] >= 'a'  && orig[i] <= 'z' ) 
	orig[i] += 'A' - 'a';
    }
  }
  
  strcpy( mangled, orig );
  if(ELMER_LINKTYP == 1) /* underscore */
  {
      strcat( mangled, "_" );
  }
  else if(ELMER_LINKTYP == 4) /* 1-2 underscores  */
  {
    uscore = 0;
    for( i=0; i<strlen(mangled); i++ )
      if(mangled[i] == '_')
	uscore++;
    
    if(uscore == 0)
    {
      strcat( mangled, "_" );
    } 
    else 
    {
      strcat( mangled, "__" );
    }
  }
}

/*--------------------------------------------------------------------------
  This routine will return address of a function given path to a dynamically
  loaded library and name of the routine.
  -------------------------------------------------------------------------*/
void *STDCALLBULL FC_FUNC(loadfunction,LOADFUNCTION) ( int *Quiet,
      FC_CHAR_PTR(Library,l1), FC_CHAR_PTR(Name,l2) )
{
/*--------------------------------------------------------------------------*/
   void (*Function)(),*Handle;
   int i;
   char *cptr;
   static char ElmerLib[2*MAX_NAME_LEN], NewLibName[3*MAX_NAME_LEN],
               NewName[MAX_NAME_LEN],   dl_err_msg[6][MAX_NAME_LEN];
/*--------------------------------------------------------------------------*/
   
   fortranMangle( Name, NewName );
   strncpy( NewLibName, Library, 3*MAX_NAME_LEN );

   if ( *Quiet==0 ) 
     fprintf(stderr,"Loading user function library: [%s]...[%s]", Library, Name );
   
#ifdef HAVE_DLOPEN_API

   /* Try again with explict ELMER_LIB dir */
   ElmerLib[0] = '\0';
   cptr = (char *)getenv( "ELMER_LIB" );
   if ( cptr != NULL ) {
      strncpy( ElmerLib, cptr, 2*MAX_NAME_LEN );
      strncat( ElmerLib, "/", 2*MAX_NAME_LEN  );
   } else {
      cptr = (char *)getenv("ELMER_HOME");
      if ( cptr != NULL  ) {
         strncpy( ElmerLib, cptr, 2*MAX_NAME_LEN );
         strncat( ElmerLib, "/share/elmersolver/lib/", 2*MAX_NAME_LEN );
      } else {
         strncpy( ElmerLib, ELMER_SOLVER_HOME, 2*MAX_NAME_LEN );
         strncat( ElmerLib, "/lib/", 2*MAX_NAME_LEN );
      }
   }

   for( i=0; i<6; i++ )
     {
        switch(i) 
        {
          case 0: strncpy( NewLibName, Library,  3*MAX_NAME_LEN );
                  break;
          case 1: case 3: case 5:
                  strncat( NewLibName, SHL_EXTENSION, 3*MAX_NAME_LEN );
                  break;
          case 2: strcpy( NewLibName, "./");
                  strncat( NewLibName, Library,  3*MAX_NAME_LEN );
                  break;
          case 4: strncpy( NewLibName, ElmerLib, 3*MAX_NAME_LEN );
                  strncat( NewLibName, Library,  3*MAX_NAME_LEN );
                  break;
        }
        if ( ( Handle = dlopen( NewLibName , RTLD_NOW ) ) == NULL )
          {
             strncpy( dl_err_msg[i], dlerror(), MAX_NAME_LEN );
          } else {
             break;
          }
     }

   if ( Handle == NULL ) 
     {
        for( i=0; i<6; i++ )
          {
             switch(i) 
             {
               case 0: strncpy( NewLibName, Library,  3*MAX_NAME_LEN );
                       break;
               case 1: case 3: case 5:
                       strncat( NewLibName, SHL_EXTENSION, 3*MAX_NAME_LEN );
                       break;
               case 2: strcpy( NewLibName, "./");
                       strncat( NewLibName, Library,  3*MAX_NAME_LEN );
                       break;
               case 4: strncpy( NewLibName, ElmerLib, 3*MAX_NAME_LEN );
                       strncat( NewLibName, Library,  3*MAX_NAME_LEN );
                       break;
             }
             fprintf( stderr, "\nLoad: Unable to open shared library: [%s]\n", NewLibName );
             fprintf( stderr, "%s\n", dl_err_msg[i] );
           }
         exit(0);
     }

   if ( (Function = (void(*)())dlsym( Handle,NewName ) ) == NULL )
   {
      fprintf( stderr, "Load: FATAL: Can't find procedure [%s]\n", NewName );
      exit(0);
   }
#elif defined(HAVE_LOADLIBRARY_API)

   if ( ( Handle = (void *)LoadLibrary( Library ) ) == NULL )
   { 
     fprintf( stderr, "Load: FATAL: Can't load shared image [%s]\n", Library );

     /* try .dll */
     strncat( NewLibName, SHL_EXTENSION, 3*MAX_NAME_LEN );
     if ( ( Handle = (void *)LoadLibrary( NewLibName ) ) == NULL ) 
     { 
       fprintf( stderr, "Load: FATAL: Can't load shared image [%s]. Exiting.\n", NewLibName );
       exit(0);
     }
   }

   for( i=0; i<strlen(Name); i++ )
   {
     if ( Name[i] >= 'a'  && Name[i] <= 'z' ) Name[i] += 'A' - 'a';
   }

   if ( (Function = (void *)GetProcAddress( Handle,Name ) ) == NULL )
   {
     fprintf( stderr,"Load: FATAL: Can't find procedure [%s]\n", Name );
     exit(0);
   }
#endif 

   return (void *)Function;
}

/*--------------------------------------------------------------------------
  INTERNAL: Execute given function returning integer value
  -------------------------------------------------------------------------*/
static int IntExec( int (STDCALLBULL *Function)(),void *Model )
{
   return (*Function)( Model );
}

/*--------------------------------------------------------------------------
   Execute given function returning integer value
   -------------------------------------------------------------------------*/
int STDCALLBULL FC_FUNC(execintfunction,EXECINTFUNCTION) ( f_ptr Function,void *Model )
{
  return IntExec( (int (STDCALLBULL *)())*Function,Model );
}

/*--------------------------------------------------------------------------
   INTERNAL: Execute given function returning double value
  -------------------------------------------------------------------------*/
static void DoubleArrayExec( double *(STDCALLBULL *Function)(), void *Model,
               int *Node, double *Value, double *Array )
{
   (*Function)( Model,Node,Value,Array );
}

/*--------------------------------------------------------------------------
   Execute given function returning double value
   -------------------------------------------------------------------------*/
void STDCALLBULL FC_FUNC(execrealarrayfunction,EXECREALARRAYFUNCTION)
     ( f_ptr Function, void *Model,
       int *Node, double *Value, double *Array )
{
   DoubleArrayExec( (double*(STDCALLBULL *)())*Function,Model,Node,Value, Array );
}

/*--------------------------------------------------------------------------
   INTERNAL: Execute given function returning double value
  -------------------------------------------------------------------------*/
static double DoubleExec( double (STDCALLBULL *Function)(), void *Model,
               int *Node, double *Value )
{
   return (*Function)( Model,Node,Value );
}

/*--------------------------------------------------------------------------
   Execute given function returning double value
   -------------------------------------------------------------------------*/
double STDCALLBULL FC_FUNC(execrealfunction,EXECREALFUNCTION)
     ( f_ptr Function, void *Model,
       int *Node, double *Value )
{
   return DoubleExec( (double (STDCALLBULL *)())*Function,Model,Node,Value );
}

/*--------------------------------------------------------------------------
   INTERNAL: Execute given function returning double value
  -------------------------------------------------------------------------*/
static double ConstDoubleExec( double (STDCALLBULL *Function)(), void *Model,
			       double *x, double *y, double *z )
{
   return (*Function)( Model, x,y,z );
}

/*--------------------------------------------------------------------------
   Execute given function returning double value
   -------------------------------------------------------------------------*/
double STDCALLBULL FC_FUNC(execconstrealfunction,EXECCONSTREALFUNCTION)
     ( f_ptr Function, void *Model,
       double *x, double *y, double *z )
{
   return ConstDoubleExec( (double (STDCALLBULL *)())*Function,Model,x,y,z );
}


/*--------------------------------------------------------------------------
   Return argument (just to fool Fortran type checking)
   -------------------------------------------------------------------------*/
void *STDCALLBULL FC_FUNC(addrfunc,ADDRFUNC) ( void *Function )
{
   return (void *)Function;
}

/*--------------------------------------------------------------------------
   INTERNAL: Call solver routines at given address
  -------------------------------------------------------------------------*/
static void DoExecSolver(
  void (STDCALLBULL *SolverProc)(), void *Model, void *Solver, void *dt, void *Transient)
{
  (*SolverProc)( Model,Solver,dt,Transient ); 
  return;
}

/*--------------------------------------------------------------------------
   Call solver routines at given address
   -------------------------------------------------------------------------*/
void STDCALLBULL FC_FUNC(execsolver,EXECSOLVER)
     ( f_ptr *SolverProc, void *Model, void *Solver, void *dt, void *Transient )
{
  DoExecSolver( (void (STDCALLBULL *)())*SolverProc,Model,Solver,dt,Transient );
}

/*--------------------------------------------------------------------------
   INTERNAL: Call lin. solve routines at given address
  -------------------------------------------------------------------------*/
static int DoLinSolveProcs(
  int (STDCALLBULL *SolverProc)(), void *Model, void *Solver, void *Matrix, void *b, 
                void *x, void *n, void *DOFs, void *Norm )
{
   return (*SolverProc)( Model,Solver,Matrix,b,x,n, DOFs,Norm );
}


/*--------------------------------------------------------------------------
   Call lin. solver routines at given address
   -------------------------------------------------------------------------*/
int STDCALLBULL FC_FUNC(execlinsolveprocs,EXECLINSOLVEPROCS)
     ( f_ptr *SolverProc, void *Model, void *Solver, void *Matrix, void *b, void *x, void *n, void *DOFs, void *Norm )
{
   return DoLinSolveProcs( (int (STDCALLBULL *)())*SolverProc,Model,Solver,Matrix,b,x,n,DOFs,Norm );
}

char *mtc_domath(char *);
void mtc_init(FILE *,FILE *, FILE *);

/*--------------------------------------------------------------------------
  This routine will call matc and return matc result
  -------------------------------------------------------------------------*/
void STDCALLBULL FC_FUNC(matc,MATC) ( FC_CHAR_PTR(cmd,l1), FC_CHAR_PTR(Value,l2), int *len )
{
#define MAXLEN 8192

  static int been_here = 0;
    char *ptr, cc[32], *cmdcopy;

    if ( been_here==0 ) {
       mtc_init( NULL, stdout, stderr ); 
       strcpy( cc, "format( 12,\"rowform\")" );
       mtc_domath( cc );
       been_here = 1;
     }

    cmd[*len] = '\0';
    if ( ptr = (char *)mtc_domath( cmd ) ) 
    {
      strcpy( Value, (char *)ptr );
      *len = strlen(Value)-1;

      if ( strncmp( Value, "MATC ERROR:", 11 ) == 0 ) {
          fprintf( stderr, "Solver input file error: %s\n", Value );
          exit(0);
      }
    } else {
      *len = 0;
      *Value = ' ';
    }

}


/*--------------------------------------------------------------------------
  INTERNAL: execute user material function
  -------------------------------------------------------------------------*/
static double DoViscFunction(double (STDCALLBULL *SolverProc)(), void *Model, void *Element, void *Nodes, void *n,
     void *Basis, void *GradBasis, void *Viscosity, void *Velo, void *GradV )
{
   double s;
   s = (*SolverProc)( Model,Element,Nodes,n,Basis,GradBasis,
                   Viscosity, Velo, GradV );
   return s;
}

/*--------------------------------------------------------------------------
  This routine will call user defined material def. function
  -------------------------------------------------------------------------*/
double STDCALLBULL FC_FUNC(materialuserfunction,MATERIALUSERFUNCTION)
  ( f_ptr Function, void *Model, void *Element, void *Nodes, void *n, void *nd, void *Basis, void *GradBasis, void *Viscosity, void *Velo, void *gradV )
{
  fprintf(stderr,"entering materialuserfunc\n");
  fflush(stderr);
   return DoViscFunction( (double (STDCALLBULL *)())*Function,Model,Element,Nodes,n,Basis,
                  GradBasis,Viscosity,Velo,gradV );
}

/*--------------------------------------------------------------------------
  INTERNAL: execute user material function
  -------------------------------------------------------------------------*/
static void DoSimulationProc( void (STDCALLBULL *SimulationProc)(), void *Model )
{ 
  (*SimulationProc)( Model ); 
}

/*--------------------------------------------------------------------------
  This routine will call user defined material def. function
  -------------------------------------------------------------------------*/
void STDCALLBULL FC_FUNC(execsimulationproc,EXECSIMULATIONPROC)
     ( f_ptr Function, void *Model )
{
   DoSimulationProc( (void (STDCALLBULL *)())*Function,Model );
}
