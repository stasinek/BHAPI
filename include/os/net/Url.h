/*
 * Copyright 2010 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _B_URL_H_
#define _B_URL_H_


#include <Archivable.h>
#include <Message.h>
#include <Path.h>
#include <String.h>


class BUrl : public BArchivable {
public:
								BUrl(const char* url);
								BUrl(BMessage* archive);
								BUrl(const BUrl& other);
								BUrl(const BUrl& base, const BString& relative);
								BUrl(const BPath& path);
								BUrl();
	virtual						~BUrl();

	// URL fields modifiers
			BUrl&				SetUrlString(const BString& url);
			BUrl&				SetProtocol(const BString& scheme);
			BUrl&				SetUserName(const BString& user);
			BUrl&				SetPassword(const BString& password);
			void				SetAuthority(const BString& authority);
			BUrl&				SetHost(const BString& host);
			BUrl&				SetPort(int port);
			BUrl&				SetPath(const BString& path);
			BUrl&				SetRequest(const BString& request);
			BUrl&				SetFragment(const BString& fragment);
			
	// URL fields access
			const BString&		UrlString() const;
			const BString&		Protocol() const;
			const BString&		UserName() const;
			const BString&		Password() const;
			const BString&		UserInfo() const;
			const BString&		Host() const;
			int 				Port() const;
			const BString&		Authority() const;
			const BString&		Path() const;
			const BString&		Request() const;
			const BString&		Fragment() const;
			
	// URL fields tests
			bool 				IsValid() const;
			bool				HasProtocol() const;
			bool				HasUserName() const;
			bool				HasPassword() const;
			bool				HasUserInfo() const;
			bool				HasHost() const;
			bool				HasPort() const;
			bool				HasAuthority() const;
			bool				HasPath() const;
			bool				HasRequest() const;
			bool				HasFragment() const;

	// Url encoding/decoding of needed fields
			void				UrlEncode(bool strict = false);
			void				UrlDecode(bool strict = false);

			status_t			IDNAToAscii();
			status_t			IDNAToUnicode();

	// Url encoding/decoding of strings
	static	BString				UrlEncode(const BString& url, 
									bool strict = false, 
									bool directory = false);
	static	BString				UrlDecode(const BString& url, 
									bool strict = false);

	// utility functionality
			bool				HasPreferredApplication() const;
			BString				PreferredApplication() const;
			status_t			OpenWithPreferredApplication(
									bool onProblemAskUser = true) const;

	// BArchivable members
	virtual	status_t			Archive(BMessage* into,
									bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

	// URL comparison
			bool				operator==(BUrl& other) const;
			bool				operator!=(BUrl& other) const;

	// URL assignment
			const BUrl&			operator=(const BUrl& other);
			const BUrl&			operator=(const BString& string);
			const BUrl&			operator=(const char* string);
			
	// URL to string conversion
								operator const char*() const;

private:
			void				_ResetFields();
			void				_ExplodeUrlString(const BString& urlString);
			BString				_MergePath(const BString& relative) const;
			void				_SetPathUnsafe(const BString& path);

	static	BString				_DoUrlEncodeChunk(const BString& chunk, 
									bool strict, bool directory = false);
	static 	BString				_DoUrlDecodeChunk(const BString& chunk, 
									bool strict);

			bool				_IsProtocolValid();
	static	bool				_IsUnreserved(char c);
	static	bool				_IsGenDelim(char c);
	static	bool				_IsSubDelim(char c);

			BString				_UrlMimeType() const;

private:
	mutable	BString				fUrlString;
	mutable	BString				fAuthority;
	mutable	BString				fUserInfo;

			BString				fProtocol;
			BString				fUser;
			BString				fPassword;
			BString				fHost;
			int					fPort;
			BString				fPath;
			BString				fRequest;
			BString				fFragment;
			
	mutable	bool				fUrlStringValid : 1;
	mutable	bool				fAuthorityValid : 1;
	mutable bool				fUserInfoValid : 1;

			bool				fHasProtocol : 1;
			bool				fHasUserName : 1;
			bool				fHasPassword : 1;
			bool				fHasHost : 1;
			bool				fHasPort : 1;
			bool				fHasPath : 1;
			bool				fHasRequest : 1;
			bool				fHasFragment : 1;
};

#endif  // _B_URL_H_
