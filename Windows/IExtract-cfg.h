/* IExtract-cfg.h.  Generated automatically by configure.  */
/* IExtract-cfg.h.in.  Generated automatically from configure.in by autoheader.  */
#ifndef IEXTRACT_CFG_H
#define IEXTRACT_CFG_H

//$Id$

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.



/* acconfig.h
   This file is in the public domain.

   Descriptive text for the C preprocessor macros that
   the distributed Autoconf macros can define.

   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  Although this order
   can split up related entries, it makes it easier to check whether
   a given entry is in the file.

   Leave the following blank line there!!  Autoheader needs it.  */


/* Version number of package */
#define RELEASE "0.5"
#define VERSION RELEASE ".12"


/* Leave that blank line there!!  Autoheader needs it.
   If you're adding to this file, keep in mind:
   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  */



/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
/* #undef CRAY_STACKSEG_END */

/* Define if using `alloca.c'. */
/* #undef C_ALLOCA */

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
#undef ENABLE_NLS

/* Define if you have `alloca', as a function or macro. */
#define HAVE_ALLOCA 1

/* Define if you have <alloca.h> and it should be used (not on Ultrix). */
/* #undef HAVE_ALLOCA_H */

/* Define if you have the <argz.h> header file. */
/* #undef HAVE_ARGZ_H */

/* Define if you have the `dcgettext' function. */
/* #undef HAVE_DCGETTEXT */

/* Define if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define if you have the `feof_unlocked' function. */
/* #undef HAVE_FEOF_UNLOCKED */

/* Define if you have the `fgets_unlocked' function. */
/* #undef HAVE_FGETS_UNLOCKED */

/* Define if you have the `getcwd' function. */
#define HAVE_GETCWD 1

/* Define if you have the `getpagesize' function. */
/* #undef HAVE_GETPAGESIZE */

/* Define if the GNU gettext() function is already present or preinstalled. */
/* #undef HAVE_GETTEXT */

/* Define if you have the iconv() function. */
/* #undef HAVE_ICONV */

/* Define if you have the <inttypes.h> header file. */
/* #undef HAVE_INTTYPES_H */

/* Define if you have <langinfo.h> and nl_langinfo(CODESET). */
/* #undef HAVE_LANGINFO_CODESET */

/* Define if your <locale.h> file defines LC_MESSAGES. */
/* #undef HAVE_LC_MESSAGES */

/* Define if you have the `Attr' library (-lAttr). */
#define HAVE_LIBATTR 1

/* Define if you have the `gmp' library (-lgmp). */
/* #undef HAVE_LIBGMP */

/* Define if you have the `stdc++' library (-lstdc++). */
/* #undef HAVE_LIBSTDC__ */

/* Define if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define if you have the <malloc.h> header file. */
#define HAVE_MALLOC_H 1

/* Define if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define if you have the `mempcpy' function. */
#define HAVE_MEMPCPY 1

/* Define if you have a working `mmap' system call. */
/* #undef HAVE_MMAP */

/* Define if you have the `munmap' function. */
/* #undef HAVE_MUNMAP */

/* Define if you have the <nl_types.h> header file. */
/* #undef HAVE_NL_TYPES_H */

/* Define if you have the `putenv' function. */
#define HAVE_PUTENV 1

/* Define if you have the `setenv' function. */
/* #undef HAVE_SETENV */

/* Define if you have the `setlocale' function. */
#define HAVE_SETLOCALE 1

/* Define if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define if you have the <stdint.h> header file. */
/* #undef HAVE_STDINT_H */

/* Define if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define if you have the `stpcpy' function. */
/* #undef HAVE_STPCPY */

/* Define if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define if you have the `strchr' function. */
/* #undef HAVE_STRCASECMP */

/* Define if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define if you have the <strings.h> header file. */
/* #undef HAVE_STRINGS_H */

/* Define if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define if you have the <sys/param.h> header file. */
/* #undef HAVE_SYS_PARAM_H */

/* Define if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the `tsearch' function. */
/* #undef HAVE_TSEARCH */

/* Define if you have the <unistd.h> header file. */
/* #undef HAVE_UNISTD_H */

/* Define if you have the `__argz_count' function. */
/* #undef HAVE___ARGZ_COUNT */

/* Define if you have the `__argz_next' function. */
/* #undef HAVE___ARGZ_NEXT */

/* Define if you have the `__argz_stringify' function. */
/* #undef HAVE___ARGZ_STRINGIFY */

/* Define as const if the declaration of iconv() needs const. */
/* #undef ICONV_CONST */

/* Name of package */
#define PACKAGE "IExtract"

/* Localedir */
#define LOCALEDIR ""

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
        STACK_DIRECTION > 0 => grows toward higher addresses
        STACK_DIRECTION < 0 => grows toward lower addresses
        STACK_DIRECTION = 0 => direction of growth unknown */
/* #undef STACK_DIRECTION */

/* Define if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define if your processor stores words with the most significant byte first
   (like Motorola and SPARC, unlike Intel and VAX). */
/* #undef WORDS_BIGENDIAN */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define as `__inline' if that's what the C compiler calls it, or to nothing
   if it is not supported. */
/* #undef inline */

/* Define to `long' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define to `unsigned' if <sys/types.h> does not define. */
/* #undef size_t */

#ifdef HAVE_GETTEXT
#  include <libintl.h>
#  define _(String)                           gettext (String)
#  define N_(String)                          gettext_noop (String)
#  define gettext_noop(String)                (String)
#else
#  define _(String)                           (String)
#  define N_(String)                          (String)
#  define ngettext(Str1, Str2, flag)          (((flag) == 1) ? (Str1) : (Str2))
#  define  binddomain(Domain)
#  define bindtextdomain(Package, Directory)
#endif

#endif
