#include "vfs.h"

#include "debug.h"

NTSTATUS VfsDirectoryControl(PDEVICE_OBJECT dev_obj, PIRP irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	struct vfs_irp_context *irp_context = NULL;
	BOOLEAN top_level = FALSE;

	VfsDbgPrintCall(dev_obj, irp);

	FsRtlEnterFileSystem();

	top_level = VfsIsIrpTopLevel(irp);

	__try {
		irp_context = VfsAllocIrpContext(irp, dev_obj);
		status = VfsCommonDirectoryControl(irp_context, irp);
	}
	__except (VfsExceptionFilter(irp_context, GetExceptionInformation())) {
		status = VfsExceptionHandler(irp_context, irp);
	}

	if (top_level) 
		IoSetTopLevelIrp(NULL);

	FsRtlExitFileSystem();

	return status;
}

NTSTATUS VfsCommonDirectoryControl(struct vfs_irp_context *irp_context, PIRP irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack_location = NULL;
	PFILE_OBJECT file_obj = NULL;
	struct vfs_vcb *vcb = NULL;
	struct vfs_fcb *fcb = NULL;
	struct vfs_ccb *ccb = NULL;

	stack_location = IoGetCurrentIrpStackLocation(irp);
	file_obj = stack_location->FileObject;
	ccb = (struct vfs_ccb*)(file_obj->FsContext2);
	ASSERT(ccb);
	fcb = ccb->fcb;
	ASSERT(fcb);

	switch (stack_location->MinorFunction) {
	case IRP_MN_QUERY_DIRECTORY:
		status = VfsQueryDirectory(irp_context, irp, stack_location, file_obj, fcb, ccb);
		break;
	case IRP_MN_NOTIFY_CHANGE_DIRECTORY:
		status = STATUS_NOT_IMPLEMENTED;
		irp->IoStatus.Status = status;
		irp->IoStatus.Information = 0;
		IoCompleteRequest(irp, IO_DISK_INCREMENT);
		break;
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		irp->IoStatus.Status = status;
		irp->IoStatus.Information = 0;
		IoCompleteRequest(irp, IO_NO_INCREMENT);
		break;
	}
	VfsFreeIrpContext(irp_context);

	return status;
}

