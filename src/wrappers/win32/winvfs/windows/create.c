#include "vfs.h"

//
//	IRP_MJ_CREATE
//
NTSTATUS VfsCreate(PDEVICE_OBJECT device, PIRP irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	struct vfs_irp_context *irp_context = NULL;
	BOOLEAN top_level = FALSE;

	VfsDbgPrintCall(device, irp);

	FsRtlEnterFileSystem();

	ASSERT(device);
	ASSERT(irp);

	if (device == vfs_data.device) {
		irp->IoStatus.Status = status;
		irp->IoStatus.Information = FILE_OPENED;
		VfsDbgPrintComplete(irp);
		IoCompleteRequest(irp, IO_NO_INCREMENT);
		FsRtlExitFileSystem();
		return status;
	}

	top_level = VfsIsIrpTopLevel(irp);

	__try
	{
		irp_context = VfsAllocIrpContext(irp, device);
		ASSERT(irp_context);
		status = VfsCommonCreate(irp_context, irp, TRUE);
	}
	__except (VfsExceptionFilter(irp_context, GetExceptionInformation()))
	{
		status = VfsExceptionHandler(irp_context, irp);
	}

	if (top_level)
		IoSetTopLevelIrp(NULL);
	FsRtlExitFileSystem();

	return status;
}

