.text
.globl flush_tlb
.globl context_switch
.globl system_call_handler

# 0x083FFFFC

#define IF_FLAG                 0x200
#define USER_CS                 0x0023
#define USER_DS                 0x002B
#define ESP_START               0x083FFFFC

# Source: https://wiki.osdev.org/TLB
flush_tlb:
    movl    %cr3, %eax
    movl    %eax, %cr3
    ret

# push artificial IRET context onto the stack
context_switch:
# Save value of eip argument into register
    xorl %ebx, %ebx 
    movl 4(%esp), %ebx

# push User DS 
    xorl %eax, %eax # Push user ds argument
    movw $USER_DS, %ax
    pushl %eax

    movw $USER_DS, %cx 
    movw %cx, %ds 

# push ESP
    xorl %eax, %eax # Push user esp argument
    movl $ESP_START, %eax
    pushl %eax

# push EFLAG
    pushfl

# set interrupt flag
    orl $IF_FLAG, %esp

# push CS
    xorl %eax, %eax
    movl $USER_CS, %eax
    pushl %eax # Push user cs argument

# push EIP 
    pushl %ebx # Push user eip argument

# Call IRET
    iret

system_call_handler:
    # Check that system call number is valid 
    cmpl $10, %eax 
    ja system_invalid 
    cmpl $0, %eax 
    jle system_invalid 

    pushl %edi
    pushl %esi

    # Push arguments to stack for function call 
    pushl %edx # third arg
    pushl %ecx # second arg
    pushl %ebx # first arg
    call *system_call_jt(,%eax,4)

    # Pop arguments off stack
    popl %ebx 
    popl %ecx 
    popl %edx

    popl %esi
    popl %edi 

    # Return value in eax should be the return value from the function call 
    iret

    # Invalid system call, return -1 
    system_invalid:

    movl $-1, %eax 
    iret

system_call_jt:
    .long 0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
