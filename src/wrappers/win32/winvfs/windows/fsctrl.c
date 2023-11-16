#include "vfs.h"
#include <asm/current.h>
#include <linux/ext2_fs.h>

//
//	IRP_MJ_FILE_SYSTEM_CONTROL
//
NTSTATUS VfsFileSystemControl(PDEVICE_OBJECT device, PIRP irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack_location = NULL;

	VfsDbgPrintCall(device, irp);

	FsRtlEnterFileSystem();

	ASSERT(device);
	ASSERT(irp);

	stack_location = IoGetCurrentIrpStackLocation(irp);

	switch (stack_location->MinorFunction) {
	case IRP_MN_MOUNT_VOLUME:
		status = VfsMountVolume(irp, stack_location);
		VfsCompleteRequest(irp, status);
		break;
	case IRP_MN_USER_FS_REQUEST:
		status = VfsUserFileSystemRequest(irp, stack_location);
		//status = STATUS_INVALID_DEVICE_REQUEST;
		VfsDbgPrintComplete(irp);
		VfsCompleteRequest(irp, status);
		break;
	case IRP_MN_VERIFY_VOLUME:
		status = VfsVerifyVolume(irp, stack_location);
		VfsDbgPrintComplete(irp);
		VfsCompleteRequest(irp, status);
		break;
	case IRP_MN_LOAD_FILE_SYSTEM:
	default:
		VfsCompleteRequest(irp, STATUS_INVALID_DEVICE_REQUEST);
		break;
	}

	FsRtlExitFileSystem();

	return status;
}

//
//	IRP_MN_MOUNT_VOLUME
//
NTSTATUS VfsMountVolume(PIRP irp, PIO_STACK_LOCATION stack_location)
{
	PVPB vpb = NULL;
	PDEVICE_OBJECT target_device = NULL;
	PDEVICE_OBJECT volume_device = NULL;
	NTSTATUS status = STATUS_UNRECOGNIZED_VOLUME;
	struct super_block *sb = NULL;
	struct vfs_vcb *vcb = NULL;
	PFILE_OBJECT *root_file_obj;
	BOOLEAN cleared_verify_bit = FALSE;
	struct file_system_type *fs;
	int i;
	LARGE_INTEGER AllocationSize;

	vpb = stack_location->Parameters.MountVolume.Vpb;
	target_device = stack_location->Parameters.MountVolume.DeviceObject;

	__try
	{
		// never verify the volume - i don't understand why this is needed anyway
		if (FLAG_ON(vpb->RealDevice->Flags, DO_VERIFY_VOLUME)) {
			CLEAR_FLAG(vpb->RealDevice->Flags, DO_VERIFY_VOLUME);
			cleared_verify_bit = TRUE;
		}

		// loop registred file systems loop to find a file system ready to mount this volume
		for (fs=file_systems; fs; fs=fs->next) {
			// alloc a new super block
			sb = VfsAllocSuperBlock(vpb, target_device);	// todo: free super block at umount
			// - also this one should have a coresponding linux function somewhere in super.c
			ASSERT(sb);
			if (fs->read_super(sb, NULL, 0)) {
				insert_super(sb, fs);
				break;	// cool - volume mounted
			}
			else {
				ExFreePool(sb);
				sb = NULL;
			}
		}

		CHECK_OUT(!sb, STATUS_UNRECOGNIZED_VOLUME);

		// create the VDO
		status = IoCreateDevice(vfs_data.driver, QUAD_ALIGN(sizeof(struct vfs_vcb)), NULL,
			FILE_DEVICE_DISK_FILE_SYSTEM, 0, FALSE, &volume_device);
		CHECK_OUT(!NT_SUCCESS(status), status);

		// i don't know why this is needend
		if (target_device->AlignmentRequirement > volume_device->AlignmentRequirement)
			volume_device->AlignmentRequirement = target_device->AlignmentRequirement;
		
		// mark device as initialized
		CLEAR_FLAG(volume_device->Flags, DO_DEVICE_INITIALIZING);

		// set stack size - i believe it's ok as long as it's greater then the one below us
		volume_device->StackSize = (CCHAR)(target_device->StackSize+1);

		// put some info in the vpb so the io mgr knows it's mounted
		vpb->DeviceObject = volume_device;
		// temp - todo: get real device name;
		// hack, pentru ext2 --sadyc
		if (sb->s_magic == EXT2_SUPER_MAGIC) {
			int size = MAXIMUM_VOLUME_LABEL_LENGTH / sizeof(WCHAR);
			if (size > 16) {
				VfsCharToWchar(vpb->VolumeLabel, sb->u.ext2_sb.s_es->s_volume_name, 16);
				vpb->VolumeLabel[16] = 0;
				vpb->VolumeLabelLength = 34;
			}
			else {
				VfsCharToWchar(vpb->VolumeLabel, sb->u.ext2_sb.s_es->s_volume_name, size);
				vpb->VolumeLabel[size-1] = 0;
				vpb->VolumeLabelLength = size*2;
			}
		}
		else {
			#define UNKNOWN_LABEL "unknown_label"
			VfsCharToWchar(vpb->VolumeLabel, UNKNOWN_LABEL , sizeof(UNKNOWN_LABEL));
			vpb->VolumeLabel[sizeof(UNKNOWN_LABEL)] = 0;
			vpb->VolumeLabelLength = sizeof(UNKNOWN_LABEL)*2;
		}
		vpb->SerialNumber = (ULONG)sb->s_magic;

		AllocationSize.QuadPart = sb->s_blocksize * sb->s_count;

		// init the vcb
		VfsInitializeVcb(volume_device, target_device, vpb, &AllocationSize);

		vcb = (struct vfs_vcb*)(volume_device->DeviceExtension);
		vcb->super_block = sb;
		// we always keep / open to prevent some ugly crashes
		//vcb->file = filp_open(vcb->super_block->s_root, "/",  0, FMODE_READ);

		// init current->fs
//		current->fs->root = sb->s_root; no need anymore --sadyc

		status = STATUS_SUCCESS;
try_exit:
		;
	}
	__finally
	{
	}

	return status;
}

