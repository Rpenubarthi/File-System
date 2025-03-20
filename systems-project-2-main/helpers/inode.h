// Inode manipulation routines.

#ifndef INODE_H
#define INODE_H

#include "blocks.h"

typedef struct inode {
  int num;   // tracks the ID of the current node
  int refs;  // reference count
  int mode;  // permission & type
  int size;  // bytes
  int block; // single block pointer (if max file size <= 4K)
} inode_t;

// initializes the inode table
void inode_table_init();

// prints out the data for the given INode
void print_inode(inode_t *node);

// gets the INode in the given index
inode_t *get_inode(int inum);

// allocates an INode
int alloc_inode();

// deallocates the given INode
void free_inode(int inum);

// makes an INode larger
int grow_inode(inode_t *node, int size);

// makes an INode smaller
int shrink_inode(inode_t *node, int size);

// gets the block number for the specified INode
int inode_get_bnum(inode_t *node, int file_bnum);

// gets the inode given a path
inode_t *get_inode_from_path(const char *path);

#endif
