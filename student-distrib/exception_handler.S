.text
.globl set_keyboard
.globl set_rtc 
.type set_keyboard, @function
.type set_rtc, @function

set_keyboard:
    pushal # Save registers
    pushfl # Save flags

    call keyboard_exception # Call keyboard_handler function
    
    popfl 
    popal

    iret

set_rtc:
    pushal # Save registers
    pushfl # Save flags

    call rtc_exception # Call rtc_handler function

    popfl
    popal

    iret