//
//	code for open/create file or volume
//
NTSTATUS VfsCommonCreate(struct vfs_irp_context *irp_context, PIRP irp, BOOLEAN first_attempt)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack_location = NULL;
	PIO_SECURITY_CONTEXT security_context = NULL;
	PFILE_OBJECT new_file_obj = NULL;
	PFILE_OBJECT related_file_obj = NULL;
	ULONG allocation_size = 0;
	PFILE_FULL_EA_INFORMATION ext_attr_buffer = NULL;
	ULONG requested_options = 0;
	ULONG requested_disposition = 0;
	UCHAR file_attributes = 0;
	USHORT share_access = 0;
	ULONG ext_attr_length = 0;
	ACCESS_MASK desired_access;
	BOOLEAN deffered_processing = FALSE;
	struct vfs_vcb *vcb = NULL;
	BOOLEAN acquired_vcb = FALSE;
	BOOLEAN directory_only_requested = FALSE;
	BOOLEAN file_only_requested = FALSE;
	BOOLEAN no_buffering_specified = FALSE;
	BOOLEAN write_through_specified = FALSE;
	BOOLEAN delete_on_close_specified = FALSE;
	BOOLEAN no_ext_attr_knowledge = FALSE;
	BOOLEAN create_tree_connection = FALSE;
	BOOLEAN open_by_file_id = FALSE;
	BOOLEAN sequential_only = FALSE;
	BOOLEAN random_access = FALSE;
	BOOLEAN page_file_manipulation = FALSE;
	BOOLEAN open_target_directory = FALSE;
	BOOLEAN ignore_case_when_checking = FALSE;
	struct vfs_ccb *related_ccb = NULL, *new_ccb = NULL;
	struct vfs_fcb *related_fcb = NULL, *new_fcb = NULL;
	ULONG returned_information = -1;
	UNICODE_STRING target_object_name;
	UNICODE_STRING related_object_name;
	UNICODE_STRING absolute_path_name = { 0, 0, NULL};
	UNICODE_STRING rename_link_target_file_name;
	LARGE_INTEGER file_allocation_size, file_end_of_file;
	struct file* file = NULL;
	PSTR filename = NULL;

	ASSERT(irp_context);
	ASSERT(irp);

	__try {
		// 1. obtain caller supplied path

		// stack location
		stack_location = IoGetCurrentIrpStackLocation(irp);
		ASSERT(stack_location);

		// can we block?
		/*if (!FLAG_ON(irp_context->flags, VFS_IRP_CONTEXT_CAN_BLOCK)) {
			status = VfsPostRequest(irp_context, irp);
			deffered_processing = TRUE;
			TRY_RETURN(status);
		}*/

		new_file_obj = stack_location->FileObject;
		target_object_name = new_file_obj->FileName;
		related_file_obj = new_file_obj->RelatedFileObject;

		// is this a relative create open?
		if (related_file_obj) {
			related_ccb = (struct vfs_ccb*)related_file_obj->FsContext2;
			ASSERT(related_ccb);
			related_fcb = related_ccb->fcb;
			ASSERT(related_fcb);
			
			related_object_name = related_file_obj->FileName;
		}

		// 2. obtain other arguments supplied by the caller
		allocation_size = irp->Overlay.AllocationSize.LowPart;
		CHECK_OUT(irp->Overlay.AllocationSize.HighPart, STATUS_INVALID_PARAMETER);	// only 32 bit files supported

		security_context = stack_location->Parameters.Create.SecurityContext;
		desired_access = security_context->DesiredAccess;

		requested_options = stack_location->Parameters.Create.Options & FILE_VALID_OPTION_FLAGS;

		requested_disposition = (stack_location->Parameters.Create.Options >> 24) & 0xFF;

		file_attributes = (UCHAR)(stack_location->Parameters.Create.FileAttributes & FILE_ATTRIBUTE_VALID_FLAGS);

		share_access = stack_location->Parameters.Create.ShareAccess;

		ext_attr_buffer = irp->AssociatedIrp.SystemBuffer;
		ext_attr_length = stack_location->Parameters.Create.EaLength;

		sequential_only = ((requested_options & FILE_SEQUENTIAL_ONLY ) ? TRUE : FALSE);
		random_access = ((requested_options & FILE_RANDOM_ACCESS ) ? TRUE : FALSE);
		directory_only_requested = ((requested_options & FILE_DIRECTORY_FILE) ? TRUE : FALSE);		
		file_only_requested = ((requested_options & FILE_NON_DIRECTORY_FILE) ? TRUE : FALSE);
		no_buffering_specified = ((requested_options & FILE_NO_INTERMEDIATE_BUFFERING) ? TRUE : FALSE);
		write_through_specified = ((requested_options & FILE_WRITE_THROUGH) ? TRUE : FALSE);
		delete_on_close_specified = ((requested_options & FILE_DELETE_ON_CLOSE) ? TRUE : FALSE);
		no_ext_attr_knowledge = ((requested_options & FILE_NO_EA_KNOWLEDGE) ? TRUE : FALSE);
		create_tree_connection = ((requested_options & FILE_CREATE_TREE_CONNECTION) ? TRUE : FALSE);
		open_by_file_id = ((requested_options & FILE_OPEN_BY_FILE_ID) ? TRUE : FALSE);
		
		page_file_manipulation = FLAG_ON(stack_location->Flags, SL_OPEN_PAGING_FILE);
		open_target_directory = FLAG_ON(stack_location->Flags, SL_OPEN_TARGET_DIRECTORY);
		ignore_case_when_checking = FLAG_ON(stack_location->Flags, SL_CASE_SENSITIVE);

		//printk("directory_only_requested: %d, file_only_requested : %d\n", 
		//	directory_only_requested, file_only_requested);
		//printk("open_target_directory: %d\n", open_target_directory);
		//printk("access_mask: %ul\n", desired_access);

		vcb = (struct vfs_vcb*)irp_context->target_device->DeviceExtension;
		ASSERT(vcb);

		if (vcb->vpb)
			new_file_obj->Vpb = vcb->vpb;

		// acquire vcb exlusively to sync with close and cleanup routines
		ExAcquireResourceExclusiveLite(&(vcb->vcb_resource), TRUE);
		acquired_vcb = TRUE;

		// fail if volume is locked
		if (FLAG_ON(vcb->flags, VFS_VCB_FLAGS_VOLUME_LOCKED))
			TRY_RETURN(STATUS_ACCESS_DENIED);

		// 3. check for volume open request
		if ((new_file_obj->FileName.Length == 0) && ((related_file_obj == NULL) || 
			(related_fcb->identifier.type == VFS_NODE_TYPE_VCB))) 
		{
			CHECK_OUT(open_target_directory || ext_attr_buffer, STATUS_INVALID_PARAMETER);
			CHECK_OUT(directory_only_requested, STATUS_NOT_A_DIRECTORY);
			CHECK_OUT((requested_disposition != FILE_OPEN) && (requested_disposition != FILE_OPEN_IF), STATUS_ACCESS_DENIED);

			status = VfsOpenVolume(vcb, irp_context, irp, share_access, security_context, new_file_obj);
			returned_information = irp->IoStatus.Information;
			TRY_RETURN(status);
		}

		// 4. create absolute file path 
		CHECK_OUT(open_by_file_id, STATUS_ACCESS_DENIED);

		if (related_file_obj) {
			// validity checks ...
			CHECK_OUT(!S_ISDIR(related_fcb->inode->i_mode), STATUS_INVALID_PARAMETER);
			CHECK_OUT((related_object_name.Length==0) || (related_object_name.Buffer[0]!=L'\\'), STATUS_INVALID_PARAMETER);
			CHECK_OUT((target_object_name.Length!=0) && (target_object_name.Buffer[0]==L'\\'), STATUS_INVALID_PARAMETER);

			absolute_path_name.MaximumLength = target_object_name.Length + related_object_name.Length + sizeof(WCHAR);
			if (!(absolute_path_name.Buffer = ExAllocatePoolWithTag(VFSPOOL, absolute_path_name.MaximumLength, 'HTAP')))
				TRY_RETURN(STATUS_INSUFFICIENT_RESOURCES);
			RtlZeroMemory(absolute_path_name.Buffer, absolute_path_name.MaximumLength);

			RtlCopyMemory(absolute_path_name.Buffer, related_object_name.Buffer, related_object_name.Length);
			absolute_path_name.Length = related_object_name.Length;
			RtlAppendUnicodeToString(&absolute_path_name, L"\\");
			RtlAppendUnicodeToString(&absolute_path_name, target_object_name.Buffer);
		} 
		else {
			CHECK_OUT(target_object_name.Buffer[0] != L'\\', STATUS_INVALID_PARAMETER);

			absolute_path_name.MaximumLength = target_object_name.Length;
			if (!(absolute_path_name.Buffer = ExAllocatePoolWithTag(VFSPOOL, absolute_path_name.MaximumLength, 'HTAP')))
				TRY_RETURN(STATUS_INSUFFICIENT_RESOURCES);
			RtlZeroMemory(absolute_path_name.Buffer, absolute_path_name.MaximumLength);

			RtlCopyMemory(absolute_path_name.Buffer, target_object_name.Buffer, target_object_name.Length);
			absolute_path_name.Length = target_object_name.Length;
		}

		// 5. now, try to locate the target object
		if (absolute_path_name.Length == 2) {
			// open root directory
			if (file_only_requested || (requested_disposition == FILE_SUPERSEDE) || (requested_disposition == FILE_OVERWRITE) ||
				(requested_disposition == FILE_OVERWRITE_IF))
				TRY_RETURN(STATUS_FILE_IS_A_DIRECTORY);
			status = VfsOpenRootDirectory(vcb, irp_context, irp, share_access, security_context, new_file_obj);
			TRY_RETURN(status);
		}

		if (open_target_directory)
			TRY_RETURN(STATUS_NOT_IMPLEMENTED);	// todo: this is for rename / move operations

		if (!(filename = VfsCopyUnicodeStringToZcharUnixPath(&absolute_path_name)))
			TRY_RETURN(STATUS_NO_MEMORY);


		//DbgPrint("PATH: %s\n", filename);

		ASSERT(vcb->super_block);
		ASSERT(vcb->super_block->s_root);
		ASSERT(vcb->super_block->s_root->d_inode);
		if (requested_disposition == FILE_OPEN) {
			//if (directory_only_requested)
				file = filp_open(vcb->super_block->s_root, filename, 0, FMODE_READ);	// flag 3 = 11 = read-write -- see filp_open implementation
			//else
			//	file = filp_open(vcb->super_block->s_root, filename, 2, FMODE_WRITE|FMODE_READ);	// flag 3 = 11 = read-write -- see filp_open implementation
		}
		else
			TRY_RETURN(STATUS_NOT_IMPLEMENTED);	// todo: create, overwrite
		//else if (requested_disposition == FILE_CREATE)
			//file = ...;

		//DbgPrint("create.c err: %d", (long)file);
		if (IS_ERR(file)) {
			TRY_RETURN(STATUS_OBJECT_PATH_NOT_FOUND);
		}

		//DbgPrint("*** create succeded ***");

		ASSERT(file);
		ASSERT(file->f_dentry);
		ASSERT(file->f_dentry->d_inode);

		new_fcb = VfsLocateFcbInCore(vcb, file->f_dentry->d_inode->i_ino);

		//DbgPrint("*** tried to locate fcb ***");

		if (!new_fcb) {
			// create the fcb
			LARGE_INTEGER zero_size = {0, 0};


			status = VfsCreateFcb(&new_fcb, zero_size, zero_size, new_file_obj, vcb, file->f_dentry);
			if (!NT_SUCCESS(status))
				TRY_RETURN(STATUS_INSUFFICIENT_RESOURCES);

			//DbgPrint("*** created fcb ***");
		}

//		new_fcb->inode = file->f_dentry->d_inode;

		status = VfsCreateNewCcb(&new_ccb, new_fcb, new_file_obj);
		//ASSERT(new_ccb);

		//DbgPrint("*** created ccb ***");

		new_ccb->file = file;

		new_file_obj->FsContext = new_fcb;
		new_file_obj->FsContext2 = new_ccb;
		new_file_obj->PrivateCacheMap = NULL;
		new_file_obj->SectionObjectPointer = &new_fcb->required_fcb.section_object;
		new_file_obj->Vpb = vcb->vpb;

		//DbgPrint("*** checking share access ***");

		if (new_fcb->handle_count > 0) {
			status = IoCheckShareAccess(desired_access, share_access, new_file_obj, &new_fcb->share_access, TRUE);
			CHECK_OUT(!NT_SUCCESS(status), status);
		} else 
			IoSetShareAccess(desired_access, share_access, new_file_obj, &new_fcb->share_access);

		if (returned_information == -1)
			returned_information = FILE_OPENED;

		//DbgPrint("*** DONE ***");

try_exit:
		;
	}
	
	__finally {
		if (acquired_vcb)
			RELEASE(&vcb->vcb_resource);
		if (absolute_path_name.Buffer)
			ExFreePool(absolute_path_name.Buffer);
		if (status != STATUS_PENDING) {
			if (NT_SUCCESS(status)) {
				if (write_through_specified)
					new_file_obj->Flags |= FO_WRITE_THROUGH;
			}
			if (!FLAG_ON(irp_context->flags, VFS_IRP_CONTEXT_EXCEPTION)) {
				irp->IoStatus.Status = status;
				irp->IoStatus.Information = returned_information;
				VfsDbgPrintComplete(irp);
				VfsFreeIrpContext(irp_context);
				
				IoCompleteRequest(irp, IO_DISK_INCREMENT);
				//DbgPrint("COMPLETED THE REQUEST ...\n");
			}
		}
	}
	if (filename) kfree(filename);
	return status;
}

