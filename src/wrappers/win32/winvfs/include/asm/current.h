#pragma once

#include "linux/kernel.h"
#include <linux/sched.h>

extern struct task_struct current_struct;

#define current (&current_struct)
