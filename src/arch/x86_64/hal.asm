BITS 64

global asm_sse_setup
global asm_hlt

section .text
; Intel System Programming Guide, Part 1 => 10.6 INITIALIZING SSE/SSE2/SSE3/SSSE3 EXTENSIONS
; Enables SSE/SSE2 safely.
;   Return 
;   EAX = 1  -> SSE available & enabled
;   EAX = 0  -> SSE not available (or not enabled)
; Calling convention: System V AMD64 (C-compatible)
asm_sse_setup:
    ; --- Prolog
    push rbp
    mov  rbp, rsp
    ; --- Prolog
    call .check_sse_feature_set
    call .enable_fpu_sse_context
    call .enable_fpu_base
    ; --- Clear TS bit in CR0 to allow FPU/SSE instructions without #NM
    clts 
    ; --- Reset x87 FPU state to default (control/status registers, tags)
    fninit

    mov  eax, 1
    leave
    ret

.check_sse_feature_set:
    mov  eax, 1         
    cpuid

    test edx, 1 << 19    ; CLFLUSH
    jz   .no_sse
    test edx, 1 << 24    ; FXSAVE/FXRSTOR
    jz   .no_sse
    test edx, 1 << 25    ; SSE
    jz   .no_sse
    test edx, 1 << 26    ; SSE2
    jz   .no_sse
    test ecx, 1 << 0     ; SSE3
    jz   .no_sse
    test ecx, 1 << 9     ; SSSE3
    jz   .no_sse

    ret

.enable_fpu_sse_context:
    mov  rax, cr4
    or   rax, (1 << 9) | (1 << 10)
    mov  cr4, rax
    ret

.enable_fpu_base:
    mov  rax, cr0
    and  rax, ~(1 << 2)     ; EM=0
    or   rax,  (1 << 1)     ; MP=1
    mov  cr0, rax
    ret

.no_sse:
    xor  eax, eax
    leave
    ret

asm_hlt:
    hlt
    jmp asm_hlt