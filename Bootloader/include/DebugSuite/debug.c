/*
 * debug.h
 *	
 *  Created on: 2024_11_12
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2023 by Rev-RoastedDuck.
 */

#include "debug.h"

/** \addtogroup measure execution time
 ** \{ */
#ifdef __linux__
#include <sys/time.h>
size_t __debug_measure_executeion_time_get_time_ms(void){
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * 1000L + t.tv_usec / 1000L;
}
#endif
/** \} */

/** \addtogroup message out
 ** \{ */
void __debug_print_values(char *name_line, double values[], int count) {
	char *token = strtok(name_line, ",");
	for (int i = 0; i < count && token; ++i) {
		while (*token == ' ') token++;
		printf("%s = %.3f", token, values[i]);
		token = strtok(NULL, ",");
		(token != NULL ? (void)printf(", ") : (void)0);
	}
	printf("\r\n");
}
/** \} */
