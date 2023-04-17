#ifndef _CODEHANDLER_H_
#define _CODEHANDLER_H_

#include <bluetooth/HCI/btHCI.h>

namespace Bluetooth
{

class CodeHandler {

public:
	/*
	 * TODO: Handler and protocol could be fit in 16 bits 12
	 * for handler and 4 for the protocol
	 */

	/*
	 * Used:
	 * - From HCI layer to send events to bluetooth server.
	 * - L2cap lower to dispatch TX packets to HCI Layer 
	 * - informing about connection handle
	 * - Transport drivers dispatch its data to HCI layer
	 *
	 */
	static hci_id Device(uint32_t code)
	{
		return ((code & 0xFF000000) >> 24);
	}


	static void SetDevice(uint32_t* code, hci_id device)
	{
		*code = *code | ((device & 0xFF) << 24);
	}


	static uint16 Handler(uint32_t code)
	{
		return ((code & 0xFFFF) >> 0);
	}


	static void SetHandler(uint32_t* code, uint16 handler)
	{
		*code = *code | ((handler & 0xFFFF) << 0);
	}


	static bt_packet_t Protocol(uint32_t code)
	{
		return (bt_packet_t)((code & 0xFF0000) >> 16);
	}


	static void SetProtocol(uint32_t* code, bt_packet_t protocol)
	{
		*code = *code | ((protocol & 0xFF) << 16);
	}


};


} // namespace


#endif
