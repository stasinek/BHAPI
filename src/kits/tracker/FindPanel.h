/*
Open Tracker License

Terms and Conditions

Copyright (c) 1991-2000, Be Incorporated. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice applies to all licensees
and shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF TITLE, MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
BE INCORPORATED BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Be Incorporated shall not be
used in advertising or otherwise to promote the sale, use or other dealings in
this Software without prior written authorization from Be Incorporated.

Tracker(TM), Be(R), BeOS(R), and BeIA(TM) are trademarks or registered trademarks
of Be Incorporated in the United States and other countries. Other brand product
names are registered trademarks or trademarks of their respective holders.
All rights reserved.
*/
#ifndef _FIND_PANEL_H
#define _FIND_PANEL_H


#include <ByteOrder.h>
#include <ObjectList.h>
#include <Window.h>
#include <View.h>

#include <DialogPane.h>
#include <MimeTypeList.h>
#include <Utilities.h>
#include <NodeWalker.h>


class BFilePanel;
class BQuery;
class BBox;
class BTextControl;
class BCheckBox;
class BMenuField;
class BFile;
class BPopUpMenu;
class BGridLayout;

namespace BPrivate {

class FindPanel;
class Model;
class DraggableIcon;
class TAttrView;

const uint32_t kVolumeItem = 'Fvol';
const uint32_t kAttributeItemMain = 'Fatr';
const uint32_t kByNameItem = 'Fbyn';
const uint32_t kByAttributeItem = 'Fbya';
const uint32_t kByFormulaItem = 'Fbyq';
const uint32_t kAddItem = 'Fadd';
const uint32_t kRemoveItem = 'Frem';

#ifdef _IMPEXP_TRACKER
_IMPEXP_TRACKER
#endif
BMenu* TrackerBuildRecentFindItemsMenu(const char* title);

struct MoreOptionsStruct {
	bool showMoreOptions;
	bool searchTrash;
	int32_t reserved1;
	bool temporary;
	bool reserved9;
	bool reserved10;
	bool reserved11;
	int32_t reserved3;
	int32_t reserved4;
	int32_t reserved5;
	int32_t reserved6;
	int32_t reserved7;
	int32_t reserved8;
	// reserve a bunch of fields so that we can add stuff later but not
	// make old queries incompatible. Reserved fields are set to 0 when
	// saved

	MoreOptionsStruct()
		:
		showMoreOptions(false),
		searchTrash(false),
		reserved1(0),
		temporary(true),
		reserved9(false),
		reserved10(false),
		reserved11(false),
		reserved3(0),
		reserved4(0),
		reserved5(0),
		reserved6(0),
		reserved7(0),
		reserved8(0)
	{
	}

	static void EndianSwap(void* castToThis);

	static void SetQueryTemporary(BNode*, bool on);
	static bool QueryTemporary(const BNode*);
};


class FindWindow : public BWindow {
public:
	FindWindow(const entry_ref* ref = NULL,
		bool editIfTemplateOnly = false);
	virtual ~FindWindow();

	FindPanel* BackgroundView() const
		{ return fBackground; }

	BNode* QueryNode() const
		{ return fFile; }

	const char* QueryName() const;
		// reads in the query name from either a saved name in a template
		// or form a saved query name

	static bool IsQueryTemplate(BNode* file);

protected:
	virtual	void MessageReceived(BMessage* message);

private:
	static BFile* TryOpening(const entry_ref* ref);
	static void GetDefaultQuery(BEntry &entry);
		// when opening an empty panel, use the default query to set the
		// panel up
	void SaveQueryAttributes(BNode* file, bool templateQuery);

	void Find();
		// retrieve the results
	void Save();
		// save the contents of the find window into the query file

	void SwitchToTemplate(const entry_ref*);
	bool FindSaveCommon(bool find);

	status_t SaveQueryAsAttributes(BNode*, BEntry*, bool queryTemplate,
		const BMessage* oldAttributes = 0, const BPoint* oldLocation = 0);

	void GetDefaultName(BString &);
	void GetPredicateString(BString &, bool &dynamicDate);
		// dynamic date is a date such as 'today'

	BFile* fFile;
	entry_ref fRef;
	bool fFromTemplate;
	bool fEditTemplateOnly;
	FindPanel* fBackground;
	mutable BString fQueryNameFromTemplate;
	BFilePanel* fSaveAsTemplatePanel;

	typedef BWindow _inherited;
};


class FindPanel : public BView {
public:
	FindPanel(BFile*, FindWindow* parent, bool fromTemplate,
		bool editTemplateOnly);
	virtual ~FindPanel();

	virtual	void AttachedToWindow();
	virtual	void Draw(BRect updateRect);
	virtual	void MessageReceived(BMessage*);

	void BuildAttrQuery(BQuery*, bool& dynamicDate) const;
	BPopUpMenu* MimeTypeMenu() const
		{ return fMimeTypeMenu; }
	BMenuItem* CurrentMimeType(const char** type = NULL) const;
	status_t SetCurrentMimeType(BMenuItem* item);
	status_t SetCurrentMimeType(const char* label);

