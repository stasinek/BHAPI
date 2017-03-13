#include "vfs.h"

BOOLEAN VfsFastIoCheckIfPossible (
              IN PFILE_OBJECT         FileObject,
              IN PLARGE_INTEGER       FileOffset,
              IN ULONG                Length,
              IN BOOLEAN              Wait,
              IN ULONG                LockKey,
              IN BOOLEAN              CheckForReadOperation,
              OUT PIO_STATUS_BLOCK    IoStatus,
              IN PDEVICE_OBJECT       DeviceObject)
{
	return TRUE;
}

BOOLEAN VfsFastIoQueryBasicInfo (
	      IN PFILE_OBJECT             FileObject,
              IN BOOLEAN                  Wait,
              OUT PFILE_BASIC_INFORMATION Buffer,
              OUT PIO_STATUS_BLOCK        IoStatus,
              IN PDEVICE_OBJECT           DeviceObject)
{
	printk("FIXME: %s:%d\n", __FILE__, __LINE__);
	return FALSE;
}

BOOLEAN VfsFastIoQueryStandardInfo (
                IN PFILE_OBJECT                 FileObject,
                IN BOOLEAN                      Wait,
                OUT PFILE_STANDARD_INFORMATION  Buffer,
                OUT PIO_STATUS_BLOCK            IoStatus,
                IN PDEVICE_OBJECT               DeviceObject)
{
	printk("FIXME: %s:%d\n", __FILE__, __LINE__);
	return FALSE;
}

BOOLEAN VfsFastIoQueryNetworkOpenInfo (
     IN PFILE_OBJECT                     FileObject,
     IN BOOLEAN                          Wait,
     OUT PFILE_NETWORK_OPEN_INFORMATION  Buffer,
     OUT PIO_STATUS_BLOCK                IoStatus,
     IN PDEVICE_OBJECT                   DeviceObject)
{
	printk("FIXME: %s:%d\n", __FILE__, __LINE__);
	return FALSE;
}

BOOLEAN
VfsFastIoLock (
           IN PFILE_OBJECT         FileObject,
           IN PLARGE_INTEGER       FileOffset,
           IN PLARGE_INTEGER       Length,
           IN PEPROCESS            Process,
           IN ULONG                Key,
           IN BOOLEAN              FailImmediately,
           IN BOOLEAN              ExclusiveLock,
           OUT PIO_STATUS_BLOCK    IoStatus,
           IN PDEVICE_OBJECT       DeviceObject)
{
	printk("FIXME: %s:%d\n", __FILE__, __LINE__);
	return FALSE;
}




BOOLEAN
VfsFastIoUnlockSingle (
               IN PFILE_OBJECT         FileObject,
               IN PLARGE_INTEGER       FileOffset,
               IN PLARGE_INTEGER       Length,
               IN PEPROCESS            Process,
               IN ULONG                Key,
               OUT PIO_STATUS_BLOCK    IoStatus,
               IN PDEVICE_OBJECT       DeviceObject)
{
	printk("FIXME: %s:%d\n", __FILE__, __LINE__);
	return FALSE;
}


BOOLEAN
VfsFastIoUnlockAll (
            IN PFILE_OBJECT         FileObject,
            IN PEPROCESS            Process,
            OUT PIO_STATUS_BLOCK    IoStatus,
            IN PDEVICE_OBJECT       DeviceObject)
{
	printk("FIXME: %s:%d\n", __FILE__, __LINE__);
	return FALSE;
}


BOOLEAN
VfsFastIoUnlockAllByKey (
             IN PFILE_OBJECT         FileObject,
             IN PEPROCESS            Process,
             IN ULONG                Key,
             OUT PIO_STATUS_BLOCK    IoStatus,
             IN PDEVICE_OBJECT       DeviceObject)
{
	printk("FIXME: %s:%d\n", __FILE__, __LINE__);
	return FALSE;
}

