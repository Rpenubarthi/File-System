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

#include "bitmap.h"
#include "storage.h"
#include "slist.h"
#include "blocks.h"
#include "inode.h"
#include "directory.h"

#define TABLE_SIZE 256

// static inode_t table[TABLE_SIZE];
static inode_t *table;

void inode_table_init() {
    table = get_inode_table();

    if(table[0].block == 0) {
        printf("This is a new table!\n");
        for(int i = 0; i < TABLE_SIZE; i ++) {
            table[i].num = i;
            table[i].refs = 0;
            table[i].mode = 0;
            table[i].size = 0;
            table[i].block = -1;
        }
        printf("Successfully initiated the INode Table at address %p\n", (void *) table);
    } else {
        printf("There is already data in this table!  It is at address %p\n", (void *) table);
    }
}

void print_inode(inode_t *node) {
    if(node == NULL) {
        printf("UH OH!  Found a NULL INode\n");
    } else {
        printf("inode #%d { refs: %d, mode: %04o, size: %d, block: %d }\n", node->num, node->refs, node->mode, node->size, node->block);
    }
}


inode_t *get_inode(int inum) {
    if(inum < 0 || inum >= TABLE_SIZE) {
        return NULL;
    }
    return &table[inum];
}

int alloc_inode() {
    void *bitmap = get_inode_bitmap();

    for(int i = 0; i < TABLE_SIZE; i ++) {
        if(!bitmap_get(bitmap, i)) {
            bitmap_put(bitmap, i, 1);
            table[i].refs = 1;
            table[i].mode = 0;
            table[i].size = 0;
            table[i].block = -1;
            printf("+ alloc_inode() -> %d\n", i);
            return i;
        }
    }
    return -1;
}

void free_inode(int inum) {
    if(inum < 0 || inum >= TABLE_SIZE) {
        return;
    }

    table[inum].refs = 0;
    table[inum].mode = 0;
    table[inum].size = 0;
    table[inum].block = -1;
    
    printf("+ free_inode(%d)\n", inum);

    void *bitmap = get_inode_bitmap();
    bitmap_put(bitmap, inum, 0);
}

int grow_inode(inode_t *node, int size) {
    if(size > BLOCK_SIZE) {
        return -1;
    }

    node->size = size;
    return 0;
}

int shrink_inode(inode_t *node, int size) {
    if(size < 0 || size > BLOCK_SIZE) {
        return -1;
    }

    node->size = size;
    return 0;
}

int inode_get_bnum(inode_t *node, int file_bnum) {
    if(file_bnum != 0) {
        return -1;
    }

    return node->block;
}

inode_t *get_inode_from_path(const char *path) {
    if(strcmp(path, "/") == 0) {
        printf("Root detected\n");
        return get_inode(0);
    }

    slist_t *components = slist_explode(path, '/');
    slist_t *current = components;

    // inode_t *current_inode = get_inode(0);

    // printf("Getting INode from path; currently %s: ", current->data);
    // print_inode(current_inode);

    /* if(current_inode == NULL || !(current_inode->mode & 040000)) {
        printf("NULL INode detected\n");
        slist_free(components);
        return NULL;
    } */

   inode_t *current_inode;

    while(current != NULL) {
        int inum;
        if(strcmp(current->data, "") == 0) {
            inum = 0;
        } else {
            inum = directory_lookup(current_inode, current->data);
        }
        printf("Result of directory lookup: %d\n", inum);
        if(inum == -1) {
            slist_free(components);
            return NULL;
        }
        current_inode = get_inode(inum);
        current = current->next;

        printf("Opening directory, next INode information: ");
        print_inode(current_inode);
    }

    slist_free(components);
    return current_inode;
}