#ifndef _LINUX_STRING_H_
#define _LINUX_STRING_H_

#include <linux/types.h>	/* for size_t */
//#include <linux/stddef.h>	/* for NULL */


extern __kernel_size_t strnlen(const char *,__kernel_size_t); 

/**
 * memscan - Find a character in an area of memory.
 * @addr: The memory area
 * @c: The byte to search for
 * @size: The size of the area.
 *
 * returns the address of the first occurrence of @c, or 1 byte past
 * the area if @c is not found
 */
void __inline * memscan(void * addr, int c, size_t size)
{
	unsigned char * p = (unsigned char *) addr;

	while (size) {
		if (*p == c)
			return (void *) p;
		p++;
		size--;
	}
  	return (void *) p;
}

extern char * strtok1(char * s, char * ct);
extern int strnicmp1(const char *s1, const char *s2, size_t len);

#endif /* _LINUX_STRING_H_ */