/*
 *  IRP_MJ_READ handler
 *  windows/read.c
 *
 *  Copyright (C) 2004 Adrian Stanciu (adrian@sadyc.net)
 */

#include "todo.h"
#include "vfs.h"

//
//	IRP_MJ_FILE_SYSTEM_CONTROL
//
NTSTATUS VfsRead(PDEVICE_OBJECT device, PIRP irp)
{
	NTSTATUS status = STATUS_NOT_IMPLEMENTED;
	PIO_STACK_LOCATION stack_location = NULL;
	BOOLEAN top_level = FALSE;
	struct vfs_irp_context *irp_context = NULL;

	VfsDbgPrintCall(device, irp);

	FsRtlEnterFileSystem();

	top_level = VfsIsIrpTopLevel(irp);

	ASSERT(device);
	ASSERT(irp);

	stack_location = IoGetCurrentIrpStackLocation(irp);

	__try
	{
		irp_context = VfsAllocIrpContext(irp, device);

		switch (stack_location->MinorFunction) {
			case IRP_MN_NORMAL:
				status = VfsReadNormal(device, irp);
				break;
			default:
				printk("not implemented\n");
				status = STATUS_NOT_IMPLEMENTED;
				break;
		}
	}
	__except (VfsExceptionFilter(irp_context, GetExceptionInformation())) {
		status = VfsExceptionHandler(irp_context, irp);
	}

	irp->IoStatus.Status = status;

	VfsDbgPrintComplete(irp);

	if (!NT_SUCCESS(status))
			IoCompleteRequest(irp, IO_NO_INCREMENT);
	else
			IoCompleteRequest(irp, IO_DISK_INCREMENT);

	if (top_level) {
		IoSetTopLevelIrp(NULL);
	}

	VfsFreeIrpContext(irp_context);
	
	FsRtlExitFileSystem();

	return status;

}

//extern asmlinkage ssize_t sys_read(file *file, char * buf, size_t count);
ssize_t sys_read(struct file *file, char * buf, size_t count);

