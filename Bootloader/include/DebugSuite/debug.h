/*
 * debug.h
 *	
 *  Doc:
 *	  \{
 *		API:
 *			measure execution time: 
 *					TIME_TAKEN_SATRT 
 *					TIME_TAKEN_END
 *			message out:
 *					DEBUG_PRINT(__enable, __fmt, ...)
 *					DEBUG_PRINT_COLOR(__enable, __font_attr,__fg_color, __bg_color, __fmt, ...)
 *						DEBUG_PRINT_INFO(__enable,fmt, ...)
 *						DEBUG_PRINT_DEBUG(__enable,fmt, ...)     
 *						DEBUG_PRINT_ERROR(__enable,fmt, ...)  
 *						DEBUG_PRINT_FATAL(__enable,fmt, ...)  
 *						DEBUG_PRINT_NOTICE(__enable,fmt, ...) 
 *						DEBUG_PRINT_SUCCESS(__enable,fmt, ...)
 *						DEBUG_PRINT_WARNING(__enable,fmt, ...)
 *					DEBUG_PRINT_VALUES(...)
 *			assert:
 *					DEBUG_ASSERT(__expr)
 *  	PARAMS:
 *				debug_enable		(1)
 *				debug_disable		(0)
 *				debug_unuse_block	(0)
 *
 *		EXAMPLE:
 *				- measure execution time:
 *						``` c
 *							TIME_TAKEN_SATRT(1);
 *							usleep(10000);
 *							TIME_TAKEN_END;
 *						```
 *				- message out:
 *						```
 *  	  					DEBUG_PRINT_DEBUG(1, "Hello debug");
 *  	  					DEBUG_PRINT_SUCCESS(1, "Task succeeded!");
 *  	  					DEBUG_PRINT_WARNING(1, "Warning: %d", 123);
 *  	  					DEBUG_PRINT_NOTICE(1, "Please notice this");
 *  	  					DEBUG_PRINT_FATAL(1, "Fatal error happened");
 *  	  					DEBUG_PRINT_INFO(debug_enable, "Info message\n");
 *							DEBUG_PRINT_COLOR(debug_enable, DEBUG_PRINT_ATTR_BOLD, DEBUG_PRINT_COLOR_FG_RED, DEBUG_PRINT_COLOR_BG_BLACK, "Hello debug");
 *						```
 *						```
 *							int a = 0;
 *							float b = 12.22;
 *							DEBUG_PRINT_VALUES(a, b);
 *						```
 *				- assert:
 *						```
 *							DEBUG_ASSERT(1 < 0);
 *						```
 * \}
 *
 *  Created on: 2025_07_31
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2025 by Rev-RoastedDuck.
 */

#ifndef DEBUG_RRD_H_
#define DEBUG_RRD_H_

#include <stdint.h>

/******************************************************************************/
/*------------------------------------CONFIG----------------------------------*/
/******************************************************************************/
/** \addtogroup measure execution time
 ** \{ */
// \def size_t fnt (void);
typedef uint32_t (*debug_get_time_fn_t)(void);
#define DEBUG_MEASURE_EXECUTEION_TIME_GET_TIME_MS   (debug_get_time_fn_t)(0)
/** \} */

/** \addtogroup message out
 ** \{ */
// \def 0: not show file name  1: show file name
#define DEBUG_PRINT_SHOW_COLOR   	(1)
#define DEBUG_PRINT_SHOW_FILENAME   (0)
/** \} */

/** \addtogroup assert
 ** \{ */
// \def void fnt (void);
#define DEBUG_ASSERT_FAILED_CALLBACK    (0)
/** \} */

/******************************************************************************/
/*----------------------------------INCLUDE-----------------------------------*/
/******************************************************************************/
#include "stdio.h"
#include "string.h"

/******************************************************************************/
/*------------------------------------MARCO-----------------------------------*/
/******************************************************************************/
/** \addtogroup measure execution time
 ** \{ */
#ifdef __linux__
    #undef DEBUG_MEASURE_EXECUTEION_TIME_GET_TIME_MS
	#include "stddef.h"
    #include <sys/time.h>
    size_t __debug_measure_executeion_time_get_time_ms(void);
    #define DEBUG_MEASURE_EXECUTEION_TIME_GET_TIME_MS   __debug_measure_executeion_time_get_time_ms
#endif

#define TIME_TAKEN_SATRT(__max_execute_time__)					\
	do {														\
		static size_t __start, __end;   				        \
		static int __execute_time = 0;							\
		static int __max_execute_time = 1;	                    \
		static long __time = 0;									\
		__start = DEBUG_MEASURE_EXECUTEION_TIME_GET_TIME_MS();

#define TIME_TAKEN_END																					\
		__end = DEBUG_MEASURE_EXECUTEION_TIME_GET_TIME_MS();                                        	\
		long elapsed_ms = (__end - __start);                      			                        	\
		__time += elapsed_ms;                                                           				\
		__execute_time++;                                                                				\
		if (__execute_time >= __max_execute_time) {                                                		\
			printf("[%s:%d] time taken: %.3ld ms\n", __FILE__, __LINE__, (__time / __execute_time)); 	\
			__execute_time = 0;                                                          				\
			__time = 0;                                                                 				\
		}																								\
	} while(0)
