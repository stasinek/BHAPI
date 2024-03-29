﻿/*
 * Copyright 2006-2007, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef	BHAPI_WINDOW_SCREEN_H
#define	BHAPI_WINDOW_SCREEN_H

#include <Accelerant.h>
#include <GraphicsCard.h>
#include <kits/kernel/OS.h>
#include <Window.h>
#include <kits/kernel/Image.h>
#include <Haiku.h>

void set_mouse_position(int32_t x, int32_t y);
    // Controls the mouse cursor position on screen

enum {
    B_ENABLE_VIEW_DRAWING	= 0x0001,
    B_ENABLE_DEBUGGER		= 0x0002
};

class BHAPI_IMPEXP BWindowScreen : public BWindow {
    public:
                            BWindowScreen(const char* title, uint32_t space,
                                status_t* _error, bool debugMode = false);
                            BWindowScreen(const char* title, uint32_t space,
                                uint32_t attributes, status_t* _error);
        virtual				~BWindowScreen();

        virtual void		Quit();
        virtual void		ScreenConnected(bool active);
        void				Disconnect();

        virtual	void		WindowActivated(bool active);
        virtual void		WorkspaceActivated(int32_t workspace, bool active);
        virtual void		ScreenChanged(BRect screenSize, color_space depth);

        virtual void		Hide();
        virtual void		Show();

        void				SetColorList(rgb_color* list, int32_t firstIndex = 0,
                                int32_t lastIndex = 255);
        status_t			SetSpace(uint32_t space);

        bool				CanControlFrameBuffer();
        status_t			SetFrameBuffer(int32_t width, int32_t height);
        status_t			MoveDisplayArea(int32_t x, int32_t y);

        rgb_color*			ColorList();
        frame_buffer_info*	FrameBufferInfo();
        graphics_card_hook	CardHookAt(int32_t index);
        graphics_card_info*	CardInfo();

        void				RegisterThread(thread_id thread);
        virtual	void		SuspensionHook(bool active);
        void				Suspend(char* label);

    private:
        virtual status_t	Perform(perform_code d, void* arg);

#if 0
        void*				IOBase();
            // deprecated API, always returns NULL
#endif

        virtual void		_ReservedWindowScreen1();
        virtual void		_ReservedWindowScreen2();
        virtual void		_ReservedWindowScreen3();
        virtual void		_ReservedWindowScreen4();

        // not implemented:
                            BWindowScreen();
                            BWindowScreen(BWindowScreen& other);
                            BWindowScreen &operator=(BWindowScreen& other);

        status_t			_InitData(uint32_t space, uint32_t attributes);
        void				_DisposeData();

        status_t			_LockScreen(bool lock);
        status_t			_Activate();
        status_t			_Deactivate();

        status_t			_SetupAccelerantHooks();
        void				_ResetAccelerantHooks();

        status_t			_GetCardInfo();
        void				_Suspend();
        void				_Resume();
        status_t			_GetModeFromSpace(uint32_t space, display_mode* mode);
        status_t			_InitClone();
        status_t			_AssertDisplayMode(display_mode* mode);

        uint16				_reserved0;
        bool				_reserved1;
        bool				fWorkState;
        bool				fWindowState;
        bool				fActivateState;
        int32_t				fLockState;
        int32_t				fWorkspaceIndex;

        display_mode*		fOriginalDisplayMode;
        display_mode*		fDisplayMode;
        sem_id				fDebugSem;
        image_id			fAddonImage;
        uint32_t				fAttributes;

        rgb_color			fPalette[256];

        graphics_card_info	fCardInfo;
        frame_buffer_info	fFrameBufferInfo;

        char*				fDebugFrameBuffer;
        bool				fDebugState;
        bool				fDebugFirst;
        int32_t				fDebugWorkspace;
        int32_t				fDebugThreadCount;
        thread_id*			fDebugThreads;

        uint32_t				fModeCount;
        display_mode*		fModeList;

        GetAccelerantHook	fGetAccelerantHook;
        wait_engine_idle	fWaitEngineIdle;

        uint32_t				_reserved[163];
};

#define BWINDOWSCREEN_I
#endif	// BHAPI_WINDOW_SCREEN_H
