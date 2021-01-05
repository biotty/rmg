bits    64
default rel
global  _start

%define t_ 1080
%define n_ 1920

section .data

t:      dd      t_
n:      dd      n_
a:      dq      2.4
b:      dq      4.0
mth:    dq      1e-6
rdn:    dd      1
rnd_s:  dd      32768
one:	dd		1
farg_msg: db "argument not in float format of 'd[.]dddddd'", 10, 0
farg_msgn: equ $ - farg_msg
%defstr t_s    t_
%defstr n_s    n_
hdr:    db      "P5", 10, n_s, 32, t_s, 32, 10, "255", 10, 0
hdrn:   equ     $ - hdr

section .bss

r:      resq    1
i:      resd    1
j:      resd    1
m:      resq    1
ba_t:   resq    1
v:      resw    n_
v_i:    resd    1

section .text

; ba_t = (b - a) / t
init:
        fld     qword [b]
		fld     qword [a]
		fsubp
		fidiv   dword [t]
		fstp    qword [ba_t]
		ret

; m = j * ba_t + a
linear:
		fild    dword [j]
		fmul    qword [ba_t]
		fadd    qword [a]
		fstp    qword [m]
		ret

; r = 0..1
rnd:
		mov     eax, 1103515245
		mul     dword [rdn]
		add     eax, 12345
		mov     [rdn], eax
		shr     eax, 16
		and     ax, 32767
		mov     word [r],ax
		fild    word [r]
		fidiv   dword [rnd_s]
		fstp    qword [r]
		ret

; y = m * y * (1 - y)
logistic:
		movq    xmm0, xmm3
		subsd   xmm0, xmm1
		mulsd   xmm0, xmm1
		mulsd   xmm0, xmm2
		movq    xmm1, xmm0
		ret

; count hits of logistic map until any is at max
feig_row:
		call    rnd
		movsd   xmm1, [r]
		movsd   xmm2, [m]
		cvtsi2sd xmm3, dword [one]
		cvtsi2sd xmm4, dword [n]
		mov     dx, 65535

feig_loop:
        movq    xmm0, xmm4
		mulsd   xmm0, xmm1
		cvtsd2si ecx, xmm0
		mov     [v_i], ecx
		lea     rbx, [v]
		mov     eax, dword [v_i]
		add     rbx, rax
		add     rbx, rax
		cmp     word [rbx], dx
		jb      feig_while
		ret

feig_while:
		inc     word [rbx]
		call	logistic
		jmp     feig_loop

; rdi c-string parsed as decimal to
; rbx and trashes all other general registers
anum:
        xor     rbx, rbx
        mov     rsi, rdi
        mov     rdi, 10
		mov     rcx, 7
        cld

anum_loop:
        xor     rax, rax
        lodsb
        test    al, al
		jz      anum_break
        sub     al, 48
		cmp     al, 9
		ja      anum_loop

        xchg    rax, rbx
        mul     rdi
        add     rbx, rax
        loop    anum_loop
        jmp     anum_leave

anum_break:
		mov     rax, 1

anum_complete:
		mul     rdi
		loop    anum_complete
		mul     rbx
		mov     rbx, rax

anum_leave:
        ret

; rdi c-string integer scaled a millionth and stored at
; r8 as a double float.  function conserves rsi only
farg:
        push    rsi
		mov     al, [rdi]
		sub     al, 48
		cmp     al, 9
		ja      farg_err
        call    anum
        mov     [r8], rbx
        fild    qword [r8]
        fmul    qword [mth]
        fstp    qword [r8]
		clc

farg_leave:
        pop     rsi
        ret

farg_err:
		lea     rsi, [farg_msg]
		mov     rdx, farg_msgn
		call    print
		stc
		jmp     farg_leave

; rsi, rdx to write (1) to fd 1 until all has been written
; and is trashing those and rax, rdi and of syscall i.e rcx.
; will return on err, in which case rsi, rdx indicates pos
print:
        mov     rdi, 1

print_loop:
        mov     rax, 1
        syscall
        cmp     rax, -1
	    je      print_leave
	    add     rsi, rax
	    sub     rdx, rax
	    jnz     print_loop

print_leave:
	    ret

; compress v inplace and write it.  erase all.
flush_v:
		lea     rsi, [v]
		mov     rdi, rsi
		inc     rsi
		mov     rcx, n_
		cld

compress_v:
        movsb
		inc     rsi
		loop    compress_v

		lea     rsi, [v]
		mov     rdx, n_
		call    print

		xor     ax, ax
		lea     rdi, [v]
		mov     rcx, n_
		cld
        rep stosw
		ret

; two args sets a and b to non-defaults
; then generate and output pnm of feigenbaum
main:
        push    rbp
        cmp     rdi, 3
        jb      args_endif

        lea     r8, [a]
        mov     rdi, [rsi+8]
        call    farg
		jc      main_leave

        lea     r8, [b]
        mov     rdi, [rsi+16]
        call    farg
		jc      main_leave

args_endif:
        mov     rsi, hdr
        mov     rdx, hdrn
		call    print
        call    init

t_loop:
		call    linear
		call    feig_row
		call    flush_v
		inc     dword [j]
		cmp     dword [j], t_
		jb      t_loop
		clc

main_leave:
        pop     rbp
        mov     eax, 0
		rcl     al, 1
		mov     edi, eax
        ret

_start:
        mov     rdi, [rsp]
        lea     rsi, [rsp+8]
        call    main
        mov     rax, 60
        syscall

