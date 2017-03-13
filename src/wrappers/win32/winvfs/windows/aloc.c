#include "vfs.h"
#include "linux/blkdev.h"

//
//	alloc struct vfs_ccb
//
struct vfs_ccb* VfsAllocCcb()
{
	struct vfs_ccb *ccb = NULL;
	
	ccb = (struct vfs_ccb*) kmem_cache_alloc(ccb_cachep, GFP_KERNEL);
	if (!ccb)
		return NULL;

	RtlZeroMemory(ccb, QUAD_ALIGN(sizeof(struct vfs_ccb)));
	ccb->identifier.type = VFS_NODE_TYPE_CCB;
	ccb->identifier.size = QUAD_ALIGN(sizeof(struct vfs_ccb));

	return ccb;
}

//
//	free struct vfs_ccb
//
void VfsCloseAndFreeCcb(struct vfs_ccb *ccb)
{
	ASSERT(ccb);
	ASSERT(ccb->file);


	/*if (ccb->file->f_dentry && ccb->file->f_dentry->d_name.name)
		printk("---> VfsCloseAndFreeCcb name= %s\n", ccb->file->f_dentry->d_name.name);
	else printk("---> VfsCloseAndFreeCcb name= <unknown>\n");
	if (ccb->search_pattern.Buffer)
		printk("---> VfsCloseAndFreeCcb search_pattern= %ws\n", ccb->search_pattern.Buffer);
*/
	// --danny -- momentan nu inchidem rootul
	// -- nu stiu cum sa il inchid
//	if (ccb->fcb->inode->i_ino != ccb->fcb->vcb->super_block->s_root->d_inode->i_ino)
			filp_close(ccb->file, 0); // --danny to check

	RtlFreeUnicodeString(&ccb->search_pattern);
	kmem_cache_free(ccb_cachep, ccb);
}

//
//	alloc struct vfs_fcb
//
struct vfs_fcb* VfsAllocFcb()
{
	struct vfs_fcb *fcb;

	fcb = (struct vfs_fcb*) kmem_cache_alloc(fcb_cachep, GFP_KERNEL);
	if (!fcb)
		return NULL;

	RtlZeroMemory(fcb, QUAD_ALIGN(sizeof(struct vfs_fcb)));
	fcb->identifier.type = VFS_NODE_TYPE_FCB;
	fcb->identifier.size = QUAD_ALIGN(sizeof(struct vfs_fcb));

	return fcb;
}

//
//	free struct vfs_fcb
//
void VfsFreeFcb(struct vfs_fcb* fcb)
{
	ASSERT(fcb);
	ASSERT(fcb->dentry);

//	if (fcb->dentry->d_name.name)
//		printk("---> VfsFreeFcb name= %s\n", fcb->dentry->d_name.name);

#ifdef SHRINK_DCACHE_ON_CLOSE
	shrink_dcache_parent(fcb->dentry);
#endif

	ExDeleteResourceLite(&fcb->required_fcb.main_resource);
	ExDeleteResourceLite(&fcb->required_fcb.paging_resource);
	kmem_cache_free(fcb_cachep, fcb);
}

//
//	alloc struct vfs_irp_context
//
struct vfs_irp_context* VfsAllocIrpContext(PIRP irp, PDEVICE_OBJECT target_device)
{
	struct vfs_irp_context *irp_context = NULL;
	PIO_STACK_LOCATION stack_location;

	irp_context = kmem_cache_alloc(irp_context_cachep, GFP_KERNEL);
	if (!irp_context)
		return NULL;

	RtlZeroMemory(irp_context, QUAD_ALIGN(sizeof(struct vfs_irp_context)));

	irp_context->identifier.type = VFS_NODE_TYPE_IRP_CONTEXT;
	irp_context->identifier.size = QUAD_ALIGN(sizeof(struct vfs_irp_context));

	irp_context->irp = irp;
	irp_context->target_device = target_device;

