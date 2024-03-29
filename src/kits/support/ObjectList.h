﻿/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
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
 * File: ObjectList.h
 *
 *
 * --------------------------------------------------------------------------*/

/*
Open Tracker License

Terms and Conditions

Copyright (c) 1991-2000, Be Incorporated. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice applies to all licensees
and shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF TITLE, MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
BE INCORPORATED BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Be Incorporated shall not be
used in advertising or otherwise to promote the sale, use or other dealings in
this Software without prior written authorization from Be Incorporated.

Tracker(TM), Be(R), BeOS(R), and BeIA(TM) are trademarks or registered trademarks
of Be Incorporated in the United States and other countries. Other brand product
names are registered trademarks or trademarks of their respective holders.
All rights reserved.
*/
#ifndef BHAPI_OBJECT_LIST_H
#define BHAPI_OBJECT_LIST_H

#ifdef __cplusplus
#include <kits/support/List.h>
#include <kits/support/SupportDefs.h>
#include <new>
//
// ObjectList is a wrapper around BList that adds type safety,
// optional object ownership, search, insert operations, etc.
//
template<class T> class BObjectList;
template<class T>
struct UnaryPredicate {
    virtual int operator()(const T *) const
        // virtual could be avoided here if FindBinaryInsertionIndex,
        // etc. were member template functions
    {
        return 0;
    }

private:
    static int _unary_predicate_glue(const void *item, void *context);

    friend class BObjectList<T>;
};


template<class T>
int UnaryPredicate<T>::_unary_predicate_glue(const void *item, void *context)
{
    return ((UnaryPredicate<T> *)context)->operator()((const T *)item);
}

class _PointerList_ : public BList {
public:
    _PointerList_(const _PointerList_ &list);
    _PointerList_(int32_t itemsPerBlock = 20, bool owning = false);
    ~_PointerList_(void);

    typedef void *(* GenericEachFunction)(void *, void *);
    typedef int (* GenericCompareFunction)(const void *, const void *);
    typedef int (* GenericCompareFunctionWithState)(const void *, const void *,
        void *);
    typedef int (* UnaryPredicateGlue)(const void *, void *);

    void *EachElement(GenericEachFunction, void *);
    void SortItems(GenericCompareFunction);
    void SortItems(GenericCompareFunctionWithState, void *state);
    void HSortItems(GenericCompareFunction);
    void HSortItems(GenericCompareFunctionWithState, void *state);

    void *BinarySearch(const void *, GenericCompareFunction) const;
    void *BinarySearch(const void *, GenericCompareFunctionWithState,
        void *state) const;

    int32_t BinarySearchIndex(const void *, GenericCompareFunction) const;
    int32_t BinarySearchIndex(const void *, GenericCompareFunctionWithState,
        void *state) const;
    int32_t BinarySearchIndexByPredicate(const void *, UnaryPredicateGlue) const;

    bool Owning() const;
    bool ReplaceItem(int32_t, void *);
    bool MoveItem(int32_t from, int32_t to);

protected:
    bool owning;

};

template<class T>
class BObjectList : private _PointerList_ {
public:
    // iteration and sorting
    typedef	T*					(*EachFunction)(T*, void*);
    typedef	const T*			(*ConstEachFunction)(const T*, void*);
    typedef	int 				(*CompareFunction)(const T*, const T*);
    typedef	int 				(*CompareFunctionWithState)(const T*, const T*,
                                    void* state);

                                BObjectList(int32_t itemsPerBlock = 20,
                                    bool owning = false);
                                BObjectList(const BObjectList& list);
                                    // clones list; if list is owning, makes
                                    // copies of all the items

    virtual						~BObjectList();

            BObjectList&		operator=(const BObjectList& list);
                                    // clones list; if list is owning, makes
                                    // copies of all the items

                                // adding and removing
            bool				AddItem(T*);
            bool				AddItem(T*, int32_t);
            bool				AddList(BObjectList*);
            bool				AddList(BObjectList*, int32_t);

