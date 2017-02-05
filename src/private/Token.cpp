/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: Token.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../kernel/Kernel.h"
#include "./../support/Locker.h"
#include "./../support/List.h"

#include "Token.h"


struct _LOCAL _etk_token_t {
	euint64 vitalities;
	e_bigtime_t time_stamp;
	void *data;
};


class _LOCAL ETokensDepotPrivateData : public EList {
public:
	ETokensDepotPrivateData();
	virtual ~ETokensDepotPrivateData();

	euint64		AddToken(void *data);
	void		RemoveToken(euint64 token);
	_etk_token_t	*TokenAt(euint64 token) const;
};


ETokensDepotPrivateData::ETokensDepotPrivateData()
	: EList()
{
}


ETokensDepotPrivateData::~ETokensDepotPrivateData()
{
	EList *list;

	while((list = (EList*)RemoveItem(0)) != NULL)
	{
		for(eint32 i = 0; i < list->CountItems(); i++)
		{
			_etk_token_t *aToken = (_etk_token_t*)list->ItemAt(i);
			if(aToken != NULL) delete aToken;
		}
		delete list;
	}
}


euint64
ETokensDepotPrivateData::AddToken(void *data)
{
	euint64 token = E_MAXUINT64;
	_etk_token_t *aToken = new _etk_token_t;

	EList *list = NULL;
	for(eint32 i = 0; i < CountItems(); i++)
	{
		list = (EList*)ItemAt(i);
		if(list->CountItems() >= E_MAXINT32 - 1 || list->AddItem(aToken) == false)
		{
			list = NULL;
			continue;
		}
		token = ((euint64)i << 32) | (euint64)(list->CountItems() - 1);
	}

	if(token == E_MAXUINT64)
	{
		if(CountItems() < E_MAXINT32 - 1)
		{
			if(AddItem(list = new EList()) == false || list->AddItem(aToken) == false)
			{
				RemoveItem(list);
				delete list;
			}
			else
			{
				token = ((euint64)(CountItems() - 1) << 32) | (euint64)(list->CountItems() - 1);
			}
		}
		else for(eint32 i = 0; i < CountItems(); i++)
		{
			list = (EList*)ItemAt(i);
			eint32 index = list->IndexOf(NULL);
			if(index < 0)
			{
				list = NULL;
				continue;
			}
			list->ReplaceItem(index, aToken);
			token = ((euint64)i << 32) | (euint64)index;
		}
	}

	if(token != E_MAXUINT64)
	{
		aToken->vitalities = 1;
		aToken->time_stamp = e_system_time();
		while(aToken->time_stamp == e_system_time())
		{
			// do nothing, waiting till "e_system_time()" changed.
		}
		aToken->data = data;
	}
	else
	{
		delete aToken;
	}

	return token;
}


void
ETokensDepotPrivateData::RemoveToken(euint64 token)
{
	euint64 index = token >> 32;
	if(index > (euint64)E_MAXINT32 - 1) return;

	EList *list = (EList*)ItemAt((eint32)index);
	if(list == NULL) return;

	index = token & 0xffffffff;
	_etk_token_t *aToken = (_etk_token_t*)(list->ItemAt((eint32)index));
	if(aToken == NULL) return;

	if(aToken->vitalities > 1)
	{
		aToken->vitalities -= 1;
		aToken->data = NULL;
	}
	else
	{
		if(index < (euint64)list->CountItems() - 1)
		{
			list->ReplaceItem((eint32)index, NULL);
		}
		else
		{
			list->RemoveItem((eint32)index);
			while(list->LastItem() == NULL && list->IsEmpty() == false) list->RemoveItem(list->CountItems() - 1);
			for(; !(list == NULL || LastItem() != (void*)list || list->IsEmpty() == false); list = (EList*)LastItem())
			{
				delete (EList*)RemoveItem(CountItems() - 1);
			}
		}
		delete aToken;
	}
}


_etk_token_t*
ETokensDepotPrivateData::TokenAt(euint64 token) const
{
	euint64 index = token >> 32;
	if(index > (euint64)E_MAXINT32 - 1) return NULL;

	EList *list = (EList*)ItemAt((eint32)index);
	if(list == NULL) return NULL;

	index = token & 0xffffffff;
	return((_etk_token_t*)(list->ItemAt((eint32)index)));
}


