#ifndef LOG_H
#define LOG_H

// stupid log just to have my own functions

#include <stdio.h>
#include <stdarg.h>

void output(FILE *stream, const char* msg, va_list valist)
{
    const char *msg_ptr = msg;
    while (*msg_ptr != 0)
    {
        if (*msg_ptr == '%')
        {
            msg_ptr++;
            if (*msg_ptr == 's')
            {
                const char *string = va_arg(valist, const char*);
                fprintf(stream, "%s", string);
            }
        }
        
        fputc(*msg_ptr, stderr);
        msg_ptr++;
    }
    
    fputc('\n', stderr);
}

void log(const char* msg, ...)
{
    va_list valist;
    va_start(valist, msg);
    output(stdout, msg, valist);
}

void error(const char* msg, ...)
{
    fprintf(stderr, "error: ");
    
    va_list valist;
    va_start(valist, msg);
    output(stdout, msg, valist);
}

#endif //LOG_H
