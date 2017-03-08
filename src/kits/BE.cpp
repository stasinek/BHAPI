#include "BE.h"

#ifdef BHAPI_BUILD_LIBRARY
EXPORT_BHAPI const  __be_uint8  b_major_version = BHAPI_MAJOR_VERSION;
EXPORT_BHAPI const  __be_uint8  b_minor_version = BHAPI_MINOR_VERSION;
EXPORT_BHAPI const  __be_uint8  b_micro_version = BHAPI_MICRO_VERSION;
EXPORT_BHAPI const  __be_uint8   __be_interface_age = BHAPI_INTERFACE_AGE;
EXPORT_BHAPI const  __be_uint16 b_binary_age = BHAPI_BINARY_AGE;
#endif
