#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <utime.h>
#include <string.h>
#include <libgen.h>

#include "storage.h"
#include "slist.h"
#include "blocks.h"
#include "inode.h"
#include "directory.h"

// initializes a directory and stores it in an inode

void directory_init() {
    int inode = alloc_inode();
    inode_t *root = get_inode(inode);
    int block = alloc_block();
    if(block == 2) {
        root->mode = 040755;
        root->size = 0;
        root->block = block;
        printf("Initiated the root directory with inode");
        print_inode(root);
    } else {
        free_block(block);
        free_inode(inode);
        printf("There is already a root directory\n");
    }
}

// Looks for a specific entry in a directory

int directory_lookup(inode_t *di, const char *name) {
    dirent_t *entries = (dirent_t *) blocks_get_block(di->block);
    int entry_count = di->size / sizeof(dirent_t);
    printf("Looking up %s with %d entries\n", name, entry_count);

    for(int i = 0; i < entry_count; i ++) {
        printf("Comparing Entry %d: %s against %s\n", i, entries[i].name, name);
        if(strcmp(entries[i].name, name) == 0) {
            return entries[i].inum;
        }
    }
    return -1;
}

// Puts an entry into the directory

int directory_put(inode_t *di, const char *name, int inum) {
    if(strlen(name) >= DIR_NAME_LENGTH) {
        return -ENAMETOOLONG;
    }

    dirent_t *entries = (dirent_t *) blocks_get_block(di->block);
    int entry_count = di->size / sizeof(dirent_t);

    for(int i = 0; i < entry_count; i ++) {
        if(strcmp(entries[i].name, name) == 0) {
            return -EEXIST;
        }
    }

    strncpy(entries[entry_count].name, name, DIR_NAME_LENGTH);
    entries[entry_count].inum = inum;
    di->size += sizeof(dirent_t);

    printf("Directory INode information: ");
    print_inode(di);

    return 0;
}

// Deletes an entry into a directory

int directory_delete(inode_t *di, const char *name) {
    dirent_t *entries = (dirent_t*) blocks_get_block(di->block);
    int entry_count = di->size / sizeof(dirent_t);

    for(int i = 0; i < entry_count; i ++) {
        if(strcmp(entries[i].name, name) == 0) {
            memmove(&entries[i], &entries[i + 1], (entry_count - i - 1) * sizeof(dirent_t));
            di->size -= sizeof(dirent_t);
            return 0;
        }
    }
    return -ENOENT;
}


slist_t *directory_list(const char *path) {
    printf("Looking for path %s\n", path);
    inode_t *directory = get_inode_from_path(path);
    printf("Current directory INode: ");
    print_inode(directory);
    if(directory == NULL) {
        return NULL;
    }

    slist_t *list = NULL;
    dirent_t *entries = (dirent_t *) blocks_get_block(directory->block);
    int entry_count = directory->size / sizeof(dirent_t);

    for(int i = 0; i < entry_count; i ++) {
        size_t path_len = strlen(path);
        size_t name_len = strlen(entries[i].name);
        char *full_path = malloc(path_len + name_len + 2);

        if(path[path_len - 1] == '/') {
            snprintf(full_path, path_len + name_len + 2, "%s%s", path, entries[i].name);
        } else {
            snprintf(full_path, path_len + name_len + 2, "%s/%s", path, entries[i].name);
        }

        printf("Found file %d of %d named %s\n", i, entry_count, entries[i].name);
        list = slist_cons(entries[i].name, list);

        free(full_path);
    }

    return list;
}

// Prints the contents of the directory
void print_directory(inode_t *dd) {
    dirent_t *entries = (dirent_t *) blocks_get_block(dd->block);
    int entry_count = dd->size / sizeof(dirent_t);

    printf("Contents:\n");
    for(int i = 0; i < entry_count; i ++) {
        printf("  %s (inode %d)\n", entries[i].name, entries[i].inum);
    }
}

char *get_dirname(const char *path) {
    char *path_copy = strdup(path);
    if(path_copy == NULL) {
        return NULL;
    }
    char *dir = dirname(path_copy);
    char *result = strdup(dir);
    free(path_copy);
    return result;
}

char *get_basename(const char *path) {
    char *path_copy = strdup(path);
    if(path_copy == NULL) {
        return NULL;
    }
    char *base = basename(path_copy);
    char *result = strdup(base);
    free (path_copy);
    return result;
}