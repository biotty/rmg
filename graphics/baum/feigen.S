bits      64
default   rel
global    _start

%define   n_        3840

section   .data

t:        dd        2160
n:        dd        n_
one:      dq        1.0
femto:    dq        1e-15
rnd_s:    dq        0.000030517578125  ; 1 / 32768
abcd:     dq        2.4, 4.0, 0.0, 1.0  ; (default) args
rdn:      dd        12, 34, 56, 78  ; random seeds

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

%macro    nextq     2
          add       %2, 8
          mov       %1, [%2]
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
          mov       eax, [n]
          push      rax
          vbroadcastsd ymm5, [abcd+16]
          movq      xmm0, [abcd+24]
          subsd     xmm0, xmm5
          cvtsi2sd  xmm1, dword [rsp]
          divsd     xmm1, xmm0
          movq      [rsp], xmm1
          vbroadcastsd ymm4, [rsp]
          add       rsp, 8
          ret

; r = four of 0..1 (POSIX.1-2001 rand() example)
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
      rep
          ret

; rdi c-str decimals as integer to rbx
; r8 nonzero means to 0-complete for a total of 16 digits
anum:
          mov       rsi, rdi
          xor       rbx, rbx
          mov       edi, 10
          mov       ecx, 16
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

anum_leave:
          ret

anum_break:
          test      r8, r8
          jz        anum_leave
          mov       eax, 1

anum_complete:
          mul       rdi
          loop      anum_complete
          mul       rbx
          mov       rbx, rax
          jmp       anum_leave

; rdi c-str digits.  i.e 123 means 1.23
; r8 result-addr of 64bit float
farg:
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
          ret

farg_err:
          stc
          jmp       farg_leave

; rsi, rdx to write (1) to fd 1 until all has been written
; or return early on any err
print:
          xor       rdi, rdi
          inc       rdi

print_loop:
          mov       rax, rdi
          syscall
          cmp       eax, -1
          je        print_leave
          add       rsi, rax
          sub       rdx, rax
          jnz       print_loop
      rep

print_leave:
          ret

; using rbx but not called conventionally
; so no need to push-pop-conserve it
_start:
          mov       rax, [rsp]
          cmp       rax, 7
          jb        args_endif
          lea       r8, [abcd]
          lea       r9, [rsp+8]
          mov       rcx, 4

args:
          nextq     rdi, r9
          call      farg
          jc        main_leave
          add       r8, 8
          loop args
          xor       r8, r8
          nextq     rdi, r9
          call      anum
          mov       [t], ebx
          nextq     rdi, r9
          call      anum
          cmp       [n], ebx
          jb        main_err
          mov       [n], ebx

args_endif:
          call      init

feigen:
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
          mov       edi, [n]
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

; format v inplace and write it.  erase all.
flush:
          lea       rsi, [v]
          mov       rdi, rsi
          mov       ecx, [n]
          cld

format:
          lodsw
          xchg      al, ah
          stosw
          loop      format

          lea       rsi, [v]
          mov       edx, [n]
          add       rdx, rdx
          call      print

          xor       ax, ax
          lea       rdi, [v]
          mov       ecx, [n]
          cld
      rep stosw

          inc       dword [j]
          mov       eax, [j]
          cmp       eax, [t]
          jb        feigen
          clc

main_leave:
          rcl       al, 1
          and       al, 1

main_done:
          movzx     edi, al
          mov       rax, 60
          syscall

main_err:
          mov       al, 2
          jmp       main_done
