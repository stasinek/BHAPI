/*
 * Copyright 2006-2015 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stefano Ceccherini, burton666@libero.it
 *		Julian Harnath, <julian.harnath@rwth-achen.de>
 */
#ifndef _PICTURE_DATA_WRITER_H
#define _PICTURE_DATA_WRITER_H


#include <AffineTransform.h>
#include <InterfaceDefs.h>
#include <Font.h>
#include <kits/kernel/OS.h>

#include <stack>


class Layer;
class BPositionIO;
class BRegion;

class PictureDataWriter {
public:
								PictureDataWriter();
								PictureDataWriter(BPositionIO* data);
	virtual						~PictureDataWriter();

			status_t			SetTo(BPositionIO* data);

			status_t			WriteSetHighColor(const rgb_color& color);
			status_t			WriteSetLowColor(const rgb_color& color);
			status_t			WriteSetOrigin(const BPoint& point);
			status_t			WriteSetDrawingMode(const drawing_mode& mode);
			status_t			WriteSetPenLocation(const BPoint& point);
			status_t			WriteSetPenSize(const float& penSize);
			status_t			WriteSetLineMode(const cap_mode& cap,
									const join_mode& join,
									const float& miterLimit);
			status_t			WriteSetScale(const float& scale);
			status_t			WriteSetTransform(BAffineTransform transform);
			status_t			WriteTranslateBy(double x, double y);
			status_t			WriteScaleBy(double x, double y);
			status_t			WriteRotateBy(double angleRadians);
			status_t			WriteSetPattern(const ::pattern& pattern);
			status_t			WriteClipToPicture(int32_t pictureToken,
									const BPoint& origin, bool inverse);
			status_t			WriteSetClipping(const BRegion& region);
			status_t			WriteClearClipping();

			status_t			WritePushState();
			status_t			WritePopState();

			status_t			WriteSetFontFamily(const font_family family);
			status_t			WriteSetFontStyle(const font_style style);
			status_t			WriteSetFontSpacing(const int32_t& spacing);
			status_t			WriteSetFontSize(const float& size);
			status_t			WriteSetFontRotation(const float& rotation);
			status_t			WriteSetFontEncoding(const int32_t& encoding);
			status_t			WriteSetFontFlags(const int32_t& flags);
			status_t			WriteSetFontShear(const float& shear);
			status_t			WriteSetFontFace(const int32_t& face);

			status_t			WriteStrokeLine(const BPoint& start,
									const BPoint& end);
			status_t			WriteInvertRect(const BRect& rect);
			status_t			WriteDrawRect(const BRect& rect,
									const bool& fill);
			status_t			WriteDrawRoundRect(const BRect& rect,
									const BPoint& radius, const bool& fill);
			status_t			WriteDrawEllipse(const BRect& rect,
									const bool& fill);
			status_t			WriteDrawArc(const BPoint& center,
									const BPoint& radius,
									const float& startTheta,
									const float& arcTheta, const bool& fill);
			status_t			WriteDrawPolygon(const int32_t& numPoints,
									BPoint* points, const bool& isClosed,
									const bool& fill);
			status_t			WriteDrawBezier(const BPoint points[4],
									const bool& fill);
			status_t			WriteDrawString(const BPoint& where,
									const char* string, const int32_t& length,
									const escapement_delta& delta);
			status_t			WriteDrawShape(const int32_t& opCount,
									const void* opList, const int32_t& ptCount,
									const void* ptList, const bool& fill);
			status_t			WriteDrawBitmap(const BRect& srcRect,
									const BRect& dstRect, const int32_t& width,
									const int32_t& height,
									const int32_t& bytesPerRow,
									const int32_t& colorSpace,
									const int32_t& flags,
									const void* data, const int32_t& length);

			status_t			WriteDrawPicture(const BPoint& where,
									const int32_t& token);

			status_t			WriteBlendLayer(Layer* layer);
			status_t			WriteClipToRect(const BRect& rect,
									bool inverse);
			status_t			WriteClipToShape(int32_t opCount,
									const void* opList, int32_t ptCount,
									const void* ptList, bool inverse);

protected:
	// throw a status_t on error
			void				BeginOp(const int16& op);
			void				EndOp();
			void				WriteData(const void* data, size_t size);
	template <typename T> void	Write(const T& data)
									{ WriteData(&data, sizeof(data)); }

private:
			BPositionIO*		fData;
			std::stack<off_t>	fStack;
};


#endif // _PICTURE_DATA_WRITER_H
