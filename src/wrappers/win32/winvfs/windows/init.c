#include "vfs.h"

//	global winvfs data structure
struct vfs_data vfs_data;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, VfsInitializeFunctionPointers)
#endif

// CCB
kmem_cache_t *ccb_cachep;
// FCB
kmem_cache_t *fcb_cachep;
// IRP context
kmem_cache_t *irp_context_cachep;

void init_extra_kmem_caches()
{
	ccb_cachep = kmem_cache_create("ccb_cache",
			QUAD_ALIGN(sizeof(struct vfs_ccb)), 0,
			SLAB_HWCACHE_ALIGN, NULL, NULL);
	if(!ccb_cachep)
		panic("Cannot create CCB SLAB cache");

	fcb_cachep = kmem_cache_create("fcb_cache", 
			QUAD_ALIGN(sizeof(struct vfs_fcb)), 0,
			SLAB_HWCACHE_ALIGN, NULL, NULL);
	if (!fcb_cachep)
		panic("Cannot create FCB SLAB cache");

	irp_context_cachep = kmem_cache_create("irp_con_cache", 
			QUAD_ALIGN(sizeof(struct vfs_irp_context)), 0,
			SLAB_HWCACHE_ALIGN, NULL, NULL);
	if (!irp_context_cachep)
		panic("Cannot create FCB SLAB cache");
}

/*
 *	initialize vfs ported from linux
 */
void init_linux_vfs()
{
	// initialize linux kernel
	kernel_entry();
	// initialize vfs core - super block processing
	super_entry();
	// initialize inodes cache - should be added to vfs caches
	inode_entry();
	// initialize vfs caches
	//vfs_caches_init(32768);
	vfs_caches_init(4096);
}

void vfs_purge_all_caches()
{
	kmem_cache_destroy(bh_cachep);
	kmem_cache_destroy(names_cachep);
	kmem_cache_destroy(filp_cachep);
#if defined (CONFIG_QUOTA)
	kmem_cache_destroy(dquot_cachep);
#endif

	//winvfs specific
	kmem_cache_destroy(ccb_cachep);
	kmem_cache_destroy(fcb_cachep);
	kmem_cache_destroy(irp_context_cachep);
}

/*
 *	cleanup for vfs ported from linux
*/
void cleanup_linux_vfs()
{
	vfs_purge_all_caches();
	inode_unload();
	dcache_unload();
	super_unload();
	kernel_unload();
}

/*
 *	winvfs entry point
 */
NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING reg_path)
{
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING device_name;
	UNICODE_STRING dos_name;
	TIME_FIELDS time_fields;

	__try 
	{
		__try
		{
			// 1. allocate memory for global data structures and initialize them - nothing to allocate
			
			// zero winvfs global data
			RtlZeroMemory(&vfs_data, sizeof(vfs_data));
			vfs_data.identifier.type = VFS_NODE_TYPE_GLOBAL_DATA;
			vfs_data.identifier.size = sizeof(vfs_data);

			// global sync resource
			status = ExInitializeResourceLite(&(vfs_data.global_resource));
			ASSERT(NT_SUCCESS(status));
			SET_FLAG(vfs_data.flags, VFS_DATA_FLAGS_RESOURCE_INITIALIZED);

			// fs driver object
			vfs_data.driver = driver;

			// init mounted vcb list
			InitializeListHead(&(vfs_data.vcb_list));

			// thread queue spin
			KeInitializeSpinLock(&vfs_data.thread_queue.spin_lock);

			// thread queue for async irp processing
			InitializeListHead(&vfs_data.thread_queue.thread_queue_list);

			// 2. read registry data - maybe later for some tuning parameters

			// 3. create the FS device
			RtlInitUnicodeString(&device_name, VFS_FS_NAME);
			status = IoCreateDevice(driver, 0, &device_name, FILE_DEVICE_DISK_FILE_SYSTEM,
				0, FALSE, &(vfs_data.device));
			CHECK_OUT(!NT_SUCCESS(status), status);

			// 4. init irp entries
			// 5. init fastio path and cache mgr callbacks
			VfsInitializeFunctionPointers(driver);

			// 6. timer objects and dpc objects required - none

			// 7. initiate asyncronous initialization
			PsCreateSystemThread(&vfs_data.thread_queue.queue_handler_thread, (ACCESS_MASK)0L,
				NULL, NULL, NULL, VfsQueueHandlerThread, NULL);
			
			// event used for async irp processing in kernel thread
			KeInitializeEvent(&vfs_data.thread_queue.queue_event, SynchronizationEvent, FALSE);

			// 8. register the file system - physical-media based
			// create visible link to the fs device for unloading (only for dbg purposes)
			RtlInitUnicodeString(&dos_name, VFS_DOS_DEVICE);
			IoCreateSymbolicLink(&dos_name, &device_name);

			// register the winvfs fs
			IoRegisterFileSystem(vfs_data.device);

			// init linux VFS
			init_linux_vfs();
			// init extra kmem_caches for windows structures
			init_extra_kmem_caches();
			// init/register filesystem
			fs_init_module();
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			status = GetExceptionCode();
		}
try_exit:
		;
	}
	__finally
	{
		// cleanup
		if (!NT_SUCCESS(status)) {
			if (vfs_data.device) {
				IoDeleteDevice(vfs_data.device);
				vfs_data.device = NULL;
			}
			if (FLAG_ON(vfs_data.flags, VFS_DATA_FLAGS_RESOURCE_INITIALIZED)) {
				ExDeleteResourceLite(&(vfs_data.global_resource));
				CLEAR_FLAG(vfs_data.flags, VFS_DATA_FLAGS_RESOURCE_INITIALIZED);
			}
		}
	}

	DbgPrint("WinVFS loaded succesfully\n");
	return status;
}

