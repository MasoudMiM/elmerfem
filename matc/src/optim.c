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
 *     MATC code optimator. Not used at the moment.
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
 *                       Date: 27 Sep 1995
 *
 *                Modified by:
 *
 *       Date of modification:
 *
 ******************************************************************************/

/*
 * $Id: optim.c,v 1.2 1998/08/01 12:34:52 jpr Exp $ 
 *
 * $Log: optim.c,v $
 * Revision 1.2  1998/08/01 12:34:52  jpr
 *
 * Added Id, started Log.
 * 
 *
 */

#include "matc.h"

TREE *optimtree(root) TREE *root;
{
  int constant = TRUE, csize = 0;
  int constsubs;

  TREE *tptr, *tprev, *prevroot;
  TREE *subs, *prevsubs;

  VARIABLE *subvar, *stmp;

  tptr = tprev = root;
  prevroot = NULL;

  while(tptr)
  {
    constsubs = TRUE; subs = NULL; subvar = NULL;

    if (SUBS(tptr) != (TREE *)NULL)
    {
      subs = SUBS(tptr) = optimtree(SUBS(tptr));
      if (subs == (TREE *)NULL) error("it's not worth it.\n");
      if (ETYPE(subs) != ETYPE_CONST || LINK(subs) != NULL) 
        constsubs = FALSE;
      prevsubs = subs; subs = NEXT(subs);

      while(subs != (TREE *)NULL)
      {
        subs = optimtree(subs);
        if (subs == (TREE *)NULL) error("it's not worth it.\n");
        if (ETYPE(subs) != ETYPE_CONST || LINK(subs) != NULL) 
          constsubs = FALSE;
        NEXT(prevsubs) = subs; prevsubs = subs; 
        subs = NEXT(subs);
      }

      if (constsubs)
      {
        subs = SUBS(tptr);
        subvar = stmp = CDATA(subs);
        subs = NEXT(subs);
        while(subs)
        {
          NEXT(stmp) = CDATA(subs);
           subs = NEXT(subs); stmp = NEXT(stmp);
        }
      }

      subs = SUBS(tptr); SUBS(tptr) = NULL;
    }

    switch(ETYPE(tptr))
    {
    /******************************************************
              some kind of existing identifier.
    *******************************************************/
    case ETYPE_NAME:
    {
      int constargs = TRUE, con = FALSE, argcount = 0;
      VARIABLE *parroot, *par, *tmp = NULL;
      TREE *args, *prevargs;
      COMMAND *com;

      if (ARGS(tptr) != (TREE *)NULL)
      {
        args = ARGS(tptr) = optimtree(ARGS(tptr));
        if (args == (TREE *)NULL) error("it's not worth it.\n");
        if (ETYPE(args) != ETYPE_CONST || LINK(args) != NULL) 
          constargs = FALSE;
        prevargs = args; args = NEXT(args); argcount++;

        while(args != (TREE *)NULL)
        {
          args = optimtree(args);
          if (args == (TREE *)NULL) error("it's not worth it.\n");
          if (ETYPE(args) != ETYPE_CONST || LINK(args) != NULL) 
            constargs = FALSE;
          NEXT(prevargs) = args; prevargs = args; 
          args = NEXT(args); argcount++;
        }
      }

      if ((com = com_check(SDATA(tptr))) != NULL && constargs)
      {
        if (com -> flags && CMDFLAG_CE)
        {

          if (argcount < com->minp || argcount > com->maxp)
          {
            if (com->minp == com->maxp)
            {
              fprintf(math_err, 
                "Builtin function [%s] requires %d argument(s).\n",
                 SDATA(tptr), com->minp);
              error("");
            }
            else
            {
              fprintf(math_err, 
                "Builtin function [%s] takes from %d to %d argument(s).\n",
                 SDATA(tptr), com->minp, com->maxp);
              error("");
            }
          }

          args = ARGS(tptr);
          if (args)
          {
            parroot = par = CDATA(args);
            args = NEXT(args);
            while(args)
            {
              NEXT(par) = CDATA(args);
              args = NEXT(args); par = NEXT(par);
            }
          }

          if (com->flags & CMDFLAG_PW)
          {
            tmp = com_pointw((double (*)())com->sub, parroot);
          }
          else
          {
            tmp = (*com->sub)(parroot);
          }

          par = parroot;
          while(par)
          {
            parroot = NEXT(par); 
            NEXT(par) = NULL;
            par = parroot;
          }

          if (tmp != (VARIABLE *)NULL)
          {

            TREE *newroot;

            newroot = newtree();
            if (tptr == root) 
              root = newroot;
            else
              LINK(tprev) = newroot;

            NEXT(newroot) = NEXT(tptr);
            NEXT(tptr) = (TREE *)NULL;
            LINK(newroot) = LINK(tptr);
            LINK(tptr) = (TREE *)NULL;
            free_tree(tptr);
            tptr = newroot;
            ETYPE(tptr) = ETYPE_CONST;
            CDATA(tptr) = tmp;
            if (constsubs)
            {
              if (!constant) prevroot = tprev;
              con = TRUE;
              csize += NROW(tmp) * NCOL(tmp);
            }
          }
        }
      }

      constant = con;
      }
      break;

    /******************************************************
                   single constant
    *******************************************************/
    case ETYPE_NUMBER:
      if (constsubs) {
        if (!constant) prevroot = tprev;
        constant = TRUE;
        csize++;
      }
      break;

    case ETYPE_STRING:
      if (constsubs)
      {
        if (!constant) prevroot = tprev;
        constant = TRUE;
        csize += strlen(SDATA(tptr));
      }
      break;

    /******************************************************
                           huh ?
    *******************************************************/
    case ETYPE_EQUAT:
    {
      TREE *leftptr;
 
      LEFT(tptr) = leftptr = optimtree(LEFT(tptr));

      if (
       leftptr != NULL && ETYPE(leftptr)==ETYPE_CONST && LINK(leftptr) == NULL
      )
      {

        TREE *newroot;

        newroot = leftptr;
        if (tptr == root) 
          root = newroot;
        else
           LINK(tprev) = newroot;
 
        NEXT(newroot) = NEXT(tptr);
        NEXT(tptr) = (TREE *)NULL;
        LINK(newroot) = LINK(tptr);
        LINK(tptr) = (TREE *)NULL;
        LEFT(tptr) = (TREE *)NULL;
        free_tree(tptr);
        tptr = newroot;
        if (constsubs)
        {
          if (!constant) prevroot = tprev;
          constant = TRUE;
          csize += NROW(CDATA(tptr)) * NCOL(CDATA(tptr));
        }
      }
      else
        constant = FALSE;
      }
      break;

    /******************************************************
         left oper [right]
         oper = divide, multiply, transpose, power,...
    *******************************************************/
    case ETYPE_OPER:
    {
      VARIABLE *tmp = (VARIABLE *)NULL;
      TREE *leftptr, *rightptr;
      MATRIX *opres = NULL;

      leftptr = LEFT(tptr) = optimtree(LEFT(tptr));
      rightptr = RIGHT(tptr) = optimtree(RIGHT(tptr));

      if (leftptr != NULL && rightptr != NULL)
      {
        if (ETYPE(leftptr) == ETYPE_CONST && ETYPE(rightptr) == ETYPE_CONST)
        {
          if (LINK(leftptr) == NULL && LINK(rightptr) == NULL)
          {
            opres = (*VDATA(tptr))(CDATA(leftptr)->this, 
                                   CDATA(rightptr)->this);
            NEXT(CDATA(leftptr)) = NULL;
          }
        }
      }
      else if (leftptr != NULL && ETYPE(leftptr) == ETYPE_CONST)
      {
        if (LINK(leftptr) == NULL)
         opres = (*VDATA(tptr))(CDATA(leftptr)->this, NULL);
      }
      else if (rightptr != NULL && ETYPE(rightptr) == ETYPE_CONST) 
      {
        if (LINK(rightptr) == NULL)
          opres = (*VDATA(tptr))(CDATA(rightptr)->this, NULL);
      }

      if (opres != NULL)
      {
        TREE *newroot;

        tmp = (VARIABLE *)ALLOCMEM(VARIABLESIZE); 
        tmp->this = opres;
        REFCNT(tmp) = 1;

        newroot = newtree();
        if (tptr == root) 
          root = newroot;
        else
          LINK(tprev) = newroot;

        NEXT(newroot) = NEXT(tptr);
        NEXT(tptr) = (TREE *)NULL;
        LINK(newroot) = LINK(tptr);
        LINK(tptr) = (TREE *)NULL;
        free_tree(tptr);
        tptr = newroot;
        ETYPE(tptr) = ETYPE_CONST;
        CDATA(tptr) = tmp;
        if (constsubs)
        {
          if (!constant) prevroot = tprev;
          constant = TRUE;
          csize += NROW(tmp) * NCOL(tmp);
        }
      }
      else
        constant = FALSE;

      }
      break;
    }

    if (constsubs && constant && subs)
    {
      if (CDATA(tptr))
      {
        csize -= NROW(CDATA(tptr)) * NCOL(CDATA(tptr));
        stmp   = CDATA(tptr);
        NEXT(stmp) = subvar;
        if ((CDATA(tptr) = com_el(stmp)) != NULL)
        {
          csize += NROW(CDATA(tptr)) * NCOL(CDATA(tptr));
        }
        var_delete_temp(stmp);
      }
      free_tree(subs);
      SUBS(tptr) = NULL;
    }
    else if (constsubs && subs) 
    {
      SUBS(tptr) = subs;
      while(subvar)
      {
        stmp = NEXT(subvar);
        NEXT(subvar) = NULL;
        subvar = stmp;
      }
    }
    else if (subs)
    {
      SUBS(tptr) = subs;
    }
    else
    {
      SUBS(tptr) = NULL;
    }

    constant &= constsubs;

    if (!constant && csize > 0)
    {

      int i = 0, j = 0, k = 0;
      TREE *ptr, *newroot;

      newroot = newtree();
      ETYPE(newroot) = ETYPE_CONST;

      if (prevroot != (TREE *)NULL)
        ptr = LINK(prevroot);
      else
        ptr = root;

      if (ETYPE(ptr) == ETYPE_STRING) 
        CDATA(newroot) = var_temp_new(TYPE_STRING, 1, csize);
      else if (ETYPE(ptr) == ETYPE_NUMBER)
        CDATA(newroot) = var_temp_new(TYPE_DOUBLE, 1, csize);
      else if (ETYPE(ptr) == ETYPE_CONST)
        CDATA(newroot) = var_temp_new(TYPE(CDATA(ptr)), 1, csize);

      while(ptr != tptr)
      {
        switch(ETYPE(ptr))
        {
        case ETYPE_NUMBER:
          M(CDATA(newroot),0,i++)=DDATA(ptr);
          break;
        case ETYPE_STRING:
          for(j = 0; j < strlen(SDATA(ptr)); j++) 
            M(CDATA(newroot),0,i++)=(double)SDATA(ptr)[j];
          break;
        case ETYPE_CONST:
          j = MATSIZE(CDATA(ptr));
          memcpy(&M(CDATA(newroot),0,i),MATR(CDATA(ptr)),j);
          i += (j>>3);
          break;
        }
        ptr = LINK(ptr);
      }
 
      LINK(newroot) = tptr;
      LINK(tprev) = (TREE *)NULL;
      if (prevroot != (TREE *)NULL)
      {
        free_tree(LINK(prevroot));
        LINK(prevroot) = newroot;
      }
      else
      {
        NEXT(newroot) = NEXT(root);
        NEXT(root) = NULL;
        free_tree(root);
        root = newroot;
      }
      constant = FALSE; 
      csize = 0;
    }

    tprev = tptr;
    tptr = LINK(tptr);
  }

  if (constant && csize > 0)
  {
    int i = 0, j = 0, k = 0;
    TREE *ptr, *newroot;

    newroot = newtree();
    ETYPE(newroot) = ETYPE_CONST;

    if (prevroot != (TREE *)NULL)
      ptr = LINK(prevroot);
    else
      ptr = root;

    if (ETYPE(ptr) == ETYPE_STRING) 
      CDATA(newroot) = var_temp_new(TYPE_STRING, 1, csize);
    else if (ETYPE(ptr) == ETYPE_NUMBER)
      CDATA(newroot) = var_temp_new(TYPE_DOUBLE, 1, csize);
    else if (ETYPE(ptr) == ETYPE_CONST)
      CDATA(newroot) = var_temp_new(TYPE(CDATA(ptr)), 1, csize);

    while(ptr)
    {
      switch(ETYPE(ptr))
      {
      case ETYPE_NUMBER:
        M(CDATA(newroot), 0, i++) = DDATA(ptr);
        break;
      case ETYPE_STRING:
        for(j = 0; j < strlen(SDATA(ptr)); j++) 
          M(CDATA(newroot), 0, i++) = (double)SDATA(ptr)[j];
        break;
      case ETYPE_CONST:
        j = MATSIZE(CDATA(ptr));
        memcpy(&M(CDATA(newroot),0,i),MATR(CDATA(ptr)),j);
        i += (j>>3);
        break;
      }
      ptr = LINK(ptr);
    }

    if (prevroot != (TREE *)NULL)
    {
      free_tree(LINK(prevroot));
      LINK(prevroot) = newroot;
    }
    else
    {
      NEXT(newroot) = NEXT(root);
      NEXT(root) = NULL;
      if (ETYPE(root) == ETYPE_CONST && LINK(root) == NULL)
      {
        NROW(CDATA(newroot)) = NROW(CDATA(root));
        NCOL(CDATA(newroot)) = NCOL(CDATA(root));
      }
      free_tree(root);
      root = newroot;
    }
  }
  else if (constant)
  {
    free_tree(root);
    root = NULL;
  }

  return root;
}


