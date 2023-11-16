/*
 * Copyright 2006-2015, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Axel Dörfler, axeld@pinc-software.de
 */
#ifndef SETTINGS_H
#define SETTINGS_H


#include <kits/netvector>

#include <kits/netMessage.h>
#include <kits/app/Messenger.h>
#include <kits/netNetworkAddress.h>
#include <kits/netPath.h>
#include <kits/netStringList.h>


namespace BNetworkKit {


class BNetworkInterfaceSettings;
class BNetworkServiceSettings;


class BNetworkSettings {
public:
	static	const uint32_t		kMsgInterfaceSettingsUpdated = 'SUif';
	static	const uint32_t		kMsgNetworkSettingsUpdated = 'SUnw';
	static	const uint32_t		kMsgServiceSettingsUpdated = 'SUsv';

public:
								BNetworkSettings();
								~BNetworkSettings();

			status_t			GetNextInterface(uint32_t& cookie,
									BMessage& interface);
			status_t			GetInterface(const char* name,
									BMessage& interface) const;
			status_t			AddInterface(const BMessage& interface);
			status_t			RemoveInterface(const char* name);
			BNetworkInterfaceSettings
								Interface(const char* name);
			const BNetworkInterfaceSettings
								Interface(const char* name) const;

			int32_t				CountNetworks() const;
			status_t			GetNextNetwork(uint32_t& cookie,
									BMessage& network) const;
			status_t			GetNetwork(const char* name,
									BMessage& network) const;
			status_t			AddNetwork(const BMessage& network);
			status_t			RemoveNetwork(const char* name);

			const BMessage&		Services() const;
			status_t			GetNextService(uint32_t& cookie,
									BMessage& service);
			status_t			GetService(const char* name,
									BMessage& service) const;
			status_t			AddService(const BMessage& service);
			status_t			RemoveService(const char* name);
			BNetworkServiceSettings
								Service(const char* name);
			const BNetworkServiceSettings
								Service(const char* name) const;

			status_t			StartMonitoring(const BMessenger& target);
			status_t			StopMonitoring(const BMessenger& target);

			status_t			Update(BMessage* message);

private:
			status_t			_Load(const char* name = NULL,
									uint32_t* _type = NULL);
			status_t			_Save(const char* name = NULL);
			BPath				_Path(BPath& parent, const char* name);
			status_t			_GetPath(const char* name, BPath& path);

			status_t			_StartWatching(const char* name,
									const BMessenger& target);

			bool				_IsWatching(const BMessenger& target) const
									{ return fListener == target; }
			bool				_IsWatching() const
									{ return fListener.IsValid(); }

			status_t			_ConvertNetworkToSettings(BMessage& message);
			status_t			_ConvertNetworkFromSettings(BMessage& message);

			status_t			_GetItem(const BMessage& container,
									const char* itemField,
									const char* nameField, const char* name,
									int32_t& _index, BMessage& item) const;
			status_t			_RemoveItem(BMessage& container,
									const char* itemField,
									const char* nameField, const char* name,
									const char* store = NULL);

private:
			BMessenger			fListener;
			BMessage			fInterfaces;
			BMessage			fNetworks;
			BMessage			fServices;
};


class BNetworkInterfaceAddressSettings {
public:
								BNetworkInterfaceAddressSettings();
								BNetworkInterfaceAddressSettings(
									const BMessage& data);
								BNetworkInterfaceAddressSettings(
									const BNetworkInterfaceAddressSettings&
										other);
								~BNetworkInterfaceAddressSettings();

			int					Family() const;
			void				SetFamily(int family);
			bool				IsAutoConfigure() const;
			void				SetAutoConfigure(bool configure);

			const BNetworkAddress&
								Address() const;
			BNetworkAddress&	Address();
			const BNetworkAddress&
								Mask() const;
			BNetworkAddress&	Mask();
			const BNetworkAddress&
								Peer() const;
			BNetworkAddress&	Peer();
			const BNetworkAddress&
								Broadcast() const;
			BNetworkAddress&	Broadcast();
			const BNetworkAddress&
								Gateway() const;
			BNetworkAddress&	Gateway();

