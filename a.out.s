ldi r14 65534
ldi r15 65534
call main
ldi r3 48
ldi r4 10
ldi r6 ...render_exit_code
...render_exit_code:
mod r1 r4 r5
add r5 r3 r5
pout r5
div r1 r4 r1
jc > r1 r0 r6
hlt
main:
ldi r1 0
ret
