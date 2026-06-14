;; Fibonacci nasm -felf64 fibonacci.asm && gcc -o fibonacci fibonacci.o && ./fibonacci <n>
    global main
    extern printf
    extern atoi

    section .text
main:
    push rbx ; save this calling convention
    sub rsp, 8

    cmp rdi, 1
    je default_param
    mov rdi, [rsi+8]
    call atoi wrt ..plt
    mov rcx, rax
    jmp start

default_param:
    mov rcx, 50
start:
    xor rax, rax
    xor rbx, rbx
    inc rbx ; so rax current number, rbx next

print:
    push rax
    push rcx

    lea rdi, [rel format] ; set param 1
    mov rsi, rax ; set param 2 current number
    xor rax, rax ; last for vaargs

    call printf wrt ..plt

    pop rcx
    pop rax

    mov rdx, rax
    mov rax, rbx
    add rbx, rdx
    dec rcx
    jnz print

    add rsp, 8
    pop rbx
    xor eax, eax
    ret


    section .rodata
format db "%15ld", 10, 0
