#ifndef BHAPI_CHARACTER_SETS_H
#define BHAPI_CHARACTER_SETS_H
#include <CharacterSet.h>
#include <Haiku.h>

namespace BPrivate {
extern const BCharacterSet* character_sets_by_id[];
extern const uint32 character_sets_by_id_count;
extern const BCharacterSet** character_sets_by_MIBenum;
extern uint32 maximum_valid_MIBenum;
}
#endif // BHAPI_character_sets_H
