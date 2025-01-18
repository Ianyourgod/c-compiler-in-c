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
lod r15 r10 -4
ldi r11 4
eq r10 r11 r12
str r15 r12 -8
lod r15 r10 -8
cmp r10 r0
jc ne .switch.case.0
lod r15 r10 -4
ldi r11 88
eq r10 r11 r12
str r15 r12 -12
lod r15 r10 -12
cmp r10 r0
jc ne .switch.case.1
.switch.case.0:
ldi r2 3
ret
jmp .1.loop.break
.switch.case.1:
ldi r2 4
ret
jmp .1.loop.break
.1.loop.break:
ldi r2 0
ret
ldi r2 0
ret
