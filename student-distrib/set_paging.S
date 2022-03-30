.text
.globl set_control_reg
.type set_control_reg, @function

/* Source: https://wiki.osdev.org/Paging#Page_Directory */

set_control_reg:
    pushl %ebp
    movl %esp, %ebp

    movl 8(%esp), %eax # Get the address of the page directory into CR3
    movl %eax, %cr3

    movl %cr4, %eax 
    orl $0x00000010, %eax # Enable the page size extension bit in CR4
    movl %eax, %cr4

    movl %cr0, %eax
    orl $0x80000001, %eax # Enable the page enable bit in CR0
    movl %eax, %cr0

    leave
    ret

.end
