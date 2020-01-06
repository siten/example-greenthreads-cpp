.CODE

Myswitch PROC
    mov [rsp + 8],rcx
    mov [rsp + 16], rdx
    mov rax, [rsp + 8]
    mov rcx, [rsp + 16]
    
    mov [rax], rsp
    mov [rax + 8], r15
    mov [rax + 10h], r14
    mov [rax + 18h], r13
    mov [rax + 20h], r12
    mov [rax + 28h], rbx
    mov [rax + 30h], rbp
    
    mov rsp, [rcx]
    mov r15, [rcx + 8]
    mov r14, [rcx + 10h]
    mov r13, [rcx + 18h]
    mov r12, [rcx + 20h]
    mov rbx, [rcx + 28h]
    mov rbp, [rcx + 30h]
    ret
Myswitch ENDP

END