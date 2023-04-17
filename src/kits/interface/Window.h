/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * BHAPI++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: Window.h
 *
 * --------------------------------------------------------------------------*/

/*
 * Copyright 2001-2015, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */


#ifndef BHAPI_WINDOW_H
#define BHAPI_WINDOW_H

#ifdef __cplusplus /* Just for C++ */
namespace bhapi {
#endif

enum window_type {
    B_UNTYPED_WINDOW					= 0,
    B_TITLED_WINDOW						= 1,
    B_MODAL_WINDOW						= 3,
    B_DOCUMENT_WINDOW					= 11,
    B_BORDERED_WINDOW					= 20,
    B_FLOATING_WINDOW					= 21
};

enum window_look {
    B_BORDERED_WINDOW_LOOK				= 20,
    B_NO_BORDER_WINDOW_LOOK				= 19,
    B_TITLED_WINDOW_LOOK				= 1,
    B_DOCUMENT_WINDOW_LOOK				= 11,
    B_MODAL_WINDOW_LOOK					= 3,
    B_FLOATING_WINDOW_LOOK				= 7
};

enum window_feel {
    B_NORMAL_WINDOW_FEEL				= 0,
    B_MODAL_SUBSET_WINDOW_FEEL			= 2,
    B_MODAL_APP_WINDOW_FEEL				= 1,
    B_MODAL_ALL_WINDOW_FEEL				= 3,
    B_FLOATING_SUBSET_WINDOW_FEEL		= 5,
    B_FLOATING_APP_WINDOW_FEEL			= 4,
    B_FLOATING_ALL_WINDOW_FEEL			= 6
};

enum window_alignment {
    B_BYTE_ALIGNMENT	= 0,
    B_PIXEL_ALIGNMENT	= 1
};

// window flags
enum {
    B_NOT_MOVABLE						= 0x00000001,
    B_NOT_CLOSABLE						= 0x00000020,
    B_NOT_ZOOMABLE						= 0x00000040,
    B_NOT_MINIMIZABLE					= 0x00004000,
    B_NOT_RESIZABLE						= 0x00000002,
    B_NOT_H_RESIZABLE					= 0x00000004,
    B_NOT_V_RESIZABLE					= 0x00000008,
    B_AVOID_FRONT						= 0x00000080,
    B_AVOID_FOCUS						= 0x00002000,
    B_WILL_ACCEPT_FIRST_CLICK			= 0x00000010,
    B_OUTLINE_RESIZE					= 0x00001000,
    B_NO_WORKSPACE_ACTIVATION			= 0x00000100,
    B_NOT_ANCHORED_ON_ACTIVATE			= 0x00020000,
    B_ASYNCHRONOUS_CONTROLS				= 0x00080000,
    B_QUIT_ON_WINDOW_CLOSE				= 0x00100000,
    B_SAME_POSITION_IN_ALL_WORKSPACES	= 0x00200000,
    B_AUTO_UPDATE_SIZE_LIMITS			= 0x00400000,
    B_CLOSE_ON_ESCAPE					= 0x00800000,
    B_NO_SERVER_SIDE_WINDOW_MODIFIERS	= 0x00000200
};

#define B_CURRENT_WORKSPACE				0
#define B_ALL_WORKSPACES				0xffffffff

// MoveOnScreen() flags
enum {
    B_DO_NOT_RESIZE_TO_FIT				= 0x0001,
    B_MOVE_IF_PARTIALLY_OFFSCREEN		= 0x0002
};

#ifdef __cplusplus /* Just for C++ */
} // namespace
#endif

#define B_CURRENT_WORKSPACE	0
#define B_ALL_WORKSPACES	0xffffffff

#ifdef __cplusplus /* Just for C++ */
class BButton;
class BMenuBar;
class BMenuItem;
class BApplication;
class BView;
class BGraphicsContext;
class BGraphicsDrawable;
class BGraphicsWindow;
class BLayoutContainer;
class BLayoutItem;
class BLayout;
class BMessage;
class BMessageRunner;
class BMessenger;
class BRegion;
class BPoint;
namespace BPrivate {
    class PortLink;
};
#include <View.h>
#include <kits/app/Looper.h>
#include <kits/interface/Rect.h>
#include <kits/support/List.h>
#include <Size.h>
#include <GraphicsDefs.h>
class BHAPI_IMPEXP BWindow : public BLooper {
public:
                                BWindow(BRect frame, const char* title,
                                    window_type type, uint32_t flags,
                                    uint32_t workspace = B_CURRENT_WORKSPACE);
                                BWindow(BRect frame, const char* title,
                                    window_look look, window_feel feel,
                                    uint32_t flags, uint32_t workspace
                                        = B_CURRENT_WORKSPACE);
    virtual						~BWindow();

