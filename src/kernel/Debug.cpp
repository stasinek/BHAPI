/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
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
 * File: Debug.cpp
 *
 * --------------------------------------------------------------------------*/

#define BHAPI_DEBUG
#define BHAPI_WARNING
#include "Debug.h"
#undef BHAPI_DEBUG
#undef BHAPI_WARNING
#include "Kernel.h"
#include "./../support/StringMe.h"

#include <stdio.h>
#include <stdlib.h>
#include <new>

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

#ifndef HAVE_VA_COPY
	#ifdef HAVE___VA_COPY
		#define va_copy(a, b)		__va_copy(a, b)
	#else
		#if defined(_MSC_VER) || (defined(__BEOS__) && defined(__POWERPC__))
			#define va_copy(a, b)	((a) = (b))
		#endif
	#endif // HAVE___VA_COPY
#endif // HAVE_VA_COPY


#ifdef _WIN32
// defined in etk-os.cpp
extern "C" char* bhapi_win32_convert_utf8_to_active(const char *str, b_int32 length);
#endif


extern "C" {

IMPEXP_BHAPI void bhapi_debug_log(bhapi_debug_level level, const char *format, va_list ap)
{
	char *buffer = NULL;
	char *prefix = NULL;

#ifndef BHAPI_ENABLE_DEBUG
	if(level != DEBUG_NORMAL)
#endif // !BHAPI_ENABLE_DEBUG
	{
		va_list args;
		va_copy(args, ap);
		buffer = b_strdup_vprintf(format, args);
		va_end(args);

		switch(level)
		{
			case DEBUG_WARNING:
				prefix = "Warning: ";
				break;

			case DEBUG_ERROR:
				prefix = "Error: ";
				break;

			default:
				prefix = "";
				break;
		}
	}

	if(level == DEBUG_ERROR)
	{
#ifdef _WIN32
		char *newLine = b_strdup_printf("%s%s\n", prefix, buffer);
		if(newLine)
		{
			if(GetVersion() < 0x80000000) // Windows NT/2000/XP
			{
				b_unichar *uStr = b_utf8_convert_to_unicode(newLine, -1);
				if(uStr != NULL)
				{
					MessageBoxW(NULL, (WCHAR*)uStr, NULL,
						    MB_ICONERROR | MB_SETFOREGROUND);
					free(uStr);
				}
			}
			else
			{
				char *aStr = bhapi_win32_convert_utf8_to_active(newLine, -1);
				if(aStr != NULL)
				{
					MessageBoxA(NULL, aStr, NULL, MB_ICONERROR | MB_SETFOREGROUND);
					free(aStr);
				}
			}
			free(newLine);
		}
#else
		fprintf(stderr, "\x1b[31m%s%s\x1b[0m\n", prefix, buffer);
#endif // _WIN32
		if(buffer)
		{
			free(buffer);
			buffer = NULL;
		}
		abort();
	}
#ifdef BHAPI_ENABLE_DEBUG
	else
#else // !BHAPI_ENABLE_DEBUG
	else if(level != DEBUG_NORMAL)
#endif // BHAPI_ENABLE_DEBUG
	{
#ifndef _WIN32
		fprintf(stdout, "%s%s%s%s%s",
			level == DEBUG_WARNING ? "\x1b[32m" : "",
			prefix, buffer,
			level == DEBUG_WARNING ? "\x1b[0m" : "",
			level == DEBUG_OUTPUT ? "" : "\n");
#else
		AllocConsole();
		HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		while(hStdOut != NULL)
		{
			char *newLine = b_strdup_printf("%s%s%s", prefix, buffer, level == DEBUG_OUTPUT ? "" : "\n");
			if(newLine == NULL) break;

			if(level == DEBUG_WARNING) SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN);
			if(GetVersion() < 0x80000000) // Windows NT/2000/XP
			{
				b_unichar *uStr = b_utf8_convert_to_unicode(newLine, -1);
				if(uStr != NULL)
				{
					DWORD wrote = 0;
                    DWORD len = (DWORD)b_unicode_strlen(uStr);
					const b_unichar *buf = uStr;
					while(true)
					{
						if(WriteConsoleW(hStdOut, (const void*)buf, len, &wrote, NULL) == 0) break;
						if(wrote >= len) break;
						len -= wrote;
						for(DWORD i = 0; i < wrote; i++) buf = b_unicode_next(buf, NULL);
					}
					free(uStr);
				}
			}
			else // Windows 95/98
			{
				char *aStr = bhapi_win32_convert_utf8_to_active(newLine, -1);
				if(aStr != NULL)
				{
					DWORD wrote = 0;
					DWORD len = strlen(aStr);
					const char *buf = aStr;
					while(true)
					{
						if(WriteConsoleA(hStdOut, (const void*)buf, len, &wrote, NULL) == 0) break;
						if(wrote >= len) break;
						len -= wrote;
						buf += wrote;
					}
					free(aStr);
				}
			}
			if(level == DEBUG_WARNING) SetConsoleTextAttribute(hStdOut, FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED);

			free(newLine);
			break;
		}
#endif
	}

	if(buffer) free(buffer);
}


