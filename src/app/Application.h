/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * ETK++ library is a freeware; it may be used and distributed according to
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

#ifndef __ETK_APPLICATION_H__
#define __ETK_APPLICATION_H__

#include "./../app/Looper.h"
#include "./../app/MessageRunner.h"
#include "./../app/Cursor.h"

#ifdef __cplusplus /* Just for C++ */


class EClipboard;
class EGraphicsEngine;


class _IMPEXP_ETK EApplication : public ELooper {
public:
	EApplication(const char *signature, bool tryInterface = true);
	virtual ~EApplication();

	// Archiving
	EApplication(const EMessage *from);
	virtual e_status_t Archive(EMessage *into, bool deep = true) const;
	static EArchivable *Instantiate(const EMessage *from);

	const char		*Signature() const;

	virtual void		*Run();
	virtual void		Quit();
	virtual bool		QuitRequested();

	// Empty functions BEGIN --- just for derivative class
	virtual void		ReadyToRun();
	virtual void		Pulse();
	// Empty functions END

	void			SetPulseRate(e_bigtime_t rate);
	e_bigtime_t		PulseRate() const;

	virtual void		MessageReceived(EMessage *msg);
	virtual void		DispatchMessage(EMessage *msg, EHandler *target);

	void			SetCursor(const void *cursor);
	void			SetCursor(const ECursor *cursor, bool sync = true);
	void			HideCursor();
	void			ShowCursor();
	void			ObscureCursor();
	bool			IsCursorHidden() const;

private:
	friend class ELooper;
	friend class EMessageRunner;
	friend class EWindow;
	friend class EView;
	friend class EBitmap;
	friend class EScreen;

	friend _IMPEXP_ETK bool etk_update_font_families(bool);

	bool fQuit;
	char *fSignature;
	e_bigtime_t fPulseRate;
	EMessageRunner *fPulseRunner;

	static EList sRunnerList;
	static e_bigtime_t sRunnerMinimumInterval;
	static void etk_dispatch_message_runners();

	bool etk_quit_all_loopers(bool force);

	EGraphicsEngine *fGraphicsEngine;
	void *fGraphicsEngineAddon;
	void InitGraphicsEngine();

	void Init(const char *signature, bool tryInterface);

	EList fModalWindows;
	bool AddModalWindow(EMessenger &msgr);
	bool RemoveModalWindow(EMessenger &msgr);

	ECursor fCursor;
	bool fCursorHidden;
	bool fCursorObscure;
};


inline void EApplication::SetCursor(const void *cursor)
{
	ECursor theCursor(cursor);
	SetCursor(&theCursor, true);
}


extern _IMPEXP_ETK EApplication *etk_app;
extern _IMPEXP_ETK EMessenger etk_app_messenger;
extern _IMPEXP_ETK EClipboard etk_clipboard;

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

#endif /* __ETK_APPLICATION_H__ */

