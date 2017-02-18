/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: Token.cpp
 *
 * --------------------------------------------------------------------------*/

#include "Token.h"

#include "../kernel/Kernel.h"
#include "../kernel/Debug.h"
#include "../support/Locker.h"
#include "../support/List.h"

struct _LOCAL _bhapi_token_t {
	b_uint64 vitalities;
	b_bigtime_t time_stamp;
	void *data;
};


class _LOCAL BTokensDepotPrivateData : public BList {
public:
	BTokensDepotPrivateData();
	virtual ~BTokensDepotPrivateData();

	b_uint64		AddToken(void *data);
	void		RemoveToken(b_uint64 token);
	_bhapi_token_t	*TokenAt(b_uint64 token) const;
};


BTokensDepotPrivateData::BTokensDepotPrivateData()
	: BList()
{
}


BTokensDepotPrivateData::~BTokensDepotPrivateData()
{
	BList *list;

	while((list = (BList*)RemoveItem(0)) != NULL)
	{
		for(b_int32 i = 0; i < list->CountItems(); i++)
		{
			_bhapi_token_t *aToken = (_bhapi_token_t*)list->ItemAt(i);
			if(aToken != NULL) delete aToken;
		}
		delete list;
	}
}


b_uint64
BTokensDepotPrivateData::AddToken(void *data)
{
	b_uint64 token = B_MAXUINT64;
	_bhapi_token_t *aToken = new _bhapi_token_t;

	BList *list = NULL;
	for(b_int32 i = 0; i < CountItems(); i++)
	{
		list = (BList*)ItemAt(i);
		if(list->CountItems() >= B_MAXINT32 - 1 || list->AddItem(aToken) == false)
		{
			list = NULL;
			continue;
		}
		token = ((b_uint64)i << 32) | (b_uint64)(list->CountItems() - 1);
	}

	if(token == B_MAXUINT64)
	{
		if(CountItems() < B_MAXINT32 - 1)
		{
			if(AddItem(list = new BList()) == false || list->AddItem(aToken) == false)
			{
				RemoveItem(list);
				delete list;
			}
			else
			{
				token = ((b_uint64)(CountItems() - 1) << 32) | (b_uint64)(list->CountItems() - 1);
			}
		}
		else for(b_int32 i = 0; i < CountItems(); i++)
		{
			list = (BList*)ItemAt(i);
			b_int32 index = list->IndexOf(NULL);
			if(index < 0)
			{
				list = NULL;
				continue;
			}
			list->ReplaceItem(index, aToken);
			token = ((b_uint64)i << 32) | (b_uint64)index;
		}
	}

	if(token != B_MAXUINT64)
	{
		aToken->vitalities = 1;
		aToken->time_stamp = b_system_time();
		while(aToken->time_stamp == b_system_time())
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
BTokensDepotPrivateData::RemoveToken(b_uint64 token)
{
	b_uint64 index = token >> 32;
	if(index > (b_uint64)B_MAXINT32 - 1) return;

	BList *list = (BList*)ItemAt((b_int32)index);
	if(list == NULL) return;

	index = token & 0xffffffff;
	_bhapi_token_t *aToken = (_bhapi_token_t*)(list->ItemAt((b_int32)index));
	if(aToken == NULL) return;

	if(aToken->vitalities > 1)
	{
		aToken->vitalities -= 1;
		aToken->data = NULL;
	}
	else
	{
		if(index < (b_uint64)list->CountItems() - 1)
		{
			list->ReplaceItem((b_int32)index, NULL);
		}
		else
		{
			list->RemoveItem((b_int32)index);
			while(list->LastItem() == NULL && list->IsEmpty() == false) list->RemoveItem(list->CountItems() - 1);
			for(; !(list == NULL || LastItem() != (void*)list || list->IsEmpty() == false); list = (BList*)LastItem())
			{
				delete (BList*)RemoveItem(CountItems() - 1);
			}
		}
		delete aToken;
	}
}


_bhapi_token_t*
BTokensDepotPrivateData::TokenAt(b_uint64 token) const
{
	b_uint64 index = token >> 32;
	if(index > (b_uint64)B_MAXINT32 - 1) return NULL;

	BList *list = (BList*)ItemAt((b_int32)index);
	if(list == NULL) return NULL;

	index = token & 0xffffffff;
	return((_bhapi_token_t*)(list->ItemAt((b_int32)index)));
}


BTokensDepot::BTokensDepot(BLocker *locker, bool deconstruct_locker)
	: fLocker(locker), fDeconstructLocker(deconstruct_locker)
{
	fData = reinterpret_cast<void*>(new BTokensDepotPrivateData());
}


BTokensDepot::~BTokensDepot()
{
	fToken.fDepot = NULL;
	delete reinterpret_cast<BTokensDepotPrivateData*>(fData);
	if(fDeconstructLocker && fLocker != NULL) delete fLocker;
}


BToken*
BTokensDepot::CreateToken(void *data)
{
	BToken *aToken = NULL;

	if(Lock())
	{
		BTokensDepotPrivateData *private_data = reinterpret_cast<BTokensDepotPrivateData*>(fData);
		b_uint64 token = private_data->AddToken(data);
		_bhapi_token_t *_token = private_data->TokenAt(token);
		if(_token != NULL)
		{
			aToken = new BToken();
			aToken->fOriginal = true;
			aToken->fToken = token;
			aToken->fTimeStamp = _token->time_stamp;
			aToken->fDepot = this;
		}
		Unlock();
	}

	return aToken;
}


BToken*
BTokensDepot::OpenToken(b_uint64 token, BToken *fetch_token)
{
	BToken *aToken = NULL;

	if(fetch_token == NULL || fetch_token->fDepot == NULL)
	{
		if(Lock())
		{
			BTokensDepotPrivateData *private_data = reinterpret_cast<BTokensDepotPrivateData*>(fData);
			_bhapi_token_t *_token = private_data->TokenAt(token);
			if(!(_token == NULL || _token->vitalities == B_MAXUINT64))
			{
				_token->vitalities += 1;
				aToken = (fetch_token != NULL ? fetch_token : new BToken());
				aToken->fToken = token;
				aToken->fTimeStamp = _token->time_stamp;
				aToken->fDepot = this;
			}
			Unlock();
		}
	}
	else
	{
		BHAPI_WARNING("[PRIVATE]: %s --- fetch_token isn't empty.\n", __PRETTY_FUNCTION__);
	}

	return aToken;
}


BToken*
BTokensDepot::FetchToken(b_uint64 token)
{
	if(!(fLocker == NULL || fLocker->IsLockedByCurrentThread()))
		BHAPI_ERROR("[PRIVATE]: %s --- Invalid operation", __PRETTY_FUNCTION__);

	BTokensDepotPrivateData *private_data = reinterpret_cast<BTokensDepotPrivateData*>(fData);
	_bhapi_token_t *_token = private_data->TokenAt(token);
	if(_token == NULL) return NULL;

	fToken.fToken = token;
	fToken.fTimeStamp = _token->time_stamp;
	fToken.fDepot = this;

	return(&fToken);
}


void
BTokensDepot::SetLocker(BLocker *locker, bool deconstruct_locker)
{
	if(fLocker != NULL && fDeconstructLocker) delete fLocker;
	fLocker = locker;
	fDeconstructLocker = deconstruct_locker;
}


BLocker*
BTokensDepot::Locker() const
{
	return fLocker;
}


bool
BTokensDepot::Lock()
{
	return(fLocker == NULL ? true : fLocker->Lock());
}


void
BTokensDepot::Unlock()
{
	if(fLocker != NULL) fLocker->Unlock();
}


BToken::BToken()
	: fOriginal(false), fToken(B_MAXUINT64), fTimeStamp(B_MAXINT64), fDepot(NULL)
{
}


BToken::~BToken()
{
	MakeEmpty();
}


bool
BToken::IsValid() const
{
	bool retVal = false;

	if(!(fToken == B_MAXUINT64 || fDepot == NULL || fDepot->Lock() == false))
	{
		BTokensDepotPrivateData *depot_private = reinterpret_cast<BTokensDepotPrivateData*>(fDepot->fData);
		_bhapi_token_t *aToken = depot_private->TokenAt(fToken);
		if(!(aToken == NULL || aToken->time_stamp != fTimeStamp)) retVal = true;
		fDepot->Unlock();
	}

	return retVal;
}


b_uint64
BToken::Token() const
{
	return fToken;
}


b_bigtime_t
BToken::TimeStamp() const
{
	return fTimeStamp;
}


BToken&
BToken::operator+=(b_uint64 vitalities)
{
	if(fToken == B_MAXUINT64 || fDepot == NULL) BHAPI_ERROR("[PRIVATE]: %s --- Invalid operation.", __PRETTY_FUNCTION__);
	if(fDepot->Lock() == false) BHAPI_ERROR("[PRIVATE]: %s --- Unable to lock depot.", __PRETTY_FUNCTION__);
	if(IsValid() == false) BHAPI_ERROR("[PRIVATE]: %s --- Invalid token.", __PRETTY_FUNCTION__);

	BTokensDepotPrivateData *depot_private = reinterpret_cast<BTokensDepotPrivateData*>(fDepot->fData);
	_bhapi_token_t *aToken = depot_private->TokenAt(fToken);
	if(aToken->vitalities > B_MAXUINT64 - vitalities) BHAPI_ERROR("[PRIVATE]: %s --- Invalid vitalities.", __PRETTY_FUNCTION__);
	aToken->vitalities += vitalities;

	fDepot->Unlock();

	return *this;
}


BToken&
BToken::operator-=(b_uint64 vitalities)
{
	if(fToken == B_MAXUINT64 || fDepot == NULL) BHAPI_ERROR("[PRIVATE]: %s --- Invalid operation.", __PRETTY_FUNCTION__);
	if(fDepot->Lock() == false) BHAPI_ERROR("[PRIVATE]: %s --- Unable to lock depot.", __PRETTY_FUNCTION__);
	if(IsValid() == false) BHAPI_ERROR("[PRIVATE]: %s --- Invalid token.", __PRETTY_FUNCTION__);

	BTokensDepotPrivateData *depot_private = reinterpret_cast<BTokensDepotPrivateData*>(fDepot->fData);
	_bhapi_token_t *aToken = depot_private->TokenAt(fToken);
	if(aToken->vitalities < vitalities) BHAPI_ERROR("[PRIVATE]: %s --- Invalid vitalities.", __PRETTY_FUNCTION__);
	if((aToken->vitalities -= vitalities) == 0)
	{
		depot_private->RemoveToken(fToken);
		fDepot->Unlock();
		fOriginal = false;
		fToken = B_MAXUINT64;
		fTimeStamp = B_MAXINT64;
		fDepot = NULL;
	}
	else
	{
		fDepot->Unlock();
	}

	return *this;
}


BToken&
BToken::operator++()
{
	return operator+=(1);
}


BToken&
BToken::operator--()
{
	return operator-=(1);
}


b_uint64
BToken::Vitalities() const
{
	b_uint64 retVal = 0;

	if(!(fToken == B_MAXUINT64 || fDepot == NULL || fDepot->Lock() == false))
	{
		if(IsValid())
		{
			BTokensDepotPrivateData *depot_private = reinterpret_cast<BTokensDepotPrivateData*>(fDepot->fData);
			retVal = depot_private->TokenAt(fToken)->vitalities;
		}
		fDepot->Unlock();
	}

	return retVal;
}


void*
BToken::Data() const
{
	void *retVal = NULL;

	if(!(fToken == B_MAXUINT64 || fDepot == NULL || fDepot->Lock() == false))
	{
		if(IsValid())
		{
			BTokensDepotPrivateData *depot_private = reinterpret_cast<BTokensDepotPrivateData*>(fDepot->fData);
			retVal = depot_private->TokenAt(fToken)->data;
		}
		fDepot->Unlock();
	}

	return retVal;
}


void
BToken::SetData(void *data)
{
	if(!(fOriginal == false || fToken == B_MAXUINT64 || fDepot == NULL || fDepot->Lock() == false))
	{
		if(IsValid())
		{
			BTokensDepotPrivateData *depot_private = reinterpret_cast<BTokensDepotPrivateData*>(fDepot->fData);
			depot_private->TokenAt(fToken)->data = data;
		}
		fDepot->Unlock();
	}
}


BTokensDepot*
BToken::Depot() const
{
	return fDepot;
}


void
BToken::MakeEmpty()
{
	if(!(fToken == B_MAXUINT64 || fDepot == NULL || fDepot->Lock() == false))
	{
		if(IsValid())
		{
			BTokensDepotPrivateData *depot_private = reinterpret_cast<BTokensDepotPrivateData*>(fDepot->fData);
			if(fOriginal)
				depot_private->RemoveToken(fToken);
			else
				operator--();
		}
		fDepot->Unlock();
	}

	fOriginal = false;
	fToken = B_MAXUINT64;
	fTimeStamp = B_MAXINT64;
	fDepot = NULL;
}

