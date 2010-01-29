// $Id$
// This file is part of SmallBASIC
//
// Support for keyboard event handling
// eg: sub foo: end: DEFINEKEY "s", foo 'run foo when s pressed
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2010 Chris Warren-Smith. [http://tinyurl.com/ja2ss]

#include "sys.h"
#include "var.h"
#include "smbas.h"
#include "keymap.h"

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
    tmp_free(km);
  }
}

/**
 * Cleanup task_t exec.keymap at program termination
 */
void keymap_free() {
  keymap_delete(keymap);
}

/**
 * DEFINEKEY command handler to add a keymap 
 */
void keymap_add(int key, addr_t ip) {
  key_map_s* km = (key_map_s*) tmp_alloc(sizeof (key_map_s));
  km->next = 0;
  km->ip = ip;
  km->key = key;

  // add the new mapping onto the linked list
  key_map_s* head = keymap;
  if (!head) {
    keymap = km;
  } 
  else {
    while (head->next) {
      head = head->next;
    }
    head->next = km;
  }
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
  if (key < SB_KEY_MK_FIRST || key > SB_KEY_MK_LAST) {
    keybuff[keytail] = key;
    keytail++;
    if (keytail >= PCKBSIZE) {
      keytail = 0;
    }
  }

  key_map_s* head = keymap;
  while (head) {
    if (head->key == key) {
      addr_t ip = prog_ip; // store current ip
      prog_ip = head->ip;  // jump to keymap ip
      bc_loop(1);          // invoke the keymap code
      prog_ip = ip;        // restore the current ip
    }
    head = head->next;
  }
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

// End of $Id$
