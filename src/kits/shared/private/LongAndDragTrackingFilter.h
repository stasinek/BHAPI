/*
 * Copyright 2009, Alexandre Deckner, alex@zappotek.com
 * Distributed under the terms of the MIT License.
 */
#ifndef LONG_AND_DRAG_TRACKING_FILTER_H
#define LONG_AND_DRAG_TRACKING_FILTER_H


#include <MessageFilter.h>
#include <Point.h>


class BHandler;
class BMessageRunner;


namespace BPrivate {

class LongAndDragTrackingFilter : public BMessageFilter {
public:
								LongAndDragTrackingFilter(
									uint32_t longMessageWhat,
									uint32_t dragMessageWhat,
									float radiusThreshold = 4.0f,
									bigtime_t durationThreshold = 0);
								~LongAndDragTrackingFilter();

			filter_result		Filter(BMessage* message, BHandler** target);

private:
			void				_StopTracking();

			uint32_t				fLongMessageWhat;
			uint32_t				fDragMessageWhat;
			BMessageRunner*		fMessageRunner;
			BPoint				fClickPoint;
			uint32_t				fClickButtons;
			float				fSquaredRadiusThreshold;
			bigtime_t			fDurationThreshold;
};

}	// namespace BPrivate

using BPrivate::LongAndDragTrackingFilter;

#endif	// LONG_AND_DRAG_TRACKING_FILTER_H

