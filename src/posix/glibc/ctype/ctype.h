/* Copyright (C) 1991,92,93,95,96,97,98,99,2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/*
 *	ISO C99 Standard 7.4: Character handling	<ctype.h>
 */

#ifndef	_CTYPE_H
#define	_CTYPE_H	1

#include <features.h>
#include <bits/types.h>

__BEGIN_DECLS

#ifndef _ISbit
/* These are all the characteristics of characters.
   If there get to be more than 16 distinct characteristics,
   many things must be changed that use `unsigned short int's.

   The characteristics are stored always in network byte order (big
   endian).  We define the bit value interpretations here dependent on the
   machine's byte order.  */

# include <endian.h>
# if __BYTE_ORDER == __BIG_ENDIAN
#  define _ISbit(bit)	(1 << (bit))
# else /* __BYTE_ORDER == __LITTLE_ENDIAN */
#  define _ISbit(bit)	((bit) < 8 ? ((1 << (bit)) << 8) : ((1 << (bit)) >> 8))
# endif

enum
{
  _ISupper = _ISbit (0),	/* UPPERCASE.  */
  _ISlower = _ISbit (1),	/* lowercase.  */
  _ISalpha = _ISbit (2),	/* Alphabetic.  */
  _ISdigit = _ISbit (3),	/* Numeric.  */
  _ISxdigit = _ISbit (4),	/* Hexadecimal numeric.  */
  _ISspace = _ISbit (5),	/* Whitespace.  */
  _ISprint = _ISbit (6),	/* Printing.  */
  _ISgraph = _ISbit (7),	/* Graphical.  */
  _ISblank = _ISbit (8),	/* Blank (usually SPC and TAB).  */
  _IScntrl = _ISbit (9),	/* Control character.  */
  _ISpunct = _ISbit (10),	/* Punctuation.  */
  _ISalnum = _ISbit (11)	/* Alphanumeric.  */
};
#endif /* ! _ISbit  */

/* These are defined in ctype-info.c.
   The declarations here must match those in localeinfo.h.

   These point into arrays of 384, so they can be indexed by any `unsigned
   char' value [0,255]; by EOF (-1); or by any `signed char' value
   [-128,-1).  ISO C requires that the ctype functions work for `unsigned
   char' values and for EOF; we also support negative `signed char' values
   for broken old programs.  The case conversion arrays are of `int's
   rather than `unsigned char's because tolower (EOF) must be EOF, which
   doesn't fit into an `unsigned char'.  But today more important is that
   the arrays are also used for multi-byte character sets.  */
extern __const unsigned short int *__ctype_b;	/* Characteristics.  */
extern __const __int32_t *__ctype_tolower; /* Case conversions.  */
extern __const __int32_t *__ctype_toupper; /* Case conversions.  */

#define	__isctype(c, type) \
  (__ctype_b[(int) (c)] & (unsigned short int) type)

#define	__isascii(c)	(((c) & ~0x7f) == 0)	/* If C is a 7 bit value.  */
#define	__toascii(c)	((c) & 0x7f)		/* Mask off high bits.  */

#define	__exctype(name)	extern int name (int) __THROW

/* The following names are all functions:
     int isCHARACTERISTIC(int c);
   which return nonzero iff C has CHARACTERISTIC.
   For the meaning of the characteristic names, see the `enum' above.  */
__exctype (isalnum);
__exctype (isalpha);
__exctype (iscntrl);
__exctype (isdigit);
__exctype (islower);
__exctype (isgraph);
__exctype (isprint);
__exctype (ispunct);
__exctype (isspace);
__exctype (isupper);
__exctype (isxdigit);

#ifdef	__USE_ISOC99
__exctype (isblank);
#endif


/* Return the lowercase version of C.  */
extern int tolower (int __c) __THROW;

/* Return the uppercase version of C.  */
extern int toupper (int __c) __THROW;


#if defined __USE_SVID || defined __USE_MISC || defined __USE_XOPEN

/* Return nonzero iff C is in the ASCII set
   (i.e., is no more than 7 bits wide).  */
extern int isascii (int __c) __THROW;

/* Return the part of C that is in the ASCII set
   (i.e., the low-order 7 bits of C).  */
extern int toascii (int __c) __THROW;

/* These are the same as `toupper' and `tolower' except that they do not
   check the argument for being in the range of a `char'.  */
__exctype (_toupper);
__exctype (_tolower);
#endif /* Use SVID or use misc.  */

/* This code is needed for the optimized mapping functions.  */
#define __tobody(c, f, a, args) \
  (__extension__							      \
   ({ int __res;							      \
      if (sizeof (c) > 1)						      \
	{								      \
	  if (__builtin_constant_p (c))					      \
	    {								      \
	      int __c = (c);						      \
	      __res = __c < -128 || __c > 255 ? __c : a[__c];		      \
	    }								      \
	  else								      \
	    __res = f args;						      \
	}								      \
      else								      \
	__res = a[(int) (c)];						      \
      __res; }))

#ifndef	__NO_CTYPE
# define isalnum(c)	__isctype((c), _ISalnum)
# define isalpha(c)	__isctype((c), _ISalpha)
# define iscntrl(c)	__isctype((c), _IScntrl)
# define isdigit(c)	__isctype((c), _ISdigit)
# define islower(c)	__isctype((c), _ISlower)
# define isgraph(c)	__isctype((c), _ISgraph)
# define isprint(c)	__isctype((c), _ISprint)
# define ispunct(c)	__isctype((c), _ISpunct)
# define isspace(c)	__isctype((c), _ISspace)
# define isupper(c)	__isctype((c), _ISupper)
# define isxdigit(c)	__isctype((c), _ISxdigit)

# ifdef	__USE_ISOC99
#  define isblank(c)	__isctype((c), _ISblank)
# endif

# ifdef __USE_EXTERN_INLINES
__extern_inline int
tolower (int __c) __THROW
{
  return __c >= -128 && __c < 256 ? __ctype_tolower[__c] : __c;
}

__extern_inline int
toupper (int __c) __THROW
{
  return __c >= -128 && __c < 256 ? __ctype_toupper[__c] : __c;
}
# endif

# if __GNUC__ >= 2 && defined __OPTIMIZE__ && !defined __cplusplus
#  define tolower(c) __tobody (c, tolower, __ctype_tolower, (c))
#  define toupper(c) __tobody (c, toupper, __ctype_toupper, (c))
# endif	/* Optimizing gcc */

# if defined __USE_SVID || defined __USE_MISC || defined __USE_XOPEN
#  define isascii(c)	__isascii (c)
#  define toascii(c)	__toascii (c)

#  define _tolower(c)	((int) __ctype_tolower[(int) (c)])
#  define _toupper(c)	((int) __ctype_toupper[(int) (c)])
# endif

#endif /* Not __NO_CTYPE.  */

__END_DECLS

#endif /* ctype.h  */