            bool				RemoveItem(T*, bool deleteIfOwning = true);
                                    // if owning, deletes the removed item
            T*					RemoveItemAt(int32_t);
                                    // returns the removed item

            void				MakeEmpty(bool deleteIfOwning = true);

                                // item access
            T*					ItemAt(int32_t) const;

            bool				ReplaceItem(int32_t index, T*);
                                    // if list is owning, deletes the item
                                    // at <index> first
            T*					SwapWithItem(int32_t index, T* newItem);
                                    // same as ReplaceItem, except does not
                                    // delete old item at <index>, returns it
                                    // instead
            bool				MoveItem(int32_t from, int32_t to);

            T*					FirstItem() const;
            T*					LastItem() const;

                                // misc. getters
            int32_t				IndexOf(const T*) const;
            bool				HasItem(const T*) const;
            bool				IsEmpty() const;
            int32_t				CountItems() const;

            T*					EachElement(EachFunction, void*);
            const T*			EachElement(ConstEachFunction, void*) const;

            void				SortItems(CompareFunction);
            void				SortItems(CompareFunctionWithState,
                                    void* state);
            void				HSortItems(CompareFunction);
            void				HSortItems(CompareFunctionWithState,
                                    void* state);

                                // linear search, returns first item that
                                // matches predicate
            const T*			FindIf(const UnaryPredicate<T>&) const;
            T*					FindIf(const UnaryPredicate<T>&);

                                // list must be sorted with CompareFunction for
                                // these to work
            T*					BinarySearch(const T&, CompareFunction) const;
            T*					BinarySearch(const T&, CompareFunctionWithState,
                                    void* state) const;

            template<typename Key>
            T*					BinarySearchByKey(const Key& key,
                                    int (*compare)(const Key*, const T*)) const;

            template<typename Key>
            T*					BinarySearchByKey(const Key& key,
                                    int (*compare)(const Key*, const T*, void*),
                                    void* state) const;

            int32_t				BinarySearchIndex(const T&item,
                                    CompareFunction compare) const;
            int32_t				BinarySearchIndex(const T&item,
                                    CompareFunctionWithState compare,
                                    void* state) const;

            template<typename Key>
            int32_t				BinarySearchIndexByKey(const Key& key,
                                    int (*compare)(const Key*, const T*)) const;

                                // Binary insertion - list must be sorted with
                                // CompareFunction for these to work

                                // simple insert
            bool				BinaryInsert(T*, CompareFunction);
            bool				BinaryInsert(T*, CompareFunctionWithState,
                                    void* state);
            bool				BinaryInsert(T*, const UnaryPredicate<T>&);

                                // unique insert, returns false if item already
                                // in list
            bool				BinaryInsertUnique(T*, CompareFunction);
            bool				BinaryInsertUnique(T*, CompareFunctionWithState,
                                    void* state);
            bool				BinaryInsertUnique(T*,
                                    const UnaryPredicate<T>&);

                                // insert a copy of the item, returns new
                                // inserted item
            T*					BinaryInsertCopy(const T& copyThis,
                                    CompareFunction);
            T*					BinaryInsertCopy(const T& copyThis,
                                    CompareFunctionWithState, void* state);

                                // insert a copy of the item if not in list
                                // already returns new inserted item or existing
                                // item in case of a conflict
            T*					BinaryInsertCopyUnique(const T& copyThis,
                                    CompareFunction);
            T*					BinaryInsertCopyUnique(const T& copyThis,
                                    CompareFunctionWithState, void* state);

            int32_t				FindBinaryInsertionIndex(
                                    const UnaryPredicate<T>&,
                                    bool* alreadyInList = 0) const;
                                    // returns either the index into which a new
                                    // item should be inserted or index of an
                                    // existing item that matches the predicate

            class Private;
private:
    friend	class Private;

            void				_SetItem(int32_t, T*);
};


template<class Item, class Result, class Param1>
Result WhileEachListItem(BObjectList<Item>* list, Result (Item::*func)(Param1),
    Param1 p1)
{
    Result result = 0;
    int32_t count = list->CountItems();

    for (int32_t index = 0; index < count; index++) {
        if ((result = (list->ItemAt(index)->*func)(p1)) != 0)
            break;
    }

    return result;
}


