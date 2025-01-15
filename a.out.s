ldi r14 65534
ldi r15 65534
call main
ldi r3 48
ldi r4 10
ldi r14 65534
ldi r15 65532
...render_exit_code:
mod r2 r4 r5
add r5 r3 r5
push r5
div r2 r4 r2
cmp r2 r0
jc > ...render_exit_code
...render_exit_code_1:
pop r5
pout r5
cmp r14 r15
jc < ...render_exit_code_1 
hlt
main:
ldi r10 3
str r15 r10 -4
lod r15 r10 -4
str r15 r10 -8
lod r15 r10 -4
ldi r11 1
add r10 r11 r12
str r15 r12 -4
lod r15 r2 -8
ret
ldi r2 0
ret