NTSTATUS VfsOpenVolume(struct vfs_vcb *vcb, struct vfs_irp_context *irp_context, PIRP irp, USHORT share_access,
					   PIO_SECURITY_CONTEXT security_context, PFILE_OBJECT new_file_obj)
{
	NTSTATUS status = STATUS_SUCCESS;
	struct vfs_ccb *ccb = NULL;

	__try {
		CHECK_OUT(!(ccb = VfsAllocCcb()), STATUS_INSUFFICIENT_RESOURCES);

		ccb->fcb = (struct vfs_fcb*)vcb;
		ccb->file_obj = new_file_obj;
		SET_FLAG(ccb->flags, VFS_CCB_VOLUME_OPEN);

		new_file_obj->FsContext = &vcb->common_header;

		vcb_inc_ref_count(vcb);

		irp->IoStatus.Information = FILE_OPENED;
try_exit:
		;
	}
	__finally {
		;
	}
	return status;
}

struct vfs_fcb* VfsLocateFcbInCore(struct vfs_vcb *vcb, ULONG inode_no)
{
	struct vfs_fcb *fcb = NULL;
	PLIST_ENTRY list_entry = NULL;

	if (IsListEmpty(&(vcb->fcb_list)))
		return NULL;
	for (list_entry = vcb->fcb_list.Flink; list_entry != &vcb->fcb_list; list_entry = list_entry->Flink) {
		fcb = CONTAINING_RECORD(list_entry, struct vfs_fcb, next);
		ASSERT(fcb);
		if (fcb->inode->i_ino == inode_no)
			return fcb;
	}
	return NULL;
}