NTSTATUS VfsReadNormal(PDEVICE_OBJECT device, PIRP Irp)
{
	PIO_STACK_LOCATION	IrpSp;
	PFILE_OBJECT		FileObject;
	NTSTATUS		Status;
	struct vfs_fcb*	Fcb;
	struct vfs_ccb*	Ccb;
	ULONG			Length;
	LARGE_INTEGER	ByteOffset;
	PUCHAR			UserBuffer;
	BOOLEAN			NoCache;
	BOOLEAN			SynchronousIo;
	BOOLEAN			FcbResourceAcquired = FALSE;
	ssize_t ret = 0;
	

	__try {
		IrpSp = IoGetCurrentIrpStackLocation(Irp);
		FileObject = IrpSp->FileObject;
		Fcb = (struct vfs_fcb*) FileObject->FsContext;
		Length = IrpSp->Parameters.Read.Length;
		ByteOffset = IrpSp->Parameters.Read.ByteOffset;
		NoCache = FLAG_ON(Irp->Flags, IRP_NOCACHE);
		SynchronousIo = FLAG_ON(FileObject->Flags, FO_SYNCHRONOUS_IO);

		//if (Irp->RequestorMode != KernelMode && !Irp->MdlAddress && Irp->UserBuffer) {
		//	ProbeForWrite(Irp->UserBuffer, Length, 1);
		//}

		if (Fcb->identifier.type != VFS_NODE_TYPE_FCB) {
			Status = STATUS_INVALID_PARAMETER;
			__leave;
		}

		if (S_ISDIR(Fcb->inode->i_mode)) {
			Status = STATUS_INVALID_PARAMETER;
			__leave;
		}

		ASSERT(Fcb);
		if (Length == 0 || Fcb->inode->i_size == 0) {
			Irp->IoStatus.Information = 0;
			Status = STATUS_SUCCESS;
			__leave;
		}
		
		if (!FLAG_ON(Fcb->flags, VFS_FCB_PAGE_FILE))
		{
			if(!ExAcquireResourceSharedLite(&Fcb->required_fcb.main_resource, TRUE)) {
				Status = STATUS_PENDING;
				goto try_exit;
			}			
			FcbResourceAcquired = TRUE;
		}

		Ccb = (struct vfs_ccb*)FileObject->FsContext2;
		ASSERT(Ccb != NULL);

		if (Irp->MdlAddress)
			UserBuffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
		else
			UserBuffer = Irp->UserBuffer;
		if (UserBuffer == NULL) {
			Status = STATUS_INVALID_USER_BUFFER;
			__leave;
		}

		if (!NoCache) {

			if (ByteOffset.QuadPart + Length > Fcb->inode->i_size) {
				if (ByteOffset.QuadPart >= Fcb->inode->i_size) {
					Status = STATUS_END_OF_FILE;
					__leave;
				}
				Length = (ULONG)Fcb->inode->i_size - (ULONG)ByteOffset.QuadPart;
			}
			
			//DbgPrint("READ FROM CACHE: ");
			if (FileObject->PrivateCacheMap == NULL) {
				CcInitializeCacheMap(FileObject, (PCC_FILE_SIZES)(&Fcb->required_fcb.common_header.AllocationSize),
					FALSE, &vfs_data.cache_mgr_callbacks, Ccb);
			}

			if (FLAG_ON(IrpSp->MinorFunction, IRP_MN_MDL))
			{
				//DbgPrint("READ: CcMdlRead\n");
				CcMdlRead(FileObject, &ByteOffset, Length, &Irp->MdlAddress, &Irp->IoStatus);
				Status = Irp->IoStatus.Status;
			}
			else
			{	

				if (!CcCopyRead(FileObject, &ByteOffset, Length, TRUE, UserBuffer, &Irp->IoStatus)) {
					Status = Irp->IoStatus.Status;
					__leave;
				} 
				Status = Irp->IoStatus.Status;

			}

		
			FileObject->CurrentByteOffset.QuadPart = Ccb->file->f_pos = 
				ByteOffset.QuadPart + Length;

			if (Ccb->file->f_pos > Ccb->file->f_dentry->d_inode->i_size) {
				//DbgPrint("BUG: file pointe over file size\n");
				Ccb->file->f_pos  = Ccb->file->f_dentry->d_inode->i_size;
			}

			ret = Length;
			//DbgPrint("READ FROM CACHE: len=%d\n", (int)ret);
		} else {
		if (ByteOffset.QuadPart + Length > Fcb->inode->i_size) {
		if (ByteOffset.QuadPart >= Fcb->required_fcb.common_header.AllocationSize.QuadPart) {
			Status = STATUS_END_OF_FILE;
			__leave;
		}
			Length = (ULONG)Fcb->required_fcb.common_header.AllocationSize.QuadPart - (ULONG)ByteOffset.QuadPart;
		}
			// ddk - IRP_MJ_READ
			//DbgPrint("READ DIRECT FROM DISK: ");
			if (ByteOffset.LowPart!=0 || ByteOffset.LowPart!=0) {
				if (ByteOffset.LowPart == FILE_USE_FILE_POINTER_POSITION && ByteOffset.HighPart == -1)
				;
				else
					Ccb->file->f_pos = ByteOffset.LowPart;
			}

			ret = sys_read(Ccb->file, UserBuffer, Length);
		
			//Irp->IoStatus.Information = ret;
			ret = Length;
		}

		if (ret >= 0) {
			Irp->IoStatus.Information = ret;
			Status = STATUS_SUCCESS;
		} else {
			Irp->IoStatus.Information = 0;
			Status = STATUS_UNEXPECTED_IO_ERROR;
		}

try_exit:
		;
	}
	__finally {
		if (FcbResourceAcquired)
			RELEASE(&Fcb->required_fcb.main_resource);
		ASSERT(Status != STATUS_PENDING);

	}
	return Status;
}