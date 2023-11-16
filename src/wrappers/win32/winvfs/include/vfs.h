/*
	/include/vfs.h - general windows file system structures definition
	some structures are inspired from existing ext2 file system implementations for Windows (see Credits file)
*/


#pragma once


#include "config.h"
#include "linux/fs.h"
#include "linux/slab.h"

// other functions from Linux VFS
asmlinkage long sys_umount(struct super_block *mnt_sb, int flags);
struct super_block *alloc_super(void);
void insert_super(struct super_block *s, struct file_system_type *type);

// names
#define VFS_FS_NAME	L"\\vfs"
#define VFS_DOS_DEVICE L"\\DosDevices\\vfs"

// for __try __finally usage: assert status always declared
#define TRY_RETURN(S)			{status=S;goto try_exit;}
#define CHECK_OUT(cond, S)		{if(cond){status=S;goto try_exit;}}

// flag manipulation
#define FLAG_ON(flag, val)		((BOOLEAN)((((flag)&(val))!=0)))
#define SET_FLAG(flag, val)		((flag)|=(val))
#define CLEAR_FLAG(flag, val)	((flag)&=~(val))

// resource
#define RELEASE(res) (ExReleaseResourceForThreadLite((res), ExGetCurrentResourceThread()))

// alignement
#define	QUAD_ALIGN(val)			((((ULONG)(val))+7)&0xfffffff8)

// identifier used for each winvfs defined data structure
typedef struct vfs_identifier {
	ULONG	type;
	ULONG	size;
};

// all defined data structures
//#define	VFS_NODE_TYPE_OBJECT_NAME			(0xfdecba01)
#define	VFS_NODE_TYPE_CCB					(0xfdecba02)
#define	VFS_NODE_TYPE_FCB					(0xfdecba03)
#define	VFS_NODE_TYPE_LOCKS					(0xfdecba04)
#define	VFS_NODE_TYPE_VCB					(0xfdecba05)
#define	VFS_NODE_TYPE_IRP_CONTEXT			(0xfdecba06)
#define	VFS_NODE_TYPE_GLOBAL_DATA			(0xfdecba07)

// global list of file locks: not to be used until we implement locks
typedef struct vfs_filelocks {
	LIST_ENTRY	granted_locks;
	LIST_ENTRY	pending_locks;
};

// file lock info
typedef struct vfs_lock_info {
	struct vfs_identifier	identifier;
	ULONG					flags;
	PVOID					owner_process;
	LARGE_INTEGER			start_offset;
	LARGE_INTEGER			length;
	LARGE_INTEGER			end_offset;
	ULONG					key;
	BOOLEAN					exclusive_lock;
	PIRP					pending_irp;
	LIST_ENTRY				next;
};

// file lock flags
#define	VFS_BYTE_LOCK_IS_PENDING	0x00000001

// ccb structure
typedef struct vfs_ccb {
	struct vfs_identifier	identifier;
	struct vfs_fcb			*fcb;			// fcb we belong to
	LIST_ENTRY				next;			// next ccb in list of ccbs belonging to this fcb
	PFILE_OBJECT			file_obj;		// file obj representing this open file
	ULONG					flags;			// flags
	LARGE_INTEGER			offset;			// current pointer offest
	UNICODE_STRING			search_pattern;	// search pattern if this file is a dir
	ULONG					user_time;		// maintain time for user specified time
	struct file				*file;			// link to linux vfs file structure representing this open file
};

// ccb flags
#define	VFS_CCB_OPENED_BY_VFS			0x00000001
#define	VFS_CCB_OPENED_FOR_SYNC_ACCESS	0x00000002
#define	VFS_CCB_OPENED_FOR_SEQ_ACCESS	0x00000004
#define	VFS_CCB_CLEANED					0x00000008
#define	VFS_CCB_ACCESSED				0x00000010
#define	VFS_CCB_MODIFIED				0x00000020
#define	VFS_CCB_ACCESS_TIME_SET			0x00000040
#define	VFS_CCB_MODIFY_TIME_SET			0x00000080
#define	VFS_CCB_CREATE_TIME_SET			0x00000100
#define	VFS_CCB_VOLUME_OPEN				0x00000200

