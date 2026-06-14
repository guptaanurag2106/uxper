;; Hello NASM nasm -felf64 misc.asm && ld -o misc misc.o && ./misc
;     global _start ; default entry point name for ld, otherwise use ld -e <entry_point>
;
;     section .text
; _start:
; mov rdi, 1
; mov rax, 1
; mov rsi, message
; mov rdx, message_len
; syscall
; mov rax, 60
; xor rdi, rdi
; syscall
;
;
;     section .data
; message: db "Hello NASM", 10
; message_len: equ $ - message

;----------------------------------------------------------------------------------------

;; Stars nasm -felf64 misc.asm && gcc -o misc misc.c misc.o && ./misc
global stars

    section .text
stars:
    push r12 ; saving r12, r13
    push r13

    mov r13, rdi

    mov r11, r13 ; 2+3+...maxLines (* and \n) (maxLines+1)(maxLines+2)/2-1
    inc r11
    mov r12, r11
    inc r12
    mul r12, r11
    shr r12, 1
    dec r12 ; size in r14

    cmp r12, 0
    jle exit

    mov rax, 9
    xor rdi, rdi
    mov rsi, r12 ; size in r14
    mov rdx, 3 ; PROT_READ | PROT_WRITE 1 | 2
    mov r10, 34 ; flags (MAP_PRIVATE | MAP_ANONYMOUS = 0x02 | 0x20)
    mov r8, -1 ; fd (ignored for anonymous maps)
    xor r9, r9 ; offset (0 for anonymous maps)
    syscall

    test rax, rax ; -ve on failure
    js exit_failure
    mov r11, rax ; buffer in r13

    mov rdx, r11
    mov r8, 0 ; line number
    mov r9, 0 ; number of * on curr line

line:
    mov r9, 0
star:
    mov byte [rdx], '*'
    inc rdx
    inc r9
    cmp r9, r8
    jle star
    mov byte [rdx], 10
    inc rdx

    inc r8
    cmp r8, r13
    jle line

end: 
    mov rax, 1
    mov rdi, 1
    mov rsi, r11
    mov rdx, r12
    syscall

exit:
    pop r13
    pop r12
    mov rax, 0
    ret

exit_failure:
    pop r13
    pop r12
    mov rax, 1
    ret
