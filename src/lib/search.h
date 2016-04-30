// This file is part of SmallBASIC
//
// Written by J.T. Conklin <jtc@netbsd.org>
// Public domain.
//

#ifndef _SEARCH_H_
#define _SEARCH_H_

#include <sys/types.h>

typedef enum {
  preorder,
  postorder,
  endorder,
  leaf
} VISIT;

typedef struct node {
  void *key;
  struct node *left, *right;
} node_t;

typedef void (*twalk_cb) (const void *nodep, VISIT value, int level);
typedef void (*tdestroy_cb) (void *__nodep);
typedef int (*tcompare_cb) (const void *, const void *);

void *tfind(const void *vkey, void **vrootp, tcompare_cb compar);
void *tsearch(const void *vkey, void **vrootp, tcompare_cb compar);
void tdestroy(void *, tdestroy_cb cb);
void twalk(const void *vroot, twalk_cb action);

#endif /* !_SEARCH_H_ */
