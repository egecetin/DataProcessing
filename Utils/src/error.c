#include "error.h"

volatile uint64_t alarmCtr;
volatile uint8_t loopFlag;
volatile uint8_t sigReadyFlag;

void alarmFunc(int signum)
{
    struct timespec ts;

    ++alarmCtr;

    if (!sigReadyFlag)
        sigReadyFlag = 1;
    if (loopFlag)
        alarm(ALARM_INTERVAL);

    return;
}

void backtracer(int signum)
{
    if (loopFlag)
    {
        // Cleanup

        // Give chance to break loops
        loopFlag = 0;
        sleep(2);

        // Trace error
        void *array[100];
        int size;

        // get void*'s for all entries on the stack
        size = backtrace(array, 100);

        // print strings for all entries
        write(STDERR_FILENO, "Error signal\n", 14);
        backtrace_symbols_fd(array, size, STDERR_FILENO);
        write(STDERR_FILENO, "Error printed\n", 15);

        sleep(3);

        write(STDERR_FILENO, "Aborting", 9);
        abort();
    }

    return;
}

void interruptFunc(int signum)
{
    if (loopFlag)
        loopFlag = false;
    else
        write(STDERR_FILENO, "Interrupt in progress...\n", 26);

    return;
}