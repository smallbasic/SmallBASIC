// -*- c-file-style: "java" -*-
// $Id: ebm_mem.cpp,v 1.2 2004-04-12 00:21:41 zeeb90au Exp $
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2003 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
/*                  _.-_:\
//                 /      \
//                 \_.--*_/
//                       v
*/
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include <malloc.h>

struct DBTable {
    void* p;
    int size;
};

int defSize = 16;
const int growSize = 16;

// Create a table
extern "C" void* dbt_create(const char *fileName) {
    DBTable* head = (DBTable*)malloc(sizeof(DBTable));
    DBTable* list = (DBTable*)malloc(sizeof(DBTable) * defSize);
    for (int i=0; i<defSize; i++) {
        list[i].size = 0;
        list[i].p = 0;
    }
    head->p = list;
    head->size = defSize;
    return head;
}

// Close table
extern "C" void dbt_close(void* t) {
    DBTable* head = (DBTable*)t;
    DBTable* list = (DBTable*)head->p;
    for (int i=0; i<head->size; i ++) {
        if (list[i].p) {
            free(list[i].p);
        }
    }
    free(list);
    free(head);
}

// Unused
extern "C" void dbt_prealloc(void* f, int num, int size) {}

// Ensure the given index is valid
void validateIndex(DBTable* head, int index, int size) {
    int tsize = head->size;
    DBTable* list = (DBTable*)head->p;
    if (tsize <= index) {
        // grow table
        defSize = tsize + growSize;
        head->p = realloc(head->p, sizeof(DBTable) * defSize);
        list = (DBTable*)head->p;
        for (int i=tsize; i<defSize; i++) {
            list[i].size = 0;
            list[i].p = 0;
        }
        head->size = defSize;
    }
    if (list[index].p == 0) {
        list[index].p = malloc(size);
        list[index].size = malloc_usable_size(list[index].p);
    } else if (size > list[index].size) { 
        free(list[index].p);
        list[index].p = malloc(size);
        list[index].size = malloc_usable_size(list[index].p);
    }
}

// Store an element
extern "C" void dbt_write(void* t, int index, void *ptr, int size) {
    validateIndex((DBTable*)t, index, size);
    DBTable* head = (DBTable*)t;
    DBTable* list = (DBTable*)head->p;
    memcpy(list[index].p, ptr, size);
}

// Load an element
extern "C" void dbt_read(void* t, int index, void *ptr, int size) {
    validateIndex((DBTable*)t, index, size);
    DBTable* head = (DBTable*)t;
    DBTable* list = (DBTable*)head->p;
    memcpy(ptr, list[index].p, size);
}

extern "C" int memmgr_getmaxalloc(void) {
    struct mallinfo x = mallinfo();
    return x.uordblks;
}

