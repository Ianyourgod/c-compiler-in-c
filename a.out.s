ldi r14 65534
ldi r15 65534
call main
ldi r3 48
ldi r4 10
ldi r6 ...render_exit_code
ldi r14 65534
ldi r15 65534
...render_exit_code:
mod r1 r4 r5
add r5 r3 r5
push r5
div r1 r4 r1
jc > r1 r0 r6
ldi r6 ...render_exit_code_1
...render_exit_code_1:
pop r5
pout r5
jc < r14 r15 r6 
hlt
main:
ldi r10 1
ldi r11 4
shl r10 r11 r12
str r15 r12 -4
lod r15 r1 -4
ret