ETokensDepot::ETokensDepot(ELocker *locker, bool deconstruct_locker)
	: fLocker(locker), fDeconstructLocker(deconstruct_locker)
{
	fData = reinterpret_cast<void*>(new ETokensDepotPrivateData());
}


ETokensDepot::~ETokensDepot()
{
	fToken.fDepot = NULL;
	delete reinterpret_cast<ETokensDepotPrivateData*>(fData);
	if(fDeconstructLocker && fLocker != NULL) delete fLocker;
}


EToken*
ETokensDepot::CreateToken(void *data)
{
	EToken *aToken = NULL;

	if(Lock())
	{
		ETokensDepotPrivateData *private_data = reinterpret_cast<ETokensDepotPrivateData*>(fData);
		euint64 token = private_data->AddToken(data);
		_etk_token_t *_token = private_data->TokenAt(token);
		if(_token != NULL)
		{
			aToken = new EToken();
			aToken->fOriginal = true;
			aToken->fToken = token;
			aToken->fTimeStamp = _token->time_stamp;
			aToken->fDepot = this;
		}
		Unlock();
	}

	return aToken;
}


EToken*
ETokensDepot::OpenToken(euint64 token, EToken *fetch_token)
{
	EToken *aToken = NULL;

	if(fetch_token == NULL || fetch_token->fDepot == NULL)
	{
		if(Lock())
		{
			ETokensDepotPrivateData *private_data = reinterpret_cast<ETokensDepotPrivateData*>(fData);
			_etk_token_t *_token = private_data->TokenAt(token);
			if(!(_token == NULL || _token->vitalities == E_MAXUINT64))
			{
				_token->vitalities += 1;
				aToken = (fetch_token != NULL ? fetch_token : new EToken());
				aToken->fToken = token;
				aToken->fTimeStamp = _token->time_stamp;
				aToken->fDepot = this;
			}
			Unlock();
		}
	}
	else
	{
		ETK_WARNING("[PRIVATE]: %s --- fetch_token isn't empty.\n", __PRETTY_FUNCTION__);
	}

	return aToken;
}


EToken*
ETokensDepot::FetchToken(euint64 token)
{
	if(!(fLocker == NULL || fLocker->IsLockedByCurrentThread()))
		ETK_ERROR("[PRIVATE]: %s --- Invalid operation", __PRETTY_FUNCTION__);

	ETokensDepotPrivateData *private_data = reinterpret_cast<ETokensDepotPrivateData*>(fData);
	_etk_token_t *_token = private_data->TokenAt(token);
	if(_token == NULL) return NULL;

	fToken.fToken = token;
	fToken.fTimeStamp = _token->time_stamp;
	fToken.fDepot = this;

	return(&fToken);
}


void
ETokensDepot::SetLocker(ELocker *locker, bool deconstruct_locker)
{
	if(fLocker != NULL && fDeconstructLocker) delete fLocker;
	fLocker = locker;
	fDeconstructLocker = deconstruct_locker;
}


ELocker*
ETokensDepot::Locker() const
{
	return fLocker;
}


bool
ETokensDepot::Lock()
{
	return(fLocker == NULL ? true : fLocker->Lock());
}


void
ETokensDepot::Unlock()
{
	if(fLocker != NULL) fLocker->Unlock();
}


EToken::EToken()
	: fOriginal(false), fToken(E_MAXUINT64), fTimeStamp(E_MAXINT64), fDepot(NULL)
{
}


EToken::~EToken()
{
	MakeEmpty();
}


bool
EToken::IsValid() const
{
	bool retVal = false;

	if(!(fToken == E_MAXUINT64 || fDepot == NULL || fDepot->Lock() == false))
	{
		ETokensDepotPrivateData *depot_private = reinterpret_cast<ETokensDepotPrivateData*>(fDepot->fData);
		_etk_token_t *aToken = depot_private->TokenAt(fToken);
		if(!(aToken == NULL || aToken->time_stamp != fTimeStamp)) retVal = true;
		fDepot->Unlock();
	}

	return retVal;
}


euint64
EToken::Token() const
{
	return fToken;
}


e_bigtime_t
EToken::TimeStamp() const
{
	return fTimeStamp;
}


