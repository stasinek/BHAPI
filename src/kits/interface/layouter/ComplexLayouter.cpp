/*
 * Copyright 2007, Ingo Weinhold <bonefish@cs.tu-berlin.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include <ComplexLayouter.h>

#include <math.h>



#include <new>

#include <kits/kernel/OS.h>
#include <Size.h>

#include <AutoDeleter.h>

#include <LayoutOptimizer.h>
#include <SimpleLayouter.h>


//#define TRACE_COMPLEX_LAYOUTER	1
#if TRACE_COMPLEX_LAYOUTER
#	define TRACE(format...)	printf(format);
#	define TRACE_ONLY(x)	x
#else
#	define TRACE(format...)
#	define TRACE_ONLY(x)
#endif

using std::nothrow;


// MyLayoutInfo
class ComplexLayouter::MyLayoutInfo : public LayoutInfo {
public:
	MyLayoutInfo(int32_t elementCount, int32_t spacing)
		: fCount(elementCount),
		  fSpacing(spacing)
	{
		// We also store the location of the virtual elementCountth element.
		// Thus fLocation[i + 1] - fLocation[i] is the size of the ith element
		// (not considering spacing).
		fLocations = new(nothrow) int32_t[elementCount + 1];
	}

	~MyLayoutInfo()
	{
		delete[] fLocations;
	}

	void InitFromSizes(int32_t* sizes)
	{
		fLocations[0] = 0;
		for (int32_t i = 0; i < fCount; i++) 
			fLocations[i + 1] = fLocations[i] + sizes[i] + fSpacing;
	}

	virtual float ElementLocation(int32_t element)
	{
		if (element < 0 || element >= fCount)
			return 0;

		return fLocations[element];
	}

	virtual float ElementSize(int32_t element)
	{
		if (element < 0 || element >= fCount)
			return -1;

		return fLocations[element + 1] - fLocations[element] - 1
			- fSpacing;
	}

	virtual float ElementRangeSize(int32_t position, int32_t length)
	{
		if (position < 0 || length < 0 || position + length > fCount)
			return -1;

		return fLocations[position + length] - fLocations[position] - 1
			- fSpacing;
	}

	void Dump()
	{
		printf("ComplexLayouter::MyLayoutInfo(): %" B_PRId32 " elements:\n",
			fCount);
		for (int32_t i = 0; i < fCount + 1; i++) {
			printf("  %2" B_PRId32 ": location: %4" B_PRId32 "\n", i,
				fLocations[i]);
		}
	}

public:
	int32_t	fCount;
	int32_t	fSpacing;
	int32_t*	fLocations;
};


// Constraint
struct ComplexLayouter::Constraint {
	Constraint(int32_t start, int32_t end, int32_t min, int32_t max)
		: start(start),
		  end(end),
		  min(min),
		  max(max),
		  next(NULL)
	{
		if (min > max)
			max = min;
		effectiveMax = max;
	}

	void Restrict(int32_t newMin, int32_t newMax)
	{
		if (newMin > min)
			min = newMin;
		if (newMax < max)
			max = newMax;
		if (min > max)
			max = min;
		effectiveMax = max;
	}

	bool IsSatisfied(int32_t* sumValues) const
	{
		int32_t value = sumValues[end] - sumValues[start - 1];
		return (value >= min && value <= max);
	}

	int32_t		start;
	int32_t		end;
	int32_t		min;
	int32_t		max;
	int32_t		effectiveMax;
	Constraint*	next;
};


// SumItem
struct ComplexLayouter::SumItem {
	int32_t	min;
	int32_t	max;
	bool	minDirty;
	bool	maxDirty;
};


// SumItemBackup
struct ComplexLayouter::SumItemBackup {
	int32_t	min;
	int32_t	max;
};


// #pragma mark - ComplexLayouter


// constructor
ComplexLayouter::ComplexLayouter(int32_t elementCount, float spacing)
	: fElementCount(elementCount),
	  fSpacing((int32_t)spacing),
	  fConstraints(new(nothrow) Constraint*[elementCount]),
	  fWeights(new(nothrow) float[elementCount]),
	  fSums(new(nothrow) SumItem[elementCount + 1]),
	  fSumBackups(new(nothrow) SumItemBackup[elementCount + 1]),
	  fOptimizer(new(nothrow) LayoutOptimizer(elementCount)),
	  fUnlimited(B_SIZE_UNLIMITED / (elementCount == 0 ? 1 : elementCount)),
	  fMinMaxValid(false),
	  fOptimizerConstraintsAdded(false)
{
	if (fConstraints)
		memset(fConstraints, 0, sizeof(Constraint*) * fElementCount);

	if (fWeights) {
		for (int32_t i = 0; i < fElementCount; i++)
			fWeights[i] = 1.0f;
	}
}


// destructor
ComplexLayouter::~ComplexLayouter()
{
	for (int32_t i = 0; i < fElementCount; i++) {
		Constraint* constraint = fConstraints[i];
		fConstraints[i] = NULL;
		while (constraint != NULL) {
			Constraint* next = constraint->next;
			delete constraint;
			constraint = next;
		}
	}

	delete[] fConstraints;
	delete[] fWeights;
	delete[] fSums;
	delete[] fSumBackups;
  	delete fOptimizer;
}


// InitCheck
status_t ComplexLayouter::InitCheck() const
{
	if (!fConstraints || !fWeights || !fSums || !fSumBackups || !fOptimizer)
		return B_NO_MEMORY;
	return fOptimizer->InitCheck();
}


// AddConstraints
void ComplexLayouter::AddConstraints(int32_t element, int32_t length,
	float _min, float _max, float _preferred)
{
	if (element < 0 || length <= 0 || element + length > fElementCount)
		return;

	TRACE("%p->ComplexLayouter::AddConstraints(%ld, %ld, %ld, %ld, %ld)\n",
		this, element, length, (int32_t)_min, (int32_t)_max, (int32_t)_preferred);

	int32_t spacing = fSpacing * (length - 1);
	int32_t min = (int32_t)_min + 1 - spacing;
	int32_t max = (int32_t)_max + 1 - spacing;

	if (min < 0)
		min = 0;
	if (max > fUnlimited)
		max = fUnlimited;

	int32_t end = element + length - 1;
	Constraint** slot = fConstraints + end;
	while (*slot != NULL && (*slot)->start > element)
		slot = &(*slot)->next;

	if (*slot != NULL && (*slot)->start == element) {
		// previous constraint exists -- use stricter values
		(*slot)->Restrict(min, max);
	} else {
		// no previous constraint -- create new one
		Constraint* constraint = new(nothrow) Constraint(element, end, min,
			max);
		if (!constraint)
			return;
		constraint->next = *slot;
		*slot = constraint;
	}

	fMinMaxValid = false;
}


// SetWeight
void ComplexLayouter::SetWeight(int32_t element, float weight)
{
	if (element < 0 || element >= fElementCount)
		return;

	fWeights[element] = max_c(weight, 0);
}


// MinSize
float
ComplexLayouter::MinSize()
{
	_ValidateLayout();
	return fMin;
}


// MaxSize
float
ComplexLayouter::MaxSize()
{
	_ValidateLayout();
	return fMax;
}


// PreferredSize
float
ComplexLayouter::PreferredSize()
{
	return fMin;
}


// CreateLayoutInfo
LayoutInfo*
ComplexLayouter::CreateLayoutInfo()
{
	MyLayoutInfo* layoutInfo = new(nothrow) MyLayoutInfo(fElementCount,
		fSpacing);
	if (layoutInfo && !layoutInfo->fLocations) {
		delete layoutInfo;
		return NULL;
	}

	return layoutInfo;
}


// Layout
void ComplexLayouter::Layout(LayoutInfo* _layoutInfo, float _size)
{
	TRACE("%p->ComplexLayouter::Layout(%ld)\n", this, (int32_t)_size);

	if (fElementCount == 0)
		return;

	_ValidateLayout();

	MyLayoutInfo* layoutInfo = (MyLayoutInfo*)_layoutInfo;

	int32_t min = fSums[fElementCount].min;
	int32_t max = fSums[fElementCount].max;

	int32_t size = (int32_t)_size + 1 - (fElementCount - 1) * fSpacing;
	if (size < min)
		size = min;
	if (size > max)
		size = max;

	SumItem sums[fElementCount + 1];
	memcpy(sums, fSums, (fElementCount + 1) * sizeof(SumItem));

	sums[fElementCount].min = size;
	sums[fElementCount].max = size;
	sums[fElementCount].minDirty = (size != min);
	sums[fElementCount].maxDirty = (size != max);

	// propagate the size
	_PropagateChangesBack(sums, fElementCount - 1, NULL);
	_PropagateChanges(sums, fElementCount - 1, NULL);

#if TRACE_COMPLEX_LAYOUTER
	TRACE("Layout(%ld)\n", size);
	for (int32_t i = 0; i < fElementCount; i++) {
		SumItem& sum = sums[i + 1];
		TRACE("[%ld] minc = %4ld,  maxc = %4ld\n", i + 1, sum.min, sum.max);
	}
#endif

	int32_t sizes[fElementCount];
	if (!_Layout(size, sums, sizes)) {
	}

	layoutInfo->InitFromSizes(sizes);
}


// CloneLayouter
Layouter*
ComplexLayouter::CloneLayouter()
{
	ComplexLayouter* layouter
		= new(nothrow) ComplexLayouter(fElementCount, fSpacing);
	ObjectDeleter<ComplexLayouter> layouterDeleter(layouter);
	if (!layouter || layouter->InitCheck() != B_OK
		|| !layouter->fOptimizer->AddConstraintsFrom(fOptimizer)) {
		return NULL;
	}

	// clone the constraints
	for (int32_t i = 0; i < fElementCount; i++) {
		Constraint* constraint = fConstraints[i];
		Constraint** end = layouter->fConstraints + i;
		while (constraint) {
			*end = new(nothrow) Constraint(constraint->start, constraint->end,
				constraint->min, constraint->max);
			if (!*end)
				return NULL;

			end = &(*end)->next;
			constraint = constraint->next;
		}
	}

	// copy the other stuff
	memcpy(layouter->fWeights, fWeights, fElementCount * sizeof(float));
	memcpy(layouter->fSums, fSums, (fElementCount + 1) * sizeof(SumItem));
	memcpy(layouter->fSumBackups, fSumBackups,
		(fElementCount + 1) * sizeof(SumItemBackup));
	layouter->fMin = fMin;
	layouter->fMax = fMax;
	layouter->fMinMaxValid = fMinMaxValid;

	return layouterDeleter.Detach();
}


// _Layout
bool ComplexLayouter::_Layout(int32_t size, SumItem* sums, int32_t* sizes)
{
	// prepare the desired solution
	SimpleLayouter::DistributeSize(size, fWeights, sizes, fElementCount);
	if (_SatisfiesConstraints(sizes)) {
		// The desired solution already satisfies all constraints.
		return true;
	}

	double realSizes[fElementCount];
	for (int32_t i = 0; i < fElementCount; i++)
		realSizes[i] = sizes[i];

	if (!_AddOptimizerConstraints())
		return false;


	// prepare a feasible solution (the minimum)
	double values[fElementCount];
	for (int32_t i = 0; i < fElementCount; i++)
		values[i] = sums[i + 1].min - sums[i].min;

#if TRACE_COMPLEX_LAYOUTER
	TRACE("feasible solution vs. desired solution:\n");
	for (int32_t i = 0; i < fElementCount; i++)
		TRACE("%8.4f   %8.4f\n", values[i], realSizes[i]);
#endif

	// solve
	TRACE_ONLY(bigtime_t time = system_time();)
	if (!fOptimizer->Solve(realSizes, size, values))
		return false;
	TRACE_ONLY(time = system_time() - time;)

	// compute integer solution
	// The basic strategy is to floor() the sums. This guarantees that the
	// difference between two rounded sums remains in the range of floor()
	// and ceil() of their real value difference. Since the constraints have
	// integer values, the integer solution will therefore satisfy all
	// constraints the real solution satisfied.
	TRACE("computed solution in %lld us:\n", time);

	double realSum = 0;
	double previousSum = 0;
	for (int32_t i = 0; i < fElementCount; i++) {
		realSum += values[i];
		double roundedRealSum = floor(realSum);
		if (fuzzy_equals(realSum, roundedRealSum + 1))
			realSum = roundedRealSum + 1;
		sizes[i] = int32_t(roundedRealSum - previousSum);
		previousSum = roundedRealSum;

		TRACE("x[%ld] = %8.4f   %4ld\n", i, values[i], sizes[i]);
	}

	return _SatisfiesConstraints(sizes);
}


// _AddOptimizerConstraints
bool ComplexLayouter::_AddOptimizerConstraints()
{
	if (fOptimizerConstraintsAdded)
		return true;

	fOptimizer->RemoveAllConstraints();

	// add constraints
	for (int32_t i = 0; i < fElementCount; i++) {
		SumItem& sum = fSums[i + 1];

		Constraint* constraint = fConstraints[i];
		while (constraint != NULL) {
			SumItem& base = fSums[constraint->start];
			int32_t sumMin = base.min + constraint->min;
			int32_t baseMax = sum.max - constraint->min;
			bool minRedundant = (sumMin < sum.min && baseMax > base.max);

			int32_t sumMax = base.max + constraint->effectiveMax;
			int32_t baseMin = sum.min - constraint->effectiveMax;
			bool maxRedundant = (sumMax > sum.max && baseMin < base.min);

			if (!minRedundant || !maxRedundant) {
				bool success = true;
				if (constraint->min == constraint->effectiveMax) {
					// min and max equal -- add an equality constraint
					success = fOptimizer->AddConstraint(constraint->start - 1,
						constraint->end, constraint->min, true);
				} else {
					// min and max not equal -- add them individually,
					// unless redundant
					if (!minRedundant) {
						success |= fOptimizer->AddConstraint(
							constraint->start - 1, constraint->end,
							constraint->min, false);
					}
					if (!maxRedundant) {
						success |= fOptimizer->AddConstraint(constraint->end,
							constraint->start - 1,
							-constraint->effectiveMax, false);
					}
				}

				if (!success)
					return false;
			}

			constraint = constraint->next;
		}
	}

	fOptimizerConstraintsAdded = true;
	return true;
}


// _SatisfiesConstraints
bool ComplexLayouter::_SatisfiesConstraints(int32_t* sizes) const
{
	int32_t sumValues[fElementCount + 1];
	sumValues[0] = 0;
	for (int32_t i = 0; i < fElementCount; i++)
		sumValues[i + 1] = sumValues[i] + sizes[i];

	return _SatisfiesConstraintsSums(sumValues);
}


// _SatisfiesConstraintsSums
bool ComplexLayouter::_SatisfiesConstraintsSums(int32_t* sumValues) const
{
	for (int32_t i = 0; i < fElementCount; i++) {
		Constraint* constraint = fConstraints[i];
		while (constraint) {
			if (!constraint->IsSatisfied(sumValues))
				return false;

			constraint = constraint->next;
		}
	}

	return true;
}


// _ValidateLayout
void ComplexLayouter::_ValidateLayout()
{
	// The general idea for computing the min and max for the given constraints
	// is that we rewrite the problem a little. Instead of considering the
	// x_1, ... x_n (n = fElementCount) and the constraints of the form
	//   x_i + ... + x_{i+j} >= min[i,j] and
	//   x_i + ... + x_{i+j} >= max[i,j], with i >= 1, j >= 0, i + j <= n
	//   and min[i,j], max[i,j] >= 0
	// we define
	//   c[0] = 0
	//   c[i] = \sum_{k=1}^i x_k, for all i, 1 <= i <= n
	// and thus the constraints read:
	//   c[i+j] - c[i-1] >= min[i,j]
	//   c[i+j] - c[i-1] <= max[i,j]
	//
	// Let minc[i] and maxc[i] the limits imposed by the given constraints, i.e.
	//   minc[i] <= c[i] <= maxc[i] for any tuple of (c[i])_i satisfying the
	// constraints (minc[i] and maxc[i] are unique), then we gain:
	//   minc[i+j] >= c[i-1] + min[i,j]
	//   maxc[i+j] <= c[i-1] + min[i,j]
	//   minc[i-1] >= minc[i+j] - max[i,j]
	//   maxc[i-1] >= maxc[i+j] - min[i,j]
	// We can compute the minc[i] and maxc[i] in an iterative process,
	// propagating the first to kinds of constraints forward and the other two
	// backwards. First we start considering all min constraints only. They
	// can't contradict each other and are usually to be enforced over max
	// constraints. Afterwards we add the max constraints one by one. For each
	// one of them we propagate resulting changes back and forth. In case of
	// a conflict, we relax the max constraint as much as necessary to yield
	// a consistent set of constraints. After all constraints have been
	// incorporated, the resulting minc[n] and maxc[n] are the min and max
	// limits we wanted to compute.

	if (fMinMaxValid)
		return;

	fSums[0].min = 0;
	fSums[0].max = 0;

	int32_t maxSum = 0;
	for (int32_t i = 0; i < fElementCount; i++) {
		SumItem& sum = fSums[i + 1];
		sum.min = 0;
		sum.max = maxSum += fUnlimited;
		sum.minDirty = false;
		sum.maxDirty = false;
	}

	// apply min constraints forward:
	//   minc[i+j] >= minc[i-1] + min[i,j]
	for (int32_t i = 0; i < fElementCount; i++) {
		SumItem& sum = fSums[i + 1];

		Constraint* constraint = fConstraints[i];
		while (constraint != NULL) {
			int32_t minSum = fSums[constraint->start].min + constraint->min;
			if (minSum > sum.min) {
				sum.min = minSum;
			} else {
				TRACE("min constraint is redundant: x%ld + ... + x%ld >= %ld\n",
					constraint->start, constraint->end, constraint->min);
			}

			constraint = constraint->next;
		}
	}

	// apply min constraints backwards:
	//   maxc[i-1] <= maxc[i+j] - min[i,j]
	for (int32_t i = fElementCount - 1; i >= 0; i--) {
		SumItem& sum = fSums[i + 1];

		Constraint* constraint = fConstraints[i];
		while (constraint != NULL) {
			SumItem& base = fSums[constraint->start];
			int32_t baseMax = sum.max - constraint->min;
			if (baseMax < base.max)
				base.max = baseMax;

			constraint = constraint->next;
		}
	}

	// apply max constraints
	for (int32_t i = 0; i < fElementCount; i++) {
		Constraint* constraint = fConstraints[i];
		while (constraint != NULL) {
			_ApplyMaxConstraint(constraint, i);

			constraint = constraint->next;
		}
	}

#if TRACE_COMPLEX_LAYOUTER
	for (int32_t i = 0; i < fElementCount; i++) {
		SumItem& sum = fSums[i + 1];
		TRACE("[%ld] minc = %4ld,  maxc = %4ld\n", i + 1, sum.min, sum.max);
	}
#endif

	if (fElementCount == 0) {
		fMin = -1;
		fMax = B_SIZE_UNLIMITED;
	} else {
		int32_t spacing = (fElementCount - 1) * fSpacing;
		fMin = fSums[fElementCount].min + spacing - 1;
		fMax = fSums[fElementCount].max + spacing - 1;
		if (fMax >= fUnlimited)
			fMax = B_SIZE_UNLIMITED;
	}

	fOptimizerConstraintsAdded = false;
	fMinMaxValid = true;
}


// _ApplyMaxConstraint
void ComplexLayouter::_ApplyMaxConstraint(Constraint* currentConstraint, int32_t index)
{
	SumItem& sum = fSums[index + 1];
	SumItem& base = fSums[currentConstraint->start];

	// We want to apply:
	//   c[i+j] <= c[i-1] + max[i,j]
	//
	// This has the following direct consequences (let k = i + j):
	// (1) maxc[k] <= maxc[i-1] + max[i,j]
	// (2) minc[i-1] >= minc[k] - max[i,j]
	//
	// If maxc[k] or minc[i-i] changed, those changes have to be propagated
	// back.

	// apply (1) maxc[k] <= maxc[i-1] + max[i,j]
	int32_t max = currentConstraint->effectiveMax;
	int32_t sumMax = base.max + max;

	// enforce maxc[i+j] >= minc[i+j]
	if (sumMax < sum.min) {
		sumMax = sum.min;
		max = sumMax - base.max;
	}

	// apply (2) minc[i-1] >= minc[k] - max[i,j]
	// and check minc[i-1] <= maxc[i-1]
	int32_t baseMin = sum.min - max;
	if (baseMin > base.max) {
		baseMin = base.max;
		max = sum.min - baseMin;
		sumMax = base.max + max;
	}

	if (currentConstraint->effectiveMax != max) {
		TRACE("relaxing conflicting max constraint (1): "
			"x%ld + ... + x%ld <= %ld -> %ld\n", currentConstraint->start,
			currentConstraint->end, currentConstraint->effectiveMax, max);
	}
	currentConstraint->effectiveMax = max;

	if (baseMin <= base.min && sumMax >= sum.max) {
		TRACE("max constraint is redundant: x%ld + ... + x%ld <= %ld\n",
			currentConstraint->start, currentConstraint->end,
			currentConstraint->effectiveMax);
		return;
	}

	// backup old values, in case we detect a conflict later
	_BackupValues(index);

	int32_t diff;
	do {
		// apply the changes
		int32_t changedIndex = currentConstraint->start;

		if (baseMin > base.min) {
			base.min = baseMin;
			base.minDirty = true;
		}

		if (sumMax < sum.max) {
			changedIndex = index;
			sum.max = sumMax;
			sum.maxDirty = true;
		}

		// propagate the changes
		_PropagateChangesBack(fSums, changedIndex, currentConstraint);
		_PropagateChanges(fSums, index, currentConstraint);

		// check the new constraint again -- if it doesn't hold, it
		// conflicts with the other constraints
		diff = 0;

		// check (1) maxc[k] <= maxc[i-1] + max[i,j]
		max = currentConstraint->effectiveMax;
		sumMax = base.max + max;
		if (sumMax < sum.max)
			diff = sum.max - sumMax;

		// check (2) minc[i-1] >= minc[k] - max[i,j]
		baseMin = sum.min - max;
		if (baseMin > base.min)
			diff = max_c(diff, baseMin - base.min);

		// clear the dirty flags
		for (int32_t i = 0; i <= changedIndex; i++) {
			SumItem& sum = fSums[i + 1];
			sum.minDirty = false;
			sum.maxDirty = false;
		}

		// if we've got a conflict, we relax the constraint and try again
		if (diff > 0) {
			max += diff;
			TRACE("relaxing conflicting max constraint (2): "
				"x%ld + ... + x%ld <= %ld -> %ld\n", currentConstraint->start,
				currentConstraint->end, currentConstraint->effectiveMax, max);
			currentConstraint->effectiveMax = max;

			_RestoreValues(index);

			sumMax = base.max + max;
			baseMin = sum.min - max;

			if (baseMin <= base.min && sumMax >= sum.max)
				return;
		}
	} while (diff > 0);
}


// _PropagateChanges
/*!	Propagate changes forward using min and max constraints. Max constraints
	Beyond \a toIndex or at \a to toIndex after (and including)
	\a lastMaxConstraint will be ignored. To have all constraints be
	considered pass \c fElementCount and \c NULL.
*/
void ComplexLayouter::_PropagateChanges(SumItem* sums, int32_t toIndex,
	Constraint* lastMaxConstraint)
{
	for (int32_t i = 0; i < fElementCount; i++) {
		SumItem& sum = sums[i + 1];

		bool ignoreMaxConstraints = (i > toIndex);

		Constraint* constraint = fConstraints[i];
		while (constraint != NULL) {
			SumItem& base = sums[constraint->start];

			if (constraint == lastMaxConstraint)
				ignoreMaxConstraints = true;

			// minc[k] >= minc[i-1] + min[i,j]
			if (base.minDirty) {
				int32_t sumMin = base.min + constraint->min;
				if (sumMin > sum.min) {
					sum.min = sumMin;
					sum.minDirty = true;
				}
			}

			// maxc[k] <= maxc[i-1] + max[i,j]
			if (base.maxDirty && !ignoreMaxConstraints) {
				int32_t sumMax = base.max + constraint->effectiveMax;
				if (sumMax < sum.max) {
					sum.max = sumMax;
					sum.maxDirty = true;
				}
			}

			constraint = constraint->next;
		}

		if (sum.minDirty || sum.maxDirty) {
			if (sum.min > sum.max) {
				// TODO: Can this actually happen?
				TRACE("adjusted max in propagation phase: index: "
					"%ld: %ld -> %ld\n", i, sum.max, sum.min);
				sum.max = sum.min;
				sum.maxDirty = true;
			}
		}
	}
}


