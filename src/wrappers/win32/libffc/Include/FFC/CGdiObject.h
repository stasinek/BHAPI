// CGDIObject.h
#ifndef __FFC_CGDIOBJECT_H__
#define __FFC_CGDIOBJECT_H__
///////////////////////////////////////////////////////////////////////////

#include "CObject.h"

///////////////////////////////////////////////////////////////////////////

class CGdiObject : public CObject
{
};

///////////////////////////////////////////////////////////////////////////

class CPen : public CGdiObject
{
};

///////////////////////////////////////////////////////////////////////////

class CBrush : public CGdiObject
{
};

///////////////////////////////////////////////////////////////////////////

class CRgn : public CGdiObject
{
};

///////////////////////////////////////////////////////////////////////////

class CFont : public CGdiObject
{
};

///////////////////////////////////////////////////////////////////////////

class CPalette : public CGdiObject
{
};

///////////////////////////////////////////////////////////////////////////

class CBitmap : public CGdiObject
{
};

///////////////////////////////////////////////////////////////////////////
#endif//__FFC_CGDIOBJECT_H__

