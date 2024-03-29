/*
 * Copyright 2008, Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PCI2_H
#define _PCI2_H


#include <kits/netdevice_manager.h>
#include <kits/netPCI.h>


typedef struct pci_device pci_device;

typedef struct pci_device_module_info {
	driver_module_info info;

	uint8	(*read_io_8)(pci_device *device, addr_t mappedIOAddress);
	void	(*write_io_8)(pci_device *device, addr_t mappedIOAddress,
				uint8 value);
	uint16	(*read_io_16)(pci_device *device, addr_t mappedIOAddress);
	void	(*write_io_16)(pci_device *device, addr_t mappedIOAddress,
				uint16 value);
	uint32_t	(*read_io_32)(pci_device *device, addr_t mappedIOAddress);
	void	(*write_io_32)(pci_device *device, addr_t mappedIOAddress,
				uint32_t value);

	void	*(*ram_address)(pci_device *device, const void *physicalAddress);

	uint32_t	(*read_pci_config)(pci_device *device, uint16 offset,
				uint8 size);
	void	(*write_pci_config)(pci_device *device, uint16 offset,
				uint8 size, uint32_t value);
	status_t (*find_pci_capability)(pci_device *device, uint8 capID,
				uint8 *offset);
	void 	(*get_pci_info)(pci_device *device, struct pci_info *info);
	status_t (*find_pci_extended_capability)(pci_device *device, uint16 capID,
				uint16 *offset);
} pci_device_module_info;


/* Attributes of PCI device nodes */
#define B_PCI_DEVICE_DOMAIN		"pci/domain"		/* uint32_t */
#define B_PCI_DEVICE_BUS		"pci/bus"			/* uint8 */
#define B_PCI_DEVICE_DEVICE		"pci/device"		/* uint8 */
#define B_PCI_DEVICE_FUNCTION	"pci/function"		/* uint8 */

#endif	/* _PCI2_H */
