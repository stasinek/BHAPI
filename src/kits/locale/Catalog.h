﻿/*
 * Copyright 2003-2012, Haiku, Inc.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_CATALOG_H
#define BHAPI_CATALOG_H

#ifdef __cplusplus
#ifndef BCATALOGDATA_DEF
#define BCATALOGDATA_DEF
class BCatalogData;
#endif
#ifndef BLOCALE_DEF
#define BLOCALE_DEF
class BLocale;
#endif
#ifndef BMESSAGE_DEF
#define BMESSAGE_DEF
class BMessage;
#endif
#include <Entry.h>
#include <LocaleRoster.h>
#include <Locker.h>
#include <kits/support/String.h>
#include <Haiku.h>

class BHAPI_IMPEXP BCatalog {
public:
                                BCatalog();
                                BCatalog(const entry_ref& catalogOwner,
                                    const char* language = NULL,
                                    uint32_t fingerprint = 0);
    virtual						~BCatalog();

            const char*			GetString(const char* string,
                                    const char* context = NULL,
                                    const char* comment = NULL);
            const char*			GetString(uint32_t id);

            status_t			GetData(const char* name, BMessage* msg);
            status_t			GetData(uint32_t id, BMessage* msg);

            status_t			GetSignature(BString* signature);
            status_t			GetLanguage(BString* language);
            status_t			GetFingerprint(uint32_t* fingerprint);

            status_t			SetTo(const entry_ref& catalogOwner,
                                    const char* language = NULL,
                                    uint32_t fingerprint = 0);

            status_t			InitCheck() const;
            int32_t				CountItems() const;

protected:
                                BCatalog(const BCatalog&);
            const BCatalog&		operator= (const BCatalog&);
                                    // hide assignment and copy-constructor

            BCatalogData*		fCatalogData;
    mutable	BLocker				fLock;

private:
    friend	class BLocale;
    friend	status_t			get_add_on_catalog(BCatalog*, const char*);
};
#define BLOCALE_DEF
#endif // C++

#undef B_TRANSLATION_SYSTEM_NAME_CONTEXT
#define B_TRANSLATION_SYSTEM_NAME_CONTEXT "System name"
#ifndef B_AVOID_TRANSLATION_MACROS
// macros for easy catalog-access, define B_AVOID_TRANSLATION_MACROS if
// you don't want these (in which case you need to collect the catalog keys
// manually, as collectcatkeys won't do it for you):
#undef B_TRANSLATION_CONTEXT
    // In a single application, several strings (e.g. 'Ok') will be used
    // more than once, in different contexts.
    // As the application programmer can not know if all translations of
    // this string will be the same for all languages, each occurrence of
    // the string must be translated on its own.
    // Specifying the context explicitly with each string allows the person
    // translating a catalog to separate these different occurrences of the
    // same string and tell which strings appears in what context of the
    // application.
    // In order to give the translator a useful hint, the application
    // programmer needs to define B_TRANSLATION_CONTEXT with the context he'd
    // like to be associated with the strings used in this specifc source file.
    // example:
    //		#define B_TRANSLATION_CONTEXT "Folder-Window"
    // Tip: Use a descriptive name of the class implemented in that
    //		source-file.

#ifdef B_COLLECTING_CATKEYS
// pull in all the macros used when collecting catalog keys.
#include <tools/CollectingCatalog.h>
#else
// Translation macros which may be used to shorten translation requests:
#undef B_TRANSLATE
#define B_TRANSLATE(string) \
    BLocaleRoster::Default()->GetCatalog()->GetString((string), \
        B_TRANSLATION_CONTEXT)

#undef B_TRANSLATE_CONTEXT
#define B_TRANSLATE_CONTEXT(string, context) \
    BLocaleRoster::Default()->GetCatalog()->GetString((string), (context))

#undef B_TRANSLATE_COMMENT
#define B_TRANSLATE_COMMENT(string, comment) \
    BLocaleRoster::Default()->GetCatalog()->GetString((string), \
        B_TRANSLATION_CONTEXT, (comment))

#undef B_TRANSLATE_ALL
#define B_TRANSLATE_ALL(string, context, comment) \
    BLocaleRoster::Default()->GetCatalog()->GetString((string), (context), \
        (comment))

#undef B_TRANSLATE_ID
#define B_TRANSLATE_ID(id) \
    BLocaleRoster::Default()->GetCatalog()->GetString((id))

#undef B_TRANSLATE_SYSTEM_NAME
#define B_TRANSLATE_SYSTEM_NAME(string) \
    BLocaleRoster::Default()->IsFilesystemTranslationPreferred() \
        ? BLocaleRoster::Default()->GetCatalog()->GetString((string), \
            B_TRANSLATION_SYSTEM_NAME_CONTEXT) \
        : (string)

// Translation markers which can be used to mark static strings/IDs which
// are used as key for translation requests (at other places in the code).
/* Example:
        #define B_TRANSLATION_CONTEXT "MyDecentApp-Menu"

        static const char* choices[] = {
            B_TRANSLATE_MARK("left"),
            B_TRANSLATE_MARK("right"),
            B_TRANSLATE_MARK("up"),
            B_TRANSLATE_MARK("down")
        };

        void MyClass::AddChoices(BMenu* menu)
        {
            for (char** ch = choices; *ch != '\0'; ++ch) {
                menu->AddItem(
                    new BMenuItem(
                        B_TRANSLATE(*ch),
                        new BMessage(...)
                    )
                );
            }
        }
*/
#undef B_TRANSLATE_MARK
#define B_TRANSLATE_MARK(string) (string)

