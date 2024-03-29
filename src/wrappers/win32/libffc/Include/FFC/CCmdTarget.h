// CCmdTarget.h
#ifndef __FFC_CCMDTARGET_H__
#define __FFC_CCMDTARGET_H__
///////////////////////////////////////////////////////////////////////////

#include "CObject.h"
#include "MessageMap.h"

///////////////////////////////////////////////////////////////////////////

struct AFX_CMDHANDLERINFO
{
};

///////////////////////////////////////////////////////////////////////////

class CCmdTarget : public CObject
{
public:
	enum _ProcessType { _Check, _Message, _Command };

	DECLARE_MESSAGE_MAP()

	DECLARE_DYNCREATE(CCmdTarget)

public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
};

///////////////////////////////////////////////////////////////////////////
#endif//__FFC_CCMDTARGET_H__

