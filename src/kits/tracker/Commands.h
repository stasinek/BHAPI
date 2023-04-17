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
#ifndef _COMMANDS_H
#define _COMMANDS_H


#include <MountServer.h>
#include <Haiku.h>

#include <tracker_private.h>
	// public commands moved here so they may be included by other apps


namespace BPrivate {

// external app messages

const uint32_t kGetInfo = 'Tinf';
const uint32_t kDelete = 'Tdel';
const uint32_t kRestoreFromTrash = 'Tres';
const uint32_t kIdentifyEntry = 'Tidt';
const uint32_t kOpenSelection = 'Tosl';
const uint32_t kOpenSelectionWith = 'Tosu';
const uint32_t kCloseAllWindows = 'Tall';
const uint32_t kCloseWindowAndChildren = 'Tcwc';
const uint32_t kCloseAllInWorkspace = 'Tciw';

// end external app messages

const uint32_t kOpenPreviouslyOpenWindows = 'Topw';
const uint32_t kRestoreState = 'Trst';

const uint32_t kCutMoreSelectionToClipboard = 'Tmvm';
const uint32_t kCopyMoreSelectionToClipboard = 'Tcpm';
const uint32_t kPasteLinksFromClipboard = 'Tplc';
const uint32_t kCancelSelectionToClipboard = 'Tesc';
const uint32_t kClipboardPosesChanged = 'Tcpc';

const uint32_t kEditItem = 'Tedt';
const uint32_t kEditQuery = 'Qedt';
const uint32_t kNewFolder = 'Tnwf';
const uint32_t kNewEntryFromTemplate = 'Tnwe';
const uint32_t kCopySelectionTo = 'Tcsl';
const uint32_t kMoveSelectionTo = 'Tmsl';
const uint32_t kCreateLink = 'Tlnk';
const uint32_t kCreateRelativeLink = 'Trln';
const uint32_t kDuplicateSelection = 'Tdsl';
const uint32_t kLoadAddOn = 'Tlda';
const uint32_t kEmptyTrash = 'Tetr';
const uint32_t kAddPrinter = 'Tadp';
const uint32_t kMakeActivePrinter = 'Tmap';
const uint32_t kRestartDeskbar = 'DBar';

const uint32_t kRunAutomounterSettings = 'Tram';

const uint32_t kOpenParentDir = 'Topt';
const uint32_t kOpenDir = 'Topd';
const uint32_t kCleanup = 'Tcln';
const uint32_t kCleanupAll = 'Tcla';

const uint32_t kArrangeBy = 'ARBY';
const uint32_t kArrangeReverseOrder = 'ARRO';

const uint32_t kResizeToFit = 'Trtf';
const uint32_t kSelectMatchingEntries = 'Tsme';
const uint32_t kShowSelectionWindow = 'Tssw';
const uint32_t kShowSettingsWindow = 'Tstw';
const uint32_t kInvertSelection = 'Tisl';

const uint32_t kCancelButton = 'Tcnl';
const uint32_t kDefaultButton = 'Tact';
const uint32_t kPauseButton = 'Tpaw';
const uint32_t kStopButton = 'Tstp';
const uint32_t kCopyAttributes = 'Tcat';
const uint32_t kPasteAttributes = 'Tpat';
const uint32_t kAttributeItem = 'Tatr';
const uint32_t kMIMETypeItem = 'Tmim';
const uint32_t kAddCurrentDir = 'Tadd';
const uint32_t kEditFavorites = 'Tedf';
const uint32_t kSwitchDirectory = 'Tswd';
const uint32_t kQuitTracker = 'Tqit';

const uint32_t kSwitchToHome = 'Tswh';

const uint32_t kTestIconCache = 'TicC';

// Observers and Notifiers:

// Settings-changed messages:
const uint32_t kDisksIconChanged = 'Dicn';
const uint32_t kDesktopIntegrationChanged = 'Dint';
const uint32_t kShowDisksIconChanged = 'Sdic';
const uint32_t kVolumesOnDesktopChanged = 'Codc';
const uint32_t kEjectWhenUnmountingChanged = 'Ewum';

const uint32_t kWindowsShowFullPathChanged = 'Wsfp';
const uint32_t kSingleWindowBrowseChanged = 'Osmw';
const uint32_t kShowNavigatorChanged = 'Snvc';
const uint32_t kShowSelectionWhenInactiveChanged = 'Sswi';
const uint32_t kTransparentSelectionChanged = 'Trse';
const uint32_t kSortFolderNamesFirstChanged = 'Sfnf';
const uint32_t kHideDotFilesChanged = 'Hdfc';
const uint32_t kTypeAheadFilteringChanged = 'Tafc';

const uint32_t kDesktopFilePanelRootChanged = 'Dfpr';
const uint32_t kFavoriteCountChanged = 'Fvct';
const uint32_t kFavoriteCountChangedExternally = 'Fvcx';

const uint32_t kDateFormatChanged = 'Date';

const uint32_t kUpdateVolumeSpaceBar = 'UpSB';
const uint32_t kShowVolumeSpaceBar = 'ShSB';
const uint32_t kSpaceBarColorChanged = 'SBcc';

const uint32_t kDontMoveFilesToTrashChanged = 'STdm';
const uint32_t kAskBeforeDeleteFileChanged = 'STad';

} // namespace BPrivate

using namespace BPrivate;


#endif	// _COMMANDS_H
