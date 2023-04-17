/*
 * Copyright 2015, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef TEAM_SIGNAL_SETTINGS_H
#define TEAM_SIGNAL_SETTINGS_H

#include <Message.h>

#include <SignalDispositionTypes.h>


class TeamSignalSettings {
public:
								TeamSignalSettings();
	virtual						~TeamSignalSettings();

			TeamSignalSettings&
								operator=(
									const TeamSignalSettings& other);
									// throws std::bad_alloc;

			const char*			ID() const;
			status_t			SetTo(const BMessage& archive);
			status_t			WriteTo(BMessage& archive) const;
			void				Unset();

			void				SetDefaultSignalDisposition(int32_t disposition);
			int32_t				DefaultSignalDisposition() const;

			int32_t				CountCustomSignalDispositions() const;
			status_t			AddCustomSignalDisposition(int32_t signal,
									int32_t disposition);
			status_t			RemoveCustomSignalDispositionAt(int32_t index);
			status_t			GetCustomSignalDispositionAt(int32_t index,
									int32_t& signal, int32_t& disposition) const;

	virtual	TeamSignalSettings*
								Clone() const;

private:
	BMessage					fValues;
};


#endif	// TEAM_SIGNAL_SETTINGS_H
