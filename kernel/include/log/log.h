/******************************************************************************
 * I know what I said about not using other people's code for this, but I did *
 * for this and probably only this, outside of C libraries. At least, I hope. *
 * (c) abb1x 2020                                                             *
 ******************************************************************************/

#ifndef LOG_H
#define LOG_H
#include <string.h>

enum status
{
    INFO,
    WARNING,
    DEBUG,
    PANIC,
    ERROR
};
void log(int status, char *format, ...);

#endif /* LOG_H */
