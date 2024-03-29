/*
 * Copyright 2007 Oliver Ruiz Dorantes, oliver.ruiz.dorantes_at_gmail.com
 *
 * All rights reserved. Distributed under the terms of the MIT License.
 *
 */


#ifdef BT_DEBUG_THIS_MODULE
	#ifndef MODULE_NAME
	//#warning MODULE_NAME not defined for Haiku BT debugging tools
	#define MODULE_NAME "BT"
	#endif
	
	#ifndef SUBMODULE_NAME
	//#warning SUBMODULE_NAME not defined for Haiku BT debugging tools
	#define SUBMODULE_NAME ""
	#endif
	
	#ifndef SUBMODULE_COLOR
	//#warning SUBMODULE_COLOR not defined for Haiku BT debugging tools
	#define SUBMODULE_COLOR 38
	#endif

	#define debugf(a,param...) dprintf("\x1b[%dm" MODULE_NAME " " SUBMODULE_NAME " " "%s\x1b[0m: " a,SUBMODULE_COLOR,__FUNCTION__, param);
	#define flowf(a) dprintf("\x1b[%dm" MODULE_NAME " " SUBMODULE_NAME " " "%s\x1b[0m: " a,SUBMODULE_COLOR,__FUNCTION__);
#else
	#define debugf(a,param...)
	#define flowf(a,param...)
#endif
#undef BT_DEBUG_THIS_MODULE

#define TOUCH(x) ((void)(x))

/* */ 
#if 0
#pragma mark - Kernel Auxiliary Stuff -
#endif

static inline uint32_t TEST_AND_SET(uint32_t *byte, uint32_t bit_mask) {

	uint32_t val = (*byte&bit_mask)!=0;
	*byte |= bit_mask;
	return val;
}

static inline uint32_t TEST_AND_CLEAR(uint32_t* byte, uint32_t bit_mask) {

	uint32_t val = (*byte&bit_mask)!=0;
	*byte &= ~bit_mask;
	return val;
}
