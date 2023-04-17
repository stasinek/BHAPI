/*
 * Copyright 2006-2007, Ingo Weinhold <bonefish@cs.tu-berlin.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 *
 * Layouter implementation that can handle simple layout constraints
 * (restricting one element) only. It is 
 */
#ifndef	SIMPLE_LAYOUTER_H
#define	SIMPLE_LAYOUTER_H

#include <Layouter.h>


class BList;

namespace BPrivate {
namespace Layout {

class SimpleLayouter : public Layouter {
public:
								SimpleLayouter(int32_t elementCount,
									float spacing);
	virtual						~SimpleLayouter();

	virtual	void				AddConstraints(int32_t element, int32_t length,
									float min, float max, float preferred);
	virtual	void				SetWeight(int32_t element, float weight);

	virtual	float				MinSize();
	virtual	float				MaxSize();
	virtual	float				PreferredSize();

	virtual	LayoutInfo*			CreateLayoutInfo();
	
	virtual	void				Layout(LayoutInfo* layoutInfo, float size);

	virtual	Layouter*			CloneLayouter();

	static	void				DistributeSize(int32_t size, float weights[],
									int32_t sizes[], int32_t count);

private:
	static	long				_CalculateSumWeight(BList& elementInfos);
	
			void				_ValidateMinMax();
			void				_LayoutMax();
			void				_LayoutStandard();

private:
			class ElementLayoutInfo;
			class ElementInfo;
			class MyLayoutInfo;

			int32_t				fElementCount;
			int32_t				fSpacing;
			ElementInfo*		fElements;

			int32_t				fMin;
			int32_t				fMax;
			int32_t				fPreferred;

			bool				fMinMaxValid;

			MyLayoutInfo*		fLayoutInfo;
};

}	// namespace Layout
}	// namespace BPrivate

using BPrivate::Layout::SimpleLayouter;

#endif	// SIMPLE_LAYOUTER_H