template<class Item, class Result, class Param1>
Result
WhileEachListItem(BObjectList<Item>* list, Result (*func)(Item*, Param1),
    Param1 p1)
{
    Result result = 0;
    int32_t count = list->CountItems();

    for (int32_t index = 0; index < count; index++) {
        if ((result = (*func)(list->ItemAt(index), p1)) != 0)
            break;
    }

    return result;
}


template<class Item, class Result, class Param1, class Param2>
Result
WhileEachListItem(BObjectList<Item>* list, Result (Item::*func)(Param1, Param2),
    Param1 p1, Param2 p2)
{
    Result result = 0;
    int32_t count = list->CountItems();

    for (int32_t index = 0; index < count; index++) {
        if ((result = (list->ItemAt(index)->*func)(p1, p2)) != 0)
            break;
    }

    return result;
}


template<class Item, class Result, class Param1, class Param2>
Result
WhileEachListItem(BObjectList<Item>* list,
    Result (*func)(Item*, Param1, Param2), Param1 p1, Param2 p2)
{
    Result result = 0;
    int32_t count = list->CountItems();

    for (int32_t index = 0; index < count; index++) {
        if ((result = (*func)(list->ItemAt(index), p1, p2)) != 0)
            break;
    }

    return result;
}


template<class Item, class Result, class Param1, class Param2, class Param3,
    class Param4>
Result
WhileEachListItem(BObjectList<Item>* list,
    Result (*func)(Item*, Param1, Param2, Param3, Param4), Param1 p1, Param2 p2,
    Param3 p3, Param4 p4)
{
    Result result = 0;
    int32_t count = list->CountItems();

    for (int32_t index = 0; index < count; index++) {
        if ((result = (*func)(list->ItemAt(index), p1, p2, p3, p4)) != 0)
            break;
    }

    return result;
}


template<class Item, class Result>
void EachListItemIgnoreResult(BObjectList<Item>* list, Result (Item::*func)())
{
    int32_t count = list->CountItems();
    for (int32_t index = 0; index < count; index++)
        (list->ItemAt(index)->*func)();
}


template<class Item, class Param1>
void EachListItem(BObjectList<Item>* list, void (*func)(Item*, Param1), Param1 p1)
{
    int32_t count = list->CountItems();
    for (int32_t index = 0; index < count; index++)
        (func)(list->ItemAt(index), p1);
}


template<class Item, class Param1, class Param2>
void EachListItem(BObjectList<Item>* list, void (Item::*func)(Param1, Param2),
    Param1 p1, Param2 p2)
{
    int32_t count = list->CountItems();
    for (int32_t index = 0; index < count; index++)
        (list->ItemAt(index)->*func)(p1, p2);
}


template<class Item, class Param1, class Param2>
void EachListItem(BObjectList<Item>* list, void (*func)(Item*,Param1, Param2),
    Param1 p1, Param2 p2)
{
    int32_t count = list->CountItems();
    for (int32_t index = 0; index < count; index++)
        (func)(list->ItemAt(index), p1, p2);
}


template<class Item, class Param1, class Param2, class Param3>
void EachListItem(BObjectList<Item>* list,
    void (*func)(Item*,Param1, Param2, Param3), Param1 p1, Param2 p2, Param3 p3)
{
    int32_t count = list->CountItems();
    for (int32_t index = 0; index < count; index++)
        (func)(list->ItemAt(index), p1, p2, p3);
}


template<class Item, class Param1, class Param2, class Param3, class Param4>
void EachListItem(BObjectList<Item>* list,
    void (*func)(Item*,Param1, Param2, Param3, Param4), Param1 p1, Param2 p2,
    Param3 p3, Param4 p4)
{
    int32_t count = list->CountItems();
    for (int32_t index = 0; index < count; index++)
        (func)(list->ItemAt(index), p1, p2, p3, p4);
}


// inline code


