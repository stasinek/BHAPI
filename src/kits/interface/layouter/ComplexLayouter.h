/*
 * Copyright 2007, Ingo Weinhold <bonefish@cs.tu-berlin.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 *
 * Layouter implementation that can handle complex constraints.
 */
#ifndef COMPLEX_LAYOUTER_H
#define COMPLEX_LAYOUTER_H

#include <kits/support/List.h>

#include <Layouter.h>


namespace BPrivate {
namespace Layout {

class LayoutOptimizer;

class ComplexLayouter : public Layouter {
public:
								ComplexLayouter(int32_t elementCount,
									float spacing);
	virtual						~ComplexLayouter();

	virtual	status_t			InitCheck() const;

	virtual	void				AddConstraints(int32_t element, int32_t length,
									float min, float max, float preferred);
	virtual	void				SetWeight(int32_t element, float weight);

	virtual	float				MinSize();
	virtual	float				MaxSize();
	virtual	float				PreferredSize();

	virtual	LayoutInfo*			CreateLayoutInfo();
	
	virtual	void				Layout(LayoutInfo* layoutInfo, float size);

	virtual	Layouter*			CloneLayouter();

private:
			class MyLayoutInfo;
			struct Constraint;
			struct SumItem;
			struct SumItemBackup;

			bool				_Layout(int32_t size, SumItem* sums,
									int32_t* sizes);
			bool				_AddOptimizerConstraints();
			bool				_SatisfiesConstraints(int32_t* sizes) const;
			bool				_SatisfiesConstraintsSums(int32_t* sums) const;

			void				_ValidateLayout();
			void				_ApplyMaxConstraint(
									Constraint* currentConstraint, int32_t index);
			void				_PropagateChanges(SumItem* sums, int32_t toIndex,
									Constraint* lastMaxConstraint);
			void				_PropagateChangesBack(SumItem* sums,
									int32_t changedIndex,
									Constraint* lastMaxConstraint);
		
			void				_BackupValues(int32_t maxIndex);
			void				_RestoreValues(int32_t maxIndex);

private:
			int32_t				fElementCount;
			int32_t				fSpacing;
			Constraint**		fConstraints;
			float*				fWeights;
			SumItem*			fSums;
			SumItemBackup*		fSumBackups;
			LayoutOptimizer*	fOptimizer;
			float				fMin;
			float				fMax;
			int32_t				fUnlimited;
			bool				fMinMaxValid;
			bool				fOptimizerConstraintsAdded;
};

}	// namespace Layout
}	// namespace BPrivate

using BPrivate::Layout::ComplexLayouter;


#endif	// COMPLEX_LAYOUTER_H
