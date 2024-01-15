/**
 * @file vnode.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef VNODE_H
#define VNODE_H

typedef enum VNODE_SEEK_MODE
{
    SEEK_SET,
    SEEK_CUR,
    SEEK_END,
} VNODE_SEEK_MODE;

typedef enum VNODE_MODE
{
    VNODE_MODE_READ,
    VNODE_MODE_WRITE,
    VNODE_MODE_APPEND,
    VNODE_MODE_INVALID,
} VNODE_MODE;

#endif