                                BWindow(BMessage* archive);
    static	BArchivable*		Instantiate(BMessage* archive);
    virtual	status_t			Archive(BMessage* archive,
                                    bool deep = true) const;

    virtual	void				Quit();
            void				Close() { Quit(); }

            void				AddChild(BView* child, BView* before = NULL);
            void				AddChild(BLayoutItem* child);
            bool				RemoveChild(BView* child);
            int32_t				CountChildren() const;
            BView*				ChildAt(int32_t index) const;

    virtual	void				DispatchMessage(BMessage* message,
                                    BHandler* handler);
    virtual	void				MessageReceived(BMessage* message);
    virtual	void				FrameMoved(BPoint newPosition);
    virtual void				WorkspacesChanged(uint32_t oldWorkspaces,
                                    uint32_t newWorkspaces);
    virtual void				WorkspaceActivated(int32_t workspace,
                                    bool state);
    virtual	void				FrameResized(float newWidth, float newHeight);
    virtual void				Minimize(bool minimize);
    virtual	void				Zoom(BPoint origin, float width, float height);
            void				Zoom();
            void				SetZoomLimits(float maxWidth, float maxHeight);
    virtual void				ScreenChanged(BRect screenSize,
                                    color_space depth);

            void				SetPulseRate(bigtime_t rate);
            bigtime_t			PulseRate() const;

            void				AddShortcut(uint32_t key, uint32_t modifiers,
                                    BMessage* message);
            void				AddShortcut(uint32_t key, uint32_t modifiers,
                                    BMessage* message, BHandler* target);
            bool				HasShortcut(uint32_t key, uint32_t modifiers);
            void				RemoveShortcut(uint32_t key, uint32_t modifiers);

            void				SetDefaultButton(BButton* button);
            BButton*			DefaultButton() const;

    virtual	void				MenusBeginning();
    virtual	void				MenusEnded();

            bool				NeedsUpdate() const;
            void				UpdateIfNeeded();

            BView*				FindView(const char* viewName) const;
            BView*				FindView(BPoint) const;
            BView*				CurrentFocus() const;

            void				Activate(bool = true);
    virtual	void				WindowActivated(bool focus);

            void				ConvertToScreen(BPoint* point) const;
            BPoint				ConvertToScreen(BPoint point) const;
            void				ConvertFromScreen(BPoint* point) const;
            BPoint				ConvertFromScreen(BPoint point) const;
            void				ConvertToScreen(BRect* rect) const;
            BRect				ConvertToScreen(BRect rect) const;
            void				ConvertFromScreen(BRect* rect) const;
            BRect				ConvertFromScreen(BRect rect) const;

            void				MoveBy(float dx, float dy);
            void				MoveTo(BPoint);
            void				MoveTo(float x, float y);
            void				ResizeBy(float dx, float dy);
            void				ResizeTo(float width, float height);
            void				ResizeToPreferred();

            void				CenterIn(const BRect& rect);
            void				CenterOnScreen();
            void				CenterOnScreen(screen_id id);
            void				MoveOnScreen(uint32_t flags = 0);

    virtual	void				Show();
    virtual	void				Hide();
            bool				IsHidden() const;
            bool				IsMinimized() const;

            void				Flush() const;
            void				Sync() const;

            status_t			SendBehind(const BWindow* window);

            void				DisableUpdates();
            void				EnableUpdates();

            void				BeginViewTransaction();
                                    // referred as OpenViewTransaction()
                                    // in BeBook
            void				EndViewTransaction();
                                    // referred as CommitViewTransaction()
                                    // in BeBook
            bool				InViewTransaction() const;

            BRect				Bounds() const;
            BRect				Frame() const;
            BRect				DecoratorFrame() const;
            BSize				Size() const;
            const char*			Title() const;
            void				SetTitle(const char* title);
            bool				IsFront() const;
            bool				IsActive() const;

            void				SetKeyMenuBar(BMenuBar* bar);
            BMenuBar*			KeyMenuBar() const;

            void				SetSizeLimits(float minWidth, float maxWidth,
                                    float minHeight, float maxHeight);
            void				GetSizeLimits(float* minWidth, float* maxWidth,
                                    float* minHeight, float* maxHeight);
            void				UpdateSizeLimits();

            status_t			SetDecoratorSettings(const BMessage& settings);
            status_t			GetDecoratorSettings(BMessage* settings) const;

            uint32_t				Workspaces() const;
            void				SetWorkspaces(uint32_t);