EToken&
EToken::operator+=(euint64 vitalities)
{
	if(fToken == E_MAXUINT64 || fDepot == NULL) ETK_ERROR("[PRIVATE]: %s --- Invalid operation.", __PRETTY_FUNCTION__);
	if(fDepot->Lock() == false) ETK_ERROR("[PRIVATE]: %s --- Unable to lock depot.", __PRETTY_FUNCTION__);
	if(IsValid() == false) ETK_ERROR("[PRIVATE]: %s --- Invalid token.", __PRETTY_FUNCTION__);

	ETokensDepotPrivateData *depot_private = reinterpret_cast<ETokensDepotPrivateData*>(fDepot->fData);
	_etk_token_t *aToken = depot_private->TokenAt(fToken);
	if(aToken->vitalities > E_MAXUINT64 - vitalities) ETK_ERROR("[PRIVATE]: %s --- Invalid vitalities.", __PRETTY_FUNCTION__);
	aToken->vitalities += vitalities;

	fDepot->Unlock();

	return *this;
}


EToken&
EToken::operator-=(euint64 vitalities)
{
	if(fToken == E_MAXUINT64 || fDepot == NULL) ETK_ERROR("[PRIVATE]: %s --- Invalid operation.", __PRETTY_FUNCTION__);
	if(fDepot->Lock() == false) ETK_ERROR("[PRIVATE]: %s --- Unable to lock depot.", __PRETTY_FUNCTION__);
	if(IsValid() == false) ETK_ERROR("[PRIVATE]: %s --- Invalid token.", __PRETTY_FUNCTION__);

	ETokensDepotPrivateData *depot_private = reinterpret_cast<ETokensDepotPrivateData*>(fDepot->fData);
	_etk_token_t *aToken = depot_private->TokenAt(fToken);
	if(aToken->vitalities < vitalities) ETK_ERROR("[PRIVATE]: %s --- Invalid vitalities.", __PRETTY_FUNCTION__);
	if((aToken->vitalities -= vitalities) == 0)
	{
		depot_private->RemoveToken(fToken);
		fDepot->Unlock();
		fOriginal = false;
		fToken = E_MAXUINT64;
		fTimeStamp = E_MAXINT64;
		fDepot = NULL;
	}
	else
	{
		fDepot->Unlock();
	}

	return *this;
}


EToken&
EToken::operator++()
{
	return operator+=(1);
}


EToken&
EToken::operator--()
{
	return operator-=(1);
}


euint64
EToken::Vitalities() const
{
	euint64 retVal = 0;

	if(!(fToken == E_MAXUINT64 || fDepot == NULL || fDepot->Lock() == false))
	{
		if(IsValid())
		{
			ETokensDepotPrivateData *depot_private = reinterpret_cast<ETokensDepotPrivateData*>(fDepot->fData);
			retVal = depot_private->TokenAt(fToken)->vitalities;
		}
		fDepot->Unlock();
	}

	return retVal;
}


void*
EToken::Data() const
{
	void *retVal = NULL;

	if(!(fToken == E_MAXUINT64 || fDepot == NULL || fDepot->Lock() == false))
	{
		if(IsValid())
		{
			ETokensDepotPrivateData *depot_private = reinterpret_cast<ETokensDepotPrivateData*>(fDepot->fData);
			retVal = depot_private->TokenAt(fToken)->data;
		}
		fDepot->Unlock();
	}

	return retVal;
}


void
EToken::SetData(void *data)
{
	if(!(fOriginal == false || fToken == E_MAXUINT64 || fDepot == NULL || fDepot->Lock() == false))
	{
		if(IsValid())
		{
			ETokensDepotPrivateData *depot_private = reinterpret_cast<ETokensDepotPrivateData*>(fDepot->fData);
			depot_private->TokenAt(fToken)->data = data;
		}
		fDepot->Unlock();
	}
}


ETokensDepot*
EToken::Depot() const
{
	return fDepot;
}


void
EToken::MakeEmpty()
{
	if(!(fToken == E_MAXUINT64 || fDepot == NULL || fDepot->Lock() == false))
	{
		if(IsValid())
		{
			ETokensDepotPrivateData *depot_private = reinterpret_cast<ETokensDepotPrivateData*>(fDepot->fData);
			if(fOriginal)
				depot_private->RemoveToken(fToken);
			else
				operator--();
		}
		fDepot->Unlock();
	}

	fOriginal = false;
	fToken = E_MAXUINT64;
	fTimeStamp = E_MAXINT64;
	fDepot = NULL;
}