// required fcb header
typedef struct vfs_required_fcb {
	FSRTL_COMMON_FCB_HEADER	common_header;
	SECTION_OBJECT_POINTERS	section_object;
	ERESOURCE				main_resource;
	ERESOURCE				paging_resource;
};

// fcb
typedef struct vfs_fcb {
	struct vfs_required_fcb			required_fcb;
	struct vfs_identifier			identifier;
	struct vfs_vcb				*vcb;				// vcb we belong to
	LIST_ENTRY					next;				// next open fcb in vcb
	ULONG						flags;				// flag
	LIST_ENTRY					ccb_list;			// head of ccb list
	SHARE_ACCESS				share_access;		// share access
//	ULONG						lazy_writer_tid;	// lazy writer tid - just a paranoic check
	ULONG						reference_count;	// references to this fcb
	ULONG						handle_count;		// open handles to this file
	LARGE_INTEGER				creation_time;		// some times kept here for easier access
	LARGE_INTEGER				lastaccess_time;
	LARGE_INTEGER				lastwrite_time;
	struct vfs_lock_info		*byte_range_lock;	// locks on this file - not yet implemented
	struct dentry				*dentry;			// dentry representing this file
													// dentry->d_inode should be used instead of
													// the following inode field
	struct inode				*inode;				// inode representing this file
};

// fcb flags
#define	VFS_FCB_IN_INIT						0x00000001
#define	VFS_FCB_IN_TEARDOWN					0x00000002
#define	VFS_FCB_PAGE_FILE					0x00000004
#define	VFS_FCB_DIRECTORY					0x00000008
#define	VFS_FCB_ROOT_DIRECTORY				0x00000018
#define	VFS_FCB_WRITE_THROUGH				0x00000020
#define	VFS_FCB_MAPPED						0x00000040
#define	VFS_FCB_FAST_IO_READ_IN_PROGESS		0x00000080
#define	VFS_FCB_FAST_IO_WRITE_IN_PROGESS	0x00000100
#define	VFS_FCB_DELETE_ON_CLOSE				0x00000200
#define	VFS_FCB_MODIFIED					0x00000400
#define	VFS_FCB_ACCESSED					0x00000800
#define	VFS_FCB_READ_ONLY					0x00001000
#define	VFS_INITIALIZED_MAIN_RESOURCE		0x00002000
#define	VFS_INITIALIZED_PAGING_IO_RESOURCE	0x00004000
#define	VFS_FCB_BLOCKS_INITIALIZED			0x00008000
#define	VFS_FCB_SPECIAL_FILE				0x00010000
#define	VFS_FCB_HIDDEN_FILE					0x00020000
#define	VFS_FCB_NOT_FROM_ZONE				0x80000000

// vcb
typedef struct vfs_vcb {
	FSRTL_COMMON_FCB_HEADER	common_header;
	struct vfs_identifier	identifier;
	SECTION_OBJECT_POINTERS	section_object;
	ERESOURCE				vcb_resource;
	//PFILE_OBJECT			stream_file_obj;		// file_object used to read/write from/to the volume using caching
	LIST_ENTRY				next;					// next mounted vcb
	PVPB					vpb;					// vpb
	ULONG					flags;					// floags
	ULONG					open_count;				// how many open handles for files in this volume
	ULONG					reference_count;		// how many files referenced in this volume
	LIST_ENTRY				fcb_list;				// head of open files list
	LIST_ENTRY				next_notify_irp;		// used for direrctory notification - not really necessary
	KMUTEX					notify_irp_mutex;		// 
	PDEVICE_OBJECT			vcb_device;				// the volume device object - VDO
	PDEVICE_OBJECT			target_device;			// the target device object - TDO - the block device
	//struct vfs_fcb			*root_fcb;				// the root fcb - keep it here - should always be open
	UCHAR					*volume_path;			// volume path
	//LIST_ENTRY			volume_open_ccb_list;	// list of ccbs for the open instances of this volume
	struct super_block		*super_block;
	struct file				*file;			// to keep / always open
};

