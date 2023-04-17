/*
 * Copyright 2009, Alexandre Deckner, alex@zappotek.com
 * Distributed under the terms of the MIT License.
 */
#ifndef SHAKE_TRACKING_FILTER_H
#define SHAKE_TRACKING_FILTER_H


#include <MessageFilter.h>
#include <Point.h>

class BView;
class BHandler;
class BMessageRunner;

namespace BPrivate {

class LowPassFilter {
public:
						LowPassFilter(uint32_t size);
						~LowPassFilter();

			void	    Input(const BPoint& p);
			BPoint		Output() const;
private:
	BPoint* fPoints;
	uint32_t	fSize;
	BPoint	fSum;
};


class ShakeTrackingFilter : public BMessageFilter {
public:
								ShakeTrackingFilter(
									BView* targetView,
									uint32_t messageWhat,
									uint32_t countThreshold = 2,
									bigtime_t timeTreshold = 400000);

								~ShakeTrackingFilter();

			filter_result		Filter(BMessage* message, BHandler** _target);

private:
			BView*				fTargetView;
			uint32_t				fMessageWhat;

			BMessageRunner*		fCancelRunner;
			LowPassFilter		fLowPass;
			BPoint				fLastDelta;
			BPoint				fLastPosition;
			uint32_t				fCounter;
			uint32_t				fCountThreshold;
			bigtime_t			fTimeThreshold;
};

}	// namespace BPrivate

using BPrivate::ShakeTrackingFilter;
using BPrivate::LowPassFilter;

#endif	// SHAKE_TRACKING_FILTER_H
