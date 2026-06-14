    global main
    extern printf

    section .text
strcmp:
    test rdi, rdi
    jz strcmp_fail
    test rsi, rsi
    jz strcmp_fail
strcmp_main:
    mov al, [rdi]
    cmp al, [rsi]
    jne strcmp_fail
    
    test al, al
    jz strcmp_pass
    
    add rdi, 1
    add rsi, 1
    jmp strcmp_main
strcmp_pass:
    mov rax, 0
    ret
strcmp_fail:
    mov rax, 1
    ret

    
main:
    push r12
    push r13
    push r14
    push r15
    sub rsp, 8
    mov r12, rdi ; argc
    mov r13, rsi ; argv

    add r13, 8 ; remove program name
    dec r12
    jz end

    lea rdi, [rel flagh]
    mov rsi, [r13]
    call strcmp
    mov r14, rax
    test r14, r14
    jz help

    lea rdi, [rel flagn]
    mov rsi, [r13]
    call strcmp
    mov r15, rax
    test r15, r15
    jnz print
    add r13, 8
    dec r12

print:
    lea rdi, [rel format]
    cmp r12, 1
    je print_help
    lea rdi, [rel formatspace]

print_help:
    mov rsi, [r13]
    xor rax, rax
    call printf wrt ..plt

    add r13, 8
    dec r12
    jnz print

end:
    test r15, r15
    jz clean
    lea rdi, [rel newline]
    xor rax, rax
    call printf wrt ..plt

clean:
    add rsp, 8
    pop r15
    pop r14
    pop r13
    pop r12

    xor rax, rax
    ret

help:
    lea rdi, [rel helpmessage]
    xor rax, rax
    call printf wrt ..plt
    mov r15, 1
    jmp end

    section .rodata
flagn db "-n", 0
flagh db "--help", 0
helpmessage db "Usage: echo [-n] [--help] [STRING]...", 0
format db "%s", 0
formatspace db "%s ", 0
newline db 10, 0
