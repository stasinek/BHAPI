/*
 * Copyright 2007, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PARTITION_REFERENCE_H
#define _PARTITION_REFERENCE_H

#include <DiskDeviceDefs.h>

#include <Referenceable.h>


namespace BPrivate {


class PartitionReference : public BReferenceable {
public:
								PartitionReference(partition_id id = -1,
									int32_t changeCounter = 0);
								~PartitionReference();

			void				SetTo(partition_id id, int32_t changeCounter);

			partition_id		PartitionID() const;
			void				SetPartitionID(partition_id id);

			int32_t				ChangeCounter() const;
			void				SetChangeCounter(int32_t counter);

private:
			partition_id		fID;
			int32_t				fChangeCounter;
};


}	// namespace BPrivate

using BPrivate::PartitionReference;

#endif	// _PARTITION_REFERENCE_H