/*
 *	define irp and fastio entry points + cache mgr callbacks + unload function
 */
void VfsInitializeFunctionPointers(PDRIVER_OBJECT driver)
{
	static FAST_IO_DISPATCH fiod;
	

	driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = VfsDummyIrp;//VfsDeviceControl;

	driver->MajorFunction[IRP_MJ_CREATE] = VfsCreate;
	driver->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = VfsFileSystemControl;
	driver->MajorFunction[IRP_MJ_CLOSE]	= VfsClose;
	driver->MajorFunction[IRP_MJ_CLEANUP] = VfsCleanup;
	driver->MajorFunction[IRP_MJ_DIRECTORY_CONTROL] = VfsDirectoryControl;
	driver->MajorFunction[IRP_MJ_READ] = VfsRead;
	driver->MajorFunction[IRP_MJ_QUERY_INFORMATION] = VfsQueryInformation;
	driver->MajorFunction[IRP_MJ_SET_INFORMATION] = VfsSetInformation;
	driver->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION] = VfsVolumeInformation;

	driver->MajorFunction[IRP_MJ_WRITE] = VfsDummyIrp;
	driver->MajorFunction[IRP_MJ_FLUSH_BUFFERS] = VfsDummyIrp;
	driver->MajorFunction[IRP_MJ_SET_VOLUME_INFORMATION] = VfsDummyIrp;
	driver->MajorFunction[IRP_MJ_SHUTDOWN] = VfsDummyIrp;
	driver->MajorFunction[IRP_MJ_LOCK_CONTROL] = VfsDummyIrp;
	driver->MajorFunction[IRP_MJ_QUERY_SECURITY] = VfsDummyIrp;
	driver->MajorFunction[IRP_MJ_SET_SECURITY] = VfsDummyIrp;
	driver->MajorFunction[IRP_MJ_QUERY_EA] = VfsDummyIrp;
	driver->MajorFunction[IRP_MJ_SET_EA] = VfsDummyIrp;

	driver->DriverUnload = VfsUnload;

	fiod.SizeOfFastIoDispatch=sizeof(FAST_IO_DISPATCH);
	fiod.FastIoCheckIfPossible=VfsFastIoCheckIfPossible;
	fiod.FastIoRead=FsRtlCopyRead;
	fiod.FastIoWrite=FsRtlCopyWrite;
	fiod.FastIoQueryBasicInfo=VfsFastIoQueryBasicInfo;
	fiod.FastIoQueryStandardInfo=VfsFastIoQueryStandardInfo;
	fiod.FastIoLock=VfsFastIoLock;
	fiod.FastIoUnlockSingle=VfsFastIoUnlockSingle;
	fiod.FastIoUnlockAll=VfsFastIoUnlockAll;
	fiod.FastIoUnlockAllByKey=VfsFastIoUnlockAllByKey;
	fiod.FastIoQueryNetworkOpenInfo=VfsFastIoQueryNetworkOpenInfo;
	driver->FastIoDispatch = &fiod;

	vfs_data.cache_mgr_callbacks.AcquireForLazyWrite = VfsAcqLazyWrite;
	vfs_data.cache_mgr_callbacks.ReleaseFromLazyWrite = VfsRelLazyWrite;
	vfs_data.cache_mgr_callbacks.AcquireForReadAhead = VfsAcqReadAhead;
	vfs_data.cache_mgr_callbacks.ReleaseFromReadAhead = VfsRelReadAhead;
}

/*
 *	unload winvfs
 */
VOID VfsUnload(PDRIVER_OBJECT driver)
{
	UNICODE_STRING dos_name;

	DbgPrint("Doing UNLOAD\n");

	// deregister filesystem
	fs_cleanup_module();
	// cleanup linux VFS
	cleanup_linux_vfs();

	RtlInitUnicodeString(&dos_name, VFS_DOS_DEVICE);
	IoDeleteSymbolicLink(&dos_name); 

	ExDeleteResourceLite(&vfs_data.global_resource);
	ZwClose(vfs_data.thread_queue.queue_handler_thread);
	DbgPrint("WinVFS unloaded succesfully\n");
}