// vcb flags
#define VFS_VCB_FLAGS_VOLUME_MOUNTED	0x00000001
#define	VFS_VCB_FLAGS_VOLUME_LOCKED		0x00000002
#define	VFS_VCB_FLAGS_BEING_DISMOUNTED	0x00000004
#define	VFS_VCB_FLAGS_SHUTDOWN			0x00000008
#define	VFS_VCB_FLAGS_VOLUME_READ_ONLY	0x00000010
#define	VFS_VCB_FLAGS_VCB_INITIALIZED	0x00000020

// bcb
typedef PVOID PBCB;

// irp context - some info extracted from the irp just to make out life easier
// also this used to save useful context information for later processing o irps
typedef struct vfs_irp_context {
	struct vfs_identifier	identifier;
	ULONG					flags;
	UCHAR					major_function;
	UCHAR					minor_function;
	PIRP					irp;
	PDEVICE_OBJECT			target_device;
	NTSTATUS				saved_exception_code;
	LIST_ENTRY				thread_queue_list;
};

// irp context flags
#define	VFS_IRP_CONTEXT_CAN_BLOCK			0x00000001
#define	VFS_IRP_CONTEXT_WRITE_THROUGH		0x00000002
#define	VFS_IRP_CONTEXT_EXCEPTION			0x00000004
#define	VFS_IRP_CONTEXT_DEFERRED_WRITE		0x00000008
#define	VFS_IRP_CONTEXT_ASYNC_PROCESSING	0x00000010
#define	VFS_IRP_CONTEXT_NOT_TOP_LEVEL		0x00000020
#define	VFS_IRP_CONTEXT_NOT_FROM_ZONE		0x80000000

// thread queue
typedef struct vfs_thread_queue {
	HANDLE		queue_handler_thread;
	LIST_ENTRY	thread_queue_list;
	KSPIN_LOCK	spin_lock;
	KEVENT		queue_event;
};

// global winvfs data
typedef struct vfs_data {
	struct vfs_identifier	identifier;
	ERESOURCE				global_resource;
	PDRIVER_OBJECT			driver;
	PDEVICE_OBJECT			device;
	LIST_ENTRY				vcb_list;
	FAST_IO_DISPATCH		fast_io_dispatch;
	CACHE_MANAGER_CALLBACKS	cache_mgr_callbacks;
	ULONG					flags;
	struct vfs_thread_queue	thread_queue;
};

// global variable defined in init.c
extern struct vfs_data vfs_data;
extern kmem_cache_t *ccb_cachep;
extern kmem_cache_t *fcb_cachep;
extern kmem_cache_t *irp_context_cachep;

// ioctl used to unload the driver
#define	IOCTL_PREPARE_TO_UNLOAD \
		CTL_CODE(FILE_DEVICE_UNKNOWN, 2048, METHOD_NEITHER, FILE_WRITE_ACCESS)

// global flags
#define	VFS_DATA_FLAGS_RESOURCE_INITIALIZED	0x00000001
#define VFS_UNLOAD_PENDING					0x00000002

struct dentry * cached_lookup(struct dentry * parent, struct qstr * name, int flags);
struct dentry * real_lookup(struct dentry * parent, struct qstr * name, int flags);

struct dirent_buffer {
	void * buffer;
	FILE_INFORMATION_CLASS info_class;
	ULONG used_length;
	ULONG query_block_length;
	NTSTATUS status;
	BOOLEAN first_time_query;
	BOOLEAN return_single_entry;
	PUNICODE_STRING search_pattern;
	ULONG buffer_length;
	struct file* file;
	PULONG next_entry_offset;
};

// init.c
NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING reg_path);
void VfsInitializeFunctionPointers(PDRIVER_OBJECT driver);
VOID VfsUnload(PDRIVER_OBJECT driver);

