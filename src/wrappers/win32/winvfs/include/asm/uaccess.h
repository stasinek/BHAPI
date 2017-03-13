#ifndef __i386_UACCESS_H
#define __i386_UACCESS_H

#include <linux/sched.h>

#define copy_to_user memcpy
#define copy_from_user memcpy
#define put_user(src, dest) memcpy(dest, &(src), sizeof(src))
#define put_user_const(src, dest) *(dest) = src

//extern unsigned long copy_to_user(void *, const void *, unsigned long);
//extern unsigned long copy_from_user(void *, const void *, unsigned long);


#define VERIFY_READ 0
#define VERIFY_WRITE 1 

#define get_user(x, ptr) ((x) = *(ptr))
static __inline int verify_area(int type, const void * addr, unsigned long size)
{
	return 0;
}

#endif /* __i386_UACCESS_H */