	BPopUpMenu* VolMenu() const
		{ return fVolMenu; }
	uint32_t Mode() const
		{ return fMode; }

	static uint32_t InitialMode(const BNode* entry);
	void SaveWindowState(BNode*, bool editTemplate);

	void SwitchToTemplate(const BNode*);

	void GetByAttrPredicate(BQuery*, bool &dynamicDate) const;
		// build up a query from by-attribute items
	void GetByNamePredicate(BQuery*) const;
		// build up a simple query from the name we are searching for

	void GetDefaultName(BString&) const;
	void GetDefaultAttrName(BString&, int32_t) const;
	const char* UserSpecifiedName() const;
		// name filled out in the query name text field

	static void AddRecentQueries(BMenu*, bool addSaveAsItem,
		const BMessenger* target, uint32_t what);
		// populate the recent query menu with query templates and recent
		// queries

private:
	void AddMimeTypesToMenu();
		// populates the type menu
	static bool AddOneMimeTypeToMenu(const ShortMimeInfo*, void* castToMenu);

	void AddVolumes(BMenu*);
		// populates the volume menu
	void ShowVolumeMenuLabel();

	void AddAttrRow();
		// add one more attribute item to the attr view
	void RemoveAttrRow();
		// remove the last attribute item
	void AddFirstAttr();

	// panel building/restoring calls
	void RestoreWindowState(const BNode*);
	void RestoreMimeTypeMenuSelection(const BNode*);
	void AddByAttributeItems(const BNode*);
	void RemoveByAttributeItems();
	void RemoveAttrViewItems(bool removeGrid = true);
	void ShowOrHideMimeTypeMenu();
		// MimeTypeWindow is only shown in kByNameItem and
		// kByAttributeItem modes

	void AddAttributeControls(int32_t);

	void ShowOrHideMoreOptions(bool show);
		// fMode gets set by this and the call relies on it being
		// up-to-date
	static int32_t InitialAttrCount(const BNode*);
	void FillCurrentQueryName(BTextControl*, FindWindow*);
	void AddByNameOrFormulaItems();
	void SetUpAddRemoveButtons();

	void SwitchMode(uint32_t);
		// go from search by name to search by attribute, etc.

	void PushMimeType(BQuery* query) const;

	void SaveAsQueryOrTemplate(const entry_ref*, const char*, bool queryTemplate);

	BView* FindAttrView(const char*, int row) const;

	void AddAttributes(BMenu* menu, const BMimeType &type);
	void AddMimeTypeAttrs(BMenu* menu);
	void RestoreAttrState(const BMessage &, int32_t);
	void SaveAttrState(BMessage*, int32_t);
	void AddLogicMenu(int32_t, bool selectAnd = true);
	void RemoveLogicMenu(int32_t);

	void ResizeMenuField(BMenuField*);

	uint32_t fMode;
	BGridLayout* fAttrGrid;
	BPopUpMenu* fMimeTypeMenu;
	BMenuField* fMimeTypeField;
	BPopUpMenu* fVolMenu;
	BPopUpMenu* fSearchModeMenu;
	BPopUpMenu* fRecentQueries;
	BBox* fMoreOptions;
	BTextControl* fQueryName;
	BString fInitialQueryName;

	BCheckBox* fTemporaryCheck;
	BCheckBox* fSearchTrashCheck;

	PaneSwitch* fLatch;
	DraggableIcon* fDraggableIcon;

	typedef BView _inherited;

	friend class RecentQueriesPopUp;
};


class DeleteTransientQueriesTask {
	// transient queries get deleted if they didn't get used in a while;
	// this is the task that takes care of it
public:
	static void StartUpTransientQueryCleaner();

	bool DoSomeWork();
	virtual ~DeleteTransientQueriesTask();

protected:
	DeleteTransientQueriesTask();
		// returns true when done

	enum State {
		kInitial,
		kAllocatedWalker,
		kTraversing,
		kError
	};

	State state;

	void Initialize();
	bool GetSome();

	bool ProcessOneRef(Model*);

private:
	BTrackerPrivate::TNodeWalker* fWalker;
};


class RecentFindItemsMenu : public BMenu {
public:
	RecentFindItemsMenu(const char* title, const BMessenger* target,
		uint32_t what);

protected:
	virtual void AttachedToWindow();

private:
	BMessenger fTarget;
	uint32_t fWhat;
};


class DraggableQueryIcon : public DraggableIcon {
	// query/query template drag&drop helper
public:
	DraggableQueryIcon(BRect frame, const char* name,
		const BMessage* message, BMessenger target,
		uint32_t resizeFlags = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32_t flags = B_WILL_DRAW);

protected:
	virtual bool DragStarted(BMessage*);
};

} // namespace BPrivate

using namespace BPrivate;


#endif	// _FIND_PANEL_H
