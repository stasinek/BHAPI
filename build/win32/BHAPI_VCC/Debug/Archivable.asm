; Listing generated by Microsoft (R) Optimizing Compiler Version 13.10.3077 

; Generated by VC++ for Common Language Runtime
.file "\Documents and Settings\stasiek.TC12\My Documents\prc++\x86_libraries\Bhapi\src\kits\support\Archivable.cpp"
	.rdata
	.space	20

$SG6268:
	.ascii	"BArchivable\000"

$SG6270:
	.ascii	"class\000"
	.space	2

$SG6279:
	.ascii	"BArchivable::Instantiate\000"
	.space	3

$SG6281:
	.ascii	"[SUPPORT]: %s --- Can't instantiate a \"BArchivable\"\000"

$SG6290:
	.ascii	"class\000"
	.space	2

$SG6296:
	.ascii	"class\000"
	.space	2

$SG6308:
	.ascii	"class\000"
	.space	2

$SG6309:
	.ascii	"class\000"
.global	
.global	
	.bss
.local	,4
.local	,4
; Function compile flags: /Odt
; File c:\program files\microsoft visual studio .net 2003\vc7\include\xmemory
	.text
	.comdat	any, ??0?$allocator@X@std@@$$FQAE@ABV01@@Z
.global	??0?$allocator@X@std@@$$FQAE@ABV01@@Z		; std::allocator<void>::allocator<void>
??0?$allocator@X@std@@$$FQAE@ABV01@@Z:			; std::allocator<void>::allocator<void>
;	.proc.def	D:I(II)

; Function Header:
; max stack depth = 1
; function size = 2 bytes
; local varsig tk = 0x0 
; Exception Information:
; 0 handlers, each consisting of filtered handlers

;	.formal.i4 0,"_this$" SIG: Optional C Binding Modifier(token:0x9DF71E).Optional C Binding Modifier(token:0x9DF720).ptr.valueType (token:0x9DF723)

;	.proc.beg

; 197  : 		{	// construct by copying (do nothing)
; 198  : 		}

	ldarg.0				; _this$
$L6474:
	ret		
 .end ??0?$allocator@X@std@@$$FQAE@ABV01@@Z		; std::allocator<void>::allocator<void>
;	.proc.end.i4
text	ENDS
PUBLIC	??0?$allocator@X@std@@QAE@ABV01@@Z		; std::allocator<void>::allocator<void>
PUBLIC	__mep@??0?$allocator@X@std@@$$FQAE@ABV01@@Z
;	COMDAT __mep@??0?$allocator@X@std@@$$FQAE@ABV01@@Z
data	SEGMENT
__mep@??0?$allocator@X@std@@$$FQAE@ABV01@@Z TOKEN 06000038
; Function compile flags: /Odt
data	ENDS
;	COMDAT ??0?$allocator@X@std@@QAE@ABV01@@Z
text	SEGMENT
??0?$allocator@X@std@@QAE@ABV01@@Z PROC NEAR		; std::allocator<void>::allocator<void>, COMDAT
	jmp	DWORD PTR __mep@??0?$allocator@X@std@@$$FQAE@ABV01@@Z
??0?$allocator@X@std@@QAE@ABV01@@Z ENDP			; std::allocator<void>::allocator<void>
.global	??_7BArchivable@@6B@				; BArchivable::`vftable'
.global	??_R4BArchivable@@6B@				; BArchivable::`RTTI Complete Object Locator'
.global	??_R0?AVBArchivable@@@8				; BArchivable `RTTI Type Descriptor'
.global	??_R3BArchivable@@8				; BArchivable::`RTTI Class Hierarchy Descriptor'
.global	??_R2BArchivable@@8				; BArchivable::`RTTI Base Class Array'
.global	??_R1A@?0A@A@BArchivable@@8			; BArchivable::`RTTI Base Class Descriptor at (0,-1,0,0)'
.global	
	.bss
.local	,4
; File c:\documents and settings\stasiek.tc12\my documents\prc++\x86_libraries\bhapi\src\kits\support\archivable.cpp
	.data
	.comdat	largest, ??_7BArchivable@@6B@

