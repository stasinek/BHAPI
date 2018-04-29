/*
 * Copyright 2014 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 */

#ifndef BHAPI_CERTIFICATE_H
#define BHAPI_CERTIFICATE_H

#ifdef __cplusplus // C++
#include <SecureSocket.h>
#include <kits/support/String.h>
#include <Haiku.h>
class BHAPI_IMPEXP BCertificate {
public:
                BCertificate(const BCertificate& other);
                ~BCertificate();

    int			Version() const;

    time_t		StartDate() const;
    time_t		ExpirationDate() const;

    bool		IsValidAuthority() const;
    bool		IsSelfSigned() const;

    BString		Issuer() const;
    BString		Subject() const;
    BString		SignatureAlgorithm() const;

    BString		String() const;

    bool		operator==(const BCertificate& other) const;

private:
    friend class BSecureSocket::Private;
    class Private;
                BCertificate(Private* data);

    Private*	fPrivate;
};
#endif // C++
#define BCERTIFICATE_I
#endif
