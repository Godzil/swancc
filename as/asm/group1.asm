	ADD	AL,#3
	ADD	AX,#$1234
	ADD	EAX,#$12345678
	ADD	BL,#3
	ADD	BX,#$1234
	ADD	EBX,#$12345678
	ADD	BYTE [BX],#3
	ADD	BYTE 3[BX],#4
	ADD	BYTE [BX+SI],#4
	ADD	WORD [BX],#$1234
	ADD	DWORD [BX],#$12345678
	ADD	BYTE [BX],#3
	ADD	WORD [BX],#-3
	ADD	DWORD [BX],#-3
	ADD	CL,BL
	ADD	CX,BX
	ADD	ECX,EBX
	ADD	[BX],CL
	ADD	[BX],CX
	ADD	[BX],ECX
	ADD	CL,[BX]
	ADD	CX,[BX]
	ADD	ECX,[BX]

	ADC	CL,BL
	AND	CL,BL
	CMP	CL,BL
	OR	CL,BL
	SUB	CL,BL
	SBB	CL,BL
	XOR	CL,BL
