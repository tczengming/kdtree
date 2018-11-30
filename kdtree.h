// Last Update:2018-11-30 17:45:16
/**
 * @file kdtree.h
 * @brief
 * @author tczengming@163.com www.benewtech.cn
 * @version 0.1.00
 * @date 2018-11-28
 */

#ifndef __KDTREE_H__
#define __KDTREE_H__

#include <stdio.h>
#include <vector>

struct Kdata
{
    Kdata();
    Kdata(float *buf, int row, int col);
    Kdata(const Kdata &r);
    Kdata & operator= (const Kdata &r);
    ~Kdata();

    int cols;
    int rows;
    float *ptr;
};

struct KdNode
{
    KdNode();
    ~KdNode();

    int dirIndex;
    int trainIdx;
    Kdata data;

    struct KdNode *left;
    struct KdNode *right;
};

struct KdTree
{
public:
    void Add(const Kdata &data);
    void Train();
    KdNode *Build(std::vector<struct Kdata> &datas, int start, int size);
    void Print(const KdNode *node, int level);
    void Print();
    bool NearestSearch(const struct Kdata &target, struct Kdata *out, double *distance);
    void PrintData(const Kdata *data);

private:
    struct KdNode *root;
    std::vector<struct Kdata> rawData;
};

#endif  /*__KDTREE_H__*/
