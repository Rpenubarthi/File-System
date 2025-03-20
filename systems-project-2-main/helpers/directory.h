// Directory manipulation functions.

#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME_LENGTH 48

#include "blocks.h"
#include "inode.h"
#include "slist.h"

typedef struct dirent {
  char name[DIR_NAME_LENGTH];
  int inum;
  char _reserved[12];
} dirent_t;

// initializes a new directory
void directory_init();

// looks up a directory
int directory_lookup(inode_t *di, const char *name);

// adds a file to a directory
int directory_put(inode_t *di, const char *name, int inum);

// deletes a file from a directory
int directory_delete(inode_t *di, const char *name);

// lists all files/directories in the current directory
slist_t *directory_list(const char *path);

// prints the contents of the current directory
void print_directory(inode_t *dd);

// gets the directory name of the current path
char *get_dirname(const char *path);

// gets the base name of the current path
char *get_basename(const char *path);

#endif
