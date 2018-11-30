// Last Update:2018-11-28 21:20:02
/**
 * @file kdnode.h
 * @brief
 * @author tczengming@163.com www.benewtech.cn
 * @version 0.1.00
 * @date 2018-11-28
 */

#ifndef __KDNODE_H__
#define __KDNODE_H__

struct Kdata
{
    int cols;
    int rows;
    void *ptr;
};

struct KdNode
{
    double *pos;
    int dir;
    Kdata data;

    struct KdNode *left;
    struct KdNode *right;
};

#endif  /*__KDNODE_H__*/