			status_t			GetMessage(BMessage& data) const;

			BNetworkInterfaceAddressSettings&
								operator=(
									const BNetworkInterfaceAddressSettings&
										other);

private:
			int32_t				fFamily;
			bool				fAutoConfigure;
			BNetworkAddress		fAddress;
			BNetworkAddress		fMask;
			BNetworkAddress		fPeer;
			BNetworkAddress		fBroadcast;
			BNetworkAddress		fGateway;
};


class BNetworkInterfaceSettings {
public:
								BNetworkInterfaceSettings();
								BNetworkInterfaceSettings(
									const BMessage& message);
								~BNetworkInterfaceSettings();

			const char*			Name() const;
			void				SetName(const char* name);

			int32_t				Flags() const;
			void				SetFlags(int32_t flags);
			int32_t				MTU() const;
			void				SetMTU(int32_t mtu);
			int32_t				Metric() const;
			void				SetMetric(int32_t metric);

			int32_t				CountAddresses() const;
			const BNetworkInterfaceAddressSettings&
								AddressAt(int32_t index) const;
			BNetworkInterfaceAddressSettings&
								AddressAt(int32_t index);
			int32_t				FindFirstAddress(int family) const;
			void				AddAddress(const
									BNetworkInterfaceAddressSettings& address);
			void				RemoveAddress(int32_t index);

			bool				IsAutoConfigure(int family) const;

			status_t			GetMessage(BMessage& data) const;

private:
			BString				fName;
			int32_t				fFlags;
			int32_t				fMTU;
			int32_t				fMetric;
			std::vector<BNetworkInterfaceAddressSettings>
								fAddresses;
};


class BNetworkServiceAddressSettings {
public:
								BNetworkServiceAddressSettings();
								BNetworkServiceAddressSettings(
									const BMessage& data, int family = -1,
									int type = -1, int protocol = -1,
									int port = -1);
								~BNetworkServiceAddressSettings();

			int					Family() const;
			void				SetFamily(int family);
			int					Protocol() const;
			void				SetProtocol(int protocol);
			int					Type() const;
			void				SetType(int type);

			const BNetworkAddress&
								Address() const;
			BNetworkAddress&	Address();

			status_t			GetMessage(BMessage& data) const;

			bool				operator==(
									const BNetworkServiceAddressSettings& other)
										const;

private:
			int32_t				fFamily;
			int					fProtocol;
			int					fType;
			BNetworkAddress		fAddress;
};


class BNetworkServiceSettings {
public:
								BNetworkServiceSettings();
								BNetworkServiceSettings(
									const BMessage& message);
								~BNetworkServiceSettings();

			status_t			InitCheck() const;

			const char*			Name() const;
			void				SetName(const char* name);
			bool				IsStandAlone() const;
			void				SetStandAlone(bool alone);
			bool				IsEnabled() const;
			void				SetEnabled(bool enable);

			int					Family() const;
			void				SetFamily(int family);
			int					Protocol() const;
			void				SetProtocol(int protocol);
			int					Type() const;
			void				SetType(int type);
			int					Port() const;
			void				SetPort(int port);

			int32_t				CountArguments() const;
			const char*			ArgumentAt(int32_t index) const;
			void				AddArgument(const char* argument);
			void				RemoveArgument(int32_t index);

			int32_t				CountAddresses() const;
			const BNetworkServiceAddressSettings&
								AddressAt(int32_t index) const;
			void				AddAddress(const
									BNetworkServiceAddressSettings& address);
			void				RemoveAddress(int32_t index);

			bool				IsRunning() const;

			status_t			GetMessage(BMessage& data) const;

private:
			BString				fName;
			int32_t				fFamily;
			int32_t				fType;
			int32_t				fProtocol;
			int32_t				fPort;
			bool				fEnabled;
			bool				fStandAlone;
			BStringList			fArguments;
			std::vector<BNetworkServiceAddressSettings>
								fAddresses;
};


}	// namespace BNetworkKit


#endif	// SETTINGS_H