??_7BArchivable@@6B@:					; BArchivable::`vftable'
	DD	??_R4BArchivable@@6B@
??_7BArchivable@@6B@:					; BArchivable::`vftable'
	DD	??_EBArchivable@@UAEPAXI@Z
??_7BArchivable@@6B@:					; BArchivable::`vftable'
	DD	?Archive@BArchivable@@UBEHPAVBMessage@@_N@Z
	.rdataseg	.rdata$r
	.comdat	any, ??_R4BArchivable@@6B@

??_R4BArchivable@@6B@:					; BArchivable::`RTTI Complete Object Locator'
	.word	0x0
??_R4BArchivable@@6B@:					; BArchivable::`RTTI Complete Object Locator'
	.word	0x0
??_R4BArchivable@@6B@:					; BArchivable::`RTTI Complete Object Locator'
	.word	0x0
??_R4BArchivable@@6B@:					; BArchivable::`RTTI Complete Object Locator'
	DD	??_R0?AVBArchivable@@@8
??_R4BArchivable@@6B@:					; BArchivable::`RTTI Complete Object Locator'
	DD	??_R3BArchivable@@8
	.data
	.comdat	any, ??_R0?AVBArchivable@@@8

??_R0?AVBArchivable@@@8:				; BArchivable `RTTI Type Descriptor'
	DD	??_7type_info@@6B@
??_R0?AVBArchivable@@@8:				; BArchivable `RTTI Type Descriptor'
	.word	0x0
??_R0?AVBArchivable@@@8:				; BArchivable `RTTI Type Descriptor'
	.ascii	".?AVBArchivable@@\000"
	.rdataseg	.rdata$r
	.comdat	any, ??_R3BArchivable@@8

??_R3BArchivable@@8:					; BArchivable::`RTTI Class Hierarchy Descriptor'
	.word	0x0
??_R3BArchivable@@8:					; BArchivable::`RTTI Class Hierarchy Descriptor'
	.word	0x0
??_R3BArchivable@@8:					; BArchivable::`RTTI Class Hierarchy Descriptor'
	.word	0x1
??_R3BArchivable@@8:					; BArchivable::`RTTI Class Hierarchy Descriptor'
	DD	??_R2BArchivable@@8
	.rdataseg	.rdata$r
	.comdat	any, ??_R2BArchivable@@8

??_R2BArchivable@@8:					; BArchivable::`RTTI Base Class Array'
	DD	??_R1A@?0A@A@BArchivable@@8
	.rdataseg	.rdata$r
	.comdat	any, ??_R1A@?0A@A@BArchivable@@8

??_R1A@?0A@A@BArchivable@@8:				; BArchivable::`RTTI Base Class Descriptor at (0,-1,0,0)'
	DD	??_R0?AVBArchivable@@@8
??_R1A@?0A@A@BArchivable@@8:				; BArchivable::`RTTI Base Class Descriptor at (0,-1,0,0)'
	.word	0x0
??_R1A@?0A@A@BArchivable@@8:				; BArchivable::`RTTI Base Class Descriptor at (0,-1,0,0)'
	.word	0x0
??_R1A@?0A@A@BArchivable@@8:				; BArchivable::`RTTI Base Class Descriptor at (0,-1,0,0)'
	.word	0xffffffff
??_R1A@?0A@A@BArchivable@@8:				; BArchivable::`RTTI Base Class Descriptor at (0,-1,0,0)'
	.word	0x0
??_R1A@?0A@A@BArchivable@@8:				; BArchivable::`RTTI Base Class Descriptor at (0,-1,0,0)'
	.word	0x0
; Function compile flags: /Odt
	.text
.global	??0BArchivable@@$$FQAE@XZ			; BArchivable::BArchivable
??0BArchivable@@$$FQAE@XZ:				; BArchivable::BArchivable
;	.proc.def	D:I(I)

; Function Header:
; max stack depth = 2
; function size = 12 bytes
; local varsig tk = 0x0 
; Exception Information:
; 0 handlers, each consisting of filtered handlers

