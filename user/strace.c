#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  int i;
  char *args[MAXARG];

  if (argc < 3 || (argv[1][0] < '0'))
  {
    fprintf(2, "Wrong usage\n", argv[0]);
    exit(1);
  }

  else
  {
    if (strace(atoi(argv[1])) < 0)
    {
      fprintf(2, "%s: strace failed\n", argv[0]);
      exit(1);
    }

    for (i = 2; i < argc && i < MAXARG; i++)
    {
      args[i - 2] = argv[i];
    }
    exec(args[0], args);

    exit(0);
  }
}