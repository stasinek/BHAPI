#include "vfs.h"


NTSTATUS VfsReadWriteCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
    PMDL Mdl;

    ASSERT(Irp != NULL);

    *Irp->UserIosb = Irp->IoStatus;

	if (Irp->PendingReturned) {
        KeSetEvent(Irp->UserEvent, IO_NO_INCREMENT, FALSE);
	}

    if (Irp->AssociatedIrp.SystemBuffer && FlagOn(Irp->Flags, IRP_DEALLOCATE_BUFFER)) {
        ExFreePool(Irp->AssociatedIrp.SystemBuffer);
    }
    else {
        while ((Mdl = Irp->MdlAddress)) {
            Irp->MdlAddress = Mdl->Next;
            IoFreeMdl(Mdl);
        }
    }

    IoFreeIrp(Irp);

    //DbgPrint("FINISHED READ COMPLETION\n");

    return STATUS_MORE_PROCESSING_REQUIRED;
}


//
//	read physical blocks
//
NTSTATUS VfsReadPhysicalBlocks(PDEVICE_OBJECT block_dev, PVOID buffer, LARGE_INTEGER start_block, ULONG blk_cnt)
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG blk_size;
	ULONG bytes_cnt;
	KEVENT event;
	PIRP irp;
	IO_STATUS_BLOCK iosb;
	LARGE_INTEGER offset;

	//DbgPrint("READING FROM DISK ... dev: %p start_block: %d count: %u\n", block_dev, (int) start_block.QuadPart, blk_cnt);

	blk_size = block_dev->SectorSize;
	bytes_cnt = blk_size * blk_cnt;
	offset.QuadPart = start_block.QuadPart * blk_size;

	__try
	{
		KeInitializeEvent(&event, NotificationEvent, FALSE);
		irp = IoBuildAsynchronousFsdRequest(IRP_MJ_READ, block_dev, buffer, bytes_cnt, &offset, &iosb);
		CHECK_OUT(!irp, STATUS_INSUFFICIENT_RESOURCES);

		irp->UserEvent = &event;
		IoSetCompletionRoutine(irp, VfsReadWriteCompletion, NULL, TRUE, TRUE, TRUE);

		status = IoCallDriver(block_dev, irp);
		if (status == STATUS_PENDING) {
			KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
			status = iosb.Status;
		}
try_exit:
		;
	}
	__finally
	{
	}

	//DbgPrint("FINISHED BREAD");
	return status;
}

//
//	write physical blocks
//
NTSTATUS VfsWritePhysicalBlocks(PDEVICE_OBJECT block_dev, PVOID buffer, LARGE_INTEGER start_block, ULONG blk_cnt)
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG blk_size;
	ULONG bytes_cnt;
	KEVENT event;
	PIRP irp;
	IO_STATUS_BLOCK iosb;
	LARGE_INTEGER offset;

	//DbgPrint("WRITING TO DISK ... \n");

	return status;

	blk_size = block_dev->SectorSize;
	bytes_cnt = blk_size * blk_cnt;
	offset.QuadPart = start_block.QuadPart * blk_size;

	__try
	{
		KeInitializeEvent(&event, NotificationEvent, FALSE);
		irp = IoBuildSynchronousFsdRequest(IRP_MJ_WRITE, block_dev, buffer, bytes_cnt, &offset, &event, &iosb);
		CHECK_OUT(!irp, STATUS_INSUFFICIENT_RESOURCES);

		status = IoCallDriver(block_dev, irp);
		if (status == STATUS_PENDING) {
			KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
			status = iosb.Status;
		}
try_exit:
		;
	}
	__finally
	{
	}
	return status;
}
