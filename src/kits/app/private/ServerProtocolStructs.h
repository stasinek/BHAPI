/*
 * Copyright 2009, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus, <superstippi@gmx.de>
 */
#ifndef APP_SERVER_PROTOCOL_STRUCTS_H
#define APP_SERVER_PROTOCOL_STRUCTS_H


#include <kits/netAffineTransform.h>
#include <kits/interface/Rect.h>


struct ViewSetStateInfo {
	BPoint						penLocation;
	float						penSize;
	rgb_color					highColor;
	rgb_color					lowColor;
	::pattern					pattern;
	drawing_mode				drawingMode;
	BPoint						origin;
	float						scale;
	BAffineTransform			transform;
	join_mode					lineJoin;
	cap_mode					lineCap;
	float						miterLimit;
	int32_t						fillRule;
	source_alpha				alphaSourceMode;
	alpha_function				alphaFunctionMode;
	bool						fontAntialiasing;
};


struct ViewGetStateInfo {
	int32_t						fontID;
	float						fontSize;
	float						fontShear;
	float						fontRotation;
	float						fontFalseBoldWidth;
	int8						fontSpacing;
	int8						fontEncoding;
	int16						fontFace;
	int32_t						fontFlags;

	ViewSetStateInfo			viewStateInfo;
};


struct ViewDragImageInfo {
	int32_t						bitmapToken;
	int32_t						dragMode;
	BPoint						offset;
	int32_t						bufferSize;
};


struct ViewSetViewCursorInfo {
	int32_t						cursorToken;
	int32_t						viewToken;
	bool						sync;
};


struct ViewBeginRectTrackingInfo {
	BRect						rect;
	uint32_t						style;
};


struct ViewSetLineModeInfo {
	join_mode					lineJoin;
	cap_mode					lineCap;
	float						miterLimit;
};


struct ViewBlendingModeInfo {
	source_alpha				sourceAlpha;
	alpha_function				alphaFunction;
};


struct ViewDrawBitmapInfo {
	int32_t						bitmapToken;
	uint32_t						options;
	BRect						viewRect;
	BRect						bitmapRect;
};


struct ViewDrawStringInfo {
	int32_t						stringLength;
	BPoint						location;
	escapement_delta			delta;
};


struct ViewStrokeLineInfo {
	BPoint						startPoint;
	BPoint						endPoint;
};


struct ViewLineArrayInfo {
	BPoint						startPoint;
	BPoint						endPoint;
	rgb_color					color;
};


#endif	// APP_SERVER_PROTOCOL_STRUCTS_H