;	.formal.i4 0,"_this$" SIG: Optional C Binding Modifier(token:0x9CA6CA).Optional C Binding Modifier(token:0x9CA6CC).ptr.valueType (token:0x9CA6CF)

;	.proc.beg

; 37   : {

	ldarg.0				; _this$
	ldsflda		??_7BArchivable@@6B@
	unaligned	1		; u32 0x1
	stind.i4	

; 38   : }

	ldarg.0				; _this$
$L6489:
	ret		
 .end ??0BArchivable@@$$FQAE@XZ				; BArchivable::BArchivable
;	.proc.end.i4
text	ENDS
PUBLIC	??0BArchivable@@QAE@XZ				; BArchivable::BArchivable
PUBLIC	__mep@??0BArchivable@@$$FQAE@XZ
;	COMDAT __mep@??0BArchivable@@$$FQAE@XZ
data	SEGMENT
__mep@??0BArchivable@@$$FQAE@XZ TOKEN 06000039
; Function compile flags: /Odt
data	ENDS
;	COMDAT ??0BArchivable@@QAE@XZ
text	SEGMENT
??0BArchivable@@QAE@XZ PROC NEAR			; BArchivable::BArchivable, COMDAT
	jmp	DWORD PTR __mep@??0BArchivable@@$$FQAE@XZ
??0BArchivable@@QAE@XZ ENDP				; BArchivable::BArchivable
.global	__unep@??1BArchivable@@$$FUAE@XZ
	.rdata
	.comdat	any, __unep@??1BArchivable@@$$FUAE@XZ

__unep@??1BArchivable@@$$FUAE@XZ:
	DD	??1BArchivable@@UAE@XZ
; Function compile flags: /Odt
	.text
	.comdat	any, ??_EBArchivable@@$$FUAEPAXI@Z
.global	??_EBArchivable@@$$FUAEPAXI@Z			; BArchivable::`vector deleting destructor'
??_EBArchivable@@$$FUAEPAXI@Z:				; BArchivable::`vector deleting destructor'
;	.proc.def	D:I(II)

; Function Header:
; max stack depth = 4
; function size = 60 bytes
; local varsig tk = 0x0 
; Exception Information:
; 0 handlers, each consisting of filtered handlers

;	.formal.u4 1,"___flags$"
;	.formal.i4 0,"_this$" SIG: Optional C Binding Modifier(token:0x9CA6CA).Optional C Binding Modifier(token:0x9CA6CC).ptr.valueType (token:0x9CA6CF)

;	.proc.beg
	ldarg.1				; ___flags$
	ldc.i.2		2		; u32 0x2
	and		
	brfalse.s	$L6250
	ldarg.0				; _this$
	ldc.i.4		4		; u32 0x4
	ldarg.0				; _this$
	ldc.i.4		4		; i32 0x4
	sub		
	ldind.i4	
	ldsfld		__unep@??1BArchivable@@$$FUAE@XZ
	call		??_M@$$FYGXPAXIHP6EX0@Z@Z
	ldarg.1				; ___flags$
	ldc.i.1		1		; u32 0x1
	and		
	brfalse.s	$L6251
	ldarg.0				; _this$
	ldc.i.4		4		; i32 0x4
	sub		
	call		??_V@$$FYAXPAX@Z
$L6251:
	ldarg.0				; _this$
	ldc.i.4		4		; i32 0x4
	sub		
	br.s		$L6512
$L6250:
	ldarg.0				; _this$
	call		??1BArchivable@@$$FUAE@XZ
	ldarg.1				; ___flags$
	ldc.i.1		1		; u32 0x1
	and		
	brfalse.s	$L6253
	ldarg.0				; _this$
	call		??3@$$FYAXPAX@Z
$L6253:
	ldarg.0				; _this$
	br.s		$L6512
