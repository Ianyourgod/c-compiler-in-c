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
.global
main:
push r15
add r14 r0 r15
ldi r10 6
sub r14 r10 r14
ldi r10 88
str r15 r10 -2
lod r15 r10 -2
ldi r11 4
eq r10 r11 r12
str r15 r12 -4
lod r15 r10 -4
cmp r10 r0
jc ne .switch.case.0
lod r15 r10 -2
ldi r11 88
eq r10 r11 r12
str r15 r12 -6
lod r15 r10 -6
cmp r10 r0
jc ne .switch.case.1
.switch.case.0:
ldi r2 3
add r15 r0 r14
pop r15
ret
jmp .1.loop.break
.switch.case.1:
ldi r2 4
add r15 r0 r14
pop r15
ret
jmp .1.loop.break
.1.loop.break:
ldi r2 0
add r15 r0 r14
pop r15
ret
ldi r2 0
add r15 r0 r14
pop r15
ret
.global
other_function:
push r15
add r14 r0 r15
ldi r10 4
sub r14 r10 r14
ldi r10 5
str r15 r10 -2
lod r15 r10 -2
ldi r11 1
add r10 r11 r12
str r15 r12 -4
lod r15 r2 -4
add r15 r0 r14
pop r15
ret
ldi r2 0
add r15 r0 r14
pop r15
ret
