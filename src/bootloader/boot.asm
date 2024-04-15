org 0x7C00	;directive to proper memory calcualation
bits 16 	;working in 16bits mode

%define ENDL 0x0D, 0x0A

;fat header BPB (BIOS parameter block)
jmp_short: 				jmp short main 				;it jumps over header to not start executing it as code (3 bytes)
nop
oem_id:					db "MSWIN4.1"				;OEM identifier MSWIN4.1 value recomended (8 bytes)
no_bytes_per_sector:	dw 0x0200					;number of bytes per sector, 512 in our case (2 bytes)
no_sectors_per_cluster: db 0x01						;number of sectors per cluster, 1 in our case (1 byte)
no_res_sectors:			dw 0x0001					;number of reserved sectors, 1 in ou]r case (1 byte)
no_FATS:				db 0x02						;number of file allocation tables on the storage, often its 2 (1 byte)
no_root_dir_entries:	dw 0x00E0					;number of root directory entries, root dir must occupie entire sectors (2 bytes)
no_total_sectors:		dw 0x0B40					;number of total sectors in logical volume 2880*512 = 1.44MB
media_type:				db 0xF0						;media type F0 is floppy 3.5'' (1 byte)
no_sectors_per_FAT:		dw 0x0009					;number of sectors per FAT (2 bytes)
no_sectors_per_track:   dw 0x0012					;number of sectors per track (2 bytes)
no_heads_or_sides:		dw 0x0002					;number of heads or sides on storage (2 bytes)
no_hidden_sectors:		db 0, 0, 0, 0				;number of hidden sectors (4 bytes)
no_large_sectors:		db 0, 0, 0, 0				;number of large sectors (4 bytes)

;EBR (Extended Boot Record)
drive_number: 			db 0x00						;drive number (1 byte)
win_flag_res:			db 0x00						;Flags in windows NT otherwise its reserved (1 byte)
signature:				db 0x29						;signature 0x29 or 0x28 (1byte)
volume_id:				db 0xDE, 0xAF, 0xF3, 0x2D	;Volume serial number
volume_label:			db "FLOPPY DISK"			;Volume label (11 bytes)
sys_id:					db "FAT12   "				;System identifier string (8 bytes)




main:
	mov ax, 0
	mov es, ax				;es = 0
	mov ds, ax				;initialize data segment to 0
	mov ss, ax				;initialize stack segment to 0
	mov sp, 0x7C00			;initialize stack pointer to 0x7C00			
	
	mov [drive_number], dl 	;write to memory disk number thats been booted

	mov si, MESS
	call print_string		;print string if bootloader loaded itself

	mov ax, 0x1				;ax = LBA where to start read
	mov dl, 1				;dl = 1 sector read count
	mov bx, 0x7E00			;es = 0 memory write to 0x0000:0x7E00
	call read_disk
	jmp $


;BIOS READ FUNCTION
;Parameters:
;	ah = 0x02
;	al - how many sectors to read
;x 	ch - cyllinder/track
;x	cl - sector
;x	dh - head
;x	dl - drive
; 	es:bx - address buffer pointer
;FLAGS:
;	if read was successfull CF = 0 else 1
;	ah - return code
;	al - actual sectors read count
;RETURN:
;	NONE

;Function for reading disk implementation
;Parameters:
;	ax - LBA start address (1-2880)
;	es:bx - memory location to store the data
;	dl - how many sectors to read 1-127
read_disk:
	call lba_chs 			;cl->SECTOR, ch->CYLLINDER, dh->HEAD
	push dx
	xor dh, dh				;set dh->0
	mov al, dl				;al->SECTOR_READ COUNT
	mov dl, [drive_number] 	;dl->DRIVE_NUMBER

	retry_read:
		stc					;set carry to 1
		mov ah, 0x02		;BIOS interrupt number for read
		int 0x13			;execute interrupt CF = 1 on error
							;ah= Return Code
							;al= Actual sectors read
		jc read_bad


	read_good:
		pop dx
		mov si, READ_OK
		call print_string
		ret

	read_bad:				
		mov si, READ_ERROR
		call print_string

		cmp dh, 0x3				;check retry attempt
		add dh, 0x1
		pop ax					;get dx to ax
		push ax					;copy again dx
		jnle retry_read			;if dh was less or equal 3 retry read
		pop dx
		jmp $					;if read wasnt successfuull halt			

;Function for converting LBA address of storage media to CHS (Logical Block Addressing ->Cyllinder->Head->Sector addressing)
;Parameters:
; 	ax - LBA address
;RETURNS:
;	ch - CYLLINDER/TRACK
;	cl - SECTOR
;	dh - Head
;Max_Track = 80 	< 	8bits
;Max_Head = 2 		< 	8bits
;Max_Sector = 18 	< 	8bits
;al - quotient
;ah - remainder
;Cyllinder = LBA / (Heads_Per_Cyllinder * Sector_Per_Track)
;Head = (LBA / Sectors_Per_Track) % Heads_Per Cyllinder
;Sector = (LBA % Sectors_per_Track) + 1
lba_chs:
	push bx				

	mov bl, [no_sectors_per_track] 	;bl=Sectors_Per_Track             12
	
	div bl						   	;al=LBA/Sectors_Per_Track=Temp
									;ah=LBA%Sectors_Per_Track
	add ah, 0x1						;ah=LBA%Sectors_Per_Track+1=SECTOR
	mov cl, ah						;cl = SECTOR

	xor ah, ah						;ah=0
	mov bl, [no_heads_or_sides]
	div bl						 	;al=Temp/Heads=CYLLINDER/TRACK    2
									;ah=Temp%Heads= HEAD
	mov ch, al						;ch = CYLLINDER/TRACK
	mov dh, ah						;dh = HEAD


	pop bx
	ret

;Function for printing string
;Parameters:
;	si - address to string start
print_string:
	push ax				;push registers that will be used to stack
	push si

	print_loop:
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
		jmp print_loop

MESS: db "Bootloader Loaded...", ENDL, 0
READ_OK: db "READ OK...", ENDL, 0
READ_ERROR: db "READ ERROR...", ENDL, 0

times 510-($-$$) db 0
dw 0xAA55 
