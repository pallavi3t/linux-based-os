#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"

#include "file_system.h"
#include "terminal.h"
#include "sys_calls.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here
/* divide by zero test */
int idt_divide_by_zero_test(){
	TEST_HEADER;

	int a,b,c;
	a = 3;
	b = 0;
	c = a / b;
	return c;
}

/* dereference null ptr test */
int* idt_dereference_test(){
	TEST_HEADER;

	int* a = NULL;
	int b;
	b = *a;
	return a;
}

/*
 * paging_kernel_test
 *   DESCRIPTION: Tests that kernel memory is valid
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: PASS
 *   SIDE EFFECTS: Should return PASS because kernel is valid memory
 */   
int paging_kernel_test()
{
	TEST_HEADER;

	int result = PASS;
	unsigned int * ptr = (unsigned int *)0x400000;
	*ptr = 0;

	return result;
}

/*
 * paging_video_test
 *   DESCRIPTION: Tests that video memory is valid
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: PASS
 *   SIDE EFFECTS: Should return PASS because video memory is valid memory
 */   
int paging_video_test()
{
	TEST_HEADER;

	int result = PASS;
	unsigned int * ptr = (unsigned int *)0xB8000;
	*ptr = 0;

	return result;
}

/*
 * paging_table_test
 *   DESCRIPTION: Tests an invalid memory location in the page table
 *   INPUTS: none
 *   OUTPUTS: PAGE FAULT Exception
 *   RETURN VALUE: PASS
 *   SIDE EFFECTS: Should PAGE FAULT because invalid memory location
 */   
int paging_table_test()
{
	TEST_HEADER;

	int result = PASS;
	unsigned int * ptr = (unsigned int *)0x0;

	*ptr = 0;

	return result;
}

/*
 * paging_dir_test
 *   DESCRIPTION: Tests an invalid memory location in the page directory
 *   INPUTS: none
 *   OUTPUTS: PAGE FAULT Exception
 *   RETURN VALUE: PASS
 *   SIDE EFFECTS: Should PAGE FAULT because invalid memory location
 */   
int paging_dir_test()
{
	TEST_HEADER;

	int result = PASS;
	unsigned int * ptr = (unsigned int *)0x800000;

	*ptr = 0;

	return result;
}


/* Checkpoint 2 tests */

/*
 * rtc_test
 *   DESCRIPTION: Tests if frequency of the rtc can be changed 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: PASS/FAIL
 *   SIDE EFFECTS: print changes in frequency to terminal
 */ 
int rtc_test(){

	int32_t fd = 0;
	int result = PASS;

	//test open
	fd = rtc_open(NULL);
	if (fd != 0){
		result = FAIL;
	}

	//test read/write
	int i, j, count;
	int freq = 1;

	clear();
	printf("\n\n");

	for (i = 1; i <= 10; i++){ // 10 different frequencies
		freq *= 2;

		if (rtc_write(fd, &freq, 4) == -1){
			result = FAIL;
			break;
		}

		//set the screen position (call function)
		count = 0;
		for (j = 0; j < freq; j++){
			if (rtc_read(fd,NULL,0) != 0){
				result = FAIL;
			}

			printf("%d", i);
			count++;
		}

		printf("\n");

	}

	//test close
	if (rtc_close(fd) != 0){
		result = FAIL;
	}

	return result;
}



/*
 * file_sys_test()
 *   DESCRIPTION: Tests file read functionality
 *   INPUTS: none
 *   OUTPUTS: Outputs the file to the terminal
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: 
 */   
void file_sys_test()
{
	TEST_HEADER;
	char * filename = "frame0.txt";
	uint8_t buf[N*ONE_KB];
	int i;
	dentry_t dentry_ptr;
	int32_t result;
	uint32_t length = strlen(filename); // Length of filename
	strcpy((int8_t *)(dentry_ptr.file_name), (int8_t *)filename); // Copy filename into dentry filename
	int32_t output = read_dentry_by_name((uint8_t *)filename, &dentry_ptr); // Find if dentry exists
	if (output == 0) // Read dentry successfully 
	{
		unsigned int inode_ptr = ptr_to_start + (dentry_ptr.inode_num+1)*FOUR_KB; // Get inode for dentry
		length = ((struct inode_t *)(inode_ptr))->length; // Length of file 
		result = read_data(dentry_ptr.inode_num, 0, buf, length); // Read data blocks for inode 
	}
	else
	{
		return;
	}
	
	
	clear();
	printf("\n");
	
	 
	for (i = 0; i < length; i++)
	{
		putc(buf[i]);
	} 

	// printf("length %d", length); // Output length of file 
	// printf("bytes %d", result); // Output number of bytes read - should be equal to length
	
}