#define BEGIN_CASE(x, X) \
	case File##x##Information:\
	{\
		PFILE_##X##_INFORMATION info=(PFILE_##X##_INFORMATION) (((char*)buf->buffer)+buf->used_length);\
		int __tmp=buf->used_length;\
		\
		info->FileIndex = (ULONG)offset;\
//		DbgPrint("query dir: info=%p\n", info);
		

		
		
#define END_CASE(x, X) \
		buf->used_length += sizeof(FILE_##X##_INFORMATION);\
		buf->used_length += buf->used_length % 8;\
		info->NextEntryOffset = buf->used_length - __tmp;\
		buf->next_entry_offset = &info->NextEntryOffset;\
		break;\
	}

#define FillStandardWinInfo	\
	RtlSecondsSince1970ToTime (inode->i_ctime, & info->CreationTime);	\
	RtlSecondsSince1970ToTime (inode->i_atime, & info->LastAccessTime); \
	RtlSecondsSince1970ToTime (inode->i_mtime, & info->LastWriteTime); \
	RtlSecondsSince1970ToTime (inode->i_mtime, & info->ChangeTime); \
	info->EndOfFile.QuadPart = inode->i_size;	\
	info->AllocationSize.QuadPart = inode->i_size;

//FiXME: symbolic links are always threated as files, even if they point to a directory
#define FillFileType \
	info->FileAttributes = FILE_ATTRIBUTE_NORMAL;	\
	if (S_ISDIR(inode->i_mode)) {	\
		SET_FLAG(info->FileAttributes, FILE_ATTRIBUTE_DIRECTORY);	\
	}

// FIXME: short name
#define FillShortName \
	info->ShortNameLength = (namlen < 12 ? namlen : 12) * 2; \
	RtlCopyMemory(info->ShortName, file_name.Buffer, info->ShortNameLength); 

#define FillFileName \
	info->FileNameLength = namlen * 2; \
	RtlCopyMemory(info->FileName, file_name.Buffer, namlen * 2); \
	buf->used_length += namlen * 2 ; 

#define FillEa \
	info->EaSize = 0;

#define FillId \
	info->FileId.QuadPart = 0;

	

int filldir(void * __buf, const char * name, int namlen, loff_t offset,
		   ino_t ino, unsigned int d_type)
{
	struct dirent_buffer* buf = (struct dirent_buffer*)__buf;
	UNICODE_STRING file_name;
	struct inode *inode;
	int ret=0, tmp;


	if (!offset) ASSERT(buf->used_length == 0);

	if (buf->used_length && buf->return_single_entry) {
		buf->status = STATUS_SUCCESS;
		return -EINVAL;
	}


	if (buf->buffer_length < (buf->query_block_length + namlen * 2 + buf->used_length)) {
		if (!buf->used_length) {
			buf->status = STATUS_INFO_LENGTH_MISMATCH;
		} else {
			buf->status = STATUS_SUCCESS;
		}
		return -EINVAL;
	}

//	printk("filldir(name='%.255s', namelen=%d, offset=%d, d_type=%d)\n",
//		name, namlen, (int)offset, d_type);

	file_name.Length = file_name.MaximumLength = namlen * 2;
	file_name.Buffer = ExAllocatePoolWithTag(VFSPOOL, namlen*2, 'LTCD');
	VfsCharToWchar(file_name.Buffer, (char*)name, namlen);

	if (!(inode = iget(buf->file->f_dentry->d_inode->i_sb, ino))) {
		DbgPrint("filldir: can't get inode 0x%x\n", (int)inode->i_ino);
		buf->status=STATUS_DISK_CORRUPT_ERROR;
		return -EIO;
	}

	if (FsRtlDoesNameContainWildCards(buf->search_pattern) ?
		FsRtlIsNameInExpression(buf->search_pattern, &file_name, TRUE, NULL) :
		!RtlCompareUnicodeString(buf->search_pattern, &file_name, TRUE)) 
	{

		switch(buf->info_class) {

		BEGIN_CASE(Directory, DIRECTORY)
			FillStandardWinInfo;
			FillFileType;
			FillFileName;
		END_CASE(Directory, DIRECTORY)

		BEGIN_CASE(BothDirectory, BOTH_DIR)
			FillStandardWinInfo;
			FillFileType;
			FillEa;
			FillShortName;
			FillFileName;
		END_CASE(BothDirectory, BOTH_DIR)

		BEGIN_CASE(Names, NAMES)
			FillFileName;
		END_CASE(Names, NAMES)

		BEGIN_CASE(FullDirectory, FULL_DIR)
			FillStandardWinInfo;
			FillFileType;
			FillEa;
			FillFileName;
		END_CASE(FullDirectory, FULL_DIR)

		BEGIN_CASE(IdFullDirectory, ID_FULL_DIR)
			FillStandardWinInfo;
			FillFileType;
			FillEa;
			FillFileName;
			FillId;
		END_CASE(FullDirectory, FULL_DIR)


		BEGIN_CASE(IdBothDirectory, ID_BOTH_DIR)
			FillStandardWinInfo;
			FillFileType;
			FillEa;
			FillShortName;
			FillFileName;
			FillId;
		END_CASE(IdBothDirectory, ID_BOTH_DIR)
		
		default:
			ASSERT(0);
		}
	}

	iput(inode);
	RtlFreeUnicodeString(&file_name);
	
	buf->status = STATUS_SUCCESS;
	return ret;
}


NTSTATUS VfsQueryDirectory(struct vfs_irp_context *irp_context, PIRP irp, PIO_STACK_LOCATION stack_location,
						   PFILE_OBJECT file_obj, struct vfs_fcb *fcb, struct vfs_ccb *ccb)
{
	NTSTATUS status = STATUS_SUCCESS;
	BOOLEAN complete_request = FALSE;
	BOOLEAN post_request = FALSE;
	struct vfs_required_fcb *required_fcb = NULL;
	BOOLEAN can_wait = FALSE;
	struct vfs_vcb *vcb = NULL;
	BOOLEAN acquired_fcb = FALSE;
	ULONG file_index = 0;
	BOOLEAN restart_scan = FALSE;
	BOOLEAN index_specified = FALSE;
	ULONG starting_index_for_search = 0;
	ULONG bytes_returned = 0;
#ifdef _GNU_NTIFS_
	PEXTENDED_IO_STACK_LOCATION stack_location_ex = (PEXTENDED_IO_STACK_LOCATION) stack_location;
#else
	PIO_STACK_LOCATION stack_location_ex = stack_location;
#endif
	struct dirent_buffer win_buffer;

	win_buffer.first_time_query = FALSE;

	__try {

		if (fcb->identifier.type == VFS_NODE_TYPE_VCB || !(S_ISDIR(fcb->inode->i_mode)))
			TRY_RETURN(STATUS_INVALID_PARAMETER);
		required_fcb = &fcb->required_fcb;
		can_wait = irp_context->flags & VFS_IRP_CONTEXT_CAN_BLOCK ? TRUE : FALSE;
		vcb = fcb->vcb;

		/*if (!can_wait) {
			post_request = TRUE;
			TRY_RETURN(STATUS_PENDING);
		}*/

		win_buffer.buffer_length = stack_location_ex->Parameters.QueryDirectory.Length;
		win_buffer.search_pattern = (PUNICODE_STRING)stack_location_ex->Parameters.QueryDirectory.FileName;
		file_index = stack_location_ex->Parameters.QueryDirectory.FileIndex;
		
		switch ((win_buffer.info_class = stack_location_ex->Parameters.QueryDirectory.FileInformationClass)) {
		case FileDirectoryInformation:
			win_buffer.query_block_length=sizeof(FILE_DIRECTORY_INFORMATION);
			break;
		case FileBothDirectoryInformation:
			win_buffer.query_block_length=sizeof(FILE_BOTH_DIR_INFORMATION);
			break;
		case FileNamesInformation:
			win_buffer.query_block_length=sizeof(FILE_NAMES_INFORMATION);
			break;
		case FileFullDirectoryInformation:
			win_buffer.query_block_length=sizeof(FILE_FULL_DIR_INFORMATION);
			break;
		case FileIdBothDirectoryInformation:
			win_buffer.query_block_length=sizeof(FILE_ID_BOTH_DIR_INFORMATION);
			break;
		case FileIdFullDirectoryInformation:
			win_buffer.query_block_length=sizeof(FILE_ID_FULL_DIR_INFORMATION);
			break;
		default:
			DbgPrint("VFS filldir: bad info_class\n");

		}

		restart_scan = stack_location->Flags & SL_RESTART_SCAN;
		win_buffer.return_single_entry = stack_location->Flags & SL_RETURN_SINGLE_ENTRY;
		index_specified = stack_location->Flags & SL_INDEX_SPECIFIED;

		ExAcquireResourceExclusiveLite(&required_fcb->main_resource, TRUE);
		acquired_fcb = TRUE;

		win_buffer.buffer = VfsGetCallersBuffer(irp);
		//DbgPrint("query dir: buffer=%p buffer_end=%p\n", win_buffer.buffer, ((char*)win_buffer.buffer)+win_buffer.buffer_length);
		CHECK_OUT(win_buffer.buffer == NULL, STATUS_INVALID_USER_BUFFER)

		if (win_buffer.search_pattern != NULL) {
			if (ccb->search_pattern.Length == 0)
				win_buffer.first_time_query = TRUE;
			else
				RtlFreeUnicodeString(&ccb->search_pattern);

			VfsCopyUnicodeString(&ccb->search_pattern, win_buffer.search_pattern);

		} else if (ccb->search_pattern.Length == 0) {
			RtlInitUnicodeString(&ccb->search_pattern, L"*");
			win_buffer.first_time_query = TRUE;
		} else {
			win_buffer.search_pattern = &ccb->search_pattern;
		}

		//search_with_wildcards = FsRtlDoesNameContainWildCards(search_pattern);

		if (index_specified) {
			DbgPrint(" ne-a dat file index: %d\n", file_index);
			starting_index_for_search = file_index;
		} else if (restart_scan) {
			//DbgPrint("Restart scan req \n");
			starting_index_for_search = 0;
		} else {
			starting_index_for_search = ccb->offset.LowPart;
		}

		RtlZeroMemory(win_buffer.buffer, win_buffer.buffer_length);


		if (win_buffer.buffer_length < win_buffer.query_block_length)
			TRY_RETURN(STATUS_INFO_LENGTH_MISMATCH);

		win_buffer.used_length = 0;
		win_buffer.file = ccb->file;
		win_buffer.next_entry_offset=0;

		ccb->file->f_pos = starting_index_for_search;

		//DbgPrint("dir_ctrl : offset : **** %d\n", starting_index_for_search);

		if (win_buffer.return_single_entry && ! FsRtlDoesNameContainWildCards(win_buffer.search_pattern)) {
			struct dentry *de;
			struct qstr name;

			
			CHECK_OUT(!(name.name=VfsCopyUnicodeStringToZcharUnixPath(win_buffer.search_pattern)),  STATUS_NO_MEMORY);
			name.len=win_buffer.search_pattern->Length/2;
			name.hash=full_name_hash(name.name, name.len);

			if (!(de=cached_lookup(fcb->dentry, &name, 0))) {
				if ((de=real_lookup(fcb->dentry, &name, 0))) {
					filldir(&win_buffer, name.name, name.len, 0, de->d_inode->i_ino, DT_UNKNOWN);
					dput(de); //real_lookup does dget --sadyc
				}
			} else {
				filldir(&win_buffer, name.name, name.len, 0, de->d_inode->i_ino, DT_UNKNOWN);
				dput(de); //cached_lookup does dget --sadyc
			}

			ExFreePool(name.name);

		} else 
			vfs_readdir(ccb->file, filldir, &win_buffer);

		if (win_buffer.next_entry_offset)
			*win_buffer.next_entry_offset=0;

		//dump_buffer(win_buffer.buffer, win_buffer.buffer_length, "VfsQueryDirectory: buffer:");

		//DbgPrint("file->f_pos : %u\n", ccb->file->f_pos);

		ccb->offset.LowPart = (ULONG)ccb->file->f_pos;

		if (!win_buffer.used_length) {
			if (win_buffer.first_time_query)
				status = STATUS_NO_SUCH_FILE;
			else
				status = STATUS_NO_MORE_FILES;
		} else {
			status = win_buffer.status;
		}

try_exit:
		;
	}
	__finally {
		if (acquired_fcb)
			RELEASE(&fcb->required_fcb.main_resource);
		if (status == STATUS_PENDING) {
			status = STATUS_NOT_IMPLEMENTED;
			irp->IoStatus.Status = status;
			VfsDbgPrintComplete(irp);
			IoCompleteRequest(irp, IO_NO_INCREMENT);
		} else {
			irp->IoStatus.Information = win_buffer.used_length;
			irp->IoStatus.Status = status;
			VfsDbgPrintComplete(irp);
			IoCompleteRequest(irp, (CCHAR)(NT_SUCCESS(status) ? IO_DISK_INCREMENT : IO_NO_INCREMENT));
		}
	}

	return status;
}
