org 0x7C00	;directive to proper memory calcualation
bits 16 	;working in 16bits mode

main:
	mov ax, 0	
	mov ds, ax		;initialize data segment registers
	mov es, ax
	mov ss, ax		;initialize stack segment and pointer
	mov sp, 0x7BFF

	mov si, MESS
	call print_string
	jmp $

;!Before calling function put *string[0] to si register!
print_string:
	push ax				;push registers that will be used to stack
	push si
	mov al, [si]		
	cmp al, 0			;check if we reached end of string
	jne print_char		;if we didnt reached end char,
						;type the char using BIOS and increment SI address by 1
	pop si				;pull back registers values
	pop ax
	ret

	print_char:
		mov ah, 0x0E
		int 0x10
		add si, 1
		jmp print_string

MESS: db "Hello World!", 0


times 510-($-$$) db 0
dw 0xAA55 
