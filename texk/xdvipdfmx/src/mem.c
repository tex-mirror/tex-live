/*  $Header: /home/cvsroot/dvipdfmx/src/mem.c,v 1.8 2009/09/18 23:56:02 matthias Exp $

    This is dvipdfmx, an eXtended version of dvipdfm by Mark A. Wicks.

    Copyright (C) 2002 by Jin-Hwan Cho and Shunsaku Hirata,
    the dvipdfmx project team <dvipdfmx@project.ktug.or.kr>
    
    Copyright (C) 1998, 1999 by Mark A. Wicks <mwicks@kettering.edu>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*/

#include <stdio.h>	
#include <stdlib.h>

#include "system.h"
#include "mem.h"
#include "error.h"


void *new (size_t size)
{
  void *result = malloc (size);
  if (!result) {
    ERROR("Out of memory - asked for %lu bytes\n", (unsigned long) size);
  }

  return result;
}

void *renew (void *mem, size_t size)
{
  if (size) {
    void *result = realloc (mem, size);
    if (!result) {
      ERROR("Out of memory - asked for %lu bytes\n", (unsigned long) size);
    }
    return result;
  } else {
    /* realloc may not return NULL if size == 0 */
    free(mem);
    return NULL;
  }
}