// misc.c
BOOLEAN VfsIsIrpTopLevel(PIRP irp);
ULONG VfsExceptionFilter(struct vfs_irp_context *irp_context, PEXCEPTION_POINTERS ex_ptrs);
NTSTATUS VfsExceptionHandler(struct vfs_irp_context *irp_context, PIRP irp);
void VfsGetFcbCcbVcbFromFileObject(PFILE_OBJECT file_obj, struct vfs_fcb **fcb, struct vfs_ccb **ccb, struct vfs_vcb **vcb);
ULONG VfsAlign(ULONG number, ULONG alignment);
LONGLONG VfsAlign64(LONGLONG number, LONGLONG alignment);
ULONG VfsGetCurrentTime();
void VfsCompleteRequest(PIRP irp, NTSTATUS status);
void* VfsGetCallersBuffer(PIRP irp);
NTSTATUS VfsDummyIrp(PDEVICE_OBJECT dev_obj, PIRP irp);

// cmcb.c
BOOLEAN VfsAcqLazyWrite(PVOID context, BOOLEAN wait);
void VfsRelLazyWrite(PVOID context);
BOOLEAN VfsAcqReadAhead(PVOID context, BOOLEAN wait);
void VfsRelReadAhead(PVOID context);

// alloc.c
struct vfs_obj_name* VfsAllocObjName();
void VfsFreeObjName(struct vfs_obj_name *obj_name);
struct vfs_ccb* VfsAllocCcb();
void VfsCloseAndFreeCcb(struct vfs_ccb *ccb);
struct vfs_fcb* VfsAllocFcb();
void VfsFreeFcb(struct vfs_fcb* fcb);
struct vfs_irp_context* VfsAllocIrpContext(PIRP irp, PDEVICE_OBJECT target_device);
void VfsFreeIrpContext(struct vfs_irp_context *irp_context);
NTSTATUS VfsCreateFcb(struct vfs_fcb **new_fcb, LARGE_INTEGER alloc_size, LARGE_INTEGER end_of_file, PFILE_OBJECT file_obj,
					  struct vfs_vcb* vcb, struct dentry *dentry);
void VfsInitializeVcb(PDEVICE_OBJECT volume_dev, PDEVICE_OBJECT target_dev, PVPB vpb, PLARGE_INTEGER alloc_size);
NTSTATUS VfsCreateNewCcb(struct vfs_ccb **new_ccb, struct vfs_fcb *fcb, PFILE_OBJECT file_obj);
struct super_block* VfsAllocSuperBlock(PVPB vpb, PDEVICE_OBJECT block_device);
void VfsFreeVcb(struct vfs_vcb *vcb);

// char.c
void VfsCopyUnicodeString(PUNICODE_STRING dest, PUNICODE_STRING src);
//void VfsCopyWcharToUnicodeString(PUNICODE_STRING dest, PCWSTR src);
//void VfsCopyCharToUnicodeString(PUNICODE_STRING dest, PCSTR src, USHORT len);
//void VfsCopyZcharToUnicodeString(PUNICODE_STRING dest, PCSTR src);
//void VfsZerooutUnicodeString(PUNICODE_STRING ustr);
void VfsFreeUnicodeString(PUNICODE_STRING ustr);
PSTR VfsCopyUnicodeStringToZcharUnixPath(PUNICODE_STRING src);
VOID VfsCharToWchar(PWCHAR Destination, PCHAR Source, ULONG Length);

// async.c
void VfsCommonDispatch(void *context);
void VfsQueueHandlerThread(PVOID context);
NTSTATUS VfsPostRequest(struct vfs_irp_context *irp_context, PIRP irp);
NTSTATUS VfsPostRequestCreateContext(PDEVICE_OBJECT device, PIRP irp);

// devctrl.c
NTSTATUS VfsDeviceControl(PDEVICE_OBJECT device, PIRP irp);
NTSTATUS VfsCommonDeviceControl(struct vfs_irp_context *irp_context, PIRP irp);
NTSTATUS VfsPrepareToUnload(struct vfs_irp_context *irp_context, PIRP irp);
NTSTATUS VfsIoctlCompletion(PDEVICE_OBJECT device, PIRP irp, PVOID context);

// create.c
NTSTATUS VfsCreate(PDEVICE_OBJECT device, PIRP irp);
NTSTATUS VfsCommonCreate(struct vfs_irp_context *irp_context, PIRP irp, BOOLEAN first_attempt);
NTSTATUS VfsOpenVolume(struct vfs_vcb *vcb, struct vfs_irp_context *irp_context, PIRP irp, USHORT share_access,
					   PIO_SECURITY_CONTEXT security_context, PFILE_OBJECT new_file_obj);
