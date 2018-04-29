#include "todo.h"
#include "vfs.h"
#include "linux/blkdev.h"

#define SECTOR_SIZE get_hardsect_size(vcb->target_device)
#define TEMP_FS_NAME "vfs"
#define TEMP_FS_LENGTH 3

NTSTATUS VfsVolumeInformation(PDEVICE_OBJECT device, PIRP irp)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	struct vfs_vcb *vcb = NULL;
	PIO_STACK_LOCATION IrpSp = NULL;
	FS_INFORMATION_CLASS FsInformationClass;
	ULONG Length;
	PVOID SystemBuffer;
	BOOLEAN VcbResourceAcquired = FALSE;
	struct statfs statfs;
	BOOLEAN top_level;

	FsRtlEnterFileSystem();

	top_level = VfsIsIrpTopLevel(irp);

	VfsDbgPrintCall(device, irp);

	__try {
	vcb = (struct vfs_vcb*)device->DeviceExtension;
	ASSERT(vcb);
	
	vfs_statfs(vcb->super_block, &statfs);

	// -- todo ->if is synchronous
	CHECK_OUT(!ExAcquireResourceSharedLite(&vcb->vcb_resource, TRUE),
		STATUS_PENDING);
        VcbResourceAcquired = TRUE;

	IrpSp = IoGetCurrentIrpStackLocation(irp);
	FsInformationClass = IrpSp->Parameters.QueryVolume.FsInformationClass;

	Length = IrpSp->Parameters.QueryVolume.Length;
        SystemBuffer = irp->AssociatedIrp.SystemBuffer;

        RtlZeroMemory(SystemBuffer, Length);

	switch (FsInformationClass)
        {
        case FileFsVolumeInformation:
            {
                PFILE_FS_VOLUME_INFORMATION Buffer;
                ULONG                       VolumeLabelLength;
                ULONG                       RequiredLength;

                CHECK_OUT(Length < sizeof(FILE_FS_VOLUME_INFORMATION), 
					STATUS_INFO_LENGTH_MISMATCH);

                Buffer = (PFILE_FS_VOLUME_INFORMATION) SystemBuffer;

                Buffer->VolumeCreationTime.QuadPart = 0;
                Buffer->VolumeSerialNumber = vcb->vpb->SerialNumber;
                VolumeLabelLength = vcb->vpb->VolumeLabelLength;
                Buffer->VolumeLabelLength = VolumeLabelLength * sizeof(WCHAR);
                // I don't know what this means.
                Buffer->SupportsObjects = FALSE;
                RequiredLength = sizeof(FILE_FS_VOLUME_INFORMATION)
					+ VolumeLabelLength * sizeof(WCHAR) - sizeof(WCHAR);

                if (Length < RequiredLength) {
                    irp->IoStatus.Information = sizeof(FILE_FS_VOLUME_INFORMATION);
                    status = STATUS_BUFFER_OVERFLOW;
					//DbgPrint("FileFsVolumeInformation\n");
                    __leave;
                }

				RtlCopyMemory(Buffer->VolumeLabel, vcb->vpb->VolumeLabel, 
					vcb->vpb->VolumeLabelLength);

                irp->IoStatus.Information = RequiredLength;
                status = STATUS_SUCCESS;
                __leave;
            }

	    case FileFsSizeInformation:
            {
                PFILE_FS_SIZE_INFORMATION Buffer;

                CHECK_OUT(Length < sizeof(FILE_FS_SIZE_INFORMATION),
					STATUS_INFO_LENGTH_MISMATCH);

                Buffer = (PFILE_FS_SIZE_INFORMATION) SystemBuffer;

                Buffer->TotalAllocationUnits.QuadPart =
                    //vcb->PartitionInformation.PartitionLength.QuadPart / -- to fix
                    1000000 / statfs.f_bsize;
                Buffer->AvailableAllocationUnits.QuadPart = statfs.f_bfree;
                Buffer->SectorsPerAllocationUnit = 
					statfs.f_bsize / SECTOR_SIZE;
                Buffer->BytesPerSector = SECTOR_SIZE;
                irp->IoStatus.Information = sizeof(FILE_FS_SIZE_INFORMATION);
                status = STATUS_SUCCESS;
				//DbgPrint("VOLUME INFORMATION SIZE\n");
                __leave;
            }

	case FileFsDeviceInformation:
            {
                PFILE_FS_DEVICE_INFORMATION Buffer;

                CHECK_OUT(Length < sizeof(FILE_FS_DEVICE_INFORMATION),
					STATUS_INFO_LENGTH_MISMATCH);

                Buffer = (PFILE_FS_DEVICE_INFORMATION) SystemBuffer;

                Buffer->DeviceType = vcb->target_device->DeviceType;
                Buffer->Characteristics =
                    vcb->target_device->Characteristics;
                irp->IoStatus.Information = sizeof(FILE_FS_DEVICE_INFORMATION);
                status = STATUS_SUCCESS;
                __leave;
	}

	    case FileFsAttributeInformation:
            {
                PFILE_FS_ATTRIBUTE_INFORMATION  Buffer;
                ULONG                           RequiredLength;
		

                CHECK_OUT(Length < sizeof(FILE_FS_ATTRIBUTE_INFORMATION),
					STATUS_INFO_LENGTH_MISMATCH);

                Buffer = (PFILE_FS_ATTRIBUTE_INFORMATION) SystemBuffer;

                Buffer->FileSystemAttributes =
                    FILE_CASE_SENSITIVE_SEARCH | FILE_CASE_PRESERVED_NAMES;
                Buffer->MaximumComponentNameLength = 255; // --- to check

                Buffer->FileSystemNameLength = TEMP_FS_LENGTH * sizeof(WCHAR);//strlen(vcb->super_block->s_type->name) * sizeof(WCHAR);
                RequiredLength = sizeof(FILE_FS_ATTRIBUTE_INFORMATION) +
                   TEMP_FS_LENGTH  * sizeof(WCHAR) - sizeof(WCHAR);
		//strlen(vcb->super_block->s_type->name)
                if (Length < RequiredLength)
                {
                    irp->IoStatus.Information =
                        sizeof(FILE_FS_ATTRIBUTE_INFORMATION);
                    status = STATUS_BUFFER_OVERFLOW;
					//DbgPrint("FileFsAttributeInformation\n");
                    __leave;
                }

                VfsCharToWchar(Buffer->FileSystemName, TEMP_FS_NAME, 
			TEMP_FS_LENGTH);

                irp->IoStatus.Information = RequiredLength;
                status = STATUS_SUCCESS;
                __leave;
            }

	    case FileFsFullSizeInformation:
            {
                PFILE_FS_FULL_SIZE_INFORMATION Buffer;

                CHECK_OUT(Length < sizeof(FILE_FS_FULL_SIZE_INFORMATION), 
					STATUS_INFO_LENGTH_MISMATCH);

                Buffer = (PFILE_FS_FULL_SIZE_INFORMATION) SystemBuffer;

                Buffer->TotalAllocationUnits.QuadPart = 
					//vcb->PartitionInformation.PartitionLength.QuadPart / -- to fix
                    1000000 / statfs.f_bsize;
                Buffer->CallerAvailableAllocationUnits.QuadPart =
                Buffer->ActualAvailableAllocationUnits.QuadPart = statfs.f_bfree;
	
                Buffer->SectorsPerAllocationUnit =
                    statfs.f_bsize / SECTOR_SIZE;
                Buffer->BytesPerSector = SECTOR_SIZE;
                irp->IoStatus.Information = sizeof(FILE_FS_FULL_SIZE_INFORMATION);
                status = STATUS_SUCCESS;
		//DbgPrint("VOLUME INFORMATION SIZE FULL\n");
                __leave;
            }

        default:
            status = STATUS_INVALID_INFO_CLASS;
        }
try_exit:
	;
	}
	__finally {
		if (VcbResourceAcquired) {
			RELEASE(&vcb->vcb_resource);
		}
		if (!AbnormalTermination())
		{
			ASSERT(status != STATUS_PENDING);
		//if (status == STATUS_PENDING)
		//{
			//VfsPostRequest(IrpContext);
		//}
		//else
		//{
			VfsDbgPrintComplete(irp);
			irp->IoStatus.Status = status;
			IoCompleteRequest(irp,
				(CCHAR)(NT_SUCCESS(status) ? IO_DISK_INCREMENT : IO_NO_INCREMENT));
		//}
		}
	}

	if (top_level)
		IoSetTopLevelIrp(NULL);

	FsRtlExitFileSystem();

	return status;
}