	if (irp) {
		stack_location = IoGetCurrentIrpStackLocation(irp);
		ASSERT(stack_location);

		irp_context->major_function = stack_location->MajorFunction;
		irp_context->minor_function = stack_location->MinorFunction;

		if (stack_location->FileObject) {
			if (IoIsOperationSynchronous(irp))
				SET_FLAG(irp_context->flags, VFS_IRP_CONTEXT_CAN_BLOCK);

		} else
			SET_FLAG(irp_context->flags, VFS_IRP_CONTEXT_CAN_BLOCK);
	}

	if (IoGetTopLevelIrp() != irp)
		SET_FLAG(irp_context->flags, VFS_IRP_CONTEXT_NOT_TOP_LEVEL);

	return irp_context;
}

//
//	free struct vfs_irp_context
//
void VfsFreeIrpContext(struct vfs_irp_context *irp_context)
{
	ASSERT(irp_context);
	kmem_cache_free(irp_context_cachep, irp_context);
}

//
//	create struct vfs_fcb
//
NTSTATUS VfsCreateFcb(struct vfs_fcb **new_fcb, LARGE_INTEGER alloc_size, 
					  LARGE_INTEGER end_of_file, PFILE_OBJECT file_obj,
					  struct vfs_vcb* vcb, struct dentry *dentry)
{
	NTSTATUS status = STATUS_SUCCESS;
	struct vfs_fcb *fcb = NULL;
	struct vfs_required_fcb *req_fcb = NULL;
	PFSRTL_COMMON_FCB_HEADER header = NULL;

	ASSERT(vcb);
	ASSERT(file_obj);
	ASSERT(dentry);

/*	if (dentry->d_name.name)
		printk("---> VfsCreateFcb name= %s\n", dentry->d_name.name);
	else
		printk("---> VfsCreateFcb name= <unknown>\n");
*/

	__try
	{
		fcb = VfsAllocFcb();
		CHECK_OUT(!fcb, STATUS_INSUFFICIENT_RESOURCES);

		req_fcb = &(fcb->required_fcb);
		header = &(req_fcb->common_header);

		ExInitializeResourceLite(&(req_fcb->main_resource));
		SET_FLAG(fcb->flags, VFS_INITIALIZED_MAIN_RESOURCE);
		ExInitializeResourceLite(&(req_fcb->paging_resource));
		SET_FLAG(fcb->flags, VFS_INITIALIZED_PAGING_IO_RESOURCE);

		header->IsFastIoPossible = FastIoIsNotPossible;
		header->Resource = &(req_fcb->main_resource);
		header->PagingIoResource = &(req_fcb->paging_resource);
		header->AllocationSize.QuadPart = (LONGLONG)(dentry->d_inode->i_blocks * dentry->d_inode->i_blksize);//alloc_size;
		header->FileSize.QuadPart = (LONGLONG)dentry->d_inode->i_size;
		header->ValidDataLength.LowPart = 0xFFFFFFFF;
		header->ValidDataLength.HighPart = 0x7FFFFFFF;

		fcb->vcb = vcb;
		InsertTailList(&(vcb->fcb_list), &(fcb->next));

		InitializeListHead(&(fcb->ccb_list));

		file_obj->PrivateCacheMap = NULL;
		file_obj->FsContext = header;
		file_obj->SectionObjectPointer = &(req_fcb->section_object);

		fcb->dentry = dentry;
		fcb->inode = dentry->d_inode;


		*new_fcb = fcb;
try_exit:
		;
	}
	__finally
	{
	}
	return status;
}