// _PropagateChangesBack
void ComplexLayouter::_PropagateChangesBack(SumItem* sums, int32_t changedIndex,
	Constraint* lastMaxConstraint)
{
	for (int32_t i = changedIndex; i >= 0; i--) {
		SumItem& sum = sums[i + 1];

		bool ignoreMaxConstraints = false;

		Constraint* constraint = fConstraints[i];
		while (constraint != NULL) {
			SumItem& base = sums[constraint->start];

			if (constraint == lastMaxConstraint)
				ignoreMaxConstraints = true;

			// minc[i-1] >= minc[k] - max[i,j]
			if (sum.minDirty && !ignoreMaxConstraints) {
				int32_t baseMin = sum.min - constraint->effectiveMax;
				if (baseMin > base.min) {
					if (baseMin > base.max) {
						TRACE("min above max in back propagation phase: index: "
							"(%ld -> %ld), min: %ld, max: %ld\n", i,
							constraint->start, baseMin, base.max);
					}
					base.min = baseMin;
					base.minDirty = true;
				}
			}

			// maxc[i-1] <= maxc[k] - min[i,j]
			if (sum.maxDirty) {
				int32_t baseMax = sum.max - constraint->min;
				if (baseMax < base.max) {
					if (baseMax < base.min) {
						TRACE("max below min in back propagation phase: index: "
							"(%ld -> %ld), max: %ld, min: %ld\n", i,
							constraint->start, baseMax, base.min);
					}
					base.max = baseMax;
					base.maxDirty = true;
				}
			}

			constraint = constraint->next;
		}
	}
}


// _BackupValues
void ComplexLayouter::_BackupValues(int32_t maxIndex)
{
	for (int32_t i = 0; i <= maxIndex; i++) {
		SumItem& sum = fSums[i + 1];
		fSumBackups[i + 1].min = sum.min;
		fSumBackups[i + 1].max = sum.max;
	}
}


// _RestoreValues
void ComplexLayouter::_RestoreValues(int32_t maxIndex)
{
	for (int32_t i = 0; i <= maxIndex; i++) {
		SumItem& sum = fSums[i + 1];
		sum.min = fSumBackups[i + 1].min;
		sum.max = fSumBackups[i + 1].max;
	}
}