inline bool _PointerList_::Owning() const
{
    return owning;
}


template<class T>
BObjectList<T>::BObjectList(int32_t itemsPerBlock, bool owning)
    :
    _PointerList_(itemsPerBlock, owning)
{
}


template<class T>
BObjectList<T>::BObjectList(const BObjectList<T>& list)
    :
    _PointerList_(list)
{
    owning = list.owning;
    if (owning) {
        // make our own copies in an owning list
        int32_t count = list.CountItems();
        for	(int32_t index = 0; index < count; index++) {
            T* item = list.ItemAt(index);
            if (item)
                item = new (std::nothrow) T(*item);
            _SetItem(index, item);
        }
    }
}


template<class T>
BObjectList<T>::~BObjectList()
{
    if (Owning()) {
        // have to nuke elements first
        MakeEmpty();
    }
}


template<class T>
BObjectList<T>&
BObjectList<T>::operator=(const BObjectList<T>& list)
{
    owning = list.owning;
    BObjectList<T> &result = (BObjectList<T>&)_PointerList_::operator=(list);
    if (owning) {
        // make our own copies in an owning list
        int32_t count = list.CountItems();
        for	(int32_t index = 0; index < count; index++) {
            T* item = list.ItemAt(index);
            if (item)
                item = new (std::nothrow) T(*item);
            _SetItem(index, item);
        }
    }
    return result;
}


template<class T>
bool BObjectList<T>::AddItem(T* item)
{
    // need to cast to void* to make T work for const pointers
    return _PointerList_::AddItem((void*)item);
}


template<class T>
bool BObjectList<T>::AddItem(T* item, int32_t index)
{
    return _PointerList_::AddItem((void*)item, index);
}


template<class T>
bool BObjectList<T>::AddList(BObjectList<T>* list)
{
    return _PointerList_::AddList(list);
}


template<class T>
bool BObjectList<T>::AddList(BObjectList<T>* list, int32_t index)
{
    return _PointerList_::AddList(list, index);
}


template<class T>
bool BObjectList<T>::RemoveItem(T* item, bool deleteIfOwning)
{
    bool result = _PointerList_::RemoveItem((void*)item);

    if (result && Owning() && deleteIfOwning)
        delete item;

    return result;
}


template<class T>
T*
BObjectList<T>::RemoveItemAt(int32_t index)
{
    return (T*)_PointerList_::RemoveItem(index);
}


template<class T>
inline T*
BObjectList<T>::ItemAt(int32_t index) const
{
    return (T*)_PointerList_::ItemAt(index);
}


template<class T>
bool BObjectList<T>::ReplaceItem(int32_t index, T* item)
{
    if (owning)
        delete ItemAt(index);

    return _PointerList_::ReplaceItem(index, (void*)item);
}


template<class T>
T*
BObjectList<T>::SwapWithItem(int32_t index, T* item)
{
    T* result = ItemAt(index);
    _PointerList_::ReplaceItem(index, (void*)item);

    return result;
}


template<class T>
bool BObjectList<T>::MoveItem(int32_t from, int32_t to)
{
    return _PointerList_::MoveItem(from, to);
}


template<class T>
void BObjectList<T>::_SetItem(int32_t index, T* newItem)
{
    _PointerList_::ReplaceItem(index, (void*)newItem);
}


template<class T>
int32_t BObjectList<T>::IndexOf(const T* item) const
{
    return _PointerList_::IndexOf((void*)item);
}


template<class T>
T*
BObjectList<T>::FirstItem() const
{
    return (T*)_PointerList_::FirstItem();
}


template<class T>
T*
BObjectList<T>::LastItem() const
{
    return (T*)_PointerList_::LastItem();
}


template<class T>
bool BObjectList<T>::HasItem(const T* item) const
{
    return _PointerList_::HasItem((void*)item);
}


template<class T>
bool BObjectList<T>::IsEmpty() const
{
    return _PointerList_::IsEmpty();
}


template<class T>
int32_t BObjectList<T>::CountItems() const
{
    return _PointerList_::CountItems();
}


