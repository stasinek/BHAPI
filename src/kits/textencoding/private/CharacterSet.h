#ifndef CHARACTER_SET_H
#define CHARACTER_SET_H
#include <Haiku.h>

namespace BPrivate {

/**
 * @file   CharacterSet.h
 * @author Andrew Bachmann
 * @brief  Defines BCharacterSet
 *
 * @see http://www.iana.org/assignments/character-sets
 **/

class BCharacterSet {
    /**
     * @class BCharacterSet
     * @brief An object holding a variety of useful information about a character set.
     *
     * This information has been derived from the IANA standards organization.  
     * Since IANA provides several names for some encodings, this object also
     * provides for aliases.
     **/
public:
	/**
     * @brief default constructor, for stack allocated character set objects
     **/
    BCharacterSet();
	/**
     * @brief constructor, for internal use only
     **/
    BCharacterSet(uint32_t id, uint32_t MIBenum, const char * print_name,
                  const char * iana_name, const char * mime_name,
                  const char ** aliases);
    /**
     * @brief returns an id for use in BFont::SetEncoding
     * @return an id for use in BFont::SetEncoding
     **/
    uint32_t GetFontID(void) const;
    /**
     * @brief returns an id for use in convert_to/from_utf8
     * @return an id for use in convert_to/from_utf8
     **/
    uint32_t GetConversionID(void) const;
    /**
     * @brief returns an id for use in MIBs to identify coded character sets
     * @return an id for use in MIBs to identify coded character sets
     **/
    uint32_t GetMIBenum(void) const;
    /**
     * @brief returns the IANA standard name for this character set
     * @return the IANA standard name for this character set
     **/
    const char * GetName(void) const;
    /**
     * @brief returns a user interface friendly name for this character set
     * @return a user interface friendly name for this character set
     **/
    const char * GetPrintName(void) const;
    /**
     * @brief returns the MIME preferred name for this character set, or null if none exists
     * @return the MIME preferred name for this character set, or null if none exists
     **/
    const char * GetMIMEName(void) const;
    /**
     * @brief returns the number of aliases for this character set
     * @return the number of aliases for this character set
     **/
    int32_t CountAliases(void) const;
    /**
     * @brief returns the index'th alias, or NULL if out of range
     * @return the index'th alias, or NULL if out of range
     **/
    const char * AliasAt(uint32_t index) const;
    
private:
    uint32_t     id;            //! id from convert_to_utf8/convert_from_utf8
    uint32_t     MIBenum;       //! for use in MIBs to identify coded character sets
    const char * print_name;  //! user interface friendly name
    const char * iana_name;   //! standard IANA name
    const char * mime_name;   //! the preferred mime name
    const char ** aliases;    //! aliases for this character set
    uint32_t     aliases_count; //! how many aliases are available
};
}
namespace BPrivate {
extern const BCharacterSet* character_sets_by_id[];
extern const uint32_t character_sets_by_id_count;
extern const BCharacterSet** character_sets_by_MIBenum;
extern uint32_t maximum_valid_MIBenum;
}
#endif // CHARACTER_SET_H