            BView*				LastMouseMovedView() const;

    virtual BHandler*			ResolveSpecifier(BMessage* message,
                                    int32_t index, BMessage* specifier,
                                    int32_t what, const char* property);
    virtual status_t			GetSupportedSuites(BMessage* data);

            status_t			AddToSubset(BWindow* window);
            status_t			RemoveFromSubset(BWindow* window);

    virtual status_t			Perform(perform_code code, void* data);

            status_t			SetType(window_type type);
            window_type			Type() const;

            status_t			SetLook(window_look look);
            window_look			Look() const;

            status_t			SetFeel(window_feel feel);
            window_feel			Feel() const;

            status_t			SetFlags(uint32_t);
            uint32_t				Flags() const;

            bool				IsModal() const;
            bool				IsFloating() const;

            status_t			SetWindowAlignment(window_alignment mode,
                                    int32_t h, int32_t hOffset = 0,
                                    int32_t width = 0, int32_t widthOffset = 0,
                                    int32_t v = 0, int32_t vOffset = 0,
                                    int32_t height = 0, int32_t heightOffset = 0);
            status_t			GetWindowAlignment(
                                    window_alignment* mode = NULL,
                                    int32_t* h = NULL, int32_t* hOffset = NULL,
                                    int32_t* width = NULL,
                                    int32_t* widthOffset = NULL,
                                    int32_t* v = NULL, int32_t* vOffset = NULL,
                                    int32_t* height = NULL,
                                    int32_t* heightOffset = NULL) const;

    virtual	bool				QuitRequested();
    virtual	void				SetLayout(BLayout* layout);
            BLayout*			GetLayout() const;

            void				InvalidateLayout(bool descendants = false);
            void				Layout(bool force);

private:
    // FBC padding and forbidden methods
    virtual	void				_ReservedWindow2();
    virtual	void				_ReservedWindow3();
    virtual	void				_ReservedWindow4();
    virtual	void				_ReservedWindow5();
    virtual	void				_ReservedWindow6();
    virtual	void				_ReservedWindow7();
    virtual	void				_ReservedWindow8();

                                BWindow();
                                BWindow(BWindow&);
            BWindow&			operator=(BWindow&);

private:
    typedef BLooper inherited;
    struct unpack_cookie;
    class Shortcut;

    friend class BAlert;
    friend class BApplication;
    friend class BBitmap;
    friend class BView;
    friend class BMenuItem;
    friend class BWindowScreen;
    friend class BDirectWindow;
    friend class BFilePanel;
    friend class BWindowStack;

    friend void _set_menu_sem_(BWindow* w, sem_id sem);
    friend status_t _safe_get_server_token_(const BLooper*, int32_t*);

                                BWindow(BRect frame, int32_t bitmapToken);
            void				_InitData(BRect frame, const char* title,
                                    window_look look, window_feel feel,
                                    uint32_t flags, uint32_t workspace,
                                    int32_t bitmapToken = -1);

    virtual	void				task_looper();

            BPoint				AlertPosition(const BRect& frame);
    virtual BMessage*			ConvertToMessage(void* raw, int32_t code);

            void				AddShortcut(uint32_t key, uint32_t modifiers,
                                    BMenuItem* item);
            BHandler*			_DetermineTarget(BMessage* message,
                                    BHandler* target);
            bool				_IsFocusMessage(BMessage* message);
            bool				_UnpackMessage(unpack_cookie& state,
                                    BMessage** _message, BHandler** _target,
                                    bool* _usePreferred);
            void				_SanitizeMessage(BMessage* message,
                                    BHandler* target, bool usePreferred);
            bool				_StealMouseMessage(BMessage* message,
                                    bool& deleteMessage);
            uint32_t				_TransitForMouseMoved(BView* view,
                                    BView* viewUnderMouse) const;

            bool				InUpdate();
            void				_DequeueAll();
            window_type			_ComposeType(window_look look,
                                    window_feel feel) const;
            void				_DecomposeType(window_type type,
                                    window_look* look,
                                    window_feel* feel) const;

            void				SetIsFilePanel(bool yes);
            bool				IsFilePanel() const;

            void				_CreateTopView();
            void				_AdoptResize();
            void				_SetFocus(BView* focusView,
                                    bool notifyIputServer = false);
            void				_SetName(const char* title);

            Shortcut*			_FindShortcut(uint32_t key, uint32_t modifiers);
            BView*				_FindView(BView* view, BPoint point) const;
            BView*				_FindView(int32_t token);
            BView*				_LastViewChild(BView* parent);

