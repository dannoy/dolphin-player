/* Determine a canonical name for the current locale's character encoding.

   Copyright (C) 2000-2006, 2008-2009 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.  */

/* Written by Bruno Haible <bruno@clisp.org>.  */

#include <config.h>

/* Specification.  */
#include "localcharset.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef DIRECTORY_SEPARATOR
# define DIRECTORY_SEPARATOR '/'
#endif

#ifndef ISSLASH
# define ISSLASH(C) ((C) == DIRECTORY_SEPARATOR)
#endif

/* The following static variable is declared 'volatile' to avoid a
   possible multithread problem in the function get_charset_aliases. If we
   are running in a threaded environment, and if two threads initialize
   'charset_aliases' simultaneously, both will produce the same value,
   and everything will be ok if the two assignments to 'charset_aliases'
   are atomic. But I don't know what will happen if the two assignments mix.  */
#if __STDC__ != 1
# define volatile /* empty */
#endif
/* Pointer to the contents of the charset.alias file, if it has already been
   read, else NULL.  Its format is:
   ALIAS_1 '\0' CANONICAL_1 '\0' ... ALIAS_n '\0' CANONICAL_n '\0' '\0'  */
static const char * volatile charset_aliases;

/* Return a pointer to the contents of the charset.alias file.  */
static const char *
get_charset_aliases (void)
{
  const char *cp;

  cp = charset_aliases;
  if (cp == NULL)
    {
      /* To avoid the trouble of installing a file that is shared by many
	 GNU packages -- many packaging systems have problems with this --,
	 simply inline the aliases here.  */
      cp = "ISO8859-1" "\0" "ISO-8859-1" "\0"
	   "ISO8859-2" "\0" "ISO-8859-2" "\0"
	   "ISO8859-4" "\0" "ISO-8859-4" "\0"
	   "ISO8859-5" "\0" "ISO-8859-5" "\0"
	   "ISO8859-7" "\0" "ISO-8859-7" "\0"
	   "ISO8859-9" "\0" "ISO-8859-9" "\0"
	   "ISO8859-13" "\0" "ISO-8859-13" "\0"
	   "ISO8859-15" "\0" "ISO-8859-15" "\0"
	   "KOI8-R" "\0" "KOI8-R" "\0"
	   "KOI8-U" "\0" "KOI8-U" "\0"
	   "CP866" "\0" "CP866" "\0"
	   "CP949" "\0" "CP949" "\0"
	   "CP1131" "\0" "CP1131" "\0"
	   "CP1251" "\0" "CP1251" "\0"
	   "eucCN" "\0" "GB2312" "\0"
	   "GB2312" "\0" "GB2312" "\0"
	   "eucJP" "\0" "EUC-JP" "\0"
	   "eucKR" "\0" "EUC-KR" "\0"
	   "Big5" "\0" "BIG5" "\0"
	   "Big5HKSCS" "\0" "BIG5-HKSCS" "\0"
	   "GBK" "\0" "GBK" "\0"
	   "GB18030" "\0" "GB18030" "\0"
	   "SJIS" "\0" "SHIFT_JIS" "\0"
	   "ARMSCII-8" "\0" "ARMSCII-8" "\0"
	   "PT154" "\0" "PT154" "\0"
	 /*"ISCII-DEV" "\0" "?" "\0"*/
	   "*" "\0" "UTF-8" "\0";

      charset_aliases = cp;
    }

  return cp;
}

/* Determine the current locale's character encoding, and canonicalize it
   into one of the canonical names listed in config.charset.
   The result must not be freed; it is statically allocated.
   If the canonical name cannot be determined, the result is a non-canonical
   name.  */

#ifdef STATIC
STATIC
#endif
const char *
locale_charset (void)
{
  const char *codeset;
  const char *aliases;

  /* On old systems which lack it, use setlocale or getenv.  */
  const char *locale = NULL;

  /* But most old systems don't have a complete set of locales.  Some
     (like SunOS 4 or DJGPP) have only the C locale.  Therefore we don't
     use setlocale here; it would return "C" when it doesn't support the
     locale name the user has set.  */
#  if 0
  locale = setlocale (LC_CTYPE, NULL);
#  endif
  if (locale == NULL || locale[0] == '\0')
    {
      locale = getenv ("LC_ALL");
      if (locale == NULL || locale[0] == '\0')
	{
	  locale = getenv ("LC_CTYPE");
	  if (locale == NULL || locale[0] == '\0')
	    locale = getenv ("LANG");
	}
    }

  /* On some old systems, one used to set locale = "iso8859_1". On others,
     you set it to "language_COUNTRY.charset". In any case, we resolve it
     through the charset.alias file.  */
  codeset = locale;

  if (codeset == NULL)
    /* The canonical name cannot be determined.  */
    codeset = "";

  /* Resolve alias. */
  for (aliases = get_charset_aliases ();
       *aliases != '\0';
       aliases += strlen (aliases) + 1, aliases += strlen (aliases) + 1)
    if (strcmp (codeset, aliases) == 0
	|| (aliases[0] == '*' && aliases[1] == '\0'))
      {
	codeset = aliases + strlen (aliases) + 1;
	break;
      }

  /* Don't return an empty string.  GNU libc and GNU libiconv interpret
     the empty string as denoting "the locale's character encoding",
     thus GNU libiconv would call this function a second time.  */
  if (codeset[0] == '\0')
    codeset = "ASCII";

  return codeset;
}
