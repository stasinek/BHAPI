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
 * File: Screen.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_SCREEN_H
#define BHAPI_SCREEN_H

#ifdef __cplusplus /* Just for C++ */
#ifndef BWINDOW_DEF
#define BWINDOW_DEF
class BWindow;
#endif
#include "../interface/Rect.h"
#include "../interface/GraphicsDefs.h"
#include <kernel/OS.h>
namespace BPrivate {
    class BPrivateScreen;
}
class BHAPI_IMPEXP BScreen {
public:
            BScreen(uint32 id);
            BScreen(bhapi::screen_id id = B_MAIN_SCREEN_ID);
            BScreen(BWindow* window);
           ~BScreen();

    bool	IsValid();
    status_t			SetToNext();

    bhapi::color_space			ColorSpace();
            BRect				Frame();
            bhapi::screen_id			ID();

            status_t			WaitForRetrace();
            status_t			WaitForRetrace(bigtime_t timeout);

            uint8				IndexForColor(bhapi::rgb_color color);
            uint8				IndexForColor(uint8 red, uint8 green,
                                    uint8 blue, uint8 alpha = 255);
            bhapi::rgb_color			ColorForIndex(uint8 index);
            uint8				InvertIndex(uint8 index);

            const bhapi::color_map*	ColorMap();

            status_t			GetBitmap(BBitmap** _bitmap,
                                    bool drawCursor = true,
                                    BRect* frame = NULL);
            status_t			ReadBitmap(BBitmap* bitmap,
                                    bool drawCursor = true,
                                    BRect* frame = NULL);

            bhapi::rgb_color			DesktopColor();
            bhapi::rgb_color			DesktopColor(uint32 workspace);
            void				SetDesktopColor(bhapi::rgb_color color,
                                    bool stick = true);
            void				SetDesktopColor(bhapi::rgb_color color,
                                    uint32 workspace, bool stick = true);

            status_t			ProposeMode(bhapi::display_mode* target,
                                    const bhapi::display_mode* low,
                                    const bhapi::display_mode* high);
            status_t			GetModeList(bhapi::display_mode** _modeList,
                                    uint32* _count);
            status_t			GetMode(bhapi::display_mode* mode);
            status_t			GetMode(uint32 workspace,
                                    bhapi::display_mode* mode);
            status_t			SetMode(bhapi::display_mode* mode,
                                    bool makeDefault = false);
            status_t			SetMode(uint32 workspace,
                                    bhapi::display_mode* mode,
                                    bool makeDefault = false);
            status_t			GetDeviceInfo(accelerant_device_info* info);
            status_t			GetMonitorInfo(bhapi::monitor_info* info);
            status_t			GetPixelClockLimits(bhapi::display_mode* mode,
                                    uint32* _low, uint32* _high);
            status_t			GetTimingConstraints(
                                    display_timing_constraints*
                                        timingConstraints);
            status_t			SetDPMS(uint32 state);
            uint32				DPMSState();
            uint32				DPMSCapabilites();

private:
    // Forbidden and deprecated methods
                                BScreen(const BScreen& other);
            BScreen&			operator=(const BScreen& other);

            BPrivate::BPrivateScreen* private_screen();
            status_t			ProposeDisplayMode(display_mode* target,
                                    const bhapi::display_mode* low,
                                    const bhapi::display_mode* high);
            void*				BaseAddress();
            uint32				BytesPerRow();

private:
            BPrivate::BPrivateScreen* fScreen;
            uint32 fID;
};

inline uint8 BScreen::IndexForColor(bhapi::rgb_color color)
{
    return IndexForColor(color.red, color.green, color.blue, color.alpha);
}
#define BWINDOW_DEF
#endif /* __cplusplus */
#endif /* BHAPI_SCREEN_H */

