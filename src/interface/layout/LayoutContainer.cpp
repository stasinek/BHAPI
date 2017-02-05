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
 * File: LayoutContainer.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../../support/ClassInfo.h"

#include "Layout.h"


ELayoutContainer::ELayoutContainer()
	: fUnitsPerPixel(1)
{
	fPrivate[0] = fPrivate[1] = NULL;
}


ELayoutContainer::~ELayoutContainer()
{
	ELayoutItem *item;
	while((item = (ELayoutItem*)fItems.RemoveItem(0)) != NULL)
	{
		item->fContainer = NULL;
		delete item;
	}

	if(fPrivate[0] != NULL && fPrivate[1] != NULL)
		((void (*)(void*))fPrivate[1])(fPrivate[0]);
}


bool
ELayoutContainer::AddItem(ELayoutItem *item, eint32 index)
{
	if(item == NULL || item->fContainer != NULL) return false;
	if(index < 0 || index > fItems.CountItems()) index = fItems.CountItems();

	if(fItems.AddItem((void*)item, index) == false) return false;

	item->fContainer = this;
	for(eint32 i = index; i < fItems.CountItems(); i++) ((ELayoutItem*)fItems.ItemAt(i))->fIndex = i;
	if(!(item->fHidden || item->fFrame.IsValid() == false))
	{
		ERect updateRect = item->fFrame;
		for(; item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
		Invalidate(updateRect);
	}

	return true;
}


bool
ELayoutContainer::RemoveItem(ELayoutItem *item)
{
	if(item == NULL || item->fContainer != this) return false;

	eint32 index = item->fIndex;
	if(fItems.RemoveItem(index) == false) return false;

	item->fContainer = NULL;
	item->fIndex = -1;
	item->UpdateVisibleRegion();

	for(eint32 i = index; i < fItems.CountItems(); i++) ((ELayoutItem*)fItems.ItemAt(i))->fIndex = i;
	if(!(item->fHidden || item->fFrame.IsValid() == false))
	{
		ERect updateRect = item->fFrame;
		for(item = (ELayoutItem*)fItems.ItemAt(index); item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
		Invalidate(updateRect);
	}

	return true;
}


ELayoutItem*
ELayoutContainer::RemoveItem(eint32 index)
{
	ELayoutItem *item = ItemAt(index);
	return(RemoveItem(item) ? item : NULL);
}


ELayoutItem*
ELayoutContainer::ItemAt(eint32 index) const
{
	return (ELayoutItem*)fItems.ItemAt(index);
}


eint32
ELayoutContainer::IndexOf(const ELayoutItem *item) const
{
	return((item == NULL || item->fContainer != this) ? -1 : item->fIndex);
}


eint32
ELayoutContainer::CountItems() const
{
	return fItems.CountItems();
}


float
ELayoutContainer::UnitsPerPixel() const
{
	return fUnitsPerPixel;
}


void
ELayoutContainer::SetUnitsPerPixel(float value, bool deep)
{
	if(value <= 0) return;

	fUnitsPerPixel = value;

	ELayoutItem *item;
	if(deep)
	{
		item = (ELayoutItem*)fItems.ItemAt(0);
		while(item != NULL)
		{
			e_cast_as(item, ELayoutContainer)->fUnitsPerPixel = value;

			if(e_cast_as(item, ELayoutContainer)->fItems.CountItems() > 0)
			{
				item = (ELayoutItem*)e_cast_as(item, ELayoutContainer)->fItems.ItemAt(0);
			}
			else if(item->fContainer == this)
			{
				item = item->NextSibling();
			}
			else
			{
				if(item->NextSibling() != NULL)
				{
					item = item->NextSibling();
					continue;
				}

				while(e_cast_as(item->fContainer, ELayoutItem)->NextSibling() == NULL)
				{
					item = e_cast_as(item->fContainer, ELayoutItem);
					if(item->fContainer == this) break;
				}
				item = e_cast_as(item->fContainer, ELayoutItem)->NextSibling();
			}
		}
	}

	ERect updateRect;
	for(item = (ELayoutItem*)fItems.ItemAt(0); item != NULL; item = item->NextSibling())
	{
		if(item->fHidden || item->fFrame.IsValid() == false) continue;
		updateRect |= item->fFrame;
		item->UpdateVisibleRegion();
	}
	Invalidate(updateRect);
}


void
ELayoutContainer::Invalidate(ERect rect)
{
}


void
ELayoutContainer::SetPrivateData(void *data, void (*destroy_func)(void*))
{
	if(fPrivate[0] != NULL && fPrivate[1] != NULL)
		((void (*)(void*))fPrivate[1])(fPrivate[0]);
	fPrivate[0] = data;
	fPrivate[1] = (void*)destroy_func;
}


void*
ELayoutContainer::PrivateData() const
{
	return fPrivate[0];
}

