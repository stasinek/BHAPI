/*
 * Copyright 2001-2014 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef	_PICTURE_H
#define	_PICTURE_H


#include <InterfaceDefs.h>
#include <Rect.h>
#include <Archivable.h>


class BDataIO;
class BView;
struct _BPictureExtent_;


class BPicture : public BArchivable {
public:
								BPicture();
								BPicture(const BPicture& other);
								BPicture(BMessage* data);
	virtual						~BPicture();

	static	BArchivable*		Instantiate(BMessage* data);
	virtual	status_t			Archive(BMessage* data, bool deep = true) const;
	virtual	status_t			Perform(perform_code code, void* arg);

			status_t			Play(void** callBackTable,
									int32_t tableEntries,
									void* userData);

			status_t			Flatten(BDataIO* stream);
			status_t			Unflatten(BDataIO* stream);

	class Private;
private:
	// FBC padding and forbidden methods
	virtual	void				_ReservedPicture1();
	virtual	void				_ReservedPicture2();
	virtual	void				_ReservedPicture3();

			BPicture&			operator=(const BPicture&);

private:
	friend class BWindow;
	friend class BView;
	friend class BPrintJob;
	friend class Private;

			void				_InitData();
			void				_DisposeData();

			void				_ImportOldData(const void* data, int32_t size);

			void				SetToken(int32_t token);
			int32_t				Token() const;

			bool				_AssertLocalCopy();
			bool				_AssertOldLocalCopy();
			bool				_AssertServerCopy();

			status_t			_Upload();
			status_t			_Download();

	// Deprecated API
								BPicture(const void* data, int32_t size);
			const void*			Data() const;
			int32_t				DataSize() const;

			void				Usurp(BPicture* lameDuck);
			BPicture*			StepDown();

private:
			int32_t				fToken;
			_BPictureExtent_*	fExtent;
			BPicture*			fUsurped;

			uint32_t				_reserved[3];
};

#endif // _PICTURE_H

