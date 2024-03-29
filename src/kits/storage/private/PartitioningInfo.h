//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//---------------------------------------------------------------------

#ifndef _PARTITIONING_INFO_H
#define _PARTITIONING_INFO_H

#include <DiskDeviceDefs.h>

struct partitionable_space_data;

class BPartitioningInfo {
public:
								BPartitioningInfo();
	virtual						~BPartitioningInfo();

			status_t			SetTo(off_t offset, off_t size);
			void				Unset();

			status_t			ExcludeOccupiedSpace(off_t offset,
									off_t size);

// TODO: We don't need the partition ID.
			partition_id		PartitionID() const;

			status_t			GetPartitionableSpaceAt(int32_t index,
									off_t* offset, off_t*size) const;
			int32_t				CountPartitionableSpaces() const;

			void				PrintToStream() const;
private:
			status_t			_InsertSpaces(int32_t index, int32_t count);
			void				_RemoveSpaces(int32_t index, int32_t count);

	friend class BPartition;

			partition_id		fPartitionID;
			partitionable_space_data* fSpaces;
			int32_t				fCount;
			int32_t				fCapacity;
};

#endif	// _PARTITIONING_INFO_H