//
//	IRP_MN_USER_REQUEST
//
NTSTATUS VfsUserFileSystemRequest(PIRP irp, PIO_STACK_LOCATION stack_location)
{
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
	ULONG fs_ctrl_code = 0;
#ifdef _GNU_NTIFS_
	PEXTENDED_IO_STACK_LOCATION stack_location_ex = (PEXTENDED_IO_STACK_LOCATION) stack_location;
#else
	PIO_STACK_LOCATION stack_location_ex = stack_location;
#endif

	fs_ctrl_code = stack_location_ex->Parameters.FileSystemControl.FsControlCode;
	switch (fs_ctrl_code) {
	case FSCTL_LOCK_VOLUME:
		status = VfsLockVolume(irp, stack_location);
		break;
	case FSCTL_UNLOCK_VOLUME:
		status = VfsUnlockVolume(irp, stack_location);
		break;
	case FSCTL_DISMOUNT_VOLUME:
		status = VfsDismountVolume(irp, stack_location);
		break;
	case FSCTL_IS_VOLUME_MOUNTED:
		status = VfsIsVolumeMounted(irp, stack_location);
		break;
	default:
		;
	}
	return status;
}

//
//	FSCTL_LOCK_VOLUME
//
NTSTATUS VfsLockVolume(PIRP irp, PIO_STACK_LOCATION stack_location)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PDEVICE_OBJECT device = NULL;
	struct vfs_vcb* vcb;
	BOOLEAN notified = FALSE;
	BOOLEAN resource_acquired = FALSE;
	PFILE_OBJECT file_obj = NULL;

	__try {
		// request to fs device not permited
		device = stack_location->DeviceObject;
		CHECK_OUT(device == vfs_data.device, STATUS_INVALID_DEVICE_REQUEST);

		// get vcb
		vcb = (struct vfs_vcb*)device->DeviceExtension;
		ASSERT(vcb);

		// file object - should be the file object for a volume open, even so we accept it for any open file
		file_obj = stack_location->FileObject;
		ASSERT(file_obj);

		// notify volume locked
		FsRtlNotifyVolumeEvent(file_obj, FSRTL_VOLUME_LOCK);
		notified = TRUE;

		// acquire vcb lock
		ExAcquireResourceSharedLite(&vcb->vcb_resource, TRUE);
		resource_acquired = TRUE;
		
		// check lock flag
		if (FLAG_ON(vcb->flags, VFS_VCB_FLAGS_VOLUME_LOCKED)) {
			DbgPrint("*** Volume is already locked ***\n");
			TRY_RETURN(STATUS_ACCESS_DENIED);
		}

		// abort if open files still exist
		if (vcb->open_count) {
			DbgPrint(": *** Open files still exist ***\n");
			TRY_RETURN(STATUS_ACCESS_DENIED);
		}

		// release lock
		RELEASE(&vcb->vcb_resource);
		resource_acquired = FALSE;
		
		// purge volume
		VfsPurgeVolume(vcb, TRUE);

		// if there are still open referneced we can't lock the volume
		if (vcb->reference_count > 1) {
			DbgPrint("*** could not purge cached files ***\n");
			TRY_RETURN(STATUS_ACCESS_DENIED);
		}

		ExAcquireResourceExclusiveLite(&vcb->vcb_resource, TRUE);
		resource_acquired = TRUE;

		// set flag in both vcb and vpb structures
		SET_FLAG(vcb->flags, VFS_VCB_FLAGS_VOLUME_LOCKED);
		VfsSetVpbFlag(vcb->vpb, VFS_VCB_FLAGS_VOLUME_LOCKED);
		DbgPrint("*** Volume LOCKED ***\n");
		status = STATUS_SUCCESS;
try_exit:
		;
	}
	__finally {
		if (resource_acquired)
			RELEASE(&vcb->vcb_resource);
		if (!NT_SUCCESS(status) && notified)
			FsRtlNotifyVolumeEvent(file_obj, FSRTL_VOLUME_LOCK_FAILED);
	}

	return status;
}

