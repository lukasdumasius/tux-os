#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "file_system.h"
#include "terminal.h"

#include "system_call.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER \
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result) \
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure()
{
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15"); //15 reserved by intel
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
int idt_test()
{
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i)
	{
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL))
		{
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here
/* Paging Test
 * 
 * Asserts that we mapped video from 0xB8000 to 0xB9000, kernel from 0x400000 to 0x800000
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Exception
 * Coverage: Paging
 * Files: paging.c, paging.h, x86_desc.h, x86_desc.c, kernel,c
 */
// Test if video memory if mapped correctly
int paging_test_1()
{
	TEST_HEADER;
	uint32_t i = 0xB8000;
	;
	while (i < 0xB9000)
	{
		uint32_t *ptr = (uint32_t *)(i);
		uint32_t val = *ptr;
		i += 0x1000;
		val++;
		// throw paging exception
	}
	return PASS;
}

// Test if kernel memory if mapped correctly
int paging_test_2()
{
	TEST_HEADER;
	uint32_t i = 0x400000;
	;
	while (i < 0x800000)
	{
		uint32_t *ptr = (uint32_t *)(i);
		uint32_t val = *ptr;
		val++;
		i += 0x10000;
	}
	return PASS;
}

// Test if the paging exception can be handled correct
int paging_test_3()
{
	TEST_HEADER;
	uint32_t i = 0x1000005;
	uint32_t *ptr = (uint32_t *)(i);
	uint32_t val = *ptr;
	val += 0x1000;
	return FAIL;
}

// Tests if we can divide by zero
// shuold throw exception
// sets one int to 4, another to 0
// and tries to divide. Return val meaningless.

int divide_by_zero_test()
{
	TEST_HEADER;
	int a = 4; // 4 is just a random number to test
	int b = 0; // 0 for divide by 0
	int c = a / b;
	return c;
}

// Tests if its possible to derefence a NULL (big nono)
// by getting pointer to null and dereferencing. return val
// meaningless.

uint32_t memory_test()
{
	TEST_HEADER;
	uint32_t *pointy = NULL; //shouldn't be dereferncable
	uint32_t tester_boy;
	tester_boy = *pointy;
	return tester_boy;
}

// Used to test a specific entry in the idt table.
// 0x80 used for sys calls.
// return val for success if executed.
int idt_table_test()
{
	TEST_HEADER;
	asm volatile("int $0x80"); //tests system call, which is 0x80
	return 1;				   // please change 0x80 to any number
} // 0->256 to test idt table entries

/* Checkpoint 2 tests */

// Tests for file system
/* Read_dentry_by_name Test
 * 
 * Asserts all 3 system module is working properly with all different filename
 * 		different boundaries, special conditions and all cases
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: File System Module
 * Files: file_system.c, file_system.h
 */
int file_system_read_dentry_by_name()
{
	TEST_HEADER;
	printf("\n");
	dentry_t test_dentry;
	if (read_dentry_by_name((const uint8_t *)"verylargetextwithverylongname.txt", &test_dentry) != FS_SUCCESS) //  find existed files
		return FAIL;
	if (read_dentry_by_name((const uint8_t *)"CS_better_than_ECE", &test_dentry) != FS_FAIL) //  find non-existed files
		return FAIL;
	if (read_dentry_by_name((const uint8_t *)".", &test_dentry) != FS_SUCCESS) //  find directory files
		return FAIL;
	if (read_dentry_by_name((const uint8_t *)"grep", &test_dentry) != FS_SUCCESS) //  find executable file
		return FAIL;
	return PASS;
}

//  Test Read_dentry_by_index
int file_system_read_dentry_by_index()
{
	TEST_HEADER;
	printf("\n");
	dentry_t test_dentry;
	if (read_dentry_by_index(0, &test_dentry) != FS_SUCCESS) // valid idx is 0-62, but we only use first ~16, need to double check -- Yan 10/24
		return FAIL;
	if (read_dentry_by_index(10, &test_dentry) != FS_SUCCESS)
		return FAIL;
	if (read_dentry_by_index(100, &test_dentry) != FS_FAIL)
		return FAIL;
	return PASS;
}

// //  Test Read_data
// int file_system_read_data()
// {
// 	TEST_HEADER;
// 	printf("\n");
// 	if (file_open((const uint8_t *)"verylargetextwithverylongname.txt") != FS_SUCCESS)
// 		return FAIL;

// 	uint8_t buf[21];
// 	buf[20] = '\0';
// 	uint8_t test_str[] = "large text file with"; // the string hardcoded to compare

// 	// Test reading at beginning
// 	if (20 != file_read(0, 5, buf, 20))
// 		return FAIL;
// 	// Compare the string
// 	if (0 != strncmp((const int8_t *)buf, (const int8_t *)test_str, (uint32_t)20))
// 		return FAIL;

// 	// Test 4096B boundary
// 	if (20 != file_read(0, 4090, buf, 20))
// 		return FAIL;

// 	// // Test reading 2nd block
// 	if (20 != file_read(0, 5000, buf, 20))
// 		return FAIL;

// 	buf[7] = '\n';
// 	// Test EOF reading,  NOTE WE HAVE 5277B in this file in total!
// 	if (0 != file_read(0, 5270, buf, 20)) // 0 indicate EOF
// 		return FAIL;

// 	// Boundary test
// 	if (0 != file_read(0, 5270, buf, 8888)) // 0 indicate EOF
// 		return FAIL;

// 	// Test reading after end
// 	if (FS_FAIL != file_read(0, 6000, buf, 32))
// 		return FAIL;
// 	file_close(0);
// 	return PASS;
// }

// // Test Read excutable file
// int file_system_read_executable()
// {
// 	TEST_HEADER;
// 	printf("\n");
// 	if (file_open((const uint8_t *)"grep") != FS_SUCCESS)
// 		return FAIL;
// 	uint8_t buf[33];
// 	buf[32] = '\0';

// 	if (32 != file_read(0, 0, buf, 32)) // first arg is fd, only use 0 rn -- Yan 10/25
// 		return FAIL;
// 	if (0 != file_read(0, 6120, buf, 32)) // 6149B in total !! first arg is fd, I don't use that rn -- Yan 10/25
// 		return FAIL;
// 	file_close(0);
// 	return PASS;
// }

// // Test Read txt file
// int file_system_read_txt()
// {
// 	TEST_HEADER;
// 	printf("\n");
// 	if (file_open((const uint8_t *)"frame0.txt") != FS_SUCCESS)
// 		return FAIL;
// 	uint8_t buf[188];
// 	buf[187] = '\0';
// 	if (187 != file_read(0, 0, buf, 187)) // first arg is fd, only use 0 rn -- Yan 10/25
// 		return FAIL;
// 	file_close(0);
// 	return PASS;
// }

// Yan's "ls" program
int file_system_Yan_ls()
{
	TEST_HEADER;
	printf("\n");
	print_file_and_size();
	return PASS;
}

int terminal_write_test()
{
	terminal_clear_and_reset();
	terminal_open(0);  //0 for random/valid fd val
	terminal_close(0); //0 for random/valid fd val
	terminal_open(0);  //0 for random/valid fd val
	char a[] = "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog.\n";
	char b[] = "This doesn't end in a newline. This tests our terminal's intelligent add newline"; //notice this doesnt end in newline!
																								   // so this actually tests if terminal
																								   // can detect that! And it does :)
	char c[] = "The race for the White House between Republican President Donald Trump and Democratic nominee Joe Biden will be determined in 13 key battleground states and a pair of congressional districts.";
	//printf("%s", a);
	terminal_write(-1, (void *)a, 225); //-1 for random/valid fd val, for all
	int i;								//225 is length of a,
	for (i = 0; i < 1000000000; i++)
	{
	};									//80, and 191 same for b, c respectively
	terminal_write(-1, (void *)a, 225); //1billion for about a 1 second wait on 1ghz clock cycles cpu
	for (i = 0; i < 1000000000; i++)
	{
	};
	for (i = 0; i < 1000000000; i++)
	{
	};
	terminal_write(-1, (void *)a, 225);
	for (i = 0; i < 1000000000; i++)
	{
	};
	terminal_write(-1, (void *)a, 225);
	for (i = 0; i < 1000000000; i++)
	{
	};
	terminal_write(-1, (void *)b, 80);
	for (i = 0; i < 1000000000; i++)
	{
	};
	terminal_write(-1, (void *)c, 191);
	for (i = 0; i < 1000000000; i++)
	{
	};
	terminal_write(-1, (void *)a, 225);
	for (i = 0; i < 1000000000; i++)
	{
	};
	terminal_write(-1, (void *)b, 80);
	for (i = 0; i < 1000000000; i++)
	{
	};
	terminal_write(-1, (void *)c, 191);
	for (i = 0; i < 1000000000; i++)
	{
	};
	terminal_write(-1, (void *)a, 225);
	for (i = 0; i < 1000000000; i++)
	{
	};
	terminal_write(-1, (void *)b, 80);
	for (i = 0; i < 1000000000; i++)
	{
	};
	terminal_write(-1, (void *)c, 191);
	for (i = 0; i < 1000000000; i++)
	{
	};
	terminal_write(-1, (void *)a, 225);
	for (i = 0; i < 1000000000; i++)
	{
	};
	terminal_write(-1, (void *)b, 80);
	for (i = 0; i < 1000000000; i++)
	{
	};
	terminal_write(-1, (void *)c, 191);
	for (i = 0; i < 1000000000; i++)
	{
	};
	terminal_write(-1, (void *)a, 225);
	for (i = 0; i < 1000000000; i++)
	{
	};
	terminal_write(-1, (void *)b, 80);
	for (i = 0; i < 1000000000; i++)
	{
	};
	terminal_write(-1, (void *)c, 191);
	return PASS;
}

int terminal_read_and_write_test()
{
	int i;
	int length;
	terminal_clear_and_reset();
	terminal_open(0);  //0 for random/valid fd val
	terminal_close(0); //0 for random/valid fd val
	terminal_open(0);  //0 for random/valid fd val
	char buffer[128];  //big enough size to fit input into buffer, 128
	for (i = 0; i < 15; i++)
	{													 //0 b/c start at 0 and run 14 times
		length = terminal_read(-1, (void *)buffer, 128); // 128 bc testing full buffer bytes copy, -1 for invalid fd
		terminal_write(-1, (void *)buffer, length);		 // -1 for invalid fd
	}
	return PASS;
}
/* Checkpoint 3 tests */
// int execute_test()
// {
// 	execute((uint8_t *)"cat test1.txt");
// 	return PASS;
// }

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests()
{
	// launch your tests here
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("paging_1", paging_test_1());
	// TEST_OUTPUT("paging_2", paging_test_2());
	// TEST_OUTPUT("paging_3", paging_test_3());

	TEST_OUTPUT("testing divide by zero test", divide_by_zero_test());
	// TEST_OUTPUT("trying to derefence NULL pointer", memory_test());
	// TEST_OUTPUT("testing idt specific entry... ", idt_table_test());

	// CP2 Test:
	// terminal_clear_and_reset();
	// TEST_OUTPUT("terminal write test ", terminal_write_test());
	// TEST_OUTPUT("terminal write test ", terminal_read_and_write_test());

	terminal_clear_and_reset();
	// File system:
	// TEST_OUTPUT("read_dentry_by_name", file_system_read_dentry_by_name());
	// TEST_OUTPUT("read_dentry_by_index", file_system_read_dentry_by_index());

	// TEST_OUTPUT("read_data", file_system_read_data());
	// TEST_OUTPUT("read_executable", file_system_read_executable());

	// TEST_OUTPUT("read_txt", file_system_read_txt());

	// TEST_OUTPUT("Yan_ls", file_system_Yan_ls());

	// CP3 Test:
	// Execute
	// TEST_OUTPUT("Execute", execute_test());
}