// Invalid test after checkpoint 3 
/*
 * list_files_test()
 *   DESCRIPTION: Tests ls functionality
 *   INPUTS: none
 *   OUTPUTS: Outputs all the directories to the terminal
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */   
void list_files_test()
{
	TEST_HEADER;
	uint8_t buf[FILE_NAME_SIZE];
	int i = 0;

	clear();
	printf("\n");

	for (i = 0; i < bblock_ptr->num_dir_entries; i++)
	{
		directory_read(-1, (uint8_t *)buf, 0);
	}
}

/*
 * terminal_write_test()
 *   DESCRIPTION: Tests terminal write functionality
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: returns PASS 
 *   SIDE EFFECTS: 
 */   
int terminal_write_test()
{
	TEST_HEADER;
	int result = PASS;

	return result;
}

/*
 * terminal_read_test()
 *   DESCRIPTION: Tests terminal read functionality
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: returns PASS
 *   SIDE EFFECTS: 
 */   
int terminal_read_test()
{
	TEST_HEADER;
	int result = PASS;

	return result;
}

/*
 * terminal_test()
 *   DESCRIPTION: Test keyboard functionality
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */   
void terminal_test()
{
	TEST_HEADER;

	clear();
	while(1);
}

/* Checkpoint 3 tests */
/*
 * test_write() 
 *   DESCRIPTION: Tests system call write functionality
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */   
int32_t test_write(int32_t fd)
{
	TEST_HEADER;
	uint8_t buf[N*ONE_KB];
	if (write(fd, buf, 9999) == -1)
		return 0;
	return -1;
}

/*
 * test_read()
 *   DESCRIPTION: Tests system call open and close functionality 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */   
int32_t test_read(int32_t fd)  
{
	TEST_HEADER;
	uint8_t buf[N*ONE_KB];
	if (read(fd, buf, 9999) != -1)
		return 0;
	return -1;
}


/*
 * test_ocrw()
 *   DESCRIPTION: Tests system call read functionality
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */   
int test_ocrw()
{
	TEST_HEADER;
	int result = PASS;

	clear();
	uint32_t fd = open((const uint8_t*)"shell");
	if (fd < FILE_DESC_TBL_OFFSET  || fd >= FILE_DESC_TBL_ENTRIES)
    {
        result = FAIL;
		printf("Open fail\n");
    }
	else
	{
		printf("Open pass\n");
	}

	if (test_read(fd) == -1){
		result = FAIL;
		printf("read failed\n");
	}
	else
	{
		printf("read passed\n");
	}

	if (test_write(fd) == -1){
		result = FAIL;
		printf("write failed\n");
	}
	else
	{
		printf("write passed\n");
	}

	if (close(fd) == -1){
		result = FAIL;
		printf("close failed\n");
	}
	else
	{
		printf("close passed\n");
	}
	

	return result;
}

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	// TEST_OUTPUT("idt_divide_by_zero_test", idt_divide_by_zero_test());
	// TEST_OUTPUT("idt_dereference_test", idt_dereference_test());
	// TEST_OUTPUT("paging_kernel_test", paging_kernel_test());
	// TEST_OUTPUT("paging_video_test", paging_video_test());
	// TEST_OUTPUT("paging_table_test", paging_table_test());
	// TEST_OUTPUT("paging_dir_test", paging_dir_test());
	// TEST_OUTPUT("file_sys_test", file_sys_test());
	// file_sys_test();
	// list_files_test(); // Need to modify this test because large name file outputting weird character at end
	// TEST_OUTPUT("rtc_test", rtc_test());
	// TEST_OUTPUT("terminal_write_test", terminal_write_test());
	// TEST_OUTPUT("terminal_read_test", terminal_read_test());
	// terminal_test();
	// TEST_OUTPUT("test_ocrw", test_ocrw());
}