typedef struct _FCB_LIST_ENTRY {
	struct vfs_fcb *fcb;
	LIST_ENTRY next;
};

//
//	purges any files that are still referenced, most probably by the cache mgr
//
void VfsPurgeVolume(struct vfs_vcb *vcb, BOOLEAN flush_before_purge)
{
	BOOLEAN vcb_acquired = FALSE;
	struct vfs_fcb* fcb = NULL;
	LIST_ENTRY fcb_list;
	PLIST_ENTRY entry = NULL;
	struct _FCB_LIST_ENTRY *fcb_list_entry;

	__try {
		ASSERT(vcb);
		// acquire vcb resource
		ExAcquireResourceSharedLite(&vcb->vcb_resource, TRUE);
		vcb_acquired = TRUE;
		
		// if volume is read only we cant purge it
		if (FLAG_ON(vcb->flags, VFS_VCB_FLAGS_VOLUME_READ_ONLY))
			flush_before_purge = FALSE;

		// for all the files's that haven't been dereferenced yet by the cache mgr
		InitializeListHead(&fcb_list);
		for (entry = vcb->fcb_list.Flink; entry != &vcb->fcb_list; entry = entry->Flink) {
			fcb = CONTAINING_RECORD(entry, struct vfs_fcb, next);
			ExAcquireResourceExclusiveLite(&fcb->required_fcb.main_resource, TRUE);
			// reference them, so they dont get closed while we do the purge
			fcb_inc_ref_count(fcb);

			RELEASE(&fcb->required_fcb.main_resource);

			fcb_list_entry = ExAllocatePoolWithTag(VFSPOOL, sizeof(struct _FCB_LIST_ENTRY), 'LBCF');
			InsertTailList(&fcb_list, &fcb_list_entry->next);
			DbgPrint("VfsPurgeVolume: inserted a fcb\n");
		}

		RELEASE(&vcb->vcb_resource);
		vcb_acquired = FALSE;

		// purge all files, after the purge, files will have a 0 reference count and they should be closed
		while (!IsListEmpty(&fcb_list)) {
			entry = RemoveHeadList(&fcb_list);
			fcb_list_entry = CONTAINING_RECORD(entry, struct _FCB_LIST_ENTRY, next);
			fcb = fcb_list_entry->fcb;

			VfsPurgeFile(fcb, flush_before_purge);
			fcb_dec_ref_count(fcb);

			if (!fcb->reference_count) {
				VfsFreeFcb(fcb);
			}
			ExFreePool(fcb_list_entry);
		}

		DbgPrint("*** Volume flushed and purged ***");
	}
	__finally {
		if (vcb_acquired)
			RELEASE(&vcb->vcb_resource);
	}
}

void VfsPurgeFile(struct vfs_fcb *fcb, BOOLEAN flush_before_purge)
{
	IO_STATUS_BLOCK iosb;

	ASSERT(fcb);

	if (flush_before_purge)
		CcFlushCache(&fcb->required_fcb.section_object, NULL, 0, &iosb);
	if (fcb->required_fcb.section_object.ImageSectionObject)
		MmFlushImageSection(&fcb->required_fcb.section_object, MmFlushForWrite);
	if (fcb->required_fcb.section_object.DataSectionObject)
		CcPurgeCacheSection(&fcb->required_fcb.section_object, NULL, 0, FALSE);
}

void VfsSetVpbFlag(PVPB vpb, USHORT flag)
{
	KIRQL irql;
	IoAcquireVpbSpinLock(&irql);
	vpb->Flags |= flag;
	IoReleaseVpbSpinLock(irql);
}

void VfsClearVpbFlag(PVPB vpb, USHORT flag)
{
	KIRQL irql;
	IoAcquireVpbSpinLock(&irql);
	vpb->Flags &= ~flag;
	IoReleaseVpbSpinLock(irql);
}

