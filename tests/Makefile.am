AUTOMAKE_OPTIONS = 1.7

INCLUDES = -I$(top_builddir) $(ETK_CFLAGS) $(ETK_THREAD_CFLAGS)
LIBS = $(top_builddir)/etk/libetkxx.la $(ETK_THREAD_LIBS)

noinst_PROGRAMS =			\
	time-test			\
	area-test			\
	locker-test			\
	semaphore-test			\
	semaphore-locking-test		\
	thread-test			\
	thread-exit-test		\
	thread-suspend-test		\
	string-test			\
	region-test			\
	path-test			\
	port-test			\
	message-test			\
	looper-test			\
	app-test			\
	window-look-test		\
	stringview-test			\
	texteditable-test		\
	button-test			\
	scroll-test			\
	xml-parser-test			\
	file-test			\
	net-test			\
	streamio-test

time_test_SOURCES = time-test.c
area_test_SOURCES = area-test.c
locker_test_SOURCES = locker-test.c
semaphore_test_SOURCES = semaphore-test.c
semaphore_locking_test_SOURCES = semaphore-locking-test.c
thread_test_SOURCES = thread-test.c
thread_exit_test_SOURCES = thread-exit-test.c
thread_suspend_test_SOURCES = thread-suspend-test.cpp
string_test_SOURCES = string-test.cpp
region_test_SOURCES = region-test.cpp
path_test_SOURCES = path-test.cpp
port_test_SOURCES = port-test.cpp
message_test_SOURCES = message-test.cpp
looper_test_SOURCES = looper-test.cpp
app_test_SOURCES = app-test.cpp
window_look_test_SOURCES = window-look-test.cpp
stringview_test_SOURCES = stringview-test.cpp
texteditable_test_SOURCES = texteditable-test.cpp
button_test_SOURCES = button-test.cpp
scroll_test_SOURCES = scroll-test.cpp
xml_parser_test_SOURCES = xml-parser-test.cpp
file_test_SOURCES = file-test.cpp
net_test_SOURCES = net-test.cpp
streamio_test_SOURCES = streamio-test.cpp

DISTCLEANFILES = Makefile.in

