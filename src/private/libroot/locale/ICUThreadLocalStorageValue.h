/*
 * Copyright 2011, Oliver Tappe, zooey@hirschkaefer.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ICU_THREAD_LOCAL_STORAGE_VALUE_H
#define _ICU_THREAD_LOCAL_STORAGE_VALUE_H


#include <pthread.h>

#include <unicode/ucnv.h>

#include <SupportDefs.h>


namespace BPrivate {
namespace Libroot {


struct ICUThreadLocalStorageValue {
			UConverter*			converter;
			char				charset[64];

								ICUThreadLocalStorageValue();
								~ICUThreadLocalStorageValue();

	static	status_t			GetInstanceForKey(pthread_key_t tlsKey,
									ICUThreadLocalStorageValue*& instanceOut);
};


}	// namespace Libroot
}	// namespace BPrivate


#endif	// _ICU_THREAD_LOCAL_STORAGE_VALUE_H
