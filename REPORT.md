# Assignment 4 Report

# Setup / Run

To run the operating system emulator, simply run the following command:

```jsx
make qemu [ARGUMENTS]
```

# System Calls

We have implemented three different system calls in this assignment - `strace`, `sigalarm`, and `sigreturn`. These system calls make use of the distinction and communication between user, and kernel privilege levels. 

## `str`

`mask` provides us the process in binary. The structure - `proc` is modified to hold the value mask in `proc.h`. `strace` is defined in `syscall.h` and have the logic for it in `syscall.c`.

`strace.c` has been added to `user` folder and `makefile` was updated accordingly. We also handle the child processes by setting the mask of the child equal to the parent in `proc.c`.

### Crucial Code Blocks

```jsx

```

### Input

```bash
strace mask command [args]
```

## `sigalarm` and `sigreturn`

These functions periodically send out an alarm at an interval specified by the function call. The system call is served to the user side through `alarm` which can be provided the following arguments:

- Interval: After how many ticks, the `alarm` function is called
- Handler: The function that will be called, when the `alarm` function is called

### Implementation

For the implantation of these system calls we have modified the following files, apart from the basic system call files:

- `alarmtest.c` - Testing the alarm function
- `trap.c` - To write the trap handler, for when the user process sends an interrupt to the kernel

```jsx
if (which_dev == 2 && p->alarm_on == 0) {
        // Save trapframe
        p->alarm_on = 1;
        // print cur ticks and ticks
        struct trapframe *tf = kalloc();
        memmove(tf, p->trapframe, PGSIZE);
        // printf("cur ticks: %d, ticks: %d\n", ticks, p->ticks);
        p->alarm_tf = tf;
        p->cur_ticks++;
        // printf("cur ticks: %d, ticks: %d\n", ticks, p->ticks);
        if (p->cur_ticks >= p->ticks){
          // printf("BONGONODSGNODGNO\n");
          // printf("alarm is being ccalled becayse ticks = %d\n", p->cur_ticks);
          
          p->cur_ticks = 0;
          p->trapframe->epc = p->handler;
        }
        else
        {
          p->alarm_on = 0;
          kfree(tf);
        }
```

- `sysproc.c` - To implement the kernel side functions for the system calls. These can in-turn, if need be, call user side functions too.

```c
// Save tf image in process new field ``alarm_tf`` Increment ticks. 
// If ticks is reaches max, set epc program counter to the handler 
// (which enables us to call the handler function)
sys_alarm(void)
{
  uint64 addr;
  int ticks;

  argint(0, &ticks);
  argaddr(1, &addr);

  myproc()->ticks = ticks;
  myproc()->handler = addr;
  return 0;
}

// this function simply initialises all the process variables to their initial state.
// it is called in each handler for clean-up.
uint64
sys_sigreturn(void)
{
  struct proc *p = myproc();
  memmove(p->trapframe, p->alarm_tf, PGSIZE);

  kfree(p->alarm_tf);
  p->alarm_tf = 0;
  p->alarm_on = 0;
  p->cur_ticks = 0;
  return 0;
}
```

### Input

```bash
alarmtest
```

### Results/Output

![Untitled](Assignment%204%20Report%208978b3c6eb42476e8f82b03fecc0de6d/Untitled.png)

# Schedulers

For the implementation of the schedulers, we have changed the following files.

```jsx
proc.c
trap.c
param.h
```

We used the `testscheduler.c` to obtain information like waiting time, creation time, etc. for our schedulers.

## First Come First Serve

Takes the processes in the order they came in and executes each one till completion before moving on to the next process.

The way the scheduler decides which process came first is by looking at the creation time of each process. For this we store the creation time of every process as a new variable called `ctime` in the proc structure and assign it in the `allocproc` fucntion when the process starts. The logic is implemented in `scheduler` fucntion in `proc.c`. This algortihm is non-preemtive which means that the clock interrupt is disabled in `usertrap` and `kerneltrap`, thus being able to complete a process before moving on to the next one. This modification was done in `trap.c`.

### Input

```jsx
make qemu SCHEDULER=FCFS
```

### Crucial Code Blocks

```jsx
struct proc *process_to_run = 0;

    for (p = proc; p < &proc[NPROC]; p++)
    {
      acquire(&p->lock);

      if (p->state != RUNNABLE)
      {
        release(&p->lock);
        continue;
      }

      if (process_to_run == 0)
        process_to_run = p;

      else
      {
        if (p->ctime < process_to_run->ctime)
        {
          release(&process_to_run->lock);
          process_to_run = p;
        }
        else
        {
          release(&p->lock);
        }
      }
    }

    if (process_to_run != 0)
    {
      process_to_run->state = RUNNING;
      c->proc = process_to_run;
      swtch(&c->context, &process_to_run->context);
      c->proc = 0;
      release(&process_to_run->lock);
    }
```

## Lottery Based Scheduling

Most of the code for the scheduler can be found in `proc.c`; the associated header file, `proc.h` too. The basic idea is simple: assign each running process a slice of the processor in proportion to the number of tickets it has. The more tickets a process has, the more it runs. Each time slice, a randomized lottery determines the winner of the lottery; that winning process is the one that runs for that time slice.

We assign tickets to a process when it is created. Specifically, we make sure a child process inherits the same number of tickets as its parent. Thus, if the parent has 10 tickets, and calls fork() to create a child process, the child should also get 10 tickets. 

### Input

```jsx
make qemu SCHEDULER=LBS 
```

### Crucial Code Blocks

