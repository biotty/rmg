bits      64
default   rel
global    _start

%define   t_        1080
%define   n_        1920

section   .data

t:        dd        t_
n:        dd        n_
one:      dq        1.0
femto:    dq        1e-15
rnd_s:    dq        0.000030517578125  ; 1 / 32768
abcd:     dq        2.4, 4.0, 0.0, 1.0  ; (default) args
rdn:      dd        1, 2, 3, 4  ; random seeds
farg_msg: db        "argument not in float format of 'd[.]dddddd'", 10
farg_msgn:equ       $ - farg_msg
%defstr   t_s       t_
%defstr   n_s       n_
hdr:      db        "P5", 10, n_s, 32, t_s, 32, 10, "255", 10
hdrn:     equ       $ - hdr

section   .bss

          align     32
r:        resq      4
j:        resd      1
m:        resq      1
ba_t:     resq      1
v:        resw      n_

section   .text

%macro    decimal   2

          sub       %1, 48
          cmp       %1, 9
          ja        %2

%endmacro

; ba_t = (b - a) / t
init:
          fld       qword [abcd+8]
          fld       qword [abcd]
          fsubp
          fidiv     dword [t]
          fstp      qword [ba_t]

; ymm4 = four of n / (d - c)
; ymm5 = four of c
          vbroadcastsd ymm5, [abcd+16]
          movq      xmm0, [abcd+24]
          subsd     xmm0, xmm5
          cvtsi2sd  xmm1, dword [n]
          divsd     xmm1, xmm0
          sub       rsp, 8
          movq      [rsp], xmm1
          vbroadcastsd ymm4, [rsp]
          add       rsp, 8
          ret

; r = four of 0..1
rnd:
          lea       rdi, [rdn]
          mov       ebx, 1103515245
          mov       rcx, 4
          cld

rnd_nums:
          mov       eax, ebx
          mul       dword [rdi]
          add       eax, 12345
          stosd
          shr       eax, 16
          and       ax, 32767
          lea       edx, [r+rcx*8-8]
          mov       dword [edx], eax
          cvtsi2sd  xmm0, dword [rdx]
          mulsd     xmm0, [rnd_s]
          movsd     [rdx], xmm0
          loop      rnd_nums
          ret

; rdi c-string parsed as decimal to
; rbx and trashes all other general registers
anum:
          xor       rbx, rbx
          mov       rsi, rdi
          mov       rdi, 10
          mov       rcx, 16
          cld

anum_loop:
          xor       rax, rax
          lodsb
          test      al, al
          jz        anum_break
          decimal   al, anum_loop
          xchg      rax, rbx
          mul       rdi
          add       rbx, rax
          loop      anum_loop
          jmp       anum_leave

anum_break:
          mov       rax, 1

anum_complete:
          mul       rdi
          loop      anum_complete
          mul       rbx
          mov       rbx, rax

anum_leave:
          ret

; rdi c-string integer scaled a millionth and stored at
; r8 as a double float.  function conserves rsi and rcx
farg:
          push      rsi
          push      rcx
          mov       al, [rdi]
          decimal   al, farg_err
          call      anum
          mov       [r8], rbx
          fild      qword [r8]
          fmul      qword [femto]
          fstp      qword [r8]
          clc

farg_leave:
          pop       rcx
          pop       rsi
          ret

farg_err:
          lea       rsi, [farg_msg]
          mov       rdx, farg_msgn
          call      print
          stc
          jmp       farg_leave

; rsi, rdx to write (1) to fd 1 until all has been written
; and is trashing those and rax, rdi and of syscall i.e rcx.
; will return on err, in which case rsi, rdx indicates pos
print:
          xor       rdi, rdi
          inc       rdi

print_loop:
          mov       rax, rdi
          syscall
          cmp       rax, -1
          je        print_leave
          add       rsi, rax
          sub       rdx, rax
          jnz       print_loop

print_leave:
          ret

main:
          push      rbp
          cmp       rdi, 5
          jb        args_endif
          lea       r8, [abcd]
          mov       rcx, 4

args:
          add       rsi, 8
          mov       rdi, [rsi]
          call      farg
          jc        main_leave
          add       r8, 8
          loop args

args_endif:
          mov       rsi, hdr
          mov       rdx, hdrn
          call      print
          call      init

feig:
; m = j * ba_t + a
          fild      dword [j]
          fmul      qword [ba_t]
          fadd      qword [abcd]
          fstp      qword [m]

; count hits of logistic map until any is at max
          call      rnd
          vmovapd   ymm1, [r]
          vbroadcastsd ymm2, [m]
          vbroadcastsd ymm3, [one]
          xor       rcx, rcx
          mov       edi, n_
          xor       si, si

row_do:
          vsubpd    ymm0, ymm1, ymm5
          vmulpd    ymm0, ymm0, ymm4
          vcvtpd2dq xmm0, ymm0
          mov       cl, 4
          mov       dl, cl

dist:
          pextrd    eax, xmm0, 0
          pshufd    xmm0, xmm0, 57
          cmp       eax, edi
          jae       oob
          lea       rbx, [v]
          add       rbx, rax
          add       rbx, rax
          inc       word [rbx]
          jnz       cont
          dec       word [rbx]
          jmp       flush

oob:
          dec       dl

cont:
          loop      dist
          test      dl, dl
          jz        none
          xor       si, si
          jmp       logistic

none:
          inc       si
          jz        flush

logistic:
          vsubpd    ymm0, ymm3, ymm1
          vmulpd    ymm0, ymm0, ymm1
          vmulpd    ymm1, ymm0, ymm2
          jmp       row_do

; compress v inplace and write it.  erase all.
flush:
          lea       rsi, [v]
          mov       rdi, rsi
          inc       rsi
          mov       rcx, n_
          cld

compress:
          movsb
          inc       rsi
          loop      compress

          lea       rsi, [v]
          mov       rdx, n_
          call      print

          xor       ax, ax
          lea       rdi, [v]
          mov       rcx, n_
          cld
      rep stosw

          inc       dword [j]
          cmp       dword [j], t_
          jb        feig
          clc

main_leave:
          pop       rbp
          rcl       al, 1
          and       al, 254
          movzx     edi, al
          ret

_start:
          mov       rdi, [rsp]
          lea       rsi, [rsp+8]
          call      main
          mov       rax, 60
          syscall

