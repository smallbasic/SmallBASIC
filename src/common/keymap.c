// This file is part of SmallBASIC
//
// Support for keyboard event handling
// eg: sub foo: end: DEFINEKEY "s", foo 'run foo when s pressed
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2010 Chris Warren-Smith. [http://tinyurl.com/ja2ss]

#include "common/sys.h"
#include "common/var.h"
#include "common/smbas.h"
#include "common/pproc.h"
#include "common/keymap.h"

//  Keyboard buffer
word keybuff[PCKBSIZE];
int keyhead;
int keytail;

typedef struct key_map_s key_map_s;

/**
 * keyboard event handler
 */
struct key_map_s {
  key_map_s *next; // next structure element
  addr_t ip;       // handler location
  int key;         // key definition
};

key_map_s* keymap = 0;

/**
 * Prepare task_t exec.keymap for keymap handling at program init
 */
void keymap_init() {
  keymap = 0;
}

/**
 * Handler for keymap_free()
 */
void keymap_delete(key_map_s* km) {
  if (km) {
    keymap_delete(km->next);
    free(km);
  }
}

/**
 * Cleanup task_t exec.keymap at program termination
 */
void keymap_free() {
  keymap_delete(keymap);
  keymap = 0;
}

/**
 * DEFINEKEY command handler to add a keymap 
 */
void keymap_add(int key, addr_t ip) {
  key_map_s* km = (key_map_s*) malloc(sizeof (key_map_s));
  km->next = 0;
  km->ip = ip;
  km->key = key;

  // add the new mapping onto the linked list
  key_map_s* head = keymap;
  if (!head) {
    keymap = km;
  } else {
    while (head->next) {
      head = head->next;
    }
    head->next = km;
  }
}

/**
 * invokes the handler for the given key
 */
int keymap_invoke(word key) {
  int result = 0;
  key_map_s* head = keymap;
  while (head) {
    if (head->key == key) {
      addr_t ip = prog_ip; // store current ip
      prog_ip = head->ip;  // jump to keymap ip
      bc_loop(1);          // invoke the keymap code
      prog_ip = ip;        // restore the current ip
      result = 1;          // key was consumed
    }
    head = head->next;
  }
  return result;
}

/**
 * returns whether a key has been pressed
 */
int keymap_kbhit() {
  return (keytail != keyhead);
}

/**
 * returns the last key without removing it
 */
int keymap_kbpeek() {
  return keybuff[keyhead];
}

/**
 * clear keyboard buffer
 */
void dev_clrkb() {
  keyhead = keytail = 0;
}

/**
 * stores a key in keyboard buffer
 */
void dev_pushkey(word key) {
  if (key <= SB_KEY_MK_LAST) {
    keybuff[keytail] = key;
    keytail++;
    if (keytail >= PCKBSIZE) {
      keytail = 0;
    }
  }

  keymap_invoke(key);
}

/**
 * returns true if there is an key in keyboard buffer
 */
int dev_kbhit() {
  if (keytail != keyhead) {
    return 1;
  }

  // conserve battery power
  int code = dev_events(2);

  if (code < 0) {
    brun_break();
  }
  return (keytail != keyhead);
}

/**
 * returns the next key in keyboard buffer (and removes it)
 */
long int dev_getch() {
  while ((dev_kbhit() == 0) && (prog_error == 0)) {
    int evc = dev_events(2);
    if (evc < 0 || prog_error) {
      return 0xFFFF;
    }
  }

  int ch = keybuff[keyhead];
  keyhead++;
  if (keyhead >= PCKBSIZE) {
    keyhead = 0;
  }
  return ch;
}

void timer_free(timer_s *timer) {
  if (timer) {
    timer_free(timer->next);
    free(timer);
  }
}

void timer_add(var_num_t interval, addr_t ip) {
  timer_s* timer = (timer_s*) malloc(sizeof (timer_s));
  timer->next = NULL;
  timer->ip = ip;
  timer->interval = interval;
  timer->value = 0;
  timer->active = 0;

  // add the new timer onto the linked list
  timer_s* head = prog_timer;
  if (!head) {
    prog_timer = timer;
  } else {
    while (head->next) {
      head = head->next;
    }
    head->next = timer;
  }
}

void timer_run(dword now) {
  timer_s* timer = prog_timer;
  while (timer) {
    if (timer->value == 0) {
      // start timer
      timer->value = now + timer->interval;
    } else if (now > timer->value && !timer->active) {
      // timer expired
      timer->active = 1;
      addr_t ip = prog_ip;
      prog_ip = timer->ip;
      bc_loop(1);
      prog_ip = ip;

      // reset for next interval
      timer->value = now + timer->interval;
      timer->active = 0;
    }
    timer = timer->next;
  }
}