$L6512:
	ret		
 .end ??_EBArchivable@@$$FUAEPAXI@Z			; BArchivable::`vector deleting destructor'
;	.proc.end.i4
text	ENDS
PUBLIC	__mep@??_EBArchivable@@$$FUAEPAXI@Z
;	COMDAT __mep@??_EBArchivable@@$$FUAEPAXI@Z
data	SEGMENT
__mep@??_EBArchivable@@$$FUAEPAXI@Z TOKEN 0600003A
; Function compile flags: /Odt
data	ENDS
;	COMDAT ??_EBArchivable@@UAEPAXI@Z
text	SEGMENT
??_EBArchivable@@UAEPAXI@Z PROC NEAR			; BArchivable::`vector deleting destructor', COMDAT
	jmp	DWORD PTR __mep@??_EBArchivable@@$$FUAEPAXI@Z
??_EBArchivable@@UAEPAXI@Z ENDP				; BArchivable::`vector deleting destructor'
.global	
	.bss
.local	,4
; Function compile flags: /Odt
	.text
.global	??0BArchivable@@$$FQAE@PBVBMessage@@@Z		; BArchivable::BArchivable
??0BArchivable@@$$FQAE@PBVBMessage@@@Z:			; BArchivable::BArchivable
;	.proc.def	D:I(II)

; Function Header:
; max stack depth = 2
; function size = 12 bytes
; local varsig tk = 0x0 
; Exception Information:
; 0 handlers, each consisting of filtered handlers

;	.formal.i4 0,"_this$" SIG: Optional C Binding Modifier(token:0x9CA6CA).Optional C Binding Modifier(token:0x9CA6CC).ptr.valueType (token:0x9CA6CF)

;	.proc.beg

; 42   : {

	ldarg.0				; _this$
	ldsflda		??_7BArchivable@@6B@
	unaligned	1		; u32 0x1
	stind.i4	

; 43   : }

	ldarg.0				; _this$
$L6525:
	ret		
 .end ??0BArchivable@@$$FQAE@PBVBMessage@@@Z		; BArchivable::BArchivable
;	.proc.end.i4
text	ENDS
PUBLIC	??0BArchivable@@QAE@PBVBMessage@@@Z		; BArchivable::BArchivable
PUBLIC	__mep@??0BArchivable@@$$FQAE@PBVBMessage@@@Z
;	COMDAT __mep@??0BArchivable@@$$FQAE@PBVBMessage@@@Z
data	SEGMENT
__mep@??0BArchivable@@$$FQAE@PBVBMessage@@@Z TOKEN 0600003B
; Function compile flags: /Odt
data	ENDS
;	COMDAT ??0BArchivable@@QAE@PBVBMessage@@@Z
text	SEGMENT
??0BArchivable@@QAE@PBVBMessage@@@Z PROC NEAR		; BArchivable::BArchivable, COMDAT
	jmp	DWORD PTR __mep@??0BArchivable@@$$FQAE@PBVBMessage@@@Z
??0BArchivable@@QAE@PBVBMessage@@@Z ENDP		; BArchivable::BArchivable
; Function compile flags: /Odt
	.text
.global	??1BArchivable@@$$FUAE@XZ			; BArchivable::~BArchivable
??1BArchivable@@$$FUAE@XZ:				; BArchivable::~BArchivable
;	.proc.def	D:V(I)

; Function Header:
; max stack depth = 2
; function size = 11 bytes
; local varsig tk = 0x0 
; Exception Information:
; 0 handlers, each consisting of filtered handlers

;	.formal.i4 0,"_this$" SIG: Optional C Binding Modifier(token:0x9CA6CA).Optional C Binding Modifier(token:0x9CA6CC).ptr.valueType (token:0x9CA6CF)

;	.proc.beg

; 47   : {

	ldarg.0				; _this$
	ldsflda		??_7BArchivable@@6B@
	unaligned	1		; u32 0x1
	stind.i4	
$L6533:

; 48   : }

	ret		
 .end ??1BArchivable@@$$FUAE@XZ				; BArchivable::~BArchivable
;	.proc.end.void
text	ENDS
PUBLIC	__mep@??1BArchivable@@$$FUAE@XZ
;	COMDAT __mep@??1BArchivable@@$$FUAE@XZ
data	SEGMENT
__mep@??1BArchivable@@$$FUAE@XZ TOKEN 0600003C
; Function compile flags: /Odt
data	ENDS
;	COMDAT ??1BArchivable@@UAE@XZ
text	SEGMENT
??1BArchivable@@UAE@XZ PROC NEAR			; BArchivable::~BArchivable, COMDAT
	jmp	DWORD PTR __mep@??1BArchivable@@$$FUAE@XZ
??1BArchivable@@UAE@XZ ENDP				; BArchivable::~BArchivable
.global	
.global	
.global	
	.bss
.local	,4
.local	,4
.local	,4
; Function compile flags: /Odt
	.text
.global	?Archive@BArchivable@@$$FUBEHPAVBMessage@@_N@Z	; BArchivable::Archive
?Archive@BArchivable@@$$FUBEHPAVBMessage@@_N@Z:		; BArchivable::Archive
;	.proc.def	D:I(III)

; Function Header:
; max stack depth = 3
; function size = 27 bytes
; local varsig tk = 0x0 
; Exception Information:
; 0 handlers, each consisting of filtered handlers

;	.formal.i4 1,"_into$" SIG: ptr.valueType (token:0x9B15D3)

;	.proc.beg

; 54   : 	if(into == NULL) return B_ERROR;

	ldarg.1				; _into$
	brtrue.s	$L6267
	ldc.i.-1	-1		; i32 0xffffffff
	br.s		$L6540
$L6267:

; 55   : 	into->AddString("class", "BArchivable");

	ldarg.1				; _into$
	ldsflda		$SG6270
	ldsflda		$SG6268
	call		?AddString@BMessage@@$$FQAE_NPBD0@Z
	pop		

; 56   : 	return B_OK;

	ldc.i.1		1		; i32 0x1
	br.s		$L6540
$L6540:

; 57   : }

	ret		
 .end ?Archive@BArchivable@@$$FUBEHPAVBMessage@@_N@Z	; BArchivable::Archive
;	.proc.end.i4
text	ENDS
PUBLIC	__mep@?Archive@BArchivable@@$$FUBEHPAVBMessage@@_N@Z
;	COMDAT __mep@?Archive@BArchivable@@$$FUBEHPAVBMessage@@_N@Z
data	SEGMENT
__mep@?Archive@BArchivable@@$$FUBEHPAVBMessage@@_N@Z TOKEN 0600003D
; Function compile flags: /Odt
data	ENDS
;	COMDAT ?Archive@BArchivable@@UBEHPAVBMessage@@_N@Z
text	SEGMENT
?Archive@BArchivable@@UBEHPAVBMessage@@_N@Z PROC NEAR	; BArchivable::Archive, COMDAT
	jmp	DWORD PTR __mep@?Archive@BArchivable@@$$FUBEHPAVBMessage@@_N@Z
?Archive@BArchivable@@UBEHPAVBMessage@@_N@Z ENDP	; BArchivable::Archive
	.bss
.local	$VarArgTok$1$05022009010f202d203504410f202d203504,4
; Function compile flags: /Odt
	.text
.global	?Instantiate@BArchivable@@$$FSAPAV1@PBVBMessage@@@Z ; BArchivable::Instantiate
?Instantiate@BArchivable@@$$FSAPAV1@PBVBMessage@@@Z:	; BArchivable::Instantiate
;	.proc.def	D:I(I)

; Function Header:
; max stack depth = 2
; function size = 19 bytes
; local varsig tk = 0x0 
; Exception Information:
; 0 handlers, each consisting of filtered handlers


;	.proc.beg

; 63   : 	BHAPI_ERROR("[SUPPORT]: %s --- Can't instantiate a \"BArchivable\"", __PRETTY_FUNCTION__);

	ldsflda		$SG6281
	ldsflda		$SG6279
	call		?BHAPI_ERROR@@$$J0YAXPBDZZ

; 64   : 	return NULL;

	ldc.i.0		0		; i32 0x0
	br.s		$L6555
$L6555:

; 65   : }

	ret		
 .end ?Instantiate@BArchivable@@$$FSAPAV1@PBVBMessage@@@Z ; BArchivable::Instantiate
;	.proc.end.i4
text	ENDS
PUBLIC	?Instantiate@BArchivable@@SAPAV1@PBVBMessage@@@Z ; BArchivable::Instantiate
PUBLIC	__mep@?Instantiate@BArchivable@@$$FSAPAV1@PBVBMessage@@@Z
;	COMDAT __mep@?Instantiate@BArchivable@@$$FSAPAV1@PBVBMessage@@@Z
data	SEGMENT
__mep@?Instantiate@BArchivable@@$$FSAPAV1@PBVBMessage@@@Z TOKEN 0600003E
; Function compile flags: /Odt
data	ENDS
;	COMDAT ?Instantiate@BArchivable@@SAPAV1@PBVBMessage@@@Z
text	SEGMENT
?Instantiate@BArchivable@@SAPAV1@PBVBMessage@@@Z PROC NEAR ; BArchivable::Instantiate, COMDAT
	jmp	DWORD PTR __mep@?Instantiate@BArchivable@@$$FSAPAV1@PBVBMessage@@@Z
?Instantiate@BArchivable@@SAPAV1@PBVBMessage@@@Z ENDP	; BArchivable::Instantiate
.global	
.global	
	.bss
.local	,4
.local	,4
.local	$T6570,4
; Function compile flags: /Odt
	.text
.global	?b_validatb_instantiation@@$$FYA_NPBVBMessage@@PBD@Z ; b_validatb_instantiation
?b_validatb_instantiation@@$$FYA_NPBVBMessage@@PBD@Z:	; b_validatb_instantiation
;	.proc.def	D:I(II)

; Function Header:
; max stack depth = 4
; function size = 98 bytes
; local varsig tk = 0x1100000A 
; Exception Information:
; 0 handlers, each consisting of filtered handlers

;	.formal.i4 0,"_from$" SIG: ptr.Optional C Binding Modifier(token:0x9CA0E3).valueType (token:0x9CA0E5)
;	.formal.i4 1,"_class_name$" SIG: ptr.Optional C Binding Modifier(token:0x9CB3DB).Optional C Binding Modifier(token:0x9CB3DD).i1
;	.local.i4 2,"_items$"
;	.local.i4 0,"_i$6291"
;	.local.i4 1,"__class_name$6295" SIG: ptr.Optional C Binding Modifier(token:0x9CB3DB).Optional C Binding Modifier(token:0x9CB3DD).i1

;	.proc.beg

; 70   : 	if(from == NULL || class_name == NULL || *class_name == 0) return false;

	ldarg.0				; _from$
	brfalse.s	$L6288
	ldarg.1				; _class_name$
	brfalse.s	$L6288
	ldarg.1				; _class_name$
	ldind.i1	
	brtrue.s	$L6287
$L6288:
	ldc.i.0		0		; u8 0x0
	br.s		$L6567
$L6287:

; 71   : 
; 72   : 	b_int32 items = from->CountItems("class", B_STRING_TYPE);

	ldarg.0				; _from$
	ldsflda		$SG6290
	ldc.i4		1129534546	; u32 0x43535452
	call		?CountItems@BMessage@@$$FQBEHPBDI@Z
	stloc.2				; _items$

; 73   : 	for(b_int32 i = 0; i < items; i++)

	ldc.i.0		0		; i32 0x0
	stloc.0				; _i$6291
	br.s		$L6292
$L6293:
	ldloc.0				; _i$6291
	ldc.i.1		1		; i32 0x1
	add		
	stloc.0				; _i$6291
$L6292:
	ldloc.0				; _i$6291
	ldloc.2				; _items$
	bge.s		$L6294

; 74   : 	{
; 75   : 		const char *_class_name = NULL;

	ldc.i.0		0		; i32 0x0
	stloc.1				; __class_name$6295

; 76   : 		from->FindString("class", i, &_class_name);

	ldarg.0				; _from$
	ldsflda		$SG6296
	ldloc.0				; _i$6291
	ldloca.s	1		; __class_name$6295
	call		?FindString@BMessage@@$$FQBE_NPBDHPAPBD@Z
	pop		

; 77   : 		if(_class_name == NULL) continue;

	ldloc.1				; __class_name$6295
	brtrue.s	$L6297
	br.s		$L6293
$L6297:

; 78   : 
; 79   : 		if(strlen(_class_name) != strlen(class_name)) continue;

	ldloc.1				; __class_name$6295
	call		?strlen@@$$J0YAIPBD@Z
	ldarg.1				; _class_name$
	call		?strlen@@$$J0YAIPBD@Z
	beq.s		$L6298
	br.s		$L6293
$L6298:

; 80   : 		if(strcmp(_class_name, class_name) == 0) return true;

	ldloc.1				; __class_name$6295
	ldarg.1				; _class_name$
	call		?strcmp@@$$J0YAHPBD0@Z
	brtrue.s	$L6299
	ldc.i.1		1		; u8 0x1
	br.s		$L6567
$L6299:

; 81   : 	}

	br.s		$L6293
$L6294:

; 82   : 
; 83   : 	return false;

	ldc.i.0		0		; u8 0x0
	br.s		$L6567
$L6567:

; 84   : }

	ret		
 .end ?b_validatb_instantiation@@$$FYA_NPBVBMessage@@PBD@Z ; b_validatb_instantiation
;	.proc.end.u1
text	ENDS
PUBLIC	?b_validatb_instantiation@@YA_NPBVBMessage@@PBD@Z ; b_validatb_instantiation
PUBLIC	__mep@?b_validatb_instantiation@@$$FYA_NPBVBMessage@@PBD@Z
;	COMDAT __mep@?b_validatb_instantiation@@$$FYA_NPBVBMessage@@PBD@Z
data	SEGMENT
__mep@?b_validatb_instantiation@@$$FYA_NPBVBMessage@@PBD@Z TOKEN 0600003F
; Function compile flags: /Odt
data	ENDS
;	COMDAT ?b_validatb_instantiation@@YA_NPBVBMessage@@PBD@Z
text	SEGMENT
?b_validatb_instantiation@@YA_NPBVBMessage@@PBD@Z PROC NEAR ; b_validatb_instantiation, COMDAT
	jmp	DWORD PTR __mep@?b_validatb_instantiation@@$$FYA_NPBVBMessage@@PBD@Z
?b_validatb_instantiation@@YA_NPBVBMessage@@PBD@Z ENDP	; b_validatb_instantiation
; Function compile flags: /Odt
	.text
.global	?b_find_instantiation_func@@$$FYAP6APAVBArchivable@@PBVBMessage@@@ZPBD@Z ; b_find_instantiation_func
?b_find_instantiation_func@@$$FYAP6APAVBArchivable@@PBVBMessage@@@ZPBD@Z: ; b_find_instantiation_func
;	.proc.def	D:I(I)

; Function Header:
; max stack depth = 1
; function size = 4 bytes
; local varsig tk = 0x0 
; Exception Information:
; 0 handlers, each consisting of filtered handlers


;	.proc.beg

; 89   : 	// TODO
; 90   : 	return NULL;

	ldc.i.0		0		; i32 0x0
	br.s		$L6582
$L6582:

; 91   : }

	ret		
 .end ?b_find_instantiation_func@@$$FYAP6APAVBArchivable@@PBVBMessage@@@ZPBD@Z ; b_find_instantiation_func
;	.proc.end.i4
text	ENDS
PUBLIC	?b_find_instantiation_func@@YAP6APAVBArchivable@@PBVBMessage@@@ZPBD@Z ; b_find_instantiation_func
PUBLIC	__mep@?b_find_instantiation_func@@$$FYAP6APAVBArchivable@@PBVBMessage@@@ZPBD@Z
;	COMDAT __mep@?b_find_instantiation_func@@$$FYAP6APAVBArchivable@@PBVBMessage@@@ZPBD@Z
data	SEGMENT
__mep@?b_find_instantiation_func@@$$FYAP6APAVBArchivable@@PBVBMessage@@@ZPBD@Z TOKEN 06000040
; Function compile flags: /Odt
data	ENDS
;	COMDAT ?b_find_instantiation_func@@YAP6APAVBArchivable@@PBVBMessage@@@ZPBD@Z
text	SEGMENT
?b_find_instantiation_func@@YAP6APAVBArchivable@@PBVBMessage@@@ZPBD@Z PROC NEAR ; b_find_instantiation_func, COMDAT
	jmp	DWORD PTR __mep@?b_find_instantiation_func@@$$FYAP6APAVBArchivable@@PBVBMessage@@@ZPBD@Z
?b_find_instantiation_func@@YAP6APAVBArchivable@@PBVBMessage@@@ZPBD@Z ENDP ; b_find_instantiation_func
	.bss
.local	$T6595,4
; Function compile flags: /Odt
	.text
.global	?b_find_instantiation_func@@$$FYAP6APAVBArchivable@@PBVBMessage@@@Z0@Z ; b_find_instantiation_func
?b_find_instantiation_func@@$$FYAP6APAVBArchivable@@PBVBMessage@@@Z0@Z: ; b_find_instantiation_func
;	.proc.def	D:I(I)

; Function Header:
; max stack depth = 5
; function size = 49 bytes
; local varsig tk = 0x1100000B 
; Exception Information:
; 0 handlers, each consisting of filtered handlers

;	.formal.i4 0,"_archive_data$" SIG: ptr.Optional C Binding Modifier(token:0x9CA0E3).valueType (token:0x9CA0E5)
;	.local.i4 0,"_class_name$" SIG: ptr.Optional C Binding Modifier(token:0x9CB3DB).Optional C Binding Modifier(token:0x9CB3DD).i1

;	.proc.beg

; 96   : 	const char *class_name = NULL;

	ldc.i.0		0		; i32 0x0
	stloc.0				; _class_name$

; 97   : 	if(archive_data == NULL) return NULL;

	ldarg.0				; _archive_data$
	brtrue.s	$L6307
	ldc.i.0		0		; i32 0x0
	br.s		$L6594
$L6307:

; 98   : 	archive_data->FindString("class", archive_data->CountItems("class", B_STRING_TYPE) - 1, &class_name);

	ldarg.0				; _archive_data$
	ldsflda		$SG6309
	ldarg.0				; _archive_data$
	ldsflda		$SG6308
	ldc.i4		1129534546	; u32 0x43535452
	call		?CountItems@BMessage@@$$FQBEHPBDI@Z
	ldc.i.1		1		; i32 0x1
	sub		
	ldloca.s	0		; _class_name$
	call		?FindString@BMessage@@$$FQBE_NPBDHPAPBD@Z
	pop		

; 99   : 	return b_find_instantiation_func(class_name);

	ldloc.0				; _class_name$
	call		?b_find_instantiation_func@@$$FYAP6APAVBArchivable@@PBVBMessage@@@ZPBD@Z
	br.s		$L6594
$L6594:

; 100  : }

	ret		
 .end ?b_find_instantiation_func@@$$FYAP6APAVBArchivable@@PBVBMessage@@@Z0@Z ; b_find_instantiation_func
;	.proc.end.i4
text	ENDS
PUBLIC	?b_find_instantiation_func@@YAP6APAVBArchivable@@PBVBMessage@@@Z0@Z ; b_find_instantiation_func
PUBLIC	__mep@?b_find_instantiation_func@@$$FYAP6APAVBArchivable@@PBVBMessage@@@Z0@Z
;	COMDAT __mep@?b_find_instantiation_func@@$$FYAP6APAVBArchivable@@PBVBMessage@@@Z0@Z
data	SEGMENT
__mep@?b_find_instantiation_func@@$$FYAP6APAVBArchivable@@PBVBMessage@@@Z0@Z TOKEN 06000041
; Function compile flags: /Odt
data	ENDS
;	COMDAT ?b_find_instantiation_func@@YAP6APAVBArchivable@@PBVBMessage@@@Z0@Z
text	SEGMENT
?b_find_instantiation_func@@YAP6APAVBArchivable@@PBVBMessage@@@Z0@Z PROC NEAR ; b_find_instantiation_func, COMDAT
	jmp	DWORD PTR __mep@?b_find_instantiation_func@@$$FYAP6APAVBArchivable@@PBVBMessage@@@Z0@Z
?b_find_instantiation_func@@YAP6APAVBArchivable@@PBVBMessage@@@Z0@Z ENDP ; b_find_instantiation_func
text	ENDS
END