IMPEXP_BHAPI void BHAPI_DEBUG(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	bhapi_debug_log(DEBUG_NORMAL, format, args);
	va_end(args);
}


IMPEXP_BHAPI void BHAPI_OUTPUT(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	bhapi_debug_log(DEBUG_OUTPUT, format, args);
	va_end(args);
}


IMPEXP_BHAPI void BHAPI_WARNING(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	bhapi_debug_log(DEBUG_WARNING, format, args);
	va_end(args);
}


IMPEXP_BHAPI void BHAPI_ERROR(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	bhapi_debug_log(DEBUG_ERROR, format, args);
	va_end(args);
}

} // extern "C"

#ifdef BHAPI_BUILD_WITH_MEMORY_TRACING

#define BHAPI_MEMORY_MANAGER_FILENAME_LENGTH 256
#define BHAPI_MEMORY_MANAGER_METHOD_LENGTH 25
struct __bhapi_mem_list_t {
	size_t size;
	char file[BHAPI_MEMORY_MANAGER_FILENAME_LENGTH];
	int line;
	char method[BHAPI_MEMORY_MANAGER_METHOD_LENGTH];
	struct __bhapi_mem_list_t *prev;
	struct __bhapi_mem_list_t *next;
};

static struct __bhapi_mem_list_t *__bhapi_memory_list = NULL;
static b_uint64 __bhapi_max_allocated_memory = B_INT64_CONSTANT(0);
static b_uint64 __bhapi_cur_allocated_memory = B_INT64_CONSTANT(0);

#undef new
#undef calloc
#undef malloc
#undef realloc
#undef free

// TODO: replace list with hash

