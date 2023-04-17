/*
 * Copyright 2011-2012 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Alexander von Gluck, kallisti5@unixzen.com
 */
#ifndef __DEV_UART_PL011_H
#define __DEV_UART_PL011_H


#include <sys/types.h>

#include <Haiku.h>

#include <arch/generic/debug_uart.h>


class ArchUARTPL011 : public DebugUART {
public:
							ArchUARTPL011(addr_t base, int64 clock);
							~ArchUARTPL011();

			void			InitEarly();
			void			InitPort(uint32_t baud);

			void			Enable();
			void			Disable();

			int				PutChar(char c);
			int				GetChar(bool wait);

			void			FlushTx();
			void			FlushRx();

private:
			void			Out32(int reg, uint32_t value);
			uint32_t			In32(int reg);
	virtual	void			Barrier();
};


ArchUARTPL011 *arch_get_uart_pl011(addr_t base, int64 clock);


#endif
