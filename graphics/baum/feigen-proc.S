;         Christian Sommerfeldt Øien
;         All rights reserved
bits      64
default   rel
global    feigen

section   .rodata
one:      dq        1.0

section   .text
%macro    report    1
          pextrd    eax, xmm0, %1
		  stosd
%endmacro

%macro    logistic  0
          vsubpd    ymm0, ymm3, ymm1
          vmulpd    ymm0, ymm0, ymm1
          vmulpd    ymm1, ymm0, ymm2
%endmacro

feigen:
          cvtsi2sd  xmm4, rsi
       vbroadcastsd ymm4, xmm4
          vmovupd   ymm1, [rdx]
       vbroadcastsd ymm2, xmm0
       vbroadcastsd ymm3, [one]
		  cld

do:
          vmulpd    ymm0, ymm1, ymm4
          vcvtpd2dq xmm0, ymm0
          report    0
          report    1
          report    2
          report    3
		  logistic
          loop      do
		  vmovupd   [rdx], ymm1
          ret