struct vfs_fcb* VfsLocateFcbInCore(struct vfs_vcb *vcb, ULONG inode_no);
NTSTATUS VfsOpenRootDirectory(struct vfs_vcb *vcb, struct vfs_irp_context *irp_context, PIRP irp, USHORT share_access,
							  PIO_SECURITY_CONTEXT security_context, PFILE_OBJECT new_file_obj);

// fsctrl.c
NTSTATUS VfsFileSystemControl(PDEVICE_OBJECT device, PIRP irp);
NTSTATUS VfsMountVolume(PIRP irp, PIO_STACK_LOCATION stack_location);
NTSTATUS VfsUserFileSystemRequest(PIRP irp, PIO_STACK_LOCATION stack_location);
NTSTATUS VfsLockVolume(PIRP irp, PIO_STACK_LOCATION stack_location);
void VfsPurgeVolume(struct vfs_vcb *vcb, BOOLEAN flush_before_purge);
void VfsPurgeFile(struct vfs_fcb *fcb, BOOLEAN flush_before_purge);
void VfsSetVpbFlag(PVPB vpb, USHORT flag);
void VfsClearVpbFlag(PVPB vpb, USHORT flag);
NTSTATUS VfsUnlockVolume(PIRP irp, PIO_STACK_LOCATION stack_location);
NTSTATUS VfsDismountVolume(PIRP irp, PIO_STACK_LOCATION stack_location);
NTSTATUS VfsIsVolumeMounted(PIRP irp, PIO_STACK_LOCATION stack_location);
NTSTATUS VfsVerifyVolume(PIRP irp, PIO_STACK_LOCATION stack_location);

// diskio.c
NTSTATUS VfsReadPhysicalBlocks(PDEVICE_OBJECT block_dev, PVOID buffer, LARGE_INTEGER start_block, ULONG blk_cnt);
NTSTATUS VfsWritePhysicalBlocks(PDEVICE_OBJECT block_dev, PVOID buffer, LARGE_INTEGER start_block, ULONG blk_cnt);

// close.c
NTSTATUS VfsClose(PDEVICE_OBJECT device, PIRP irp);
NTSTATUS VfsCommonClose(struct vfs_irp_context *irp_context, PIRP irp, BOOLEAN first_attempt);

// cleanup.c
NTSTATUS VfsCleanup(PDEVICE_OBJECT device, PIRP irp);
NTSTATUS VfsCommonCleanup(struct vfs_irp_context *irp_context, PIRP irp, BOOLEAN first_attempt);

// debug.c
extern ULONG ProcessNameOffset;
#define VfsGetCurrentProcessName() ((PUCHAR)PsGetCurrentProcess() + ProcessNameOffset)

ULONG VfsGetProcessNameOffset(VOID);
PUCHAR VfsNtStatusToString(IN NTSTATUS Status);

// dirctrl.c
NTSTATUS VfsDirectoryControl(PDEVICE_OBJECT dev_obj, PIRP irp);
NTSTATUS VfsCommonDirectoryControl(struct vfs_irp_context *irp_context, PIRP irp);
NTSTATUS VfsQueryDirectory(struct vfs_irp_context *irp_context, PIRP irp, PIO_STACK_LOCATION stack_location,
			   PFILE_OBJECT file_obj, struct vfs_fcb *fcb, struct vfs_ccb *ccb);

// read.c
NTSTATUS VfsRead(PDEVICE_OBJECT device, PIRP irp);
NTSTATUS VfsReadNormal(PDEVICE_OBJECT device, PIRP Irp);

// fileinfo.c
NTSTATUS VfsQueryInformation(PDEVICE_OBJECT device ,PIRP irp);

// volinfo.c
NTSTATUS VfsVolumeInformation(PDEVICE_OBJECT device, PIRP irp);
NTSTATUS VfsSetInformation(PDEVICE_OBJECT device ,PIRP irp);

