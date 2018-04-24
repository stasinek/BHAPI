#include <BE.h>

#ifdef BHAPI_BUILD_LIBRARY

#ifdef __cplusplus
namespace bhapi {
#endif // __cplusplus
const  uint8  be_major_version = BHAPI_MAJOR_VERSION;
const  uint8  be_minor_version = BHAPI_MINOR_VERSION;
const  uint8  be_micro_version = BHAPI_MICRO_VERSION;
const  uint8  be_interface_age = BHAPI_INTERFACE_AGE;
const  uint16 be_binary_age = BHAPI_BINARY_AGE;
#ifdef __cplusplus
} /* namespace */
#endif // __cplusplus
#endif
