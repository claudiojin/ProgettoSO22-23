#include "library.h"

#define TRANSMITTED 5
#define ACK 1
#define PRINTCHR 2
#define CHAROFFSET 8
#define STATUSMASK 0xFF
#define TERM0ADDR 0x10000254

char okbuf[2048]; /* sequence of progress messages */
char errbuf[128]; /* contains reason for failing */
char msgbuf[128]; /* nonrecoverable error message before shut down */
char *mp = okbuf;

devreg termstat(memaddr *stataddr)
{
    return ((*stataddr) & STATUSMASK);
}

unsigned int termprint(char *str, unsigned int term)
{
    memaddr *statusp;
    memaddr *commandp;
    devreg stat;
    devreg cmd;
    unsigned int error = FALSE;

    if (term < DEVPERINT)
    {
        /* terminal is correct */
        /* compute device register field addresses */
        statusp = (devreg *)(TERM0ADDR + (term * DEVREGSIZE) + (TRANSTATUS * DEVREGLEN));
        commandp = (devreg *)(TERM0ADDR + (term * DEVREGSIZE) + (TRANCOMMAND * DEVREGLEN));

        /* test device status */
        stat = termstat(statusp);
        if (stat == READY || stat == TRANSMITTED)
        {
            /* device is available */

            /* print cycle */
            while (*str != EOS && !error)
            {
                cmd = (*str << CHAROFFSET) | PRINTCHR;
                *commandp = cmd;

                /* busy waiting */
                stat = termstat(statusp);
                while (stat == BUSY)
                    stat = termstat(statusp);

                /* end of wait */
                if (stat != TRANSMITTED)
                    error = TRUE;
                else
                    /* move to next char */
                    str++;
            }
        }
        else
            /* device is not available */
            error = TRUE;
    }
    else
        /* wrong terminal device number */
        error = TRUE;

    return (!error);
}

void addokbuf(char *strp)
{
    char *tstrp = strp;
    while ((*mp++ = *strp++) != '\0')
        ;
    mp--;
    termprint(tstrp, 0);
}

void adderrbuf(char *strp)
{
    char *ep = errbuf;
    char *tstrp = strp;

    while ((*ep++ = *strp++) != '\0')
        ;

    termprint(tstrp, 0);

    PANIC();
}