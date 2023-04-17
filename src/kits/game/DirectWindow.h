/*
 * Copyright 2001-2008 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stefano Ceccherini <stefano.ceccherini@gmail.com>
 */
#ifndef	BHAPI_DIRECT_WINDOW_H
#define	BHAPI_DIRECT_WINDOW_H

#include <Region.h>
#include <Window.h>

enum direct_buffer_state {
    B_DIRECT_MODE_MASK	= 15,

    B_DIRECT_START		= 0,
    B_DIRECT_STOP		= 1,
    B_DIRECT_MODIFY		= 2,

    B_CLIPPING_MODIFIED = 16,
    B_BUFFER_RESIZED 	= 32,
    B_BUFFER_MOVED 		= 64,
    B_BUFFER_RESET	 	= 128
};

enum direct_driver_state {
    B_DRIVER_CHANGED	= 0x0001,
    B_MODE_CHANGED		= 0x0002
};

typedef struct {
    direct_buffer_state	buffer_state;
    direct_driver_state	driver_state;
    void				*bits;
    void				*pci_bits;
    int32_t				bytes_per_row;
    uint32_t				bits_per_pixel;
    color_space			pixel_format;
    buffer_layout		layout;
    buffer_orientation	orientation;
    uint32_t				_reserved[9];
    uint32_t				_dd_type_;
    uint32_t				_dd_token_;
    uint32_t				clip_list_count;
    clipping_rect		window_bounds;
    clipping_rect		clip_bounds;
    clipping_rect		clip_list[1];
} direct_buffer_info;

class BDirectWindow : public BWindow {
public:
                                BDirectWindow(BRect frame, const char* title,
                                    window_type type, uint32_t flags,
                                    uint32_t workspace = B_CURRENT_WORKSPACE);
                                BDirectWindow(BRect frame, const char* title,
                                    window_look look, window_feel feel,
                                    uint32_t flags,
                                    uint32_t workspace = B_CURRENT_WORKSPACE);
    virtual						~BDirectWindow();

    static	BArchivable*		Instantiate(BMessage* data);
    virtual	status_t			Archive(BMessage* data,
                                    bool deep = true) const;

    virtual	void				Quit();
    virtual	void				DispatchMessage(BMessage* message,
                                    BHandler* handler);
    virtual	void				MessageReceived(BMessage* message);
    virtual	void				FrameMoved(BPoint newPosition);
    virtual	void				WorkspacesChanged(uint32_t oldWorkspaces,
                                    uint32_t newWorkspaces);
    virtual	void				WorkspaceActivated(int32_t workspaceIndex,
                                    bool state);
    virtual	void				FrameResized(float newWidth, float newHeight);
    virtual	void				Minimize(bool minimize);
    virtual	void				Zoom(BPoint recPosition, float recWidth,
                                    float recHeight);
    virtual	void				ScreenChanged(BRect screenFrame,
                                    color_space depth);
    virtual	void				MenusBeginning();
    virtual	void				MenusEnded();
    virtual	void				WindowActivated(bool state);
    virtual	void				Show();
    virtual	void				Hide();
    virtual	BHandler*			ResolveSpecifier(BMessage* message,
                                    int32_t index, BMessage* specifier,
                                    int32_t what, const char* property);
    virtual	status_t			GetSupportedSuites(BMessage* data);
    virtual	status_t			Perform(perform_code code, void* arg);

private:
    virtual	void				task_looper();
    virtual	BMessage*			ConvertToMessage(void* raw, int32_t code);

    public:
    virtual	void				DirectConnected(direct_buffer_info* info);
            status_t			GetClippingRegion(BRegion* region,
                                    BPoint* origin = NULL) const;
            status_t			SetFullScreen(bool enable);
            bool				IsFullScreen() const;

    static	bool				SupportsWindowMode(
                                    screen_id id = B_MAIN_SCREEN_ID);

private:
    typedef	BWindow				inherited;

    virtual	void				_ReservedDirectWindow1();
    virtual	void				_ReservedDirectWindow2();
    virtual	void				_ReservedDirectWindow3();
    virtual	void				_ReservedDirectWindow4();

                                BDirectWindow();
                                BDirectWindow(BDirectWindow& other);
            BDirectWindow&		operator=(BDirectWindow& other);

    static	int32_t				_daemon_thread(void* arg);
            int32_t				_DirectDaemon();
            bool				_LockDirect() const;
            void				_UnlockDirect() const;

            void				_InitData();
            void				_DisposeData();

            bool				fDaemonKiller;
            bool				fConnectionEnable;
            bool				fIsFullScreen;
            bool				_unused;
            bool				fInDirectConnect;

            int32_t				fDirectLock;
            sem_id				fDirectSem;
            uint32_t				fDirectLockCount;
            thread_id			fDirectLockOwner;
            char*				fDirectLockStack;

            sem_id				fDisableSem;
            sem_id				fDisableSemAck;

            uint32_t				fInitStatus;
            uint32_t				fInfoAreaSize;

            uint32_t				_reserved[2];

            area_id				fClonedClippingArea;
            area_id				fSourceClippingArea;
            thread_id			fDirectDaemonId;
            direct_buffer_info*	fBufferDesc;

            uint32_t				_more_reserved_[17];
};


#endif	// _DIRECT_WINDOW_H
