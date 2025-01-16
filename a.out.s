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
ldi r10 88
str r15 r10 -4
ldi r10 2
str r15 r10 -8
ldi r10 0
str r15 r10 -12
.1.loop.start:
lod r15 r10 -12
ldi r11 5
lt r10 r11 r12
str r15 r12 -16
lod r15 r10 -16
cmp r10 r0
jc eq .1.loop.break
lod r15 r10 -8
ldi r11 2
mul r10 r11 r12
str r15 r12 -8
.1.loop.continue:
lod r15 r10 -12
str r15 r10 -20
lod r15 r10 -12
ldi r11 1
add r10 r11 r12
str r15 r12 -12
jmp .1.loop.start
.1.loop.break:
lod r15 r2 -8
ret
ldi r2 0
ret
