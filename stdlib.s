section .text


CallRead        equ 0
CallWrite       equ 1

StdIn           equ 0
StdOut          equ 1

BufSize         equ 12

MaxSqrtIter     equ 100
SqrtEpsilon     equ 5

;----------------------------------------
; Reads int32 in RAX
;----------------------------------------
; Enter:        None
; Exit:         RAX = integer
; Destr:        RAX, RBX, RCX, RDX, R11, RDI, RSI
;----------------------------------------

FUNC_22B14C_00076DC0:
    ; Allocate buffer
    sub rsp, BufSize

    ; Read in buffer
    mov rdx, BufSize
    mov rax, CallRead
    mov rdi, StdIn
    mov rsi, rsp
    syscall

    ; Set rdi to buffer
    mov rdi, rsp

    ; Clear RAX, RCX, RSI, RDX
    xor rax, rax
    xor rcx, rcx

    ; Get number sign
    xor r11, r11
    mov cl, [rdi]

    cmp cl, '-'
    jne .Loop

    inc r11                 ; Remember number sign in R11
    inc rdi                 ; To next symbol

.Loop:
    ; Get digits          
    mov cl, [rdi]
    inc rdi

    ; Check for '.'
    cmp cl, '.'
    je .Loop

    sub cl, '0'

    ; Check if not a number
    cmp cl, 9
    ja .Sign

    ; Add digit
    mov rbx, rax
    shl rbx, 3
    shl rax, 1
    add rax, rbx
    add rax, rcx

    jmp .Loop

.Sign:
    ; Check if number is negative
    test r11, r11
    je .Positive

    ; Make number negative
    mov r11, rax
    shl r11, 1
    sub rax, r11

.Positive:
    add rsp, BufSize 

    ret

;----------------------------------------


;----------------------------------------
; Prints int32 in decimal format
;----------------------------------------
; Enter:        RAX = integer
; Exit:         None
; Destr:        RAX, RBX, RCX, RDX, R11, RDI, RSI
;----------------------------------------

FUNC_22B14C_01435CD4:
    ; Get RAX from stack
    mov rax, [rsp+8]

    ; Allocate buffer
    sub rsp, BufSize

    ; Set RDI to buffer
    mov rdi, rsp

    ; Set loop length and prepare for division
    xor rdx, rdx
    mov rbx, 10

    mov rcx, BufSize - 1

    ; Set '\n' to the end of the buffer
    mov BYTE [rdi, rcx], 10
    dec rcx

    ; Get sign
    xor r11, r11

    cmp eax, 0
    jge .Loop

    ; Take number module
    mov r11, 1 << 32        
    sub r11, rax
    mov rax, r11

    mov r11, 1              ; Remember number sign in R11

.Loop:
    ; Set digits          
    div ebx
    add dl, '0'

    mov [rdi, rcx], dl

    xor rdx, rdx
    dec rcx

    cmp eax, 0
    jne .Loop

    ; Check if number was negative
    cmp r11, 0
    je .Test

    ; Set sign
    mov byte [rdi, rcx], '-'
    dec rcx

    jmp .Test

.Next:
    ; Set forward zeros
    mov byte [rdi, rcx], 0
    dec rcx

.Test:          
    cmp rcx, -1
    jne .Next

    ; Flush buffer
    mov rdx, BufSize
    mov rax, CallWrite
    mov rdi, StdOut
    mov rsi, rsp
    syscall

    ; Delete buffer
    add rsp, BufSize

    ret

;----------------------------------------


;----------------------------------------
; Calculates int sqrt
;----------------------------------------
; Enter:        Int in stack
; Exit:         RAX = Root
; Destr:        RAX, RBX, RCX, RDX, RDI, RSI, r8, r9
;----------------------------------------

FUNC_22B14C_0062909C:
    ; Get int from stack
    mov rsi, [rsp+8]        ; X

    ; Prepare registers for binary search
    mov rcx, MaxSqrtIter    ; Max number of iterations
    mov rbx, 0              ; Left
    mov r8, rsi             ; Right
    xor r9, r9              ; Root
    xor rax, rax            ; Result

.Loop:
    ; root = (right - left) << 1 + left
    mov r9, r8
    sub r9, rbx
    shr r9, 1
    add r9, rbx

    ; result = root * root
    mov rax, r9
    mul r9

    shl rdx, 32
    add rax, rdx

    mov rdx, rax
    shr rdx, 32
    mov edi, 1000
    idiv edi

    ; if (|result - x| < Epsilon) return root
    sub rax, rsi
    cmp rax, SqrtEpsilon
    ja .NotRoot

    mov rax, r9
    ret

.NotRoot:
    ; else if (result > x) right = root
    cmp rax, 0
    jl .SetLeft

    mov r8, r9
    jmp .Cycle

.SetLeft:
    ; else left = root
    mov rbx, r9

.Cycle:
    ; Check for loop end
    loop .Loop

    ; return root
    mov rax, r9
    ret

;----------------------------------------
