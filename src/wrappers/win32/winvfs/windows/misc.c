#include "vfs.h"
#include "ntstrsafe.h"

BOOLEAN VfsIsIrpTopLevel(PIRP irp)
{
	if (IoGetTopLevelIrp() == NULL) {
		IoSetTopLevelIrp(irp);
		return TRUE;
	}
	return FALSE;
}

ULONG VfsExceptionFilter(struct vfs_irp_context *irp_context, PEXCEPTION_POINTERS ex_ptrs)
{
	ULONG ret = EXCEPTION_EXECUTE_HANDLER;
	NTSTATUS ex_code = STATUS_SUCCESS;

	ex_code = ex_ptrs->ExceptionRecord->ExceptionCode;
	if ((ex_code == STATUS_IN_PAGE_ERROR) && (ex_ptrs->ExceptionRecord->NumberParameters >= 3))
		ex_code = ex_ptrs->ExceptionRecord->ExceptionInformation[2];
	if (irp_context) {
		irp_context->saved_exception_code = ex_code;
		SET_FLAG(irp_context->flags, VFS_IRP_CONTEXT_EXCEPTION);
	}

	if (!FsRtlIsNtstatusExpected(ex_code)) {
		ret = EXCEPTION_CONTINUE_SEARCH;
		if (irp_context)
			VfsFreeIrpContext(irp_context);
	}
	return ret;
}

NTSTATUS VfsExceptionHandler(struct vfs_irp_context *irp_context, PIRP irp)
{
	NTSTATUS status;

	DbgPrint("VfsExceptionHandler");

	ASSERT(irp);
	if (irp_context) {
		status = irp_context->saved_exception_code;
		VfsFreeIrpContext(irp_context);
	} else
		status = STATUS_INSUFFICIENT_RESOURCES;

	irp->IoStatus.Status = status;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return status;
}

void VfsGetFcbCcbVcbFromFileObject(PFILE_OBJECT file_obj, struct vfs_fcb **fcb,
								   struct vfs_ccb **ccb, struct vfs_vcb **vcb)
{
	*ccb = (struct vfs_ccb*)(file_obj->FsContext2);
	if (*ccb) {
		*fcb = (*ccb)->fcb;
		if ((*fcb)->identifier.type == VFS_NODE_TYPE_VCB) {
			*vcb = (struct vfs_vcb*)(*fcb);
			*fcb = NULL;
		} else {
			*vcb = (*fcb)->vcb;
		}
	}else {
		(*fcb) = CONTAINING_RECORD(file_obj->FsContext, struct vfs_fcb, required_fcb);
		if ((*fcb)->identifier.type == VFS_NODE_TYPE_FCB) {
			*vcb = (*fcb)->vcb;
		} else {
			*vcb = CONTAINING_RECORD(file_obj->FsContext, struct vfs_vcb, common_header);
			*fcb = NULL;
		}
	}
}

ULONG VfsAlign(ULONG number, ULONG alignment)
{
	if (alignment & (alignment-1))
		return number;
	if ((number & (alignment-1)) != 0) {
		number += alignment;
		number &= (~(alignment-1));
	}
	return number;
}

LONGLONG VfsAlign64(LONGLONG number, LONGLONG alignment)
{
	if (alignment & (alignment-1))
		return number;
	if ((number & (alignment-1)) != 0) {
		number += alignment;
		number &= (~(alignment-1));
	}
	return number;
}

ULONG VfsGetCurrentTime()
{
	LARGE_INTEGER curr_time;
	ULONG t;

	KeQuerySystemTime(&curr_time);

	t = 0;
	RtlTimeToSecondsSince1970(&curr_time, &t);

	return t;
}

void VfsCompleteRequest(PIRP irp, NTSTATUS status)
{
	if (irp != NULL) {
		if (NT_ERROR(status) && FLAG_ON(irp->Flags, IRP_INPUT_OPERATION))
			irp->IoStatus.Information = 0;
		irp->IoStatus.Status = status;
		IoCompleteRequest(irp, IO_DISK_INCREMENT);
	}
	return;
}

void* VfsGetCallersBuffer(PIRP irp)
{
	void *returned_buffer = NULL;

	if (irp->MdlAddress)
		returned_buffer = MmGetSystemAddressForMdl(irp->MdlAddress);
	else
		returned_buffer = irp->UserBuffer;

	return returned_buffer;
}

NTSTATUS VfsDummyIrp(PDEVICE_OBJECT dev_obj, PIRP irp)
{
	BOOLEAN top_level = FALSE;
	NTSTATUS status = STATUS_NOT_IMPLEMENTED;

	DbgPrint("****** DUMMY IRP Handler *******\n");
	VfsDbgPrintCall(dev_obj, irp);

	FsRtlEnterFileSystem();

	top_level = VfsIsIrpTopLevel(irp);

	irp->IoStatus.Status = status;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_DISK_INCREMENT);

	if (top_level) 
		IoSetTopLevelIrp(NULL);

	FsRtlExitFileSystem();

	return status;
}


const char * bdevname(kdev_t dev)
{
	static char buffer[32];
	RtlStringCbPrintfA(buffer, sizeof(buffer), "%#x", dev);
	return buffer;
//	return "unknown-block";
}


void fcb_inc_ref_count(struct vfs_fcb *fcb)
{
	InterlockedIncrement(&fcb->reference_count);
	if (fcb->dentry->d_name.name)
		printk(" ---------> FCB->reference_count: name= %s count= %d\n", fcb->dentry->d_name.name, fcb->reference_count);	
	else printk(" ---------> FCB->reference_count: name= <unknown> count= %d\n", fcb->reference_count);
}
void fcb_dec_ref_count(struct vfs_fcb *fcb)
{
	InterlockedDecrement(&fcb->reference_count);
	if (fcb->dentry->d_name.name)
		printk(" ---------> FCB->reference_count: name= %s count= %d\n", fcb->dentry->d_name.name, fcb->reference_count);	
	else printk(" ---------> FCB->reference_count: name= <unknown> count= %d\n", fcb->reference_count);
}
void vcb_inc_ref_count(struct vfs_vcb *vcb)
{
	InterlockedIncrement(&vcb->reference_count);
	printk(" ---------> VCB->refference_count: %d\n", vcb->reference_count);

}
void vcb_dec_ref_count(struct vfs_vcb *vcb)
{
	InterlockedDecrement(&vcb->reference_count);
	printk(" ---------> VCB->refference_count: %d\n", vcb->reference_count);
}