// for load/unload of ported vfs linux module
void kernel_entry();
void kernel_unload();
void super_entry();
void super_unload();
void inode_entry();
void inode_unload();
void dcache_entry();
void dcache_unload();
void register_all_filesystems();
void unregister_all_filesystems();

extern struct file_system_type *file_systems;

extern int fs_init_module(void);
extern void fs_cleanup_module(void); 

// fcb, vcb count modifiers
void fcb_inc_ref_count(struct vfs_fcb *fcb);
void fcb_dec_ref_count(struct vfs_fcb *fcb);
void vcb_inc_ref_count(struct vfs_vcb *vcb);
void vcb_dec_ref_count(struct vfs_vcb *vcb);

BOOLEAN VfsFastIoCheckIfPossible (
              IN PFILE_OBJECT         FileObject,
              IN PLARGE_INTEGER       FileOffset,
              IN ULONG                Length,
              IN BOOLEAN              Wait,
              IN ULONG                LockKey,
              IN BOOLEAN              CheckForReadOperation,
              OUT PIO_STATUS_BLOCK    IoStatus,
              IN PDEVICE_OBJECT       DeviceObject);

BOOLEAN VfsFastIoQueryBasicInfo (
	      IN PFILE_OBJECT             FileObject,
              IN BOOLEAN                  Wait,
              OUT PFILE_BASIC_INFORMATION Buffer,
              OUT PIO_STATUS_BLOCK        IoStatus,
              IN PDEVICE_OBJECT           DeviceObject);

BOOLEAN VfsFastIoQueryStandardInfo (
                IN PFILE_OBJECT                 FileObject,
                IN BOOLEAN                      Wait,
                OUT PFILE_STANDARD_INFORMATION  Buffer,
                OUT PIO_STATUS_BLOCK            IoStatus,
                IN PDEVICE_OBJECT               DeviceObject);

BOOLEAN VfsFastIoQueryNetworkOpenInfo (
     IN PFILE_OBJECT                     FileObject,
     IN BOOLEAN                          Wait,
     OUT PFILE_NETWORK_OPEN_INFORMATION  Buffer,
     OUT PIO_STATUS_BLOCK                IoStatus,
     IN PDEVICE_OBJECT                   DeviceObject);

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
           IN PDEVICE_OBJECT       DeviceObject);

BOOLEAN VfsFastIoLock (
           IN PFILE_OBJECT         FileObject,
           IN PLARGE_INTEGER       FileOffset,
           IN PLARGE_INTEGER       Length,
           IN PEPROCESS            Process,
           IN ULONG                Key,
           IN BOOLEAN              FailImmediately,
           IN BOOLEAN              ExclusiveLock,
           OUT PIO_STATUS_BLOCK    IoStatus,
           IN PDEVICE_OBJECT       DeviceObject);


BOOLEAN
VfsFastIoUnlockSingle (
               IN PFILE_OBJECT         FileObject,
               IN PLARGE_INTEGER       FileOffset,
               IN PLARGE_INTEGER       Length,
               IN PEPROCESS            Process,
               IN ULONG                Key,
               OUT PIO_STATUS_BLOCK    IoStatus,
               IN PDEVICE_OBJECT       DeviceObject);

BOOLEAN
VfsFastIoUnlockAll (
            IN PFILE_OBJECT         FileObject,
            IN PEPROCESS            Process,
            OUT PIO_STATUS_BLOCK    IoStatus,
            IN PDEVICE_OBJECT       DeviceObject);

BOOLEAN
VfsFastIoUnlockAllByKey (
             IN PFILE_OBJECT         FileObject,
             IN PEPROCESS            Process,
             IN ULONG                Key,
             OUT PIO_STATUS_BLOCK    IoStatus,
             IN PDEVICE_OBJECT       DeviceObject);



//#if DBG && IRP_DEBUG
#if IRP_DEBUG
VOID VfsDbgPrintCall(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
VOID VfsDbgPrintComplete(IN PIRP Irp);
#else
#define VfsDbgPrintCall(DeviceObject, Irp)
#define VfsDbgPrintComplete(Irp)
#endif