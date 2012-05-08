// This file is part of SmallBASIC
//
// Task manager
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/smbas.h"
#include "common/tasks.h"

static task_t *tasks; /**< tasks table												@ingroup sys */
static int task_count; /**< total number of tasks										@ingroup sys */
static int task_index; /**< current task number										@ingroup sys */

/**
 *	@ingroup sys
 *
 *	return the number of the tasks
 *
 *	@return the number of the tasks
 */
int count_tasks() {
  return task_count;
}

/**
 *	@ingroup sys
 *
 *	return the active task-id
 *
 *	@return the active task-id
 */
int current_tid() {
  return task_index;
}

/**
 *	@ingroup sys
 *
 *	return the active task-structure
 *
 *	@return the active task-structure
 */
task_t *current_task() {
  return &tasks[task_index];
}

/**
 *	@ingroup sys
 *
 *	return the nth task-structure
 *
 *	@return the nth task-structure
 */
task_t *taskinfo(int n) {
  return &tasks[n];
}

/**
 *	@ingroup sys
 *
 *	initialize tasks manager
 */
int init_tasks() {
  int tid;

  tasks = NULL;
  task_count = 0;
  task_index = 0;
  ctask = NULL;
  tid = create_task("main");
  activate_task(tid);
  return tid;
}

/**
 *	@ingroup sys
 *
 *	destroys tasks and closes task manager
 */
void destroy_tasks() {
  int i;

  for (i = 0; i < task_count; i++)
    close_task(i);

  task_count = 0;
  task_index = 0;
  ctask = NULL;
  tmp_free(tasks);
  tasks = NULL;
}

/**
 *	@ingroup sys
 *
 *	create an empty new task
 *
 *	@param name is the task name
 *	@return the task-id
 */
int create_task(const char *name) {
  int tid = -1;
  int i;

  if (task_count == 0) {
    // this is the first task
    tid = task_count;
    task_count++;
    tasks = (task_t *) tmp_alloc(sizeof(task_t) * task_count);
  } else {
    // search for an available free entry
    for (i = 0; i < task_count; i++) {
      if (tasks[i].status == tsk_free) {
        tid = i;
        break;
      }
    }

    // create a new task
    if (tid == -1) {
      tid = task_count;
      task_count++;
      tasks = (task_t *) tmp_realloc(tasks, sizeof(task_t) * task_count);
    }
  }

  // init task
  memset(&tasks[tid], 0, sizeof(task_t));
  strcpy(tasks[tid].file, name);
  tasks[tid].status = tsk_ready;
  tasks[tid].parent = task_index;
  tasks[tid].tid = tid;

  return tid;
}

/**
 *	@ingroup sys
 *
 *	closes a task and activate the next
 */
void close_task(int tid) {
//      memset(&tasks[tid], 0, sizeof(task_t));
  tasks[tid].status = tsk_free;
  if (task_index == tid)
    ctask = NULL;

  // select next task
  if (task_count) {
    if (task_index == tid) {
      if (task_count > task_index + 1)
        task_index++;
      else
        task_index = 0;
      ctask = &tasks[task_index];
    }
  }
}

/**
 *	@ingroup sys
 *
 *	set active task
 *
 *	@param tid the task-id
 *	@return the previous task-id
 */
int activate_task(int tid) {
  int prev_tid;

  prev_tid = task_index;
  task_index = tid;
  ctask = &tasks[tid];
  return prev_tid;
}

/**
 *	@ingroup sys
 *
 *	search for a task
 *
 *	@param task_name the name of the task
 *	@return the task-id; or -1 on error
 */
int search_task(const char *task_name) {
  int i;

  for (i = 0; i < task_count; i++) {
    if (strcmp(tasks[i].file, task_name) == 0)
      return i;
  }
  return -1;
}
