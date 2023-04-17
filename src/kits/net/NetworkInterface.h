/*
 * Copyright 2010, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _NETWORK_INTERFACE_H
#define _NETWORK_INTERFACE_H


#include <net/if.h>
#include <net/if_types.h>

#include <ObjectList.h>
#include <NetworkAddress.h>


class BNetworkInterface;
class BNetworkRoute;


class BNetworkInterfaceAddress {
public:
								BNetworkInterfaceAddress();
								~BNetworkInterfaceAddress();

			status_t			SetTo(const BNetworkInterface& interface,
									int32_t index);

			void				SetAddress(const BNetworkAddress& address);
			void				SetMask(const BNetworkAddress& mask);
			void				SetBroadcast(const BNetworkAddress& broadcast);
			void				SetDestination(
									const BNetworkAddress& destination);

			BNetworkAddress&	Address() { return fAddress; }
			BNetworkAddress&	Mask() { return fMask; }
			BNetworkAddress&	Broadcast() { return fBroadcast; }
			BNetworkAddress&	Destination() { return fBroadcast; }

			const BNetworkAddress& Address() const { return fAddress; }
			const BNetworkAddress& Mask() const { return fMask; }
			const BNetworkAddress& Broadcast() const { return fBroadcast; }
			const BNetworkAddress& Destination() const { return fBroadcast; }

			void				SetFlags(uint32_t flags);
			uint32_t				Flags() const { return fFlags; }

			int32_t				Index() const { return fIndex; }

private:
			int32_t				fIndex;
			BNetworkAddress		fAddress;
			BNetworkAddress		fMask;
			BNetworkAddress		fBroadcast;
			uint32_t				fFlags;
};


class BNetworkInterface {
public:
								BNetworkInterface();
								BNetworkInterface(const char* name);
								BNetworkInterface(uint32_t index);
								~BNetworkInterface();

			void				Unset();
			void				SetTo(const char* name);
			status_t			SetTo(uint32_t index);

			bool				Exists() const;

			const char*			Name() const;
			uint32_t				Index() const;
			uint32_t				Flags() const;
			uint32_t				MTU() const;
			int32_t				Media() const;
			uint32_t				Metric() const;
			uint32_t				Type() const;
			status_t			GetStats(ifreq_stats& stats);
			bool				HasLink() const;

			status_t			SetFlags(uint32_t flags);
			status_t			SetMTU(uint32_t mtu);
			status_t			SetMedia(int32_t media);
			status_t			SetMetric(uint32_t metric);

			int32_t				CountAddresses() const;
			status_t			GetAddressAt(int32_t index,
									BNetworkInterfaceAddress& address);
			int32_t				FindAddress(const BNetworkAddress& address);
			int32_t				FindFirstAddress(int family);

			status_t			AddAddress(
									const BNetworkInterfaceAddress& address);
			status_t			AddAddress(const BNetworkAddress& address);
			status_t			SetAddress(
									const BNetworkInterfaceAddress& address);
			status_t			RemoveAddress(
									const BNetworkInterfaceAddress& address);
			status_t			RemoveAddress(const BNetworkAddress& address);
			status_t			RemoveAddressAt(int32_t index);

			status_t			GetHardwareAddress(BNetworkAddress& address);

			status_t			AddRoute(const BNetworkRoute& route);
			status_t			AddDefaultRoute(const BNetworkAddress& gateway);
			status_t			RemoveRoute(const BNetworkRoute& route);
			status_t			RemoveRoute(int family,
									const BNetworkRoute& route);
			status_t			RemoveDefaultRoute(int family);
			status_t			GetRoutes(int family,
									BObjectList<BNetworkRoute>& routes) const;
			status_t			GetDefaultRoute(int family,
									BNetworkRoute& route) const;
			status_t			GetDefaultGateway(int family,
									BNetworkAddress& gateway) const;

			status_t			AutoConfigure(int family);

private:
			char				fName[IF_NAMESIZE];
			BList				fAddresses;
};


#endif	// _NETWORK_INTERFACE_H
