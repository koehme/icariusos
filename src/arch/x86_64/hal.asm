BITS 64

global asm_sse_setup
global asm_hlt
global asm_busy_wait

section .text

;/**
; * @brief Safely enables SSE/SSE2/SSE3/SSSE3 instruction set support
; *
; * Intel® 64 and IA-32 Architectures Software Developer’s Manual, Vol. 1
; * Section 10.6: Initializing SSE/SSE2/SSE3/SSSE3 Extensions
; *
; * @return EAX
; *   - 0x0            : SSE available and enabled
; *   - -K_EOPNOTSUPP  : SSE not available or could not be enabled
; *
; * @note
; * Calling convention: System V AMD64 (C-compatible)
; */
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

    xor rax, rax        ; K_OK = 0
    xor rdx, rdx        ; msg = NULL

    leave
    ret

.check_sse_feature_set:
    push rbx             ; callee-saved

    mov  rax, 1         
    cpuid

    test rdx, 1 << 19    ; CLFLUSH
    jz   .no_sse
    test rdx, 1 << 24    ; FXSAVE/FXRSTOR
    jz   .no_sse
    test rdx, 1 << 25    ; SSE
    jz   .no_sse
    test rdx, 1 << 26    ; SSE2
    jz   .no_sse
    test rcx, 1 << 0     ; SSE3
    jz   .no_sse
    test rcx, 1 << 9     ; SSSE3
    jz   .no_sse

    pop rbx
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
    mov rax, -95        ; -K_EOPNOTSUPP
    xor rdx, rdx        ; NULL

    pop rbx
    leave
    ret

asm_hlt:
    hlt
    jmp asm_hlt

asm_busy_wait:
    test    rdi, rdi
    jz      .done
.loop:
    dec     rdi
    jnz     .loop
.done:
    ret