NTSTATUS VfsUnlockVolume(PIRP irp, PIO_STACK_LOCATION stack_location)
{
	PDEVICE_OBJECT device = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	struct vfs_vcb *vcb = NULL;
	BOOLEAN vcb_acquired = FALSE;
	PFILE_OBJECT file_obj = NULL;

	__try {
		device = stack_location->DeviceObject;
		CHECK_OUT(device == vfs_data.device, STATUS_INVALID_DEVICE_REQUEST);

		vcb = (struct vfs_vcb*)device->DeviceExtension;
		ASSERT(vcb);

		file_obj = stack_location->FileObject;
		ASSERT(file_obj);

		ExAcquireResourceExclusiveLite(&vcb->vcb_resource, TRUE);
		vcb_acquired = TRUE;

		if (!FLAG_ON(vcb->flags, VFS_VCB_FLAGS_VOLUME_LOCKED)) {
			DbgPrint("*** volume is NOT LOCKED ***\n");
			TRY_RETURN(STATUS_ACCESS_DENIED);
		}
		
		CLEAR_FLAG(vcb->flags, VFS_VCB_FLAGS_VOLUME_LOCKED);
		VfsClearVpbFlag(vcb->vpb, VFS_VCB_FLAGS_VOLUME_LOCKED);
		DbgPrint("*** volume UNLOCKED ***\n");
		
		status = STATUS_SUCCESS;
try_exit:
		;
	}
	__finally {
		if (vcb_acquired)
			RELEASE(&vcb->vcb_resource);
		FsRtlNotifyVolumeEvent(file_obj, FSRTL_VOLUME_UNLOCK);
	}
	return status;
}

NTSTATUS VfsDismountVolume(PIRP irp, PIO_STACK_LOCATION stack_location)
{
	PDEVICE_OBJECT device = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	struct vfs_vcb *vcb = NULL;
	BOOLEAN notified = FALSE;
	BOOLEAN vcb_acquired = FALSE;
	PFILE_OBJECT file_obj = NULL;

	__try {
		device = stack_location->DeviceObject;
		CHECK_OUT(device == vfs_data.device, STATUS_INVALID_DEVICE_REQUEST);

		vcb = (struct vfs_vcb*)device->DeviceExtension;
		ASSERT(vcb);

		file_obj = stack_location->FileObject;
		ASSERT(file_obj);

		FsRtlNotifyVolumeEvent(file_obj, FSRTL_VOLUME_DISMOUNT);
		notified = TRUE;

		ExAcquireResourceExclusiveLite(&vcb->vcb_resource, TRUE);
		vcb_acquired = TRUE;

		if (!FLAG_ON(vcb->flags, VFS_VCB_FLAGS_VOLUME_LOCKED)) {
			DbgPrint("*** Volume is NOT LOCKED ***\n");
			TRY_RETURN(STATUS_ACCESS_DENIED);
		}

		SET_FLAG(vcb->flags, VFS_VCB_FLAGS_BEING_DISMOUNTED);
		DbgPrint("Volume beeing DISMOUNTED\n");

		sys_umount(vcb->super_block, MNT_DETACH);

		status = STATUS_SUCCESS;
try_exit:
		;
	}
	__finally {
		if (vcb_acquired)
			RELEASE(&vcb->vcb_resource);
		if (!NT_SUCCESS(status) && notified)
			FsRtlNotifyVolumeEvent(file_obj, FSRTL_VOLUME_DISMOUNT_FAILED);
	}
	return status;
}

NTSTATUS VfsIsVolumeMounted(PIRP irp, PIO_STACK_LOCATION stack_location)
{
	return VfsVerifyVolume(irp, stack_location);
}

NTSTATUS VfsVerifyVolume(PIRP irp, PIO_STACK_LOCATION stack_location)
{
	PDEVICE_OBJECT device = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	struct vfs_vcb *vcb = NULL;
	BOOLEAN vcb_acquired = FALSE;

	__try {
		device = stack_location->DeviceObject;
		CHECK_OUT(device == vfs_data.device, STATUS_INVALID_DEVICE_REQUEST);

		vcb = (struct vfs_vcb*)device->DeviceExtension;
		ASSERT(vcb);

		ExAcquireResourceExclusiveLite(&vcb->vcb_resource, TRUE);
		vcb_acquired = TRUE;

		CLEAR_FLAG(vcb->target_device->Flags, DO_VERIFY_VOLUME);

		status = STATUS_SUCCESS;
try_exit:
		;
	}
	__finally {
		if (vcb_acquired)
			RELEASE(&vcb->vcb_resource);
	}
	return status;
}
