#include "vfs.h"

//
//	copy unicode string to unicode string and allocate the new string
//

void VfsCopyUnicodeString(PUNICODE_STRING dest, PUNICODE_STRING src)
{
	int cnt;
	
	dest->Length = src->Length;
	dest->MaximumLength = src->MaximumLength = (USHORT)QUAD_ALIGN(src->Length+2);
	dest->Buffer = ExAllocatePoolWithTag(VFSPOOL, dest->MaximumLength, 'RAHC');

	RtlCopyUnicodeString(dest, src);
}

//
//	copy wchar string to unicode string and allocate the new string
//
/* unused --sadyc
void VfsCopyWcharToUnicodeString(PUNICODE_STRING dest, PCWSTR src)
{
	int cnt;

	for (cnt=0; src[cnt]!=0; cnt++)
		;
	dest->Length = cnt*2;
	dest->MaximumLength = (USHORT)QUAD_ALIGN(cnt*2+2);
	dest->Buffer = ExAllocatePool(VFSPOOL, dest->MaximumLength);

	for (cnt=0; ; cnt++) {
		dest->Buffer[cnt] = src[cnt];
		if (src[cnt] == 0)
			return;
	}
}
*/

//
// copy char string to unicode string and allocate the new string
//
/* unused --sadyc
void VfsCopyCharToUnicodeString(PUNICODE_STRING dest, PCSTR src, USHORT len)
{
	int cnt;

	dest->Length = len*2;
	dest->MaximumLength = (USHORT)QUAD_ALIGN(len*2+2);
	dest->Buffer = ExAllocatePool(VFSPOOL, dest->MaximumLength);

	for (cnt=0; cnt<len; cnt++)
		dest->Buffer[cnt] = src[cnt];
	dest->Buffer[cnt] = 0;
}
*/

//
// copy sz string to unicode string and allocate the new string
//
/* unused --sadyc
void VfsCopyZcharToUnicodeString(PUNICODE_STRING dest, PCSTR src)
{
	int cnt;

	for (cnt=0; src[cnt]!=0; cnt++)
		;
	dest->Length = cnt*2;
	dest->MaximumLength = (USHORT)QUAD_ALIGN(cnt*2+2);
	dest->Buffer = ExAllocatePool(VFSPOOL, dest->MaximumLength);

	for (cnt=0; ; cnt++) {
		dest->Buffer[cnt] = src[cnt];
		if (src[cnt] == 0)
			return;
	}
}
*/
//
//	truncate unicode string to 0 size
//
/* unused --sadyc
void VfsZerooutUnicodeString(PUNICODE_STRING ustr)
{
	ustr->Length = ustr->MaximumLength = 0;
	ustr->Buffer = NULL;
}
*/
//
//	free unicode string
//
/* not used --sadyc
void VfsFreeUnicodeString(PUNICODE_STRING ustr)
{
	if (ustr) {
		if (ustr->Buffer) ExFreePool(ustr->Buffer);
		ustr->Buffer = NULL;
		ustr->Length = ustr->MaximumLength = 0;
	}
}
*/
//
//	copy unicode string to sz string
//
PSTR VfsCopyUnicodeStringToZcharUnixPath(PUNICODE_STRING src)
{
	int i, length;
	PSTR dest;

	length = src->Length / sizeof(WCHAR);
	dest = ExAllocatePoolWithTag(VFSPOOL, length + 1, 'RHCU');
	if (!dest)
		return NULL;
	for (i = 0; i < length; i++) {
		dest[i] = (char)src->Buffer[i];
		if (dest[i] == '\\') dest[i] = '/';
	}
	dest[length] = 0;

	return dest;
}

VOID VfsCharToWchar(PWCHAR Destination, PCHAR Source, ULONG Length)
{
	ULONG	Index;

	ASSERT(Destination != NULL);
	ASSERT(Source != NULL);

	for (Index = 0; Index < Length; Index++) {
		Destination[Index] = (WCHAR)Source[Index];
	}
}
