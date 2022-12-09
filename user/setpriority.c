#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(2, "Usage: setpriority <priority> <pid>\n");
        exit(1);
    }

    int priority = atoi(argv[1]);
    int process = atoi(argv[2]);

    if (priority > 0 && priority < 100)
    {
        // for debugging
        printf("pid = %d, priority = %d\n", process, priority);
        setpriority(priority, process);
        exit(0);
    }

    fprintf(2, "Invalid priority set a value from 0-100\n");
    exit(1);
}