/** \} */

/** \addtogroup message out
 ** \{ */
// Attrition
#define DEBUG_PRINT_ATTR_RESET      		0
#define DEBUG_PRINT_ATTR_BOLD       		1
#define DEBUG_PRINT_ATTR_UNDERLINE  		4
#define DEBUG_PRINT_ATTR_BLINK      		5 
#define DEBUG_PRINT_ATTR_REVERSE    		7

// Foreground color
#define DEBUG_PRINT_COLOR_FG_BLACK   		30
#define DEBUG_PRINT_COLOR_FG_RED     		31
#define DEBUG_PRINT_COLOR_FG_GREEN   		32
#define DEBUG_PRINT_COLOR_FG_YELLOW  		33
#define DEBUG_PRINT_COLOR_FG_BLUE    		34
#define DEBUG_PRINT_COLOR_FG_PURPLE  		35
#define DEBUG_PRINT_COLOR_FG_CYAN    		36
#define DEBUG_PRINT_COLOR_FG_WHITE   		37

#define DEBUG_PRINT_COLOR_FG_BRIGHT_BLACK   90
#define DEBUG_PRINT_COLOR_FG_BRIGHT_RED     91
#define DEBUG_PRINT_COLOR_FG_BRIGHT_GREEN   92
#define DEBUG_PRINT_COLOR_FG_BRIGHT_YELLOW  93
#define DEBUG_PRINT_COLOR_FG_BRIGHT_BLUE    94
#define DEBUG_PRINT_COLOR_FG_BRIGHT_PURPLE  95
#define DEBUG_PRINT_COLOR_FG_BRIGHT_CYAN    96
#define DEBUG_PRINT_COLOR_FG_BRIGHT_WHITE   97

// Background color
#define DEBUG_PRINT_COLOR_BG_BLACK   		40
#define DEBUG_PRINT_COLOR_BG_RED     		41
#define DEBUG_PRINT_COLOR_BG_GREEN   		42
#define DEBUG_PRINT_COLOR_BG_YELLOW  		43
#define DEBUG_PRINT_COLOR_BG_BLUE    		44
#define DEBUG_PRINT_COLOR_BG_PURPLE  		45
#define DEBUG_PRINT_COLOR_BG_CYAN    		46
#define DEBUG_PRINT_COLOR_BG_WHITE   		47

#define DEBUG_PRINT_COLOR_BG_BRIGHT_BLACK   100
#define DEBUG_PRINT_COLOR_BG_BRIGHT_RED     101
#define DEBUG_PRINT_COLOR_BG_BRIGHT_GREEN   102
#define DEBUG_PRINT_COLOR_BG_BRIGHT_YELLOW  103
#define DEBUG_PRINT_COLOR_BG_BRIGHT_BLUE    104
#define DEBUG_PRINT_COLOR_BG_BRIGHT_PURPLE  105
#define DEBUG_PRINT_COLOR_BG_BRIGHT_CYAN    106
#define DEBUG_PRINT_COLOR_BG_BRIGHT_WHITE   107

#if defined(__GNUC__) || defined(__clang__)
    #define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#elif defined(_WIN32)
    #include <string.h>
    #define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
    #define __FILENAME__ (__FILE__)
#endif

