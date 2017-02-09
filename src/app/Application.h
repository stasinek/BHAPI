/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
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
 * File: Application.h
 * Description: Application model to support Looper/Message
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_APPLICATION__H
#define BHAPI_APPLICATION__H
#include "./../support/SupportDefs.h"
#include "./../app/Looper.h"
#include "./../app/MessageRunner.h"
#include "./../app/Cursor.h"

#ifdef __cplusplus /* Just for C++ */


class BClipboard;
class BGraphicsEngine;


class IMPEXP_BHAPI BApplication : public BLooper {
public:
	BApplication(const char *signature, bool tryInterface = true);
	virtual ~BApplication();

	// Archiving
	BApplication(const BMessage *from);
	virtual b_status_t Archive(BMessage *into, bool deep = true) const;
	static BArchivable *Instantiate(const BMessage *from);

	const char		*Signature() const;

	virtual void		*Run();
	virtual void		Quit();
	virtual bool		QuitRequested();

	// Empty functions BEGIN --- just for derivative class
	virtual void		ReadyToRun();
	virtual void		Pulse();
	// Empty functions END

    void			SetPulseRate(b_bigtime_t rate);
	b_bigtime_t		PulseRate() const;

	virtual void		MessageReceived(BMessage *msg);
	virtual void		DispatchMessage(BMessage *msg, BHandler *target);

	void			SetCursor(const void *cursor);
    void			SetCursor(const BCursor *cursor, bool sync = true);
	void			HideCursor();
	void			ShowCursor();
	void			ObscureCursor();
	bool			IsCursorHidden() const;

private:
	friend class BLooper;
	friend class BMessageRunner;
    friend class BWindow;
    friend class BView;
    friend class BBitmap;
    friend class BScreen;

    friend IMPEXP_BHAPI bool bhapi_updatb_font_families(bool);

	bool fQuit;
	char *fSignature;
	b_bigtime_t fPulseRate;
	BMessageRunner *fPulseRunner;

	static BList sRunnerList;
	static b_bigtime_t sRunnerMinimumInterval;
	static void bhapi_dispatch_message_runners();

	bool bhapi_quit_all_loopers(bool force);

    BGraphicsEngine *fGraphicsEngine;
	void *fGraphicsEngineAddon;
	void InitGraphicsEngine();

	void Init(const char *signature, bool tryInterface);

	BList fModalWindows;
    bool AddModalWindow(BMessenger &msgr);
    bool RemoveModalWindow(BMessenger &msgr);

    BCursor fCursor;
	bool fCursorHidden;
	bool fCursorObscure;
};


inline void BApplication::SetCursor(const void *cursor)
{
    BCursor theCursor(cursor);
	SetCursor(&theCursor, true);
}


extern IMPEXP_BHAPI BApplication *bhapi_app;
extern IMPEXP_BHAPI BMessenger bhapi_app_messenger;
extern IMPEXP_BHAPI BClipboard bhapi_clipboard;

#endif /* __cplusplus */

/*#ifdef WIN32
#ifdef COPYOF_SendMessage
#define SendMessage COPYOF_SendMessage
#endif
#ifdef COPYOF_DispatchMessage
#define DispatchMessage COPYOF_DispatchMessage
#endif
#endif
*/

#endif /* BHAPI_APPLICATION__H */

