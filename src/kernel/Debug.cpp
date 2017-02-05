/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
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
 * File: Debug.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <new>

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

#include "./../support/SupportDefs.h"

#define ETK_DEBUG
#define ETK_WARNING
#include "Debug.h"
#undef ETK_DEBUG
#undef ETK_WARNING

#include "Kernel.h"

#include "./../support/String.h"

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
extern "C" char* etk_win32_convert_utf8_to_active(const char *str, eint32 length);
#endif


extern "C" {

_IMPEXP_ETK void etk_debug_log(etk_debug_level level, const char *format, va_list ap)
{
	char *buffer = NULL;
	char *prefix = NULL;

#ifndef ETK_ENABLE_DEBUG
	if(level != DEBUG_NORMAL)
#endif // !ETK_ENABLE_DEBUG
	{
		va_list args;
		va_copy(args, ap);
		buffer = e_strdup_vprintf(format, args);
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
		char *newLine = e_strdup_printf("%s%s\n", prefix, buffer);
		if(newLine)
		{
			if(GetVersion() < 0x80000000) // Windows NT/2000/XP
			{
				eunichar *uStr = e_utf8_convert_to_unicode(newLine, -1);
				if(uStr != NULL)
				{
					MessageBoxW(NULL, (WCHAR*)uStr, NULL,
						    MB_ICONERROR | MB_SETFOREGROUND);
					free(uStr);
				}
			}
			else
			{
				char *aStr = etk_win32_convert_utf8_to_active(newLine, -1);
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
#ifdef ETK_ENABLE_DEBUG
	else
#else // !ETK_ENABLE_DEBUG
	else if(level != DEBUG_NORMAL)
#endif // ETK_ENABLE_DEBUG
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
			char *newLine = e_strdup_printf("%s%s%s", prefix, buffer, level == DEBUG_OUTPUT ? "" : "\n");
			if(newLine == NULL) break;

			if(level == DEBUG_WARNING) SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN);
			if(GetVersion() < 0x80000000) // Windows NT/2000/XP
			{
				eunichar *uStr = e_utf8_convert_to_unicode(newLine, -1);
				if(uStr != NULL)
				{
					DWORD wrote = 0;
					DWORD len = (DWORD)e_unicode_strlen(uStr);
					const eunichar *buf = uStr;
					while(true)
					{
						if(WriteConsoleW(hStdOut, (const void*)buf, len, &wrote, NULL) == 0) break;
						if(wrote >= len) break;
						len -= wrote;
						for(DWORD i = 0; i < wrote; i++) buf = e_unicode_next(buf, NULL);
					}
					free(uStr);
				}
			}
			else // Windows 95/98
			{
				char *aStr = etk_win32_convert_utf8_to_active(newLine, -1);
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


_IMPEXP_ETK void ETK_DEBUG(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	etk_debug_log(DEBUG_NORMAL, format, args);
	va_end(args);
}


_IMPEXP_ETK void ETK_OUTPUT(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	etk_debug_log(DEBUG_OUTPUT, format, args);
	va_end(args);
}


_IMPEXP_ETK void ETK_WARNING(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	etk_debug_log(DEBUG_WARNING, format, args);
	va_end(args);
}


_IMPEXP_ETK void ETK_ERROR(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	etk_debug_log(DEBUG_ERROR, format, args);
	va_end(args);
}

} // extern "C"

#ifdef ETK_BUILD_WITH_MEMORY_TRACING

#define ETK_MEMORY_MANAGER_FILENAME_LENGTH 256
#define ETK_MEMORY_MANAGER_METHOD_LENGTH 25
struct __etk_mem_list_t {
	size_t size;
	char file[ETK_MEMORY_MANAGER_FILENAME_LENGTH];
	int line;
	char method[ETK_MEMORY_MANAGER_METHOD_LENGTH];
	struct __etk_mem_list_t *prev;
	struct __etk_mem_list_t *next;
};

static struct __etk_mem_list_t *__etk_memory_list = NULL;
static euint64 __etk_max_allocated_memory = E_INT64_CONSTANT(0);
static euint64 __etk_cur_allocated_memory = E_INT64_CONSTANT(0);

#undef new
#undef calloc
#undef malloc
#undef realloc
#undef free

// TODO: replace list with hash

extern "C" {

_IMPEXP_ETK void* etk_malloc(size_t size, const char *file, int line, const char *method)
{
	struct __etk_mem_list_t *entry = (struct __etk_mem_list_t *)malloc(sizeof(struct __etk_mem_list_t) + size);
	if(entry == NULL)
	{
		fprintf(stdout, "\x1b[31m[KERNEL]: out of memory when allocate %u bytes.\x1b[0m\n", size);
		return NULL;
	}

	void *ptr = (char*)entry + sizeof(struct __etk_mem_list_t);

	entry->size = size;
	bzero(entry->file, ETK_MEMORY_MANAGER_FILENAME_LENGTH);
	bzero(entry->method, ETK_MEMORY_MANAGER_METHOD_LENGTH);
	strncpy(entry->file, (file == NULL ? "<Unknown>" : file), ETK_MEMORY_MANAGER_FILENAME_LENGTH - 1);
	strncpy(entry->method, (method == NULL ? "<Unknown>" : method), ETK_MEMORY_MANAGER_METHOD_LENGTH - 1);
	entry->line = line;
	entry->prev = NULL;

	if(etk_memory_tracing_lock() == false)
	{
		fprintf(stdout, "\x1b[32m[KERNEL]: %s - system don't support global_static_locker!\x1b[0m\n", __PRETTY_FUNCTION__);
		free(entry);
		return NULL;
	}

	entry->next = __etk_memory_list;
	if(__etk_memory_list != NULL) __etk_memory_list->prev = entry;
	__etk_memory_list = entry;

	__etk_cur_allocated_memory += (euint64)size;
	if(__etk_cur_allocated_memory > __etk_max_allocated_memory) __etk_max_allocated_memory = __etk_cur_allocated_memory;

	etk_memory_tracing_unlock();

//	fprintf(stdout, "[KERNEL]: %s - %u bytes to %p on line (%s:%d)\n",
//		(method == NULL ? "<Unknown>" : method), size, ptr, (file == NULL ? "<Unknown>" : file), line);

	return ptr;
}


_IMPEXP_ETK void* etk_calloc(size_t nmemb, size_t size, const char *file, int line, const char *method)
{
	struct __etk_mem_list_t *entry = (struct __etk_mem_list_t *)malloc(sizeof(struct __etk_mem_list_t) + (nmemb * size));
	if(entry == NULL)
	{
		fprintf(stdout, "\x1b[31m[KERNEL]: out of memory when allocate %u per %u bytes.\x1b[0m\n", nmemb, size);
		return NULL;
	}

	void *ptr = (char*)entry + sizeof(struct __etk_mem_list_t);
	bzero(ptr, nmemb * size);

	entry->size = (nmemb * size);
	bzero(entry->file, ETK_MEMORY_MANAGER_FILENAME_LENGTH);
	bzero(entry->method, ETK_MEMORY_MANAGER_METHOD_LENGTH);
	strncpy(entry->file, (file == NULL ? "<Unknown>" : file), ETK_MEMORY_MANAGER_FILENAME_LENGTH - 1);
	strncpy(entry->method, (method == NULL ? "<Unknown>" : method), ETK_MEMORY_MANAGER_METHOD_LENGTH - 1);
	entry->line = line;
	entry->prev = NULL;

	if(etk_memory_tracing_lock() == false)
	{
		fprintf(stdout, "\x1b[32m[KERNEL]: %s - system don't support memory_tracing_locker!\x1b[0m\n", __PRETTY_FUNCTION__);
		free(entry);
		return NULL;
	}

	entry->next = __etk_memory_list;
	if(__etk_memory_list != NULL) __etk_memory_list->prev = entry;
	__etk_memory_list = entry;

	__etk_cur_allocated_memory += (euint64)(nmemb * size);
	if(__etk_cur_allocated_memory > __etk_max_allocated_memory) __etk_max_allocated_memory = __etk_cur_allocated_memory;

	etk_memory_tracing_unlock();

//	fprintf(stdout, "[KERNEL]: %s - %u per %u bytes to %p on line (%s:%d)\n",
//		(method == NULL ? "<Unknown>" : method), nmemb, size, ptr, (file == NULL ? "<Unknown>" : file), line);

	return ptr;
}


_IMPEXP_ETK void* etk_realloc(void *ptr, size_t size, const char *file, int line, const char *method)
{
	if(ptr == NULL) return etk_malloc(size, file, line, method);
	if(size == 0) {etk_free(ptr, file, line, method); return NULL;}

	struct __etk_mem_list_t *entryFound = NULL;

	if(etk_memory_tracing_lock() == false)
	{
		fprintf(stdout, "\x1b[32m[KERNEL]: %s - system don't support memory_tracing_locker!\x1b[0m\n", __PRETTY_FUNCTION__);
		return NULL;
	}

	struct __etk_mem_list_t *entry;
	for(entry = __etk_memory_list; entry != NULL; entry = entry->next)
	{
		if((char*)entry + sizeof(struct __etk_mem_list_t) == ptr)
		{
			if(entry->prev != NULL) entry->prev->next = entry->next;
			if(entry->next != NULL) entry->next->prev = entry->prev;
			if(entry->prev == NULL) __etk_memory_list = entry->next;
			entryFound = entry;
			break;
		}
	}

	etk_memory_tracing_unlock();

	if(entryFound == NULL)
	{
		fprintf(stdout, "\x1b[32m[KERNEL]: realloc(method %s, %s:%d) invalid pointer %p.\x1b[0m\n",
			(method == NULL ? "<Unknown>" : method), file, line, ptr);
		return NULL;
	}

	entryFound->prev = NULL;
	entryFound->next = NULL;

	size_t oldSize = entryFound->size;
	entry = (struct __etk_mem_list_t *)realloc(entryFound, sizeof(struct __etk_mem_list_t) + size);
	void *retPtr = NULL;

	if(entry != NULL)
	{
		entry->size = size;
		bzero(entry->file, ETK_MEMORY_MANAGER_FILENAME_LENGTH);
		bzero(entry->method, ETK_MEMORY_MANAGER_METHOD_LENGTH);
		strncpy(entry->file, (file == NULL ? "<Unknown>" : file), ETK_MEMORY_MANAGER_FILENAME_LENGTH - 1);
		strncpy(entry->method, (method == NULL ? "<Unknown>" : method), ETK_MEMORY_MANAGER_METHOD_LENGTH - 1);
		entry->line = line;

//		fprintf(stdout, "[KERNEL]: realloc(method %s, %s:%d) object at %p (method %s, size %u, %s:%d)\n",
//			(method == NULL ? "<Unknown>" : method), file, line,
//			(char*)entry + sizeof(struct __etk_mem_list_t),
//			entry->method, entry->size, entry->file, entry->line);

		retPtr = (char*)entry + sizeof(struct __etk_mem_list_t);
	}
	else
	{
		entry = entryFound;
	}

	etk_memory_tracing_lock();

	entry->next = __etk_memory_list;
	if(__etk_memory_list != NULL) __etk_memory_list->prev = entry;
	__etk_memory_list = entry;

	if(retPtr != NULL)
	{
		__etk_cur_allocated_memory -= (euint64)oldSize;
		__etk_cur_allocated_memory += (euint64)size;
		if(__etk_cur_allocated_memory > __etk_max_allocated_memory) __etk_max_allocated_memory = __etk_cur_allocated_memory;
	}

	etk_memory_tracing_unlock();

	if(retPtr == NULL)
		fprintf(stdout, "\x1b[32m[KERNEL]: realloc(method %s, %s:%d) pointer %p failed.\x1b[0m\n",
			(method == NULL ? "<Unknown>" : method), file, line, ptr);

	return retPtr;
}


_IMPEXP_ETK void etk_free(void *ptr, const char *file, int line, const char *method)
{
	if(ptr == NULL) return;

	struct __etk_mem_list_t *entryFound = NULL;

	if(etk_memory_tracing_lock() == false)
	{
		fprintf(stdout, "\x1b[32m[KERNEL]: %s - system don't support memory_tracing_locker!\x1b[0m\n", __PRETTY_FUNCTION__);
		return;
	}

	struct __etk_mem_list_t *entry;
	for(entry = __etk_memory_list; entry != NULL; entry = entry->next)
	{
		if((char*)entry + sizeof(struct __etk_mem_list_t) == ptr)
		{
			if(entry->prev != NULL) entry->prev->next = entry->next;
			if(entry->next != NULL) entry->next->prev = entry->prev;
			if(entry->prev == NULL) __etk_memory_list = entry->next;
			entryFound = entry;
			break;
		}
	}

	if(entryFound != NULL) __etk_cur_allocated_memory -= (euint64)(entryFound->size);

	etk_memory_tracing_unlock();

	if(entryFound != NULL)
	{
//		fprintf(stdout, "[KERNEL]: free(method %s, %s:%d) object at %p (method %s, size %u, %s:%d)\n",
//			(method == NULL ? "<Unknown>" : method), file, line,
//			(char*)entryFound + sizeof(struct __etk_mem_list_t),
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


_IMPEXP_ETK void* operator new(size_t size, const char *file, int line, const char *method, struct etk_memory_flag_t *flag)
{
	if(file == NULL) file = "<Unknown>";
	if(method == NULL) method = "new";
	return etk_malloc(size, file, line, method);
}


_IMPEXP_ETK void* operator new[](size_t size, const char *file, int line, const char *method, struct etk_memory_flag_t *flag)
{
	if(method == NULL) method = "new[]";
	return operator new(size, file, line, method, flag);
}


_IMPEXP_ETK void operator delete(void *ptr, const char *file, int line, const char *method, struct etk_memory_flag_t *flag)
{
	if(file == NULL) file = "<Unknown>";
	if(method == NULL) method = "delete";
	etk_free(ptr, file, line, method);
}


_IMPEXP_ETK void operator delete[](void *ptr, const char *file, int line, const char *method, struct etk_memory_flag_t *flag)
{
	if(method == NULL) method = "delete[]";
	operator delete(ptr, file, line, method, flag);
}


_IMPEXP_ETK void* operator new(size_t size)
{
	return operator new(size, NULL, 0, "new", (struct etk_memory_flag_t*)0);
}


_IMPEXP_ETK void* operator new[](size_t size)
{
	return operator new[](size, NULL, 0, "new[]", (struct etk_memory_flag_t*)0);
}


_IMPEXP_ETK void* operator new(size_t size, const std::nothrow_t&) throw()
{
	return operator new(size, NULL, 0, "new(nothrow_t)", (struct etk_memory_flag_t*)0);
}


_IMPEXP_ETK void* operator new[](size_t size, const std::nothrow_t&) throw()
{
	return operator new[](size, NULL, 0, "new[](nothrow_t)", (struct etk_memory_flag_t*)0);
}


_IMPEXP_ETK void operator delete(void *ptr)
{
	operator delete(ptr, NULL, 0, "delete", (struct etk_memory_flag_t*)0);
}


_IMPEXP_ETK void operator delete[](void *ptr)
{
	operator delete[](ptr, NULL, 0, "delete[]", (struct etk_memory_flag_t*)0);
}


_IMPEXP_ETK void operator delete(void* ptr, const std::nothrow_t&)
{
	operator delete(ptr, NULL, 0, "delete(nothrow_t)", (struct etk_memory_flag_t*)0);
}


_IMPEXP_ETK void operator delete[](void* ptr, const std::nothrow_t&)
{
	operator delete[](ptr, NULL, 0, "delete[](nothrow_t)", (struct etk_memory_flag_t*)0);
}


inline void etk_memory_check_leak()
{
	if(etk_memory_tracing_lock() == false) return;

#ifdef _WIN32
	AllocConsole();
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

	if(__etk_memory_list != NULL)
	{
		struct __etk_mem_list_t *entry;
#ifdef _WIN32
		SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN);
#endif
		for(entry = __etk_memory_list; entry != NULL; entry = entry->next)
		{
#ifndef _WIN32
			fprintf(stdout, "\x1b[31m[KERNEL]: leaked object at %p (method %s, size %u, %s:%d)\x1b[0m\n",
				(char*)entry + sizeof(struct __etk_mem_list_t),
				entry->method, entry->size, entry->file, entry->line);
#else
			fprintf(stdout, "[KERNEL]: leaked object at %p (method %s, size %u, %s:%d)\n",
				(char*)entry + sizeof(struct __etk_mem_list_t),
				entry->method, entry->size, entry->file, entry->line);
#endif
		}
#ifdef _WIN32
		SetConsoleTextAttribute(hStdOut, FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED);
#endif
	}

	if(__etk_cur_allocated_memory != E_INT64_CONSTANT(0))
#ifndef _WIN32 // this just to be debug, so i don't care sth.
		fprintf(stdout, "\x1b[31m[KERNEL]: something error, current allocated: %llu\x1b[0m\n", __etk_cur_allocated_memory);
#else
	{
		SetConsoleTextAttribute(hStdOut, FOREGROUND_RED);
		fprintf(stdout, "[KERNEL]: something error, current allocated: %I64u\n", __etk_cur_allocated_memory);
		SetConsoleTextAttribute(hStdOut, FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED);
	}
#endif

#ifndef _WIN32 // this just to be debug, so i don't care sth.
	fprintf(stdout, "\x1b[32m[KERNEL]: max allocated: %llu\x1b[0m\n", __etk_max_allocated_memory);
#else
	SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN);
	fprintf(stdout, "[KERNEL]: max allocated: %I64u\n", __etk_max_allocated_memory);
	SetConsoleTextAttribute(hStdOut, FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED);
#endif

	etk_memory_tracing_unlock();
}


#if !defined(HAVE_ON_EXIT) && !defined(HAVE_ATEXIT)
class __etk_memory_check_leak {
public:
	inline __etk_memory_check_leak() {};
	inline ~__etk_memory_check_leak() {etk_memory_check_leak();};
};
static __etk_memory_check_leak ___etk_memory_check_leak;
#else
class __etk_memory_check_leak {
public:
	inline __etk_memory_check_leak()
	{
#ifdef HAVE_ON_EXIT
		on_exit((void (*)(int, void*))etk_memory_check_leak, NULL);
#else
		atexit((void (*)(void))etk_memory_check_leak);
#endif
	};
};
static __etk_memory_check_leak ___etk_memory_check_leak;
#endif


#endif /* ETK_BUILD_WITH_MEMORY_TRACING */

