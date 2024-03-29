/*
 * Copyright 2002-2007, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _MIME_SNIFFER_RULES_H
#define _MIME_SNIFFER_RULES_H


#include <Haiku.h>

#include <List.h>
#include <kits/support/String>

class BFile;
class BString;
struct entry_ref;


namespace BPrivate {
namespace Storage {

namespace Sniffer {
	class Rule;
}

namespace Mime {


class DatabaseLocation;
class MimeSniffer;


class SnifferRules {
public:
	SnifferRules(DatabaseLocation* databaseLocation, MimeSniffer* mimeSniffer);
	~SnifferRules();
	
	status_t GuessMimeType(const entry_ref *ref, BString *type);
	status_t GuessMimeType(const void *buffer, int32_t length, BString *type);
	
	status_t SetSnifferRule(const char *type, const char *rule);
	status_t DeleteSnifferRule(const char *type);
	
	void PrintToStream() const;

	struct sniffer_rule {
		std::string type;							// The mime type that own the rule
		std::string rule_string;					// The unparsed string version of the rule
		BPrivate::Storage::Sniffer::Rule *rule;		// The parsed rule
		
		sniffer_rule(BPrivate::Storage::Sniffer::Rule *rule = NULL);
		~sniffer_rule(); 
	};		
private:
	status_t BuildRuleList();
	status_t GuessMimeType(BFile* file, const void *buffer, int32_t length,
		BString *type);
	ssize_t MaxBytesNeeded();
	status_t ProcessType(const char *type, ssize_t *bytesNeeded);

	std::list<sniffer_rule> fRuleList;

private:
	DatabaseLocation*	fDatabaseLocation;
	MimeSniffer*		fMimeSniffer;
	ssize_t				fMaxBytesNeeded;
	bool				fHaveDoneFullBuild;
};

} // namespace Mime
} // namespace Storage
} // namespace BPrivate

#endif	// _MIME_SNIFFER_H
