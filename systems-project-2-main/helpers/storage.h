// Disk storage abstracttion.

#ifndef STORAGE_H
#define STORAGE_H

#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "slist.h"

// initializes the storage
void storage_init(const char *path);

// gets the attributes of the given file/directory
// result will be placed in *st
int storage_stat(const char *path, struct stat *st);

// read the contents of a file
// result will be placed in buf
// size - # of bytes to read; offset - # of bytes to offset
int storage_read(const char *path, char *buf, size_t size, off_t offset);

// write data to a file
// result will be read from buf
// size - # of bytes to write; offset - # of bytes to offset
int storage_write(const char *path, const char *buf, size_t size, off_t offset);

// changes the size of a file
int storage_truncate(const char *path, off_t size);

// makes a new file
int storage_mknod(const char *path, int mode);

// deletes a file
int storage_unlink(const char *path);

// creates a new link to an existing file
int storage_link(const char *from, const char *to);

// renames a file/directory
int storage_rename(const char *from, const char *to);

// updates the access and modification times of a file/directory
int storage_set_time(const char *path, const struct timespec ts[2]);

// lists the contents of the directory
slist_t *storage_list(const char *path);

#endif
