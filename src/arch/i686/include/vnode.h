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
    V_READ,
    V_WRITE,
    V_APPEND,
    V_INVALID,
} VNODE_MODE;

#endif