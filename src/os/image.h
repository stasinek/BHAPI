/*
 * Copyright 2007-2009, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT license.
 */
//-------------------------------------------------------------------------------------------------
#ifndef BHAPI_IMAGE_H
#define	BHAPI_IMAGE_H
#include <kits/kernel/OS.h>
#include <sys/param.h>
//-------------------------------------------------------------------------------------------------
#ifdef __cplusplus
namespace bhapi {
#endif
//-------------------------------------------------------------------------------------------------
typedef	int32_t image_id;
typedef enum {
    B_APP_IMAGE         = 1,
    B_LIBRARY_IMAGE,
    B_ADD_ON_IMAGE,
    B_SYSTEM_IMAGE
} image_type;
typedef struct {
    image_id	id;
    image_type	type;
    int32_t		sequence;
    int32_t		init_order;
    void		(*init_routine)();
    void		(*term_routine)();
    dev_t		device;
    ino_t		node;
    char		name[MAXPATHLEN];
    void		*text;
    void		*data;
    int32_t		text_size;
    int32_t		data_size;
    // Haiku R1 extensions
    int32_t		api_version;	// the Haiku API version used by the image
    int32_t		abi;			// the Haiku ABI used by the image
} image_info;
#ifdef __cplusplus
}
#endif
//-------------------------------------------------------------------------------------------------
// flags for clear_caches()
#define B_FLUSH_DCACHE				0x0001	/* data cache */
#define B_FLUSH_ICACHE				0x0004	/* instruction cache */
#define B_INVALIDATE_DCACHE			0x0002
#define B_INVALIDATE_ICACHE			0x0008
//-------------------------------------------------------------------------------------------------
// symbol types
#define	B_SYMBOL_TYPE_DATA			0x1
#define	B_SYMBOL_TYPE_TEXT			0x2
#define B_SYMBOL_TYPE_ANY			0x5
//-------------------------------------------------------------------------------------------------
// initialization/termination functions of shared objects
#define	B_INIT_BEFORE_FUNCTION_NAME	"initialize_before"
#define B_INIT_AFTER_FUNCTION_NAME	"initialize_after"
#define	B_TERM_BEFORE_FUNCTION_NAME	"terminate_before"
#define B_TERM_AFTER_FUNCTION_NAME	"terminate_after"
//-------------------------------------------------------------------------------------------------
#define B_APP_IMAGE_SYMBOL		((void*)(addr_t)0)
// value that can be used instead of a pointer to a symbol in the program image.
//-------------------------------------------------------------------------------------------------
#define B_CURRENT_IMAGE_SYMBOL	((void*)&__haiku_init_before)
// pointer to a symbol in the callers image flags for _kern_load_image() (private API)
enum {  B_WAIT_TILL_LOADED	= 0x01,
// Wait till the loader has loaded and relocated (but not yet initialized) the application
// image and all dependencies. If not supplied, the function returns before the loader
// started to do anything at all, i.e. it returns success, even if the executable
// doesn't exist.
};
//-------------------------------------------------------------------------------------------------
#ifdef __cplusplus
namespace bhapi {
#endif
BHAPI_IMPEXP thread_id load_image(int32_t argc, const char **argv, const char **environ);
BHAPI_IMPEXP image_id load_add_on(const char *path);
BHAPI_IMPEXP status_t unload_add_on(image_id image);
BHAPI_IMPEXP status_t get_image_symbol(image_id image, const char *name, int32_t symbolType, void **_symbolLocation);
BHAPI_IMPEXP status_t get_nth_image_symbol(image_id image, int32_t n, char *nameBuffer, int32_t *_nameLength, int32_t *_symbolType, void **_symbolLocation);
BHAPI_IMPEXP void clear_caches(void *address, size_t length, uint32_t flags);
#define get_image_info(image, info) __get_image_info((image), (info), sizeof(*(info)))
#define get_next_image_info(team, cookie, info) __get_next_image_info((team), (cookie), (info), sizeof(*(info)))
/* private, use the macros above */
BHAPI_IMPEXP status_t __get_image_info(bhapi::image_id image, image_info *info, size_t size);
BHAPI_IMPEXP status_t __get_next_image_info(bhapi::team_id team, int32_t *cookie, image_info *info, size_t size);
/* private */
BHAPI_IMPEXP void __haiku_init_before(image_id id);
#ifdef __cplusplus
} // namespace bhapi
#endif
//-------------------------------------------------------------------------------------------------
#endif	/* BHAPI_IMAGE_H */
//-------------------------------------------------------------------------------------------------