extern "C" {

IMPEXP_BHAPI void* bhapi_malloc(size_t size, const char *file, int line, const char *method)
{
	struct __bhapi_mem_list_t *entry = (struct __bhapi_mem_list_t *)malloc(sizeof(struct __bhapi_mem_list_t) + size);
	if(entry == NULL)
	{
		fprintf(stdout, "\x1b[31m[KERNEL]: out of memory when allocate %u bytes.\x1b[0m\n", size);
		return NULL;
	}

	void *ptr = (char*)entry + sizeof(struct __bhapi_mem_list_t);

	entry->size = size;
	bzero(entry->file, BHAPI_MEMORY_MANAGER_FILENAME_LENGTH);
	bzero(entry->method, BHAPI_MEMORY_MANAGER_METHOD_LENGTH);
	strncpy(entry->file, (file == NULL ? "<Unknown>" : file), BHAPI_MEMORY_MANAGER_FILENAME_LENGTH - 1);
	strncpy(entry->method, (method == NULL ? "<Unknown>" : method), BHAPI_MEMORY_MANAGER_METHOD_LENGTH - 1);
	entry->line = line;
	entry->prev = NULL;

	if(bhapi_memory_tracing_lock() == false)
	{
		fprintf(stdout, "\x1b[32m[KERNEL]: %s - system don't support global_static_locker!\x1b[0m\n", __PRETTY_FUNCTION__);
		free(entry);
		return NULL;
	}

	entry->next = __bhapi_memory_list;
	if(__bhapi_memory_list != NULL) __bhapi_memory_list->prev = entry;
	__bhapi_memory_list = entry;

	__bhapi_cur_allocated_memory += (b_uint64)size;
	if(__bhapi_cur_allocated_memory > __bhapi_max_allocated_memory) __bhapi_max_allocated_memory = __bhapi_cur_allocated_memory;

	bhapi_memory_tracing_unlock();

//	fprintf(stdout, "[KERNEL]: %s - %u bytes to %p on line (%s:%d)\n",
//		(method == NULL ? "<Unknown>" : method), size, ptr, (file == NULL ? "<Unknown>" : file), line);

	return ptr;
}


IMPEXP_BHAPI void* bhapi_calloc(size_t nmemb, size_t size, const char *file, int line, const char *method)
{
	struct __bhapi_mem_list_t *entry = (struct __bhapi_mem_list_t *)malloc(sizeof(struct __bhapi_mem_list_t) + (nmemb * size));
	if(entry == NULL)
	{
		fprintf(stdout, "\x1b[31m[KERNEL]: out of memory when allocate %u per %u bytes.\x1b[0m\n", nmemb, size);
		return NULL;
	}

	void *ptr = (char*)entry + sizeof(struct __bhapi_mem_list_t);
	bzero(ptr, nmemb * size);

	entry->size = (nmemb * size);
	bzero(entry->file, BHAPI_MEMORY_MANAGER_FILENAME_LENGTH);
	bzero(entry->method, BHAPI_MEMORY_MANAGER_METHOD_LENGTH);
	strncpy(entry->file, (file == NULL ? "<Unknown>" : file), BHAPI_MEMORY_MANAGER_FILENAME_LENGTH - 1);
	strncpy(entry->method, (method == NULL ? "<Unknown>" : method), BHAPI_MEMORY_MANAGER_METHOD_LENGTH - 1);
	entry->line = line;
	entry->prev = NULL;

	if(bhapi_memory_tracing_lock() == false)
	{
		fprintf(stdout, "\x1b[32m[KERNEL]: %s - system don't support memory_tracing_locker!\x1b[0m\n", __PRETTY_FUNCTION__);
		free(entry);
		return NULL;
	}

	entry->next = __bhapi_memory_list;
	if(__bhapi_memory_list != NULL) __bhapi_memory_list->prev = entry;
	__bhapi_memory_list = entry;

	__bhapi_cur_allocated_memory += (b_uint64)(nmemb * size);
	if(__bhapi_cur_allocated_memory > __bhapi_max_allocated_memory) __bhapi_max_allocated_memory = __bhapi_cur_allocated_memory;

	bhapi_memory_tracing_unlock();

//	fprintf(stdout, "[KERNEL]: %s - %u per %u bytes to %p on line (%s:%d)\n",
//		(method == NULL ? "<Unknown>" : method), nmemb, size, ptr, (file == NULL ? "<Unknown>" : file), line);

	return ptr;
}


IMPEXP_BHAPI void* bhapi_realloc(void *ptr, size_t size, const char *file, int line, const char *method)
{
	if(ptr == NULL) return bhapi_malloc(size, file, line, method);
	if(size == 0) {bhapi_free(ptr, file, line, method); return NULL;}

	struct __bhapi_mem_list_t *entryFound = NULL;

	if(bhapi_memory_tracing_lock() == false)
	{
		fprintf(stdout, "\x1b[32m[KERNEL]: %s - system don't support memory_tracing_locker!\x1b[0m\n", __PRETTY_FUNCTION__);
		return NULL;
	}

	struct __bhapi_mem_list_t *entry;
	for(entry = __bhapi_memory_list; entry != NULL; entry = entry->next)
	{
		if((char*)entry + sizeof(struct __bhapi_mem_list_t) == ptr)
		{
			if(entry->prev != NULL) entry->prev->next = entry->next;
			if(entry->next != NULL) entry->next->prev = entry->prev;
			if(entry->prev == NULL) __bhapi_memory_list = entry->next;
			entryFound = entry;
			break;
		}
	}

	bhapi_memory_tracing_unlock();

	if(entryFound == NULL)
	{
		fprintf(stdout, "\x1b[32m[KERNEL]: realloc(method %s, %s:%d) invalid pointer %p.\x1b[0m\n",
			(method == NULL ? "<Unknown>" : method), file, line, ptr);
		return NULL;
	}

	entryFound->prev = NULL;
	entryFound->next = NULL;

	size_t oldSize = entryFound->size;
	entry = (struct __bhapi_mem_list_t *)realloc(entryFound, sizeof(struct __bhapi_mem_list_t) + size);
	void *retPtr = NULL;

	if(entry != NULL)
	{
		entry->size = size;
		bzero(entry->file, BHAPI_MEMORY_MANAGER_FILENAME_LENGTH);
		bzero(entry->method, BHAPI_MEMORY_MANAGER_METHOD_LENGTH);
		strncpy(entry->file, (file == NULL ? "<Unknown>" : file), BHAPI_MEMORY_MANAGER_FILENAME_LENGTH - 1);
		strncpy(entry->method, (method == NULL ? "<Unknown>" : method), BHAPI_MEMORY_MANAGER_METHOD_LENGTH - 1);
		entry->line = line;

//		fprintf(stdout, "[KERNEL]: realloc(method %s, %s:%d) object at %p (method %s, size %u, %s:%d)\n",
//			(method == NULL ? "<Unknown>" : method), file, line,
//			(char*)entry + sizeof(struct __bhapi_mem_list_t),
//			entry->method, entry->size, entry->file, entry->line);

		retPtr = (char*)entry + sizeof(struct __bhapi_mem_list_t);
	}
	else
	{
		entry = entryFound;
	}

	bhapi_memory_tracing_lock();

	entry->next = __bhapi_memory_list;
	if(__bhapi_memory_list != NULL) __bhapi_memory_list->prev = entry;
	__bhapi_memory_list = entry;

	if(retPtr != NULL)
	{
		__bhapi_cur_allocated_memory -= (b_uint64)oldSize;
		__bhapi_cur_allocated_memory += (b_uint64)size;
		if(__bhapi_cur_allocated_memory > __bhapi_max_allocated_memory) __bhapi_max_allocated_memory = __bhapi_cur_allocated_memory;
	}

	bhapi_memory_tracing_unlock();

	if(retPtr == NULL)
		fprintf(stdout, "\x1b[32m[KERNEL]: realloc(method %s, %s:%d) pointer %p failed.\x1b[0m\n",
			(method == NULL ? "<Unknown>" : method), file, line, ptr);

	return retPtr;
}


IMPEXP_BHAPI void bhapi_free(void *ptr, const char *file, int line, const char *method)
{
	if(ptr == NULL) return;

	struct __bhapi_mem_list_t *entryFound = NULL;

	if(bhapi_memory_tracing_lock() == false)
	{
		fprintf(stdout, "\x1b[32m[KERNEL]: %s - system don't support memory_tracing_locker!\x1b[0m\n", __PRETTY_FUNCTION__);
		return;
	}

	struct __bhapi_mem_list_t *entry;
	for(entry = __bhapi_memory_list; entry != NULL; entry = entry->next)
	{
		if((char*)entry + sizeof(struct __bhapi_mem_list_t) == ptr)
		{
			if(entry->prev != NULL) entry->prev->next = entry->next;
			if(entry->next != NULL) entry->next->prev = entry->prev;
			if(entry->prev == NULL) __bhapi_memory_list = entry->next;
			entryFound = entry;
			break;
		}
	}

	if(entryFound != NULL) __bhapi_cur_allocated_memory -= (b_uint64)(entryFound->size);

	bhapi_memory_tracing_unlock();

	if(entryFound != NULL)
	{
//		fprintf(stdout, "[KERNEL]: free(method %s, %s:%d) object at %p (method %s, size %u, %s:%d)\n",
//			(method == NULL ? "<Unknown>" : method), file, line,
//			(char*)entryFound + sizeof(struct __bhapi_mem_list_t),
//			entryFound->method, entryFound->size, entryFound->file, entryFound->line);
		free(entryFound);
	}
	else
	{
		fprintf(stdout, "\x1b[31m[KERNEL]: free(method %s, %s:%d) invalid pointer %p.\x1b[0m\n",
			(method == NULL ? "<Unknown>" : method), file, line, ptr);
		abort();
	}
}

} // extern "C"


