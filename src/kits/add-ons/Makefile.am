AUTOMAKE_OPTIONS = 1.7

SUBDIRS = font graphics theme

EFENCE = 

add_ons_la = font/libBHAPI_wrapper_add-ons-font.la graphics/libBHAPI_wrapper_add-ons-graphics.la theme/libBHAPI_wrapper_add-ons-theme.la

noinst_LTLIBRARIES = libBHAPI_wrapper_add-ons.la

libetk_add_ons_la_SOURCES =

libetk_add_ons_la_LIBADD = $(add_ons_la)
libetk_add_ons_la_DEPENDENCIES = $(add_ons_la)

DISTCLEANFILES = Makefile.in