template<class T>
void BObjectList<T>::MakeEmpty(bool deleteIfOwning)
{
    if (owning && deleteIfOwning) {
        int32_t count = CountItems();
        for (int32_t index = 0; index < count; index++)
            delete ItemAt(index);
    }
    _PointerList_::MakeEmpty();
}


template<class T>
T*
BObjectList<T>::EachElement(EachFunction func, void* params)
{
    return (T*)_PointerList_::EachElement((GenericEachFunction)func, params);
}


template<class T>
const T*
BObjectList<T>::EachElement(ConstEachFunction func, void* params) const
{
    return (const T*)
        const_cast<BObjectList<T>*>(this)->_PointerList_::EachElement(
            (GenericEachFunction)func, params);
}

template<class T>
const T*
BObjectList<T>::FindIf(const UnaryPredicate<T>& predicate) const
{
    int32_t count = CountItems();
    for (int32_t index = 0; index < count; index++) {
        if (predicate.operator()(ItemAt(index)) == 0)
            return ItemAt(index);
    }
    return 0;
}

template<class T>
T*
BObjectList<T>::FindIf(const UnaryPredicate<T>& predicate)
{
    int32_t count = CountItems();
    for (int32_t index = 0; index < count; index++) {
        if (predicate.operator()(ItemAt(index)) == 0)
            return ItemAt(index);
    }
    return 0;
}


template<class T>
void BObjectList<T>::SortItems(CompareFunction function)
{
    _PointerList_::SortItems((GenericCompareFunction)function);
}


template<class T>
void BObjectList<T>::SortItems(CompareFunctionWithState function, void* state)
{
    _PointerList_::SortItems((GenericCompareFunctionWithState)function, state);
}


template<class T>
void BObjectList<T>::HSortItems(CompareFunction function)
{
    _PointerList_::HSortItems((GenericCompareFunction)function);
}


template<class T>
void BObjectList<T>::HSortItems(CompareFunctionWithState function, void* state)
{
    _PointerList_::HSortItems((GenericCompareFunctionWithState)function, state);
}


template<class T>
T*
BObjectList<T>::BinarySearch(const T& key, CompareFunction func) const
{
    return (T*)_PointerList_::BinarySearch(&key, (GenericCompareFunction)func);
}


template<class T>
T*
BObjectList<T>::BinarySearch(const T& key, CompareFunctionWithState func,
    void* state) const
{
    return (T*)_PointerList_::BinarySearch(&key,
        (GenericCompareFunctionWithState)func, state);
}


template<class T>
template<typename Key>
T*
BObjectList<T>::BinarySearchByKey(const Key& key,
    int (*compare)(const Key*, const T*)) const
{
    return (T*)_PointerList_::BinarySearch(&key,
        (GenericCompareFunction)compare);
}


template<class T>
template<typename Key>
T*
BObjectList<T>::BinarySearchByKey(const Key &key,
    int (*compare)(const Key*, const T*, void*), void* state) const
{
    return (T*)_PointerList_::BinarySearch(&key,
        (GenericCompareFunctionWithState)compare, state);
}


template<class T>
int32_t BObjectList<T>::BinarySearchIndex(const T& item, CompareFunction compare) const
{
    return _PointerList_::BinarySearchIndex(&item,
        (GenericCompareFunction)compare);
}


template<class T>
int32_t BObjectList<T>::BinarySearchIndex(const T& item,
    CompareFunctionWithState compare, void* state) const
{
    return _PointerList_::BinarySearchIndex(&item,
        (GenericCompareFunctionWithState)compare, state);
}


template<class T>
template<typename Key>
int32_t BObjectList<T>::BinarySearchIndexByKey(const Key& key,
    int (*compare)(const Key*, const T*)) const
{
    return _PointerList_::BinarySearchIndex(&key,
        (GenericCompareFunction)compare);
}


template<class T>
bool BObjectList<T>::BinaryInsert(T* item, CompareFunction func)
{
    int32_t index = _PointerList_::BinarySearchIndex(item,
        (GenericCompareFunction)func);
    if (index >= 0) {
        // already in list, add after existing
        return AddItem(item, index + 1);
    }

    return AddItem(item, -index - 1);
}


