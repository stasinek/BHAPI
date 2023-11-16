/*
 * Copyright 2006-2011, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef	_GRID_LAYOUT_H
#define	_GRID_LAYOUT_H


#include <TwoDimensionalLayout.h>


class BGridLayout : public BTwoDimensionalLayout {
public:
								BGridLayout(float horizontal
										= B_USE_DEFAULT_SPACING,
									float vertical = B_USE_DEFAULT_SPACING);
								BGridLayout(BMessage* from);
	virtual						~BGridLayout();

			int32_t				CountColumns() const;
			int32_t				CountRows() const;

			float				HorizontalSpacing() const;
			float				VerticalSpacing() const;

			void				SetHorizontalSpacing(float spacing);
			void				SetVerticalSpacing(float spacing);
			void				SetSpacing(float horizontal, float vertical);

			float				ColumnWeight(int32_t column) const;
			void				SetColumnWeight(int32_t column, float weight);

			float				MinColumnWidth(int32_t column) const;
			void				SetMinColumnWidth(int32_t column, float width);

			float				MaxColumnWidth(int32_t column) const;
			void				SetMaxColumnWidth(int32_t column, float width);

			float				RowWeight(int32_t row) const;
			void				SetRowWeight(int32_t row, float weight);

			float				MinRowHeight(int row) const;
			void				SetMinRowHeight(int32_t row, float height);

			float				MaxRowHeight(int32_t row) const;
			void				SetMaxRowHeight(int32_t row, float height);

			BLayoutItem*		ItemAt(int32_t column, int32_t row) const;

	virtual	BLayoutItem*		AddView(BView* child);
	virtual	BLayoutItem*		AddView(int32_t index, BView* child);
	virtual	BLayoutItem*		AddView(BView* child, int32_t column, int32_t row,
									int32_t columnCount = 1, int32_t rowCount = 1);

	virtual	bool				AddItem(BLayoutItem* item);
	virtual	bool				AddItem(int32_t index, BLayoutItem* item);
	virtual	bool				AddItem(BLayoutItem* item, int32_t column,
									int32_t row, int32_t columnCount = 1,
									int32_t rowCount = 1);

	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* from);

	virtual	status_t			Perform(perform_code d, void* arg);

protected:
	virtual status_t			AllArchived(BMessage* into) const;
	virtual	status_t			AllUnarchived(const BMessage* from);
	virtual status_t			ItemArchived(BMessage* into,
									BLayoutItem* item, int32_t index) const;
	virtual status_t			ItemUnarchived(const BMessage* from,
									BLayoutItem* item, int32_t index);

	virtual	bool				ItemAdded(BLayoutItem* item, int32_t atIndex);
	virtual	void				ItemRemoved(BLayoutItem* item, int32_t fromIndex);

	virtual	bool				HasMultiColumnItems();
	virtual	bool				HasMultiRowItems();

	virtual	int32_t				InternalCountColumns();
	virtual	int32_t				InternalCountRows();
	virtual	void				GetColumnRowConstraints(
									orientation orientation,
									int32_t index,
									ColumnRowConstraints* constraints);
	virtual	void				GetItemDimensions(BLayoutItem* item,
									Dimensions* dimensions);
private:
			class DummyLayoutItem;
			class RowInfoArray;
			struct ItemLayoutData;

			bool				_IsGridCellEmpty(int32_t column, int32_t row);
			bool				_AreGridCellsEmpty(int32_t column, int32_t row,
									int32_t columnCount, int32_t rowCount);

			bool				_InsertItemIntoGrid(BLayoutItem* item);
			bool				_ResizeGrid(int32_t columnCount, int32_t rowCount);

			ItemLayoutData*		_LayoutDataForItem(BLayoutItem* item) const;

private:

	// FBC padding
	virtual	void				_ReservedGridLayout1();
	virtual	void				_ReservedGridLayout2();
	virtual	void				_ReservedGridLayout3();
	virtual	void				_ReservedGridLayout4();
	virtual	void				_ReservedGridLayout5();
	virtual	void				_ReservedGridLayout6();
	virtual	void				_ReservedGridLayout7();
	virtual	void				_ReservedGridLayout8();
	virtual	void				_ReservedGridLayout9();
	virtual	void				_ReservedGridLayout10();

	// forbidden methods
								BGridLayout(const BGridLayout&);
			void				operator =(const BGridLayout&);

			BLayoutItem***		fGrid;
			int32_t				fColumnCount;
			int32_t				fRowCount;

			RowInfoArray*		fRowInfos;
			RowInfoArray*		fColumnInfos;

			int32_t				fMultiColumnItems;
			int32_t				fMultiRowItems;

			uint32_t				_reserved[5];
};


#endif	// _GRID_LAYOUT_H