IMPEXP_BHAPI void* operator new(size_t size, const char *file, int line, const char *method, struct bhapi_memory_flag_t *flag)
{
	if(file == NULL) file = "<Unknown>";
	if(method == NULL) method = "new";
	return bhapi_malloc(size, file, line, method);
}


IMPEXP_BHAPI void* operator new[](size_t size, const char *file, int line, const char *method, struct bhapi_memory_flag_t *flag)
{
	if(method == NULL) method = "new[]";
	return operator new(size, file, line, method, flag);
}


IMPEXP_BHAPI void operator delete(void *ptr, const char *file, int line, const char *method, struct bhapi_memory_flag_t *flag)
{
	if(file == NULL) file = "<Unknown>";
	if(method == NULL) method = "delete";
	bhapi_free(ptr, file, line, method);
}


IMPEXP_BHAPI void operator delete[](void *ptr, const char *file, int line, const char *method, struct bhapi_memory_flag_t *flag)
{
	if(method == NULL) method = "delete[]";
	operator delete(ptr, file, line, method, flag);
}


IMPEXP_BHAPI void* operator new(size_t size)
{
	return operator new(size, NULL, 0, "new", (struct bhapi_memory_flag_t*)0);
}


IMPEXP_BHAPI void* operator new[](size_t size)
{
	return operator new[](size, NULL, 0, "new[]", (struct bhapi_memory_flag_t*)0);
}