CLAUSE *optimclause(root) CLAUSE *root;
{
  CLAUSE *cptr = root;

  while(cptr)
  {

    switch(cptr->data)
    {
    /************************************************************
                     Function definition
    ************************************************************/
    case funcsym:
      cptr -> this = optimtree(cptr->this);
      LINK(cptr) = optimclause(LINK(cptr));
      return root;

    /***************************************************************
                           statement
    ****************************************************************/
    case assignsym:
      if (cptr->this)
      {
        cptr->this = optimtree(cptr->this);
      }
      LINK(cptr)->this = optimtree(LINK(cptr)->this);
      cptr = LINK(cptr);
      break;

    /***************************************************************
                           if statement
    ****************************************************************/
    case ifsym:

      cptr -> this = optimtree(cptr->this);
      LINK(cptr) = optimclause(LINK(cptr));
      cptr = cptr->jmp;
      if (cptr->data == elsesym)
      {
        LINK(cptr) = optimclause(LINK(cptr));
        cptr = cptr -> jmp;
      }
      break;

    /***************************************************************
                           while statement
    ****************************************************************/
    case whilesym:

      cptr -> this = optimtree(cptr->this);
      LINK(cptr) = optimclause(LINK(cptr));
      cptr = cptr->jmp;
      break;

    /***************************************************************
                           for statement
    ****************************************************************/
    case forsym:

      LINK(cptr->this) = optimtree(LINK(cptr->this));
      LINK(cptr) = optimclause(LINK(cptr));
      cptr = cptr->jmp;
    break;

    case endsym:
      return root;
    }

    cptr = LINK(cptr);
  }
  return root;
}