template<class T>
bool BObjectList<T>::BinaryInsert(T* item, CompareFunctionWithState func,
    void* state)
{
    int32_t index = _PointerList_::BinarySearchIndex(item,
        (GenericCompareFunctionWithState)func, state);
    if (index >= 0) {
        // already in list, add after existing
        return AddItem(item, index + 1);
    }

    return AddItem(item, -index - 1);
}


template<class T>
bool BObjectList<T>::BinaryInsertUnique(T* item, CompareFunction func)
{
    int32_t index = _PointerList_::BinarySearchIndex(item,
        (GenericCompareFunction)func);
    if (index >= 0)
        return false;

    return AddItem(item, -index - 1);
}


template<class T>
bool BObjectList<T>::BinaryInsertUnique(T* item, CompareFunctionWithState func,
    void* state)
{
    int32_t index = _PointerList_::BinarySearchIndex(item,
        (GenericCompareFunctionWithState)func, state);
    if (index >= 0)
        return false;

    return AddItem(item, -index - 1);
}


template<class T>
T*
BObjectList<T>::BinaryInsertCopy(const T& copyThis, CompareFunction func)
{
    int32_t index = _PointerList_::BinarySearchIndex(&copyThis,
        (GenericCompareFunction)func);

    if (index >= 0)
        index++;
    else
        index = -index - 1;

    T* newItem = new (std::nothrow) T(copyThis);
    AddItem(newItem, index);
    return newItem;
}


template<class T>
T*
BObjectList<T>::BinaryInsertCopy(const T& copyThis,
    CompareFunctionWithState func, void* state)
{
    int32_t index = _PointerList_::BinarySearchIndex(&copyThis,
        (GenericCompareFunctionWithState)func, state);

    if (index >= 0)
        index++;
    else
        index = -index - 1;

    T* newItem = new (std::nothrow) T(copyThis);
    AddItem(newItem, index);
    return newItem;
}


template<class T>
T*
BObjectList<T>::BinaryInsertCopyUnique(const T& copyThis, CompareFunction func)
{
    int32_t index = _PointerList_::BinarySearchIndex(&copyThis,
        (GenericCompareFunction)func);
    if (index >= 0)
        return ItemAt(index);

    index = -index - 1;
    T* newItem = new (std::nothrow) T(copyThis);
    AddItem(newItem, index);
    return newItem;
}


template<class T>
T*
BObjectList<T>::BinaryInsertCopyUnique(const T& copyThis,
    CompareFunctionWithState func, void* state)
{
    int32_t index = _PointerList_::BinarySearchIndex(&copyThis,
        (GenericCompareFunctionWithState)func, state);
    if (index >= 0)
        return ItemAt(index);

    index = -index - 1;
    T* newItem = new (std::nothrow) T(copyThis);
    AddItem(newItem, index);
    return newItem;
}


template<class T>
int32_t BObjectList<T>::FindBinaryInsertionIndex(const UnaryPredicate<T>& pred,
    bool* alreadyInList) const
{
    int32_t index = _PointerList_::BinarySearchIndexByPredicate(&pred,
        (UnaryPredicateGlue)&UnaryPredicate<T>::_unary_predicate_glue);

    if (alreadyInList)
        *alreadyInList = index >= 0;

    if (index < 0)
        index = -index - 1;

    return index;
}


template<class T>
bool BObjectList<T>::BinaryInsert(T* item, const UnaryPredicate<T>& pred)
{
    return AddItem(item, FindBinaryInsertionIndex(pred));
}


template<class T>
bool BObjectList<T>::BinaryInsertUnique(T* item, const UnaryPredicate<T>& pred)
{
    bool alreadyInList;
    int32_t index = FindBinaryInsertionIndex(pred, &alreadyInList);
    if (alreadyInList)
        return false;

    AddItem(item, index);
    return true;
}

#endif // __cplusplus
#endif	// BHAPI_OBJECT_LIST_H
