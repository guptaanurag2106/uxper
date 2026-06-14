    global main
    extern printf

    section .text
main:
    push rbx
    
    add rsi, 8 ; get input path argv++
    dec rdi ; argc--
    test rdi, rdi
    jz end_usage
    mov r9, rsi ; copy argv[1] pointer

open:
    mov rax, 2
    mov rdi, [r9]
    mov rsi, 0
    syscall
    cmp rax, 0
    jle end_fread
    mov r10, rax ; r10=fd

    mov byte [rel lastWPos], 1 ; last char not space

read:
    mov rax, 0
    mov rdi, r10
    lea rsi, [rel buffer]
    mov rdx, bufSize
    syscall
    cmp rax, 0
    jl end_fread
    je close

    lea r11, [rel buffer]

loop:
    mov dl, [r11]
    cmp dl, 32 ; if curren_char = <space>/newline and last char not space/newline then word++,laswpos=curr+1
    sete r8b
    cmp dl, 10
    sete bl
    or r8b, bl
    cmp byte [rel lastWPos], 1
    sete bl
    or r8b, bl
    movzx r8, r8b
    test r8, r8
    jnz continue

    inc [rel words]

continue:
    mov dl, [r11]
    cmp dl, 32 ; if curren_char = <space>/newline and last char not space/newline then word++,laswpos=curr+1
    setne r8b
    cmp dl, 10
    setne bl
    and r8b, bl
    mov byte [rel lastWPos], r8b

    inc [rel char]

    cmp byte [r11], 10
    mov rdi, [rel lines]
    mov r8, rdi
    lea r8, [rdi + 1] ; can't use inc r8 as inc modifies flags
    cmove rdi, r8
    mov [rel lines], rdi

    dec rax
    inc r11
    test rax, rax ; rax=length of read bytes
    jz read
    jmp loop

close:
    mov rax, 3
    mov rdi, r10
    syscall

    cmp [rel char], 0
    mov rdi, [rel words]
    mov r8, rdi
    lea r8, [rdi+1]
    cmovne rdi, r8
    mov [rel words], rdi

print:
    lea rdi, [rel format]
    mov rsi, [rel lines]
    mov rdx, [rel words]
    mov rcx, [rel char]
    mov r8, [r9]
    xor al, al
    call printf wrt ..plt

end_success:
    pop rbx
    xor rax, rax
    ret

end_usage:
    lea rdi, [rel argErrorMsg]
    xor al, al
    call printf wrt ..plt
    pop rbx
    mov rax, 1
    ret

end_fread:
    lea rdi, [rel fileErrorMsg]
    mov rsi, [r9]
    xor al, al
    call printf wrt ..plt
    pop rbx
    mov rax, 1
    ret

    section .bss
lines: resq 1
words: resq 1
char: resq 1
buffer: resb bufSize
lastWPos: resb 1

    section .rodata
format db "%d %d %d %s", 10, 0
fileErrorMsg db "Error while reading %s", 10, 0
argErrorMsg db "Usage: wc <file_path>", 10, 0
bufSize equ 1024

 ; 10  25 155 misc.c
