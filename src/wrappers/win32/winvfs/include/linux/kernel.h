#ifndef _LINUX_KERNEL_H
#define _LINUX_KERNEL_H

#include "todo.h"

#include <stdarg.h>
#include <linux/types.h>

#define INT_MAX         ((int)(~0U>>1)) 

#define	KERN_EMERG	"<0>"	/* system is unusable			*/
#define	KERN_ALERT	"<1>"	/* action must be taken immediately	*/
#define	KERN_CRIT	"<2>"	/* critical conditions			*/
#define	KERN_ERR	"<3>"	/* error conditions			*/
#define	KERN_WARNING	"<4>"	/* warning conditions			*/
#define	KERN_NOTICE	"<5>"	/* normal but significant condition	*/
#define	KERN_INFO	"<6>"	/* informational			*/
#define	KERN_DEBUG	"<7>"	/* debug-level messages			*/

extern unsigned long simple_strtoul(const char *,char **,unsigned int);


#define printk DbgPrint

#define panic(fmt) { printk("########### PANIC at %s:%d ! ", __FILE__, __LINE__); printk(fmt); /*ASSERT(0);*/ }

#define BUG() do { printk("########### kernel BUG at %s:%d!\n", __FILE__, __LINE__); /**(int *)0=0;*/ } while (0)

#define __out_of_line_bug(line) { printk("kernel BUG in header file at line %d\n", line); BUG(); }
#define out_of_line_bug() __out_of_line_bug(__LINE__)

#endif /* _LINUX_KERNEL_H */
