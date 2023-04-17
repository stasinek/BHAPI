/*
 * Copyright 2006, Ingo Weinhold <bonefish@cs.tu-berlin.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include <GridLayoutBuilder.h>

#include <new>

#include <SpaceLayoutItem.h>


using std::nothrow;


// constructor
BGridLayoutBuilder::BGridLayoutBuilder(float horizontalSpacing,
		float verticalSpacing)
	: fLayout((new BGridView(horizontalSpacing, verticalSpacing))
					->GridLayout())
{
}

// constructor
BGridLayoutBuilder::BGridLayoutBuilder(BGridLayout* layout)
	: fLayout(layout)
{
}


// constructor
BGridLayoutBuilder::BGridLayoutBuilder(BGridView* view)
	: fLayout(view->GridLayout())
{
}

// GridLayout
BGridLayout*
BGridLayoutBuilder::GridLayout() const
{
	return fLayout;
}

// View
BView*
BGridLayoutBuilder::View() const
{
	return fLayout->Owner();
}

// GetGridLayout
BGridLayoutBuilder&
BGridLayoutBuilder::GetGridLayout(BGridLayout** _layout)
{
	*_layout = fLayout;
	return *this;
}

// GetView
BGridLayoutBuilder&
BGridLayoutBuilder::GetView(BView** _view)
{
	*_view = fLayout->Owner();
	return *this;
}

// Add
BGridLayoutBuilder&
BGridLayoutBuilder::Add(BView* view, int32_t column, int32_t row,
	int32_t columnCount, int32_t rowCount)
{
	fLayout->AddView(view, column, row, columnCount, rowCount);
	return *this;
}

// Add
BGridLayoutBuilder&
BGridLayoutBuilder::Add(BLayoutItem* item, int32_t column, int32_t row,
	int32_t columnCount, int32_t rowCount)
{
	fLayout->AddItem(item, column, row, columnCount, rowCount);
	return *this;
}

// SetColumnWeight
BGridLayoutBuilder&
BGridLayoutBuilder::SetColumnWeight(int32_t column, float weight)
{
	fLayout->SetColumnWeight(column, weight);
	return *this;
}

// SetRowWeight
BGridLayoutBuilder&
BGridLayoutBuilder::SetRowWeight(int32_t row, float weight)
{
	fLayout->SetRowWeight(row, weight);
	return *this;
}

// SetInsets
BGridLayoutBuilder&
BGridLayoutBuilder::SetInsets(float left, float top, float right, float bottom)
{
	fLayout->SetInsets(left, top, right, bottom);
	return *this;
}
	
// cast operator BGridLayout*
BGridLayoutBuilder::operator BGridLayout*()
{
	return fLayout;
}

