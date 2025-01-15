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
ldi r10 1
str r15 r10 -4
lod r15 r10 -4
cmp r10 r0
jc eq .t.0
ldi r10 78
str r15 r10 -8
jmp .t.1
.t.0:
ldi r10 43
str r15 r10 -8
.t.1:
lod r15 r2 -8
ret
ldi r2 0
ret
