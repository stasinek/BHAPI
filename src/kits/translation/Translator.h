/*
 * Copyright 2002-2009, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _TRANSLATOR_H
#define _TRANSLATOR_H


#include <Archivable.h>
#include <TranslationDefs.h>
#include <TranslatorRoster.h>


class BTranslator : public BArchivable {
public:
								BTranslator();

			BTranslator*		Acquire();
			BTranslator*		Release();

			int32_t				ReferenceCount();

	virtual	const char*			TranslatorName() const = 0;
	virtual const char*			TranslatorInfo() const = 0;
	virtual int32_t				TranslatorVersion() const = 0;

	virtual const translation_format* InputFormats(int32_t* _count) const = 0;
	virtual const translation_format* OutputFormats(int32_t* _count) const = 0;

	virtual status_t			Identify(BPositionIO* source,
									const translation_format* format,
									BMessage* extension, translator_info* info,
									uint32_t outType) = 0;
	virtual status_t			Translate(BPositionIO* source,
									const translator_info* info,
									BMessage* extension, uint32_t outType,
									BPositionIO* destination) = 0;
	virtual status_t			MakeConfigurationView(BMessage* extension,
									BView** _view, BRect* _extent);
	virtual status_t			GetConfigurationMessage(BMessage* extension);

protected:
	virtual						~BTranslator();
		// this is protected because the object is deleted by the
		// Release() function instead of being deleted directly by
		// the user

private:
	friend class BTranslatorRoster::Private;

	virtual status_t			_Reserved_Translator_0(int32_t, void*);
	virtual status_t			_Reserved_Translator_1(int32_t, void*);
	virtual status_t			_Reserved_Translator_2(int32_t, void*);
	virtual status_t			_Reserved_Translator_3(int32_t, void*);
	virtual status_t			_Reserved_Translator_4(int32_t, void*);
	virtual status_t			_Reserved_Translator_5(int32_t, void*);
	virtual status_t			_Reserved_Translator_6(int32_t, void*);
	virtual status_t			_Reserved_Translator_7(int32_t, void*);

			BTranslatorRoster::Private* fOwningRoster;
			translator_id		fID;
			int32_t				fRefCount;

			uint32_t				_reserved[7];
};


// The post-4.5 API suggests implementing this function in your translator
// add-on rather than the separate functions and variables of the previous
// API. You will be called for values of n starting at 0 and increasing;
// return 0 when you can't make another kind of translator (i.e. for n = 1
// if you only implement one subclass of BTranslator). Ignore flags for now.
extern "C" BTranslator* make_nth_translator(int32_t n, image_id you,
	uint32_t flags, ...);


#endif	// _TRANSLATOR_H
