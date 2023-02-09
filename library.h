#ifndef LIBRARY_H
#define LIBRARY_H
#include <umps3/umps/libumps.h>
#include "pandos_types.h"

typedef unsigned int devreg;
/* This function returns the terminal transmitter status value given its address */
devreg termstat(memaddr *stataddr);
/* This function prints a string on specified terminal and returns TRUE if
 * print was successful, FALSE if not   */
unsigned int termprint(char *str, unsigned int term);
/* This function placess the specified character string in okbuf and
 *	causes the string to be written out to terminal0 */
void addokbuf(char *strp);
/* This function placess the specified character string in errbuf and
 *	causes the string to be written out to terminal0.  After this is done
 *	the system shuts down with a panic message */
void adderrbuf(char *strp);
#endif