```jsx
// To get the number of tickets assigned till now
    struct proc *pr;
    int total = 0;
    for (pr = proc; pr < &proc[NPROC]; pr++)
    {
      if (pr->state == RUNNABLE)
      {
        total += pr->tickets;
      }
    }

    int total_tickets = total;
    int ticket_drawn = -1;
    int count_tickets = 0;

    // Drawing a random ticket
    if (total_tickets > 0 || ticket_drawn <= 0)
    {
      ticket_drawn = random() % total_tickets;
    }

    struct proc *process_to_run = 0;

    for (p = proc; p < &proc[NPROC]; p++)
    {
      acquire(&p->lock);

      if (p->state != RUNNABLE)
      {
        release(&p->lock);
        continue;
      }

      if (process_to_run == 0)
        process_to_run = p;

      else
      {
        if (count_tickets + p->tickets >= ticket_drawn)
        {
          release(&process_to_run->lock);
          process_to_run = p;
        }
        else
        {
          release(&p->lock);
        }
      }
    }

    if (process_to_run != 0)
    {
      process_to_run->state = RUNNING;
      c->proc = process_to_run;
      swtch(&c->context, &process_to_run->context);
      c->proc = 0;
      release(&process_to_run->lock);
    }

    //   if (p->state == RUNNABLE)
    //   {
    //     int total_tickets = totalTickets();
    //     int number_drawn = -1;
    //     if (!(total_tickets <= 0) || !(number_drawn > 0))
    //       number_drawn = random() % total_tickets;

    //     number_drawn = number_drawn - p->tickets;

    //     if (!(number_drawn < 0))
    //     {
    //       release(&p->lock);
    //       continue;
    //     }
    //     p->state = RUNNING;
    //     c->proc = p;
    //     swtch(&c->context, &p->context);
    //     c->proc = 0;
    //   }
    //   release(&p->lock);
```

## Priority Based Scheduling

The process with the highest priority is chosen by a scheduler with priorities. Every process has a dynamic priority that is determined and assigned during runtime.

The data that we already have are the `niceness` values, which are by default set to 5 and the static priority, which is set by default to $60$. We store these values as "niceness" and "static priority," respectively, in the `proc` structure.

A process's scheduled times, sleep time since its most recent run, and runtime since its most recent run, all of which are likewise stored in the proc structure, are additional pieces of information that we require.

This scheduling is non-preemptive, and hence clock interrupts will be disabled.

### Crucial Code Block

```jsx
struct proc *process_to_run = 0;

    for (p = proc; p < &proc[NPROC]; p++)
    {
      acquire(&p->lock);

      if (p->state != RUNNABLE)
      {
        release(&p->lock);
        continue;
      }

      if ((p->rtime + p->stime) != 0)
        p->niceness = ((p->stime) / ((p->rtime) + (p->stime))) * 10;

      else
        p->niceness = 0;

      int dp = p->priority - p->niceness + 5;

      if (dp > 100)
        dp = 100;

      else if (dp < 0)
        dp = 0;

      p->dynamic_priority = dp;

      if (process_to_run != 0)
      {
        if (p->dynamic_priority < process_to_run->dynamic_priority)
        {
          release(&process_to_run->lock);
          process_to_run = p;
        }
        else
          release(&p->lock);
      }
      else
        process_to_run = p;
    }

    if (process_to_run != 0)
    {
      process_to_run->num_of_runs += 1;
      process_to_run->state = RUNNING;
      c->proc = process_to_run;
      swtch(&c->context, &process_to_run->context);
      c->proc = 0;
      release(&process_to_run->lock);
    }
```

### Input

```jsx
make qemu SCHEDULER=PBS
```

## Comparison of results

We get the same benchmarks for Round Robin scheduler, that comes inbuilt with the basic xv6 OS. 

| Scheduler | Average Waiting Time | Average Running Time |
| --- | --- | --- |
| Round Robin Scheduler | 17 | 170 |
| Multi Level Feedback Queue Scheduler | 17 | 168 |
| First Come First Serve Scheduler | 35 | 163 |
| Lottery Based Scheduler | 23 | 167 |
| Priority Based Scheduler | 18 | 226 |

## Benchmarking Conclusion

From the table above, we notice that RR and MLFQ fares the best in terms of average waiting time (response time), and the same in terms of average running time (turnaround time). We see that FCFS has the best turnaround time, as the whole process is finished at once, without preemption.

# MLFQ Benchmarking

### Output

```jsx
$ testscheduler
exec testscheduler failed
$ schedulertest
Process 6 finished
Process 7 finished
Process 5 finished
Process 8 finished
Process 9 finished
Process 21 finished

Process 3 finished
Process shed
4 finished
process 0 finished
Average rtime 26,  wtime 123
```

### Method

```jsx
void update_time()
{
  struct proc *p;
  for (p = proc; p < &proc[NPROC]; p++)
  {
    acquire(&p->lock);
    if (p->state == RUNNING)
    {
      p->rtime++;
      p->rtime_lastrun++;
    }
    else if (p->state == SLEEPING)
    {
      p->stime++;
      p->stime_lastrun++;
    }
    else if (p->state == RUNNABLE)
    {
      p->wtime++;
      p->age_lastrun++;
    }
    if (p->state == RUNNING || p->state == RUNNABLE)
    {
      p->pool_time[p->pool]++;
    }

    if (p->isReset)
    {
      p->niceness = 5;
      p->isReset = 0;
    }
    else if (p->rtime_lastrun > 0 || p->stime_lastrun > 0) // denominatior > 0
      p->niceness = (int)(((p->stime_lastrun * 10) / (p->stime_lastrun + p->rtime_lastrun)));
    release(&p->lock);
  }
}
```

### Graph

![Untitled](Assignment%204%20Report%208978b3c6eb42476e8f82b03fecc0de6d/Untitled%201.png)