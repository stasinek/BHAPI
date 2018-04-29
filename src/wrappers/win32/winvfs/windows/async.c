#include "vfs.h"

//
//	async irp processing thread
//
void VfsQueueHandlerThread(PVOID context)
{
	while (1) {
		KeWaitForSingleObject(&vfs_data.thread_queue.queue_event, Executive, KernelMode,
			FALSE, (PLARGE_INTEGER)NULL);

		while (!IsListEmpty(&vfs_data.thread_queue.thread_queue_list)) {
			HANDLE thread_handle;
			PLIST_ENTRY entry = NULL;
			struct vfs_irp_context *irp_context = NULL;

			entry = ExInterlockedRemoveHeadList(&vfs_data.thread_queue.thread_queue_list,
				&vfs_data.thread_queue.spin_lock);
			ASSERT(entry);
			irp_context = CONTAINING_RECORD(entry, struct vfs_irp_context, thread_queue_list);

			PsCreateSystemThread(&thread_handle, (ACCESS_MASK)0L, NULL, NULL, NULL, 
				VfsCommonDispatch, irp_context);
		}
	}
}

//
//	post request to system thread
//
NTSTATUS VfsPostRequest(struct vfs_irp_context *irp_context, PIRP irp)
{
	NTSTATUS status = STATUS_PENDING;

	IoMarkIrpPending(irp);

	ExInterlockedInsertTailList(&vfs_data.thread_queue.thread_queue_list, 
		&irp_context->thread_queue_list, &vfs_data.thread_queue.spin_lock);
	KeSetEvent(&vfs_data.thread_queue.queue_event, 0, FALSE);

	return status;
}

//
//	post request to system thread
//
NTSTATUS VfsPostRequestCreateContext(PDEVICE_OBJECT device, PIRP irp)
{
	NTSTATUS status = STATUS_PENDING;
	struct vfs_irp_context *irp_context = VfsAllocIrpContext(irp, device);

	ASSERT(irp_context);

	IoMarkIrpPending(irp);

	ExInterlockedInsertTailList(&vfs_data.thread_queue.thread_queue_list, 
		&irp_context->thread_queue_list, &vfs_data.thread_queue.spin_lock);
	KeSetEvent(&vfs_data.thread_queue.queue_event, 0, FALSE);

	return status;
}


//
//	common irp dispatch by system thread
//
void VfsCommonDispatch(void *context)
{
	NTSTATUS status;
	struct vfs_irp_context *irp_context = NULL;
	PIRP irp = NULL;

	irp_context = (struct vfs_irp_context*)context;
	ASSERT(irp_context);
	irp = irp_context->irp;
	ASSERT(irp);

	//DbgPrint("THREAD DISPATCHER\n");
	VfsDbgPrintCall(irp_context->target_device, irp);

	if (FLAG_ON(irp_context->flags, VFS_IRP_CONTEXT_NOT_TOP_LEVEL))
		IoSetTopLevelIrp((PIRP)FSRTL_FSP_TOP_LEVEL_IRP);

	SET_FLAG(irp_context->flags, VFS_IRP_CONTEXT_CAN_BLOCK);

	FsRtlEnterFileSystem();

	__try
	{
		switch(irp_context->major_function) {
		case IRP_MJ_CREATE:
			status = VfsCommonCreate(irp_context, irp_context->irp, FALSE);
			break;
		case IRP_MJ_CLEANUP:
			status = VfsCommonCleanup(irp_context, irp_context->irp, FALSE);
			break;
		case IRP_MJ_CLOSE:
			status = VfsCommonClose(irp_context, irp_context->irp, FALSE);
			break;
		case IRP_MJ_READ:
		case IRP_MJ_WRITE:
		default:

			irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
			irp->IoStatus.Information = 0;
			VfsDbgPrintComplete(irp);
			IoCompleteRequest(irp, IO_NO_INCREMENT);
			VfsFreeIrpContext(irp_context);
			break;
		}
	}
	__except (VfsExceptionFilter(irp_context, GetExceptionInformation()))
	{
		status = VfsExceptionHandler(irp_context, irp);
	}

	FsRtlExitFileSystem();

	IoSetTopLevelIrp(NULL);

	PsTerminateSystemThread(status);
}
