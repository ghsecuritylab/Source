/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/* $LP: LPlib/source/LPdir_vms.c,v 1.20 2004/08/26 13:36:05 _cvs_levitte Exp $ */
/*
 * Copyright (c) 2004, Richard Levitte <richard@levitte.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <descrip.h>
#include <namdef.h>
#include <rmsdef.h>
#include <libfildef.h>
#include <lib$routines.h>
#include <strdef.h>
#include <str$routines.h>
#include <stsdef.h>
#ifndef LPDIR_H
#include "LPdir.h"
#endif

/* Because some compiler options hide this macor */
#ifndef EVMSERR
#define EVMSERR		65535  /* error for non-translatable VMS errors */
#endif

struct LP_dir_context_st
{
  unsigned long VMS_context;
#ifdef NAML$C_MAXRSS
  char filespec[NAML$C_MAXRSS+1];
  char result[NAML$C_MAXRSS+1];
#else
  char filespec[256];
  char result[256];
#endif
  struct dsc$descriptor_d filespec_dsc;
  struct dsc$descriptor_d result_dsc;
};

const char *LP_find_file(LP_DIR_CTX **ctx, const char *directory)
{
  int status;
  char *p, *r;
  size_t l;
  unsigned long flags = 0;
#ifdef NAML$C_MAXRSS
  flags |= LIB$M_FIL_LONG_NAMES;
#endif

  if (ctx == NULL || directory == NULL)
    {
      errno = EINVAL;
      return 0;
    }

  errno = 0;
  if (*ctx == NULL)
    {
      size_t filespeclen = strlen(directory);
      char *filespec = NULL;

      /* MUST be a VMS directory specification!  Let's estimate if it is. */
      if (directory[filespeclen-1] != ']'
	  && directory[filespeclen-1] != '>'
	  && directory[filespeclen-1] != ':')
	{
	  errno = EINVAL;
	  return 0;
	}

      filespeclen += 4;		/* "*.*;" */

      if (filespeclen >
#ifdef NAML$C_MAXRSS
	  NAML$C_MAXRSS
#else
	  255
#endif
	  )
	{
	  errno = ENAMETOOLONG;
	  return 0;
	}

      *ctx = (LP_DIR_CTX *)malloc(sizeof(LP_DIR_CTX));
      if (*ctx == NULL)
	{
	  errno = ENOMEM;
	  return 0;
	}
      memset(*ctx, '\0', sizeof(LP_DIR_CTX));

      strcpy((*ctx)->filespec,directory);
      strcat((*ctx)->filespec,"*.*;");
      (*ctx)->filespec_dsc.dsc$w_length = filespeclen;
      (*ctx)->filespec_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
      (*ctx)->filespec_dsc.dsc$b_class = DSC$K_CLASS_S;
      (*ctx)->filespec_dsc.dsc$a_pointer = (*ctx)->filespec;
      (*ctx)->result_dsc.dsc$w_length = 0;
      (*ctx)->result_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
      (*ctx)->result_dsc.dsc$b_class = DSC$K_CLASS_D;
      (*ctx)->result_dsc.dsc$a_pointer = 0;
    }

  (*ctx)->result_dsc.dsc$w_length = 0;
  (*ctx)->result_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
  (*ctx)->result_dsc.dsc$b_class = DSC$K_CLASS_D;
  (*ctx)->result_dsc.dsc$a_pointer = 0;

  status = lib$find_file(&(*ctx)->filespec_dsc, &(*ctx)->result_dsc,
			 &(*ctx)->VMS_context, 0, 0, 0, &flags);

  if (status == RMS$_NMF)
    {
      errno = 0;
      vaxc$errno = status;
      return NULL;
    }

  if(!$VMS_STATUS_SUCCESS(status))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return NULL;
    }

  /* Quick, cheap and dirty way to discard any device and directory,
     since we only want file names */
  l = (*ctx)->result_dsc.dsc$w_length;
  p = (*ctx)->result_dsc.dsc$a_pointer;
  r = p;
  for (; *p; p++)
    {
      if (*p == '^' && p[1] != '\0') /* Take care of ODS-5 escapes */
	{
	  p++;
	}
      else if (*p == ':' || *p == '>' || *p == ']')
	{
	  l -= p + 1 - r;
	  r = p + 1;
	}
      else if (*p == ';')
	{
	  l = p - r;
	  break;
	}
    }

  strncpy((*ctx)->result, r, l);
  (*ctx)->result[l] = '\0';
  str$free1_dx(&(*ctx)->result_dsc);

  return (*ctx)->result;
}

int LP_find_file_end(LP_DIR_CTX **ctx)
{
  if (ctx != NULL && *ctx != NULL)
    {
      int status = lib$find_file_end(&(*ctx)->VMS_context);

      free(*ctx);

      if(!$VMS_STATUS_SUCCESS(status))
	{
	  errno = EVMSERR;
	  vaxc$errno = status;
	  return 0;
	}
      return 1;
    }
  errno = EINVAL;
  return 0;
}

