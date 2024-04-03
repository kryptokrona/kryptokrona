#if defined(__x86_64__)
.globl _setmcontext
_setmcontext:
    /* x64 assembly code */
    movq    16(%rdi), %rsi
    movq    24(%rdi), %rdx
    movq    32(%rdi), %rcx
    movq    40(%rdi), %r8
    movq    48(%rdi), %r9
    movq    56(%rdi), %rax
    movq    64(%rdi), %rbx
    movq    72(%rdi), %rbp
    movq    80(%rdi), %r10
    movq    88(%rdi), %r11
    movq    96(%rdi), %r12
    movq    104(%rdi), %r13
    movq    112(%rdi), %r14
    movq    120(%rdi), %r15
    movq    184(%rdi), %rsp
    pushq    160(%rdi)    /* new %eip */
    movq    8(%rdi), %rdi
    ret

.globl _getmcontext
_getmcontext:
    /* x64 assembly code */
    movq    %rdi, 8(%rdi)
    movq    %rsi, 16(%rdi)
    movq    %rdx, 24(%rdi)
    movq    %rcx, 32(%rdi)
    movq    %r8, 40(%rdi)
    movq    %r9, 48(%rdi)
    movq    $1, 56(%rdi)    /* %rax */
    movq    %rbx, 64(%rdi)
    movq    %rbp, 72(%rdi)
    movq    %r10, 80(%rdi)
    movq    %r11, 88(%rdi)
    movq    %r12, 96(%rdi)
    movq    %r13, 104(%rdi)
    movq    %r14, 112(%rdi)
    movq    %r15, 120(%rdi)
    movq    (%rsp), %rcx    /* %rip */
    movq    %rcx, 160(%rdi)
    leaq    8(%rsp), %rcx    /* %rsp */
    movq    %rcx, 184(%rdi)
    movq    32(%rdi), %rcx    /* restore %rcx */
    movq    $0, %rax
    ret

#elif defined(__arm64__)
.globl _setmcontext
_setmcontext:
    /* ARM assembly code */
    ldr x1, [x0, #16]
    ldr x2, [x0, #24]
    ldr x3, [x0, #32]
    ldr x4, [x0, #40]
    ldr x5, [x0, #48]
    ldr x6, [x0, #56]
    ldr x7, [x0, #64]
    ldr x8, [x0, #72]
    ldr x9, [x0, #80]
    ldr x10, [x0, #88]
    ldr x11, [x0, #96]
    ldr x12, [x0, #104]
    ldr x13, [x0, #112]
    ldr x14, [x0, #120]
    ldr x15, [x0, #128]
    ldr x16, [x0, #136]
    ldr x17, [x0, #144]
    ldr x18, [x0, #152]
    ldr x19, [x0, #160]
    ldr x20, [x0, #168]
    ldr x21, [x0, #176]
    ldr x22, [x0, #184]
    ldr x23, [x0, #192]
    ldr x24, [x0, #200]
    ldr x25, [x0, #208]
    ldr x26, [x0, #216]
    ldr x27, [x0, #224]
    ldr x28, [x0, #232]
    ldr x29, [x0, #240] ; frame pointer (x29)
    ldr x30, [x0, #248] ; link register (x30)
    ldr x0, [x0, #8]
    ret

.globl _getmcontext
_getmcontext:
    /* ARM assembly code */
    str x0, [x0, #8]
    str x1, [x0, #16]
    str x2, [x0, #24]
    str x3, [x0, #32]
    str x4, [x0, #40]
    str x5, [x0, #48]
    str x6, [x0, #56]
    str x7, [x0, #64]
    str x8, [x0, #72]
    str x9, [x0, #80]
    str x10, [x0, #88]
    str x11, [x0, #96]
    str x12, [x0, #104]
    str x13, [x0, #112]
    str x14, [x0, #120]
    str x15, [x0, #128]
	str x16, [x0, #136]
    str x17, [x0, #144]
    str x18, [x0, #152]
    str x19, [x0, #160]
    str x20, [x0, #168]
    str x21, [x0, #176]
    str x22, [x0, #184]
    str x23, [x0, #192]
    str x24, [x0, #200]
    str x25, [x0, #208]
    str x26, [x0, #216]
    str x27, [x0, #224]
    str x28, [x0, #232]
    str x29, [x0, #240] ; frame pointer (x29)
    str x30, [x0, #248] ; link register (x30)

    mov x1, x30         ; store x30 (link register) temporarily in x1
    str x1, [x0, #256]  ; store x30 in the context

    mov x0, #0          ; clear x0
    ret

#endif
