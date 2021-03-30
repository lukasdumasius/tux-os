#define ASM 1


# system_call_linkage
# DESCRIPTION: assembly wrapper for the system call. We first check for valid range(explained below), then find the correct function ptr to jump to.
#              After restoring all regs, we iret to user.
# INPUTS: EAX with system call #
# OUPTUTS:  NONE
# RETURN VALUE: return value of system call also in EAX
# SIDE EFFECTS: NONE
#
.globl system_call_linkage

# Remeber now we already have sys_call # in eax
# In our system valid sys_call # should be in range [1, 10](inclusive) CHECK ece391sysnum.h FOR MORE DETAILS

# Reference: https://courses.grainger.illinois.edu/ece391/fa2020/secure/fa20_lectures/ECE391_Lecture19(F).pdf Page8
#           https://courses.grainger.illinois.edu/ece391/fa2020/secure/fa20_lectures/discussion_week_10-11.pdf Page3
system_call_linkage:
    # CHECK PAGE 8 STACK FIGURE !!! SHOULD MAP THE EXACT ORDER, callee saved reg, 
    PUSHL %EBX 
    PUSHL %EDI
    PUSHL %ESI

    # args pushed from right(3rd) to left(1st)
    PUSHL %EDX 
    PUSHL %ECX
    PUSHL %EBX

    # check if sys_call # valid, should be in range [1, 10](inclusive)
    CMPL $1, %EAX
    JL invalid_sys_call_num
    CMPL $10, %EAX
    JG invalid_sys_call_num

    # now # is valid, we call specific routine using jmp_table
    # CHECK ece391sysnum.h FOR MORE DETAILS

    # VERY IMPORTANT to -4 cuz our sys_call # start from 1, not 0
    # Also note have to "*" since we are indicating function pointer -- check MP1 for more details
    CALL * sys_call_table-4(, %EAX, 4)
    #  MOVL $sys_call_table, %EDI
    #  ADDL $-4, %EDI
    #  CALL * (%EDI, %EAX, 4)
    JMP return_sequence # Notice here EAX should have the return value of the system call


invalid_sys_call_num:
    MOVL $-1, %EAX 

return_sequence:
    POPL %EBX
    POPL %ECX
    POPL %EDX

    # callee saved reg popped
    POPL %ESI
    POPL %EDI 
    POPL %EBX 

    # THIS is step 10 in discussion slides
    IRET

# REMEBER: the below are all function ptrs, not actual function
sys_call_table:
    .long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
