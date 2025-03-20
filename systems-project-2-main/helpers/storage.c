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
#include <libgen.h>
#include <string.h>

#include "storage.h"
#include "slist.h"
#include "blocks.h"
#include "inode.h"
#include "directory.h"

void storage_init(const char *path) {
    blocks_init(path);
    inode_table_init();
    directory_init();
}

int storage_stat(const char *path, struct stat *st) {
    inode_t *inode = get_inode_from_path(path);

    printf("Inode Information for %s: ", path);
    fflush(stdout);
    print_inode(inode);

    if(inode == NULL) {
        return -ENOENT;
    }

    st->st_mode = inode->mode;
    st->st_size = inode->size;
    st->st_nlink = inode->refs;
    return 0;
}

int storage_read(const char *path, char *buf, size_t size, off_t offset) {
    inode_t *inode = get_inode_from_path(path);
    if(inode == NULL || !(inode->mode & 0100000)) {
        return -ENOENT;
    }

    int block_num = inode_get_bnum(inode, offset / BLOCK_SIZE);
    void *block_data = blocks_get_block(block_num);
    memcpy(buf, (char *) block_data + (offset % BLOCK_SIZE), size);

    return size;
}

int storage_write(const char *path, const char *buf, size_t size, off_t offset) {
    inode_t *inode = get_inode_from_path(path);
    if(inode == NULL || !(inode->mode & 0100000)) {
        return -ENOENT;
    }

    int block_num = inode_get_bnum(inode, offset / BLOCK_SIZE);
    if(block_num == -1) {
        block_num = alloc_block();
        inode->block = block_num;
    }

    void *block_data = blocks_get_block(block_num);
    memcpy((char *) block_data + (offset % BLOCK_SIZE), buf, size);

    if(offset + size > inode->size) {
        inode->size = offset + size;
    }

    return size;
}

int storage_truncate(const char *path, off_t size) {
    if(truncate(path, size) == -1) {
        return -errno;
    }
    return 0;
}

int storage_mknod(const char *path, int mode) {
    int inum = alloc_inode();
    if(inum == -1) {
        return -ENOSPC;
    }

    inode_t *inode = get_inode(inum);
    inode->mode = mode;
    inode->block = alloc_block();
    inode->size = 0;

    inode_t *parent_inode = get_inode_from_path(get_dirname(path));
    if(parent_inode == NULL) {
        return -ENOENT;
    }

    printf("Parent INode information (%s): ", get_dirname(path));
    print_inode(parent_inode);

    printf("Current INode information (%s): ", path);
    print_inode(inode);
    
    return directory_put(parent_inode, get_basename(path), inum);
}

int storage_unlink(const char *path) {
    inode_t *inode = get_inode_from_path(path);
    if(inode == NULL) {
        return -ENOENT;
    }

    inode_t *parent_inode = get_inode_from_path(get_dirname(path));
    if(parent_inode == NULL) {
        return -ENOENT;
    }

    int inum = directory_lookup(parent_inode, get_basename(path));
    if(inum == -1) {
        return -ENOENT;
    }

    free_block(inode->block);
    free_inode(inum);
    return directory_delete(parent_inode, get_basename(path));
}

int storage_link(const char *from, const char *to) {
    if(link(from, to) == -1) {
        return -errno;
    }
    return 0;
}

int storage_rename(const char *from, const char *to) {
    inode_t *from_inode = get_inode_from_path(from);
    if(from_inode == NULL) {
        return -ENOENT;
    }

    char *from_dir = get_dirname(from);
    char *from_name = get_basename(from);
    char *to_dir = get_dirname(to);
    char *to_name = get_basename(to);

    inode_t *from_dir_inode = get_inode_from_path(from_dir);
    inode_t *to_dir_inode = get_inode_from_path(to_dir);

    if(from_dir_inode == NULL || to_dir_inode == NULL) {
        free(from_dir);
        free(from_name);
        free(to_dir);
        free(to_name);
        return -ENOENT;
    }

    int put = directory_put(to_dir_inode, to_name, from_inode->num);

    if(put != 0) {
        free(from_dir);
        free(from_name);
        free(to_dir);
        free(to_name);
        return put;
    }

    int del = directory_delete(from_dir_inode, from_name);

    if(del != 0) {
        free(from_dir);
        free(from_name);
        free(to_dir);
        free(to_name);
        return del;
    }

    free(from_dir);
    free(from_name);
    free(to_dir);
    free(to_name);

    return 0;
}

int storage_set_time(const char *path, const struct timespec ts[2]) {
    /*struct utimbuf times;
    times.actime = ts[0].tv_sec;
    times.modtime = ts[1].tv_sec;
    if(utime(path, &times) == -1) {
        return -errno;
    }*/

    return 0;
}

slist_t *storage_list(const char *path) {
    return directory_list(path);
}