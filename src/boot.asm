bits 16
org 0x7c00

;  label KernelErr
toto:

cli ; запрещаем прерывания
xor ax, ax
mov ds, ax  ; зануляем регистры
mov es, ax
mov ss, ax

mov ah, 0x00
mov al, 0x03
int 0x10

mov bx, MsgGreet
call print

;чтение ядра
mov ah, 0x02 ;чтение секторов
mov al, 36 ; количество секторов
mov ch, 0 ; цилиндр
mov cl, 2 ; сектор (номер начала 1 - boot 2- kernel)
mov dh, 0 ; головка
mov dl, 0x0 ; первый floppy
mov bx, 0x1000 ; адрес
int 0x13 ; прерывание

jc .KernelErr

test ah, ah
jnz .KernelErr

push es
push bx

mov bx, MsgKernel
call print

jmp 0x0000:0x1000

.KernelErr:
mov bx, MsgKernelErr
call print

jmp $

print:
        ;bx указывает на строку
.loop:
        mov al, [bx]
        test al, al
        jz .done
        inc bx
        mov ah, 0x0e
        int 0x10

        jmp .loop

.done:
        ret

MsgGreet db 'Bootloader v0.1 started...', 0
MsgKernel db 'Attempting to load kernel...', 0
MsgKernelErr db 'Error while loading kernel. Entering infinite loop...', 0
times 510 - ($-$$) db 0

dw 0xAA55