            BView*				_FindNextNavigable(BView* focus, uint32_t flags);
            BView*				_FindPreviousNavigable(BView* focus,
                                    uint32_t flags);
            void				_Switcher(int32_t rawKey, uint32_t modifiers,
                                    bool repeat);
            bool				_HandleKeyDown(BMessage* event);
            bool				_HandleUnmappedKeyDown(BMessage* event);
            void				_KeyboardNavigation();

            void				_GetDecoratorSize(float* _borderWidth,
                                    float* _tabHeight) const;
            void				_SendShowOrHideMessage();

private:
            char*				fTitle;
            int32_t				_unused0;
            bool				fInTransaction;
            bool				fActive;
            short				fShowLevel;
            uint32_t				fFlags;

            BView*				fTopView;
            BView*				fFocus;
            BView*				fLastMouseMovedView;
            uint32_t				_unused1;
            BMenuBar*			fKeyMenuBar;
            BButton*			fDefaultButton;
            BList				fShortcuts;
            int32_t				fTopViewToken;
            bool				fUpdateRequested;
            bool				fOffscreen;
            bool				fIsFilePanel;
            bool				_unused4;
            bigtime_t			fPulseRate;
            bool				_unused5;
            bool				fMinimized;
            bool				fNoQuitShortcut;
            bool				_unused6;
            sem_id				fMenuSem;
            float				fMaxZoomHeight;
            float				fMaxZoomWidth;
            float				fMinHeight;
            float				fMinWidth;
            float				fMaxHeight;
            float				fMaxWidth;
            BRect				fFrame;
            window_look			fLook;
            window_feel			fFeel;
            int32_t				fLastViewToken;
            ::BPrivate::PortLink* fLink;
            BMessageRunner*		fPulseRunner;
            BRect				fPreviousFrame;

            uint32_t				_reserved[9];
public:
    bool		IsActivate() const;


    void		ConvertToScreen(BRegion *region) const;
    BRegion		ConvertToScreen(const BRegion &region) const;
    void		ConvertFromScreen(BRegion *region) const;
    BRegion		ConvertFromScreen(const BRegion &region) const;

    void		MoveToCenter();

    void		Invalidate(BRect invalRect, bool redraw = true);

    virtual void	SetBackgroundColor(bhapi::rgb_color c);
    void		SetBackgroundColor(uint8 r,  uint8 g,  uint8 b,  uint8 a = 255);
    bhapi::rgb_color	BackgroundColor() const;

    void		GetSizeLimits(float *min_h, float *max_h, float *min_v, float *max_v) const;

protected:
    bool GrabMouse();
    bool IsMouseGrabbed() const;
    void UngrabMouse();
    bool GrabKeyboard();
    bool IsKeyboardGrabbed() const;
    void UngrabKeyboard();

private:
    friend class BGraphicsEngine;

    BGraphicsWindow *fWindow;
    BGraphicsDrawable *fPixmap;
    BGraphicsContext *fDC;
    BLayoutContainer *fLayout;

    char *fWindowTitle;
    bhapi::window_look fWindowLook;
    bhapi::window_feel fWindowFeel;
     uint32_t fWindowFlags;
     uint32_t fWindowWorkspaces;

    BList fMouseInterestedViews;
    BList fKeyboardInterestedViews;
    BList fMouseInsideViews;

    static void AddViewChildrenToHandlersList(BWindow *win, BView *child);
    static void RemoveViewChildrenFromHandlersList(BWindow *win, BView *child);

     int64 fUpdateHolderThreadId;
     int64 fUpdateHolderCount;
    BRect fUpdateRect;
    BRect fExposeRect;
    bool fInUpdate;

    void _Update(BRect rect, bool force_update);
    void _Expose(BRect rect, bigtime_t when);
    void _UpdateIfNeeded(bigtime_t when);
    bool InUpdate() const;

    bool _HasResizeMessage(bool setBrokeOnExpose);

    bool fActivated;

    bigtime_t fActivatedTimeStamp;
    bigtime_t fPositionChangedTimeStamp;
    bigtime_t fSizeChangedTimeStamp;

     uint32_t fMouseGrabCount;
     uint32_t fKeyboardGrabCount;
    bool _GrabMouse();
    bool _GrabKeyboard();
    void _UngrabMouse();
    void _UngrabKeyboard();

    bool fBrokeOnExpose;
    BList fNeededToPulseViews;

    void InitSelf(BRect, const char*, bhapi::window_look, bhapi::window_feel,  uint32_t,  uint32_t);
};

#endif /* __cplusplus */

#endif /* BHAPI_WINDOW_H */

