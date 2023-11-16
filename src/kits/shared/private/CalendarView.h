/*
 * Copyright 2007-2011, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CALENDAR_VIEW_H_
#define _CALENDAR_VIEW_H_


#include <DateTime.h>


#include <DateFormat.h>
#include <Invoker.h>
#include <kits/support/List.h>
#include <LocaleClass.h>
#include <kits/support/String.h>
#include <View.h>


class BMessage;


namespace BPrivate {


class BCalendarView : public BView, public BInvoker {
public:
								BCalendarView(BRect frame, const char* name,
									uint32_t resizeMask = B_FOLLOW_LEFT
										| B_FOLLOW_TOP,
									uint32_t flags = B_WILL_DRAW | B_FRAME_EVENTS
										| B_NAVIGABLE);

								BCalendarView(const char* name,
									uint32_t flags = B_WILL_DRAW | B_FRAME_EVENTS
										| B_NAVIGABLE);

	virtual						~BCalendarView();

								BCalendarView(BMessage* archive);
	static 	BArchivable*		Instantiate(BMessage* archive);
	virtual status_t			Archive(BMessage* archive,
									bool deep = true) const;

	virtual void				AttachedToWindow();

	virtual void				FrameResized(float width, float height);

	virtual void				Draw(BRect updateRect);

	virtual void				DrawDay(BView* owner, BRect frame,
									const char* text, bool isSelected = false,
									bool isEnabled = true, bool focus = false);
	virtual void				DrawDayName(BView* owner, BRect frame,
									const char* text);
	virtual void				DrawWeekNumber(BView* owner, BRect frame,
									const char* text);

			uint32_t				SelectionCommand() const;
			BMessage*			SelectionMessage() const;
	virtual void				SetSelectionMessage(BMessage* message);

			uint32_t				InvocationCommand() const;
			BMessage*			InvocationMessage() const;
	virtual void				SetInvocationMessage(BMessage* message);

	virtual void				MakeFocus(bool state = true);
	virtual status_t			Invoke(BMessage* message = NULL);

	virtual void				MouseDown(BPoint where);

	virtual void				KeyDown(const char* bytes, int32_t numBytes);

	virtual void				ResizeToPreferred();
	virtual void				GetPreferredSize(float* width, float* height);

	virtual	BSize				MaxSize();
	virtual	BSize				MinSize();
	virtual	BSize				PreferredSize();

			int32_t				Day() const;
			int32_t				Month() const;
			int32_t				Year() const;

			bool				SetDay(int32_t day);
			bool				SetMonth(int32_t month);
			bool				SetYear(int32_t year);

			BDate				Date() const;
			bool				SetDate(const BDate& date);
			bool				SetDate(int32_t year, int32_t month, int32_t day);

			BWeekday			StartOfWeek() const;
			void				SetStartOfWeek(BWeekday startOfWeek);

			bool				IsDayNameHeaderVisible() const;
			void				SetDayNameHeaderVisible(bool visible);

			bool				IsWeekNumberHeaderVisible() const;
			void				SetWeekNumberHeaderVisible(bool visible);

private:
			struct 				Selection {
									Selection()
										: row(0), column(0)
									{
									}

									void 									SetTo(int32_t _row, int32_t _column)
									{
										row = _row;
										column = _column;
									}

									int32_t row;
									int32_t column;

									Selection& operator=(const Selection& s)
									{
										row = s.row;
										column = s.column;
										return *this;
									}

									bool operator==(const Selection& s) const
									{
										return row == s.row
											&& column == s.column;
									}

									bool operator!=(const Selection& s) const
									{
										return row != s.row
											|| column != s.column;
									}
								};

			void				_InitObject();

			void				_SetToDay();
			void				_GetYearMonthForSelection(
									const Selection& selection, int32_t* year,
									int32_t* month) const;
			void				_GetPreferredSize(float* width, float* height);

			void				_SetupDayNames();
			void				_SetupDayNumbers();
			void				_SetupWeekNumbers();

			void				_DrawDays();
			void				_DrawFocusRect();
			void				_DrawDayHeader();
			void				_DrawWeekHeader();
			void				_DrawDay(int32_t curRow, int32_t curColumn,
									int32_t row, int32_t column, int32_t counter,
									BRect frame, const char* text,
									bool focus = false);
			void				_DrawItem(BView* owner, BRect frame,
									const char* text, bool isSelected = false,
									bool isEnabled = true, bool focus = false);

			void				_UpdateSelection();
			BRect				_FirstCalendarItemFrame() const;
			BRect				_SetNewSelectedDay(const BPoint& where);

			BRect				_RectOfDay(const Selection& selection) const;

private:
			BMessage*			fSelectionMessage;

			BDate				fDate;

			Selection			fFocusedDay;
			Selection			fNewFocusedDay;
			bool				fFocusChanged;

			Selection			fSelectedDay;
			Selection			fNewSelectedDay;
			bool				fSelectionChanged;

			int32_t				fStartOfWeek;
			bool				fDayNameHeaderVisible;
			bool				fWeekNumberHeaderVisible;

			BString				fDayNames[7];
			BString				fWeekNumbers[6];
			BString				fDayNumbers[6][7];

			// hide copy constructor & assignment
								BCalendarView(const BCalendarView& view);
			BCalendarView&		operator=(const BCalendarView& view);
};


}	// namespace BPrivate


#endif	// _CALENDAR_VIEW_H_
