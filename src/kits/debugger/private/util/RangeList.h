/*
 * Copyright 2013, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef RANGE_LIST_H
#define RANGE_LIST_H


#include <ObjectList.h>
#include <Haiku.h>


struct Range {
	int32_t lowerBound;
	int32_t upperBound;

	Range()
		:
		lowerBound(-1),
		upperBound(-1)
	{
	}

	Range(int32_t lowValue, int32_t highValue)
		:
		lowerBound(lowValue),
		upperBound(highValue)
	{
	}
};


class RangeList : private BObjectList<Range>
{
public:
							RangeList();
	virtual					~RangeList();


			status_t		AddRange(int32_t lowValue, int32_t highValue);
			status_t		AddRange(const Range& range);

			void			RemoveRangeAt(int32_t index);

			int32_t			CountRanges() const;
			const Range*	RangeAt(int32_t index) const;

			bool			Contains(int32_t value) const;

private:
			void			_CollapseOverlappingRanges(int32_t startIndex,
								int32_t highValue);
};

#endif // RANGE_LIST_H