#undef B_TRANSLATE_MARK_COMMENT
#define B_TRANSLATE_MARK_COMMENT(string, comment) (string)

#undef B_TRANSLATE_MARK_ALL
#define B_TRANSLATE_MARK_ALL(string, context, comment) (string)

#undef B_TRANSLATE_MARK_ID
#define B_TRANSLATE_MARK_ID(id) (id)

#undef B_TRANSLATE_MARK_SYSTEM_NAME
#define B_TRANSLATE_MARK_SYSTEM_NAME(string) (string)
// the same for void contexts:
#undef B_TRANSLATE_MARK_VOID
#define B_TRANSLATE_MARK_VOID(string)

#undef B_TRANSLATE_MARK_COMMENT_VOID
#define B_TRANSLATE_MARK_COMMENT_VOID(string, comment)

#undef B_TRANSLATE_MARK_ALL_VOID
#define B_TRANSLATE_MARK_ALL_VOID(string, context, comment)

#undef B_TRANSLATE_MARK_ID_VOID
#define B_TRANSLATE_MARK_ID_VOID(id)

#undef B_TRANSLATE_MARK_SYSTEM_NAME_VOID
#define B_TRANSLATE_MARK_SYSTEM_NAME_VOID(string)

// Translation macros which cause collectcatkeys to ignore this key
// (useful in combination with the marking macros above):
#undef B_TRANSLATE_NOCOLLECT
#define B_TRANSLATE_NOCOLLECT(string) \
    B_TRANSLATE(string)

#undef B_TRANSLATE_NOCOLLECT_COMMENT
#define B_TRANSLATE_NOCOLLECT_COMMENT(string, comment) \
    B_TRANSLATE_COMMENT(string, comment)

#undef B_TRANSLATE_NOCOLLECT_ALL
#define B_TRANSLATE_NOCOLLECT_ALL(string, context, comment) \
    B_TRANSLATE_ALL(string, context, comment)

#undef B_TRANSLATE_NOCOLLECT_ID
#define B_TRANSLATE_NOCOLLECT_ID(id) \
    B_TRANSLATE_ID(id)

#undef B_TRANSLATE_NOCOLLECT_SYSTEM_NAME
#define B_TRANSLATE_NOCOLLECT_SYSTEM_NAME(string) \
    B_TRANSLATE_SYSTEM_NAME(string)

#endif	/* B_COLLECTING_CATKEYS */
#endif	/* B_AVOID_TRANSLATION_MACROS */
#endif  /* BHAPI_CATALOG_H */