NTSTATUS VfsOpenRootDirectory(struct vfs_vcb *vcb, struct vfs_irp_context *irp_context, PIRP irp, USHORT share_access,
							  PIO_SECURITY_CONTEXT security_context, PFILE_OBJECT new_file_obj)
{
	struct vfs_fcb *fcb = NULL;
	struct vfs_ccb *ccb = NULL;
	NTSTATUS status = STATUS_SUCCESS;

	__try {
		ASSERT(vcb->super_block);
		ASSERT(vcb->super_block->s_root);
		ASSERT(vcb->super_block->s_root->d_inode);
		fcb = VfsLocateFcbInCore(vcb, vcb->super_block->s_root->d_inode->i_ino);
		if (!fcb) {
			// create the root fcb
//			struct vfs_obj_name *obj_name = VfsAllocObjName();
			LARGE_INTEGER zero_size = {0, 0};

//			RtlInitUnicodeString(&obj_name->name, L"\\");
//			VfsCopyWcharToUnicodeString(&obj_name->name, L"\\");

			status = VfsCreateFcb(&fcb, zero_size, zero_size, new_file_obj, vcb, vcb->super_block->s_root);
			if (!NT_SUCCESS(status))
				TRY_RETURN(STATUS_INSUFFICIENT_RESOURCES);
		}
		ASSERT(fcb);

//		fcb->inode = vcb->super_block->s_root->d_inode;

		status = VfsCreateNewCcb(&ccb, fcb, new_file_obj);
		ASSERT(ccb);

		DbgPrint("create.c: calling dentry_open\n");
		// don't use dentry_open !!! because it implies you have already increased dentry & inode counters --sadyc
		ccb->file = filp_open(vcb->super_block->s_root, "/",  0, FMODE_READ);
//		ccb->file = dentry_open(vcb->super_block->s_root, NULL, FMODE_READ|FMODE_WRITE);
		ASSERT(ccb->file);

		new_file_obj->FsContext = fcb;
		new_file_obj->FsContext2 = ccb;
		new_file_obj->PrivateCacheMap = NULL;
		new_file_obj->SectionObjectPointer = &fcb->required_fcb.section_object;
		new_file_obj->Vpb = vcb->vpb;
try_exit:
		;
	}
	__finally {
		;
	}
	return status;
}