#if DEBUG_PRINT_SHOW_FILENAME == 1
    #define DEBUG_PRINT(__enable, __fmt, ...)  \
        do {  \
            if (__enable) {  \
                printf("[%s:%d] " __fmt, __FILENAME__, __LINE__, ##__VA_ARGS__); \
            }  \
        } while (0)
#else
	#undef DEBUG_PRINT
    #define DEBUG_PRINT(__enable, __fmt, ...)  \
        do {  \
            if (__enable) {  \
                printf(__fmt, ##__VA_ARGS__); \
            }  \
        } while (0)
#endif

#if DEBUG_PRINT_SHOW_COLOR == 1
	#define DEBUG_PRINT_COLOR(__enable, __font_attr,__fg_color, __bg_color, __fmt, ...) 	\
		do { 																				\
			if(__enable){ 																	\
				printf("\033[");															\
				__font_attr >= 0 && (printf("%d", 	__font_attr),1); 						\
				__fg_color  >= 0 && (printf(";%d",  __fg_color), 1); 						\
				__bg_color  >= 0 && (printf(";%d",  __bg_color), 1); 						\
				printf("m");																\
				DEBUG_PRINT(__enable,__fmt,##__VA_ARGS__);									\
				printf("\033[0m\r\n");														\
			}																				\
		} while (0)

	#define DEBUG_PRINT_DEBUG(__enable,fmt, ...)   DEBUG_PRINT_COLOR(__enable, -1, DEBUG_PRINT_COLOR_FG_BRIGHT_BLACK, 	-1, fmt, ##__VA_ARGS__)
	#define DEBUG_PRINT_INFO(__enable,fmt, ...)    DEBUG_PRINT_COLOR(__enable, -1, DEBUG_PRINT_COLOR_FG_BLUE, 			-1, fmt, ##__VA_ARGS__)
	#define DEBUG_PRINT_NOTICE(__enable,fmt, ...)  DEBUG_PRINT_COLOR(__enable, -1, DEBUG_PRINT_COLOR_FG_CYAN, 			-1, fmt, ##__VA_ARGS__)
	#define DEBUG_PRINT_WARNING(__enable,fmt, ...) DEBUG_PRINT_COLOR(__enable, -1, DEBUG_PRINT_COLOR_FG_YELLOW, 		-1, fmt, ##__VA_ARGS__)
	#define DEBUG_PRINT_ERROR(__enable,fmt, ...)   DEBUG_PRINT_COLOR(__enable, -1, DEBUG_PRINT_COLOR_FG_RED, 			-1, fmt, ##__VA_ARGS__)
	#define DEBUG_PRINT_FATAL(__enable,fmt, ...)   DEBUG_PRINT_COLOR(__enable,  1, DEBUG_PRINT_COLOR_FG_BRIGHT_RED, 	-1, fmt, ##__VA_ARGS__)
	#define DEBUG_PRINT_SUCCESS(__enable,fmt, ...) DEBUG_PRINT_COLOR(__enable, -1, DEBUG_PRINT_COLOR_FG_GREEN, 			-1, fmt, ##__VA_ARGS__)
#else
	#define DEBUG_PRINT_COLOR(__enable, __font_attr,__fg_color, __bg_color, __fmt, ...) 	\
		do { 																				\
			if(__enable){ 																	\
				DEBUG_PRINT(__enable,__fmt,##__VA_ARGS__);									\
				printf("\r\n");																\
			}																				\
		} while (0)

	#define DEBUG_PRINT_DEBUG(__enable,fmt, ...)   DEBUG_PRINT_COLOR(__enable, -1, DEBUG_PRINT_COLOR_FG_BRIGHT_BLACK, 	-1, fmt, ##__VA_ARGS__)
	#define DEBUG_PRINT_INFO(__enable,fmt, ...)    DEBUG_PRINT_COLOR(__enable, -1, DEBUG_PRINT_COLOR_FG_BLUE, 			-1, fmt, ##__VA_ARGS__)
	#define DEBUG_PRINT_NOTICE(__enable,fmt, ...)  DEBUG_PRINT_COLOR(__enable, -1, DEBUG_PRINT_COLOR_FG_CYAN, 			-1, fmt, ##__VA_ARGS__)
	#define DEBUG_PRINT_WARNING(__enable,fmt, ...) DEBUG_PRINT_COLOR(__enable, -1, DEBUG_PRINT_COLOR_FG_YELLOW, 		-1, fmt, ##__VA_ARGS__)
	#define DEBUG_PRINT_ERROR(__enable,fmt, ...)   DEBUG_PRINT_COLOR(__enable, -1, DEBUG_PRINT_COLOR_FG_RED, 			-1, fmt, ##__VA_ARGS__)
	#define DEBUG_PRINT_FATAL(__enable,fmt, ...)   DEBUG_PRINT_COLOR(__enable,  1, DEBUG_PRINT_COLOR_FG_BRIGHT_RED, 	-1, fmt, ##__VA_ARGS__)
	#define DEBUG_PRINT_SUCCESS(__enable,fmt, ...) DEBUG_PRINT_COLOR(__enable, -1, DEBUG_PRINT_COLOR_FG_GREEN, 			-1, fmt, ##__VA_ARGS__)
#endif

void __debug_print_values(char *name_line, double values[], int count);
#define DEBUG_PRINT_VALUES(...) 														\
	do { 																				\
		char __name_line[] = #__VA_ARGS__; 												\
		double __values[] = { __VA_ARGS__ }; 											\
		__debug_print_values(__name_line, __values, sizeof(__values) / sizeof(double));\
	} while (0)
/** \} */

/** \addtogroup assert
 ** \{ */
#if DEBUG_ASSERT_FAILED_CALLBACK == 0
    #undef DEBUG_ASSERT_FAILED_CALLBACK
    #define DEBUG_ASSERT_FAILED_CALLBACK()      \
        do{                                     \
        } while (1);
#endif

#define DEBUG_ASSERT(__expr) 																			\
    do { 																								\
        if (!(__expr)) { 																				\
            DEBUG_PRINT_FATAL(1,"Assertion failed: \"%s\" in %s:%d", #__expr, __FILENAME__, __LINE__);	\
			DEBUG_ASSERT_FAILED_CALLBACK(); 															\
        } 																								\
    } while (0)
/** \} */

#define debug_enable		(1)
#define debug_disable		(0)
#define debug_unuse_block	(0)

#endif