//
//	init struct vfs_vcb
//
void VfsInitializeVcb(PDEVICE_OBJECT volume_dev, PDEVICE_OBJECT target_dev, PVPB vpb,
					  PLARGE_INTEGER alloc_size)
{
	NTSTATUS status = STATUS_SUCCESS;
	struct vfs_vcb *vcb = NULL;

	vcb = (struct vfs_vcb*)(volume_dev->DeviceExtension);
	RtlZeroMemory(vcb, sizeof(struct vfs_vcb));

	vcb->identifier.type = VFS_NODE_TYPE_VCB;
	vcb->identifier.size = sizeof(struct vfs_vcb);

	status = ExInitializeResourceLite(&(vcb->vcb_resource));
	ASSERT(NT_SUCCESS(status));

	vcb->target_device = target_dev;
	vcb->vcb_device = volume_dev;
	vcb->vpb = vpb;

	InitializeListHead(&(vcb->fcb_list));
	InitializeListHead(&(vcb->next_notify_irp));

	KeInitializeMutex(&(vcb->notify_irp_mutex), 0);

	vcb->common_header.AllocationSize.QuadPart = alloc_size->QuadPart;
	vcb->common_header.FileSize.QuadPart = alloc_size->QuadPart;
	vcb->common_header.ValidDataLength.LowPart = 0xFFFFFFFF;
	vcb->common_header.ValidDataLength.HighPart = 0x7FFFFFFF;
	vcb->common_header.IsFastIoPossible = FastIoIsNotPossible;
	vcb->common_header.Resource = &(vcb->vcb_resource);

	//vcb->stream_file_obj = IoCreateStreamFileObject(NULL, vcb->vpb->RealDevice);
	//ASSERT(vcb->stream_file_obj);

	//vcb->stream_file_obj->FsContext = (void*)(&vcb->common_header);
	//vcb->stream_file_obj->FsContext2 = NULL;
	//vcb->stream_file_obj->SectionObjectPointer = &(vcb->section_object);
	//vcb->stream_file_obj->Vpb = vpb;
	//vcb->stream_file_obj->ReadAccess = TRUE;
	//vcb->stream_file_obj->WriteAccess = TRUE;

	ExAcquireResourceExclusiveLite(&(vfs_data.global_resource), TRUE);
	
	InsertTailList(&(vfs_data.vcb_list), &(vcb->next));
	
	//CcInitializeCacheMap(vcb->stream_file_obj, 
	//	(PCC_FILE_SIZES)(&(vcb->common_header.AllocationSize)), TRUE, 
	//	&(vfs_data.cache_mgr_callbacks), vcb);

	RELEASE(&(vfs_data.global_resource));

	SET_FLAG(vcb->flags, VFS_VCB_FLAGS_VCB_INITIALIZED);
}

//
//	create struct vfs_ccb
//
NTSTATUS VfsCreateNewCcb(struct vfs_ccb **new_ccb, struct vfs_fcb *fcb, PFILE_OBJECT file_obj)
{
	struct vfs_ccb *ccb = NULL;
	NTSTATUS status = STATUS_SUCCESS;

	__try
	{
		ccb = VfsAllocCcb();
		CHECK_OUT(!ccb, STATUS_INSUFFICIENT_RESOURCES);
		ccb->fcb = fcb;
		ccb->file_obj = file_obj;
		ccb->offset.QuadPart = 0;

		fcb_inc_ref_count(fcb);
		InterlockedIncrement(&fcb->handle_count);

		vcb_inc_ref_count(fcb->vcb);

		fcb->vcb->open_count++;

		InsertTailList(&(fcb->ccb_list), &(ccb->next));

		*new_ccb = ccb;
try_exit:
		;
	}
	__finally
	{
	}
	return status;
}

//
//	alloc struct super_block
//
struct super_block* VfsAllocSuperBlock(PVPB vpb, PDEVICE_OBJECT block_device)
{
	struct super_block *sb = alloc_super();
	int i;

	__try
	{
		sb->s_dev = block_device;
		sb->s_blocksize = get_hardsect_size(block_device);
		sb->s_blocksize_bits = (unsigned char)blocksize_bits(sb->s_blocksize);
	}
	__finally
	{
	}
	return sb;
}

//
//	free struct vcb
//
void VfsFreeVcb(struct vfs_vcb *vcb)
{
	VfsClearVpbFlag(vcb->vpb, VPB_MOUNTED);

	ExAcquireResourceExclusiveLite(&vfs_data.global_resource, TRUE);
	RemoveEntryList(&vcb->next);
	RELEASE(&vfs_data.global_resource);

	ExDeleteResourceLite(&vcb->vcb_resource);

	IoDeleteDevice(vcb->vcb_device);  // it's vcb->vcb_device not vcb->target_device --sadyc

	// hack to avoid user IOCTL --sadyc;
	// todo: implement IOCTL
	IoUnregisterFileSystem(vfs_data.device);
	IoDeleteDevice(vfs_data.device);
}
