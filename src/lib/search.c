// This file is part of SmallBASIC
//
// Tree search generalized from Knuth (6.2.2) Algorithm T just like
// the AT&T man page says.
//
// The node_t structure is for internal use only, lint doesn't grok it.
//
// Written by reading the System V Interface Definition, not the code.
//
// Totally public domain.
//

#include <sys/cdefs.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "search.h"

/* Walk the nodes of a tree */
tdestroy_recurse(node_t *root, tdestroy_cb freefct) {
  if (root->left != NULL) {
    tdestroy_recurse(root->left, freefct);
  }
  if (root->right != NULL) {
    tdestroy_recurse(root->right, freefct);
  }
  freefct((void *) root->key);
  free(root);
}

/* Walk the nodes of a tree */
void trecurse(const node_t *root, twalk_cb action, int level) {
  if (root->left == NULL && root->right == NULL) {
    action(root, leaf, level);
  } else {
    action(root, preorder, level);
    if (root->left != NULL) {
      trecurse(root->left, action, level + 1);
    }
    action(root, postorder, level);
    if (root->right != NULL) {
      trecurse(root->right, action, level + 1);
    }
    action(root, endorder, level);
  }
}

/* find a node, or return 0 */
void *tfind(const void *vkey, void **vrootp, tcompare_cb compar) {
  node_t **rootp = (node_t **)vrootp;

  while (rootp != NULL && *rootp != NULL) {
    int r = compar(vkey, (*rootp)->key);
    if (r == 0) {
      // found
      return *rootp;
    }
    rootp = (r < 0) ? &(*rootp)->left : &(*rootp)->right;
  }
  return NULL;
}

/* find or insert datum into search tree */
void *tsearch(const void *vkey, void **vrootp, tcompare_cb compar) {
  node_t **rootp = (node_t **)vrootp;
  if (rootp == NULL) {
    return NULL;
  }
  while (*rootp != NULL) {
    int r;
    if ((r = compar(vkey, (*rootp)->key)) == 0) {
      return *rootp;
    }
    rootp = (r < 0) ? &(*rootp)->left : &(*rootp)->right;
  }
  node_t *q = malloc(sizeof(node_t));
  if (q != 0) {
    *rootp = q;
    q->key = (void *)vkey;
    q->left = q->right = NULL;
  }
  return q;
}

void tdestroy(void *vroot, tdestroy_cb freefct) {
  node_t *root = (node_t *)vroot;
  if (root != NULL) {
    tdestroy_recurse(root, freefct);
  }
}

/* Walk the nodes of a tree */
void twalk(const void *vroot, twalk_cb action) {
  if (vroot != NULL && action != NULL) {
    trecurse(vroot, action, 0);
  }
}
