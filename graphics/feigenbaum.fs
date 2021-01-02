\ Produces Feigenbaums fractal PNM image
\ Args: a b scaled a million as integers
\ Assumes separate FP stack and down f>s
1080 constant t
1920 constant n
: linear ( n i FP a b -- FP i*{b-a}/n+a)
  fover f- s>f f* s>f f/ f+ ;
create rdn 1 ,
: rnd ( -- FP {0..1} )
  rdn dup @ 1103515245 * 12345 + dup rot
  ! 65536 / 0x7fff and s>f 32768.0e f/ ;
: logistic ( FP y m -- FP y*m*{1-y})
  fover 1.0e fswap f- f* f* ; 
: feig_row ( n addr FP m --)
  rnd begin fdup over s>f dup
  f* f>s cells +
  1 over +! @ 65535 < while
  fover logistic repeat
  2drop fdrop fdrop ;
.( P5) cr n . 32 emit t . cr 255 . cr
variable v n 1- cells allot
create a 2.4e f,
create b 4.0e f,
: main
  t 0 do v n cells erase
  t i a f@ b f@ linear n v feig_row
    n 0 do
    v i cells + @ 8 rshift emit
    loop
  loop ;
: anum ( addr:cstr -- n)
  0 99 0 do over i + c@
    dup 48 < if drop leave then
    48 - swap 10 * + loop nip ;
: farg ( n -- n)
  cells + @ anum s>f 1e6 f/ ;
: args argc @ 2 > if
  argv @ dup
  1 farg a f!
  2 farg b f!
  then ;
args main bye
