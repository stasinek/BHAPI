/*
 * Copyright 2002-2010 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _SHAPE_H
#define _SHAPE_H


#include <Archivable.h>


class BPoint;
class BRect;
class BShape;

namespace BPrivate {
	class ServerLink;
	class PicturePlayer;
};


class BShapeIterator {
public:
								BShapeIterator();
	virtual						~BShapeIterator();

	virtual	status_t			IterateMoveTo(BPoint* point);
	virtual	status_t			IterateLineTo(int32_t lineCount,
									BPoint* linePoints);
	virtual	status_t			IterateBezierTo(int32_t bezierCount,
									BPoint* bezierPoints);
	virtual	status_t			IterateClose();

	virtual	status_t			IterateArcTo(float& rx, float& ry,
									float& angle, bool largeArc,
									bool counterClockWise, BPoint& point);

			status_t			Iterate(BShape* shape);

private:
	virtual	void				_ReservedShapeIterator2();
	virtual	void				_ReservedShapeIterator3();
	virtual	void				_ReservedShapeIterator4();

			uint32_t				reserved[4];
};


class BShape : public BArchivable {
public:
								BShape();
								BShape(const BShape& other);
								BShape(BMessage* archive);
	virtual						~BShape();

	static	BArchivable*		Instantiate(BMessage* archive);
	virtual	status_t			Archive(BMessage* archive,
									bool deep = true) const;

			BShape&				operator=(const BShape& other);

			bool				operator==(const BShape& other) const;
			bool				operator!=(const BShape& other) const;

			void				Clear();
			BRect				Bounds() const;
			BPoint				CurrentPosition() const;

			status_t			AddShape(const BShape* other);

			status_t			MoveTo(BPoint point);
			status_t			LineTo(BPoint linePoint);
			status_t			BezierTo(BPoint controlPoints[3]);
			status_t			BezierTo(const BPoint& control1,
									const BPoint& control2,
									const BPoint& endPoint);
			status_t			ArcTo(float rx, float ry,
									float angle, bool largeArc,
									bool counterClockWise,
									const BPoint& point);
			status_t			Close();

private:
	// FBC padding
	virtual	status_t			Perform(perform_code code, void* data);

	virtual	void				_ReservedShape1();
	virtual	void				_ReservedShape2();
	virtual	void				_ReservedShape3();
	virtual	void				_ReservedShape4();

private:
	friend class BShapeIterator;
	friend class BView;
	friend class BFont;
	friend class BPrivate::PicturePlayer;
	friend class BPrivate::ServerLink;

			void				GetData(int32_t* opCount, int32_t* ptCount,
									uint32_t** opList, BPoint** ptList);
			void				SetData(int32_t opCount, int32_t ptCount,
									const uint32_t* opList,
									const BPoint* ptList);
			void				InitData();
			bool				AllocatePts(int32_t count);
			bool				AllocateOps(int32_t count);

private:
			uint32_t				fState;
			uint32_t				fBuildingOp;
			void*				fPrivateData;

			uint32_t				reserved[4];
};


#endif // _SHAPE_H