IMPEXP_BHAPI void* operator new(size_t size, const std::nothrow_t&) throw()
{
	return operator new(size, NULL, 0, "new(nothrow_t)", (struct bhapi_memory_flag_t*)0);
}


IMPEXP_BHAPI void* operator new[](size_t size, const std::nothrow_t&) throw()
{
	return operator new[](size, NULL, 0, "new[](nothrow_t)", (struct bhapi_memory_flag_t*)0);
}


IMPEXP_BHAPI void operator delete(void *ptr)
{
	operator delete(ptr, NULL, 0, "delete", (struct bhapi_memory_flag_t*)0);
}


IMPEXP_BHAPI void operator delete[](void *ptr)
{
	operator delete[](ptr, NULL, 0, "delete[]", (struct bhapi_memory_flag_t*)0);
}


IMPEXP_BHAPI void operator delete(void* ptr, const std::nothrow_t&)
{
	operator delete(ptr, NULL, 0, "delete(nothrow_t)", (struct bhapi_memory_flag_t*)0);
}


IMPEXP_BHAPI void operator delete[](void* ptr, const std::nothrow_t&)
{
	operator delete[](ptr, NULL, 0, "delete[](nothrow_t)", (struct bhapi_memory_flag_t*)0);
}


inline void bhapi_memory_check_leak()
{
	if(bhapi_memory_tracing_lock() == false) return;

#ifdef _WIN32
	AllocConsole();
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

	if(__bhapi_memory_list != NULL)
	{
		struct __bhapi_mem_list_t *entry;
#ifdef _WIN32
		SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN);
#endif
		for(entry = __bhapi_memory_list; entry != NULL; entry = entry->next)
		{
#ifndef _WIN32
			fprintf(stdout, "\x1b[31m[KERNEL]: leaked object at %p (method %s, size %u, %s:%d)\x1b[0m\n",
				(char*)entry + sizeof(struct __bhapi_mem_list_t),
				entry->method, entry->size, entry->file, entry->line);
#else
			fprintf(stdout, "[KERNEL]: leaked object at %p (method %s, size %u, %s:%d)\n",
				(char*)entry + sizeof(struct __bhapi_mem_list_t),
				entry->method, entry->size, entry->file, entry->line);
#endif
		}
#ifdef _WIN32
		SetConsoleTextAttribute(hStdOut, FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED);
#endif
	}

	if(__bhapi_cur_allocated_memory != B_INT64_CONSTANT(0))
#ifndef _WIN32 // this just to be debug, so i don't care sth.
		fprintf(stdout, "\x1b[31m[KERNEL]: something error, current allocated: %llu\x1b[0m\n", __bhapi_cur_allocated_memory);
#else
	{
		SetConsoleTextAttribute(hStdOut, FOREGROUND_RED);
		fprintf(stdout, "[KERNEL]: something error, current allocated: %I64u\n", __bhapi_cur_allocated_memory);
		SetConsoleTextAttribute(hStdOut, FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED);
	}
#endif

#ifndef _WIN32 // this just to be debug, so i don't care sth.
	fprintf(stdout, "\x1b[32m[KERNEL]: max allocated: %llu\x1b[0m\n", __bhapi_max_allocated_memory);
#else
	SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN);
	fprintf(stdout, "[KERNEL]: max allocated: %I64u\n", __bhapi_max_allocated_memory);
	SetConsoleTextAttribute(hStdOut, FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED);
#endif

	bhapi_memory_tracing_unlock();
}


#if !defined(HAVE_ON_EXIT) && !defined(HAVE_ATEXIT)
class __bhapi_memory_check_leak {
public:
	inline __bhapi_memory_check_leak() {};
	inline ~__bhapi_memory_check_leak() {bhapi_memory_check_leak();};
};
static __bhapi_memory_check_leak ___bhapi_memory_check_leak;
#else
class __bhapi_memory_check_leak {
public:
	inline __bhapi_memory_check_leak()
	{
#ifdef HAVE_ON_EXIT
		on_exit((void (*)(int, void*))bhapi_memory_check_leak, NULL);
#else
		atexit((void (*)(void))bhapi_memory_check_leak);
#endif
	};
};
static __bhapi_memory_check_leak ___bhapi_memory_check_leak;
#endif


#endif /* BHAPI_BUILD_WITH_MEMORY_TRACING */

