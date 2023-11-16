#include "vfs.h"

#define BEGIN_CASE(x, X) \
	case File##x##Information:\
	{\
		PFILE_##X##_INFORMATION Buffer=(PFILE_##X##_INFORMATION) SystemBuffer;\
		\
		CHECK_OUT(Length < sizeof(FILE_##X##_INFORMATION), STATUS_INFO_LENGTH_MISMATCH);
		


#define END_CASE(x, X) \
		irp->IoStatus.Information = sizeof(FILE_##X##_INFORMATION);\
		status = STATUS_SUCCESS;\
		__leave;\
	}\

#define FillBasic(Buffer)	\
	RtlSecondsSince1970ToTime (Fcb->inode->i_ctime, & Buffer->CreationTime);\
	RtlSecondsSince1970ToTime (Fcb->inode->i_atime, & Buffer->LastAccessTime);\
	RtlSecondsSince1970ToTime (Fcb->inode->i_mtime, & Buffer->LastWriteTime);\
	RtlSecondsSince1970ToTime (Fcb->inode->i_mtime, & Buffer->ChangeTime);\
        Buffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;\
	if (S_ISDIR(Fcb->inode->i_mode)) {\
		SET_FLAG(Buffer->FileAttributes, FILE_ATTRIBUTE_DIRECTORY);\
	}

#define FillStandard(Buffer) \
	Buffer->AllocationSize.QuadPart = Fcb->inode->i_size; \
	Buffer->EndOfFile.QuadPart = Fcb->inode->i_size; \
	Buffer->NumberOfLinks = Fcb->inode->i_nlink; \
	Buffer->DeletePending = FLAG_ON(Fcb->flags, VFS_FCB_DELETE_ON_CLOSE); \
	Buffer->Directory = S_ISDIR(Fcb->inode->i_mode);

#define FillInternal(Buffer) \
	Buffer->IndexNumber.QuadPart = Fcb->inode->i_ino;\
	//DbgPrint("FillInternal: Index=%d\n", (int)Buffer->IndexNumber.QuadPart);

#define FillEa(Buffer) \
	Buffer->EaSize = 0;

#define FillPosition(Buffer) \
	Buffer->CurrentByteOffset.QuadPart = Ccb->file->f_pos; 

#define FillName(buffer) \
	buffer->FileNameLength = Ccb->file->f_dentry->d_name.len * 2;\
	{\
		UNICODE_STRING str;\
		\
		str.Length = str.MaximumLength = Ccb->file->f_dentry->d_name.len * 2;	\
		str.Buffer = ExAllocatePoolWithTag(VFSPOOL, Ccb->file->f_dentry->d_name.len * 2, 'BRTS');	\
		VfsCharToWchar(str.Buffer, Ccb->file->f_dentry->d_name.name,	\
					(USHORT)Ccb->file->f_dentry->d_name.len);	\
		RtlCopyMemory(buffer->FileName, str.Buffer, str.Length);	\
		ExFreePool(str.Buffer);	\
	}


NTSTATUS VfsQueryInformation(PDEVICE_OBJECT device ,PIRP irp)
{
	PDEVICE_OBJECT	DeviceObject;
	NTSTATUS	status = STATUS_UNSUCCESSFUL;
	PFILE_OBJECT	FileObject;
	struct vfs_fcb* Fcb;
	struct vfs_ccb*	Ccb;
	PIO_STACK_LOCATION	IrpSp;
	FILE_INFORMATION_CLASS	FileInformationClass;
	ULONG		Length;
	PVOID		SystemBuffer;
	BOOLEAN		FcbResourceAcquired = FALSE;
	BOOLEAN top_level = FALSE;

	VfsDbgPrintCall(device, irp);

	FsRtlEnterFileSystem();

	top_level = VfsIsIrpTopLevel(irp);

	__try {
		IrpSp = IoGetCurrentIrpStackLocation(irp);
		FileObject = IrpSp->FileObject;
		Fcb = (struct vfs_fcb*) FileObject->FsContext;
		ASSERT(Fcb);

		if (!FLAG_ON(Fcb->flags, VFS_FCB_PAGE_FILE))
		{
			if(!ExAcquireResourceSharedLite(&Fcb->required_fcb.main_resource, TRUE)) {
				status = STATUS_PENDING;
				goto try_exit;
			}
			
			FcbResourceAcquired = TRUE;
		}

		Ccb = (struct vfs_ccb*)FileObject->FsContext2;
		ASSERT(Ccb != NULL);

		FileInformationClass = IrpSp->Parameters.QueryFile.FileInformationClass;
		Length = IrpSp->Parameters.QueryFile.Length;
	        SystemBuffer = irp->AssociatedIrp.SystemBuffer;
		RtlZeroMemory(SystemBuffer, Length);

		switch(FileInformationClass){
		
		BEGIN_CASE(Basic, BASIC)
			FillBasic(Buffer);
		END_CASE(Basic, BASIC)

		BEGIN_CASE(Standard, STANDARD)
			FillStandard(Buffer);
        END_CASE(Standard, STANDARD)

		BEGIN_CASE(Internal, INTERNAL)
			FillInternal(Buffer);
		END_CASE(Internal, INTERNAL)

		BEGIN_CASE(Ea, EA)
			FillEa(Buffer);
		END_CASE(Ea, EA)

		BEGIN_CASE(Name, NAME)
			CHECK_OUT(Length < sizeof(FILE_NAME_INFORMATION) +
				Ccb->file->f_dentry->d_name.len * 2 - sizeof(WCHAR),
					STATUS_INFO_LENGTH_MISMATCH);

			FillName(Buffer);

			irp->IoStatus.Information = sizeof(FILE_NAME_INFORMATION) +
				Ccb->file->f_dentry->d_name.len * 2 - sizeof(WCHAR);
			status = STATUS_SUCCESS;
			__leave;
		}

        BEGIN_CASE(Position, POSITION)
			FillPosition(Buffer);			
		END_CASE(Position, POSITION);

		BEGIN_CASE(All, ALL)
			FillBasic((&Buffer->BasicInformation));
			FillStandard((&Buffer->StandardInformation));
			FillInternal((&Buffer->InternalInformation));
			FillEa((&Buffer->EaInformation));
			FillPosition((&Buffer->PositionInformation));

			if (Length < sizeof(FILE_ALL_INFORMATION) +
				Ccb->file->f_dentry->d_name.len * 2 - sizeof(WCHAR)) {
				irp->IoStatus.Information = sizeof(FILE_ALL_INFORMATION);
				status = STATUS_BUFFER_OVERFLOW;
				__leave;
			}

			FillName((&Buffer->NameInformation));

			//DbgPrint("FillInternal: Index=%d\n", (int)Buffer->InternalInformation.IndexNumber.QuadPart);


			irp->IoStatus.Information = sizeof(FILE_ALL_INFORMATION) +
				Ccb->file->f_dentry->d_name.len * 2- sizeof(WCHAR);
			status = STATUS_SUCCESS;
			__leave;
		}

		BEGIN_CASE(NetworkOpen, NETWORK_OPEN)
			FillBasic(Buffer);
			Buffer->AllocationSize.QuadPart = Fcb->inode->i_size;
			Buffer->EndOfFile.QuadPart = Fcb->inode->i_size;
		END_CASE(NetworkOpen, NETWORK_OPEN)

		BEGIN_CASE(AttributeTag, ATTRIBUTE_TAG)
			Buffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;
			if (S_ISDIR(Fcb->inode->i_mode)) {
				SET_FLAG(Buffer->FileAttributes, FILE_ATTRIBUTE_DIRECTORY);
			}
			Buffer->ReparseTag = 0;
		END_CASE(AttributeTag, ATTRIBUTE_TAG);
                
		default:
			status = STATUS_INVALID_INFO_CLASS;
		}

try_exit:
		;
	}
	__finally {
		if (FcbResourceAcquired)
			RELEASE(&Fcb->required_fcb.main_resource);
		ASSERT(status != STATUS_PENDING);

		// irp->IoStatus.Information = 0;
		irp->IoStatus.Status = status;  // added by sadyc
		VfsDbgPrintComplete(irp);
		if (!NT_SUCCESS(status))
			IoCompleteRequest(irp, IO_NO_INCREMENT);
		else
			IoCompleteRequest(irp, IO_DISK_INCREMENT);
	}

	if (top_level)
		IoSetTopLevelIrp(NULL);

	FsRtlExitFileSystem();

	return status;
}


NTSTATUS VfsSetInformation(PDEVICE_OBJECT device ,PIRP irp)
{
	PDEVICE_OBJECT	DeviceObject;
	NTSTATUS	status = STATUS_UNSUCCESSFUL;
	PFILE_OBJECT	FileObject;
	struct vfs_fcb* Fcb;
	struct vfs_ccb*	Ccb;
	PIO_STACK_LOCATION	IrpSp;
	FILE_INFORMATION_CLASS	FileInformationClass;
	ULONG		Length;
	PVOID		SystemBuffer;
	BOOLEAN		FcbResourceAcquired = FALSE;
	BOOLEAN top_level = FALSE;

	VfsDbgPrintCall(device, irp);

	FsRtlEnterFileSystem();

	top_level = VfsIsIrpTopLevel(irp);

	__try {
		IrpSp = IoGetCurrentIrpStackLocation(irp);
		FileObject = IrpSp->FileObject;
		Fcb = (struct vfs_fcb*) FileObject->FsContext;
		ASSERT(Fcb);

		if (!FLAG_ON(Fcb->flags, VFS_FCB_PAGE_FILE))
		{
			if(!ExAcquireResourceSharedLite(&Fcb->required_fcb.main_resource, TRUE)) {
				status = STATUS_PENDING;
				goto try_exit;
			}
			
			FcbResourceAcquired = TRUE;
		}

		Ccb = (struct vfs_ccb*)FileObject->FsContext2;
		ASSERT(Ccb != NULL);

		FileInformationClass = IrpSp->Parameters.QueryFile.FileInformationClass;
		Length = IrpSp->Parameters.QueryFile.Length;
	        SystemBuffer = irp->AssociatedIrp.SystemBuffer;
		RtlZeroMemory(SystemBuffer, Length);

		switch(FileInformationClass){
		BEGIN_CASE(Position, POSITION)
			Ccb->file->f_pos = Buffer->CurrentByteOffset.QuadPart; 
		END_CASE(Position, POSITION)

		BEGIN_CASE(Basic, BASIC)
		END_CASE(Basic, BASIC)
		default:
			status = STATUS_INVALID_INFO_CLASS;
		}

try_exit:
		;
	}
	__finally {
		if (FcbResourceAcquired)
			RELEASE(&Fcb->required_fcb.main_resource);
		ASSERT(status != STATUS_PENDING);

		// irp->IoStatus.Information = 0;
		irp->IoStatus.Status = status;  // added by sadyc
		
		VfsDbgPrintComplete(irp);

		if (!NT_SUCCESS(status))
			IoCompleteRequest(irp, IO_NO_INCREMENT);
		else
			IoCompleteRequest(irp, IO_DISK_INCREMENT);
	}

	if (top_level)
		IoSetTopLevelIrp(NULL);

	FsRtlExitFileSystem();

	return status;
}

