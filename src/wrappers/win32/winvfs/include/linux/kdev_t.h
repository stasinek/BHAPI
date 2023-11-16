#ifndef _LINUX_KDEV_T_H
#define _LINUX_KDEV_T_H

#define NODEV           0

#define kdev_t PDEVICE_OBJECT

#define MINORBITS   8
#define MINORMASK   ((1U << MINORBITS) - 1)

#define MAJOR(dev)   ((unsigned int)((int)(dev) >> MINORBITS))
#define MINOR(dev)   ((unsigned int)((int)(dev) & MINORMASK))

static inline unsigned int kdev_t_to_nr(kdev_t dev) {
	return (MAJOR(dev)<<8) | MINOR(dev);
} 

#define NODEV		0
#define MKDEV(ma,mi)	(((ma) << MINORBITS) | (mi)) 

static inline kdev_t to_kdev_t(int dev)
{
	int major, minor;
#if 0
	major = (dev >> 16);
	if (!major) {
		major = (dev >> 8);
		minor = (dev & 0xff);
	} else
		minor = (dev & 0xffff);
#else
	major = (dev >> 8);
	minor = (dev & 0xff);
#endif
	return (kdev_t) MKDEV(major, minor);
} 

#endif
