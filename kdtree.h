// Last Update:2018-12-05 18:11:14
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
    int DataSize();
    ~Kdata();

    int cols;
    int rows;
    float *ptr;
};

struct KdNode
{
    KdNode();
    ~KdNode();

    int compareCol;
    int imgIdx;
    int trainIdx;

    struct KdNode *left;
    struct KdNode *right;
};

struct Match
{
    int imgIdx;
    int trainIdx;
    int queryIdx;
    double distance;
};

struct KdTree
{
public:
    KdTree();
    ~KdTree();

    void Add(const Kdata &data);
    void Train();
    void Print();
    void PrintData(const Kdata *data);
    bool NearestSearch(const struct Kdata &target, struct Kdata *out, double *distance);
    void Knearest(const Kdata &query, const int k, std::vector<struct Match> *matchs);

private:
    void Print(const KdNode *node, int level);
    bool Cmp(int a, int b);
    KdNode *Build(std::vector<int> &inds, int start, int size, int compareCol);
    bool RowInMergeData2TrainIdxImgIdx(int rowInMergeData, int *trainIdx, int *imgIdx);
    int RowIndex(int trainIdx, int imgIdx);
    Kdata GetData(int trainIdx, int imgIdx);
    Kdata GetData(const struct KdNode &node);
    void Knearest(const KdNode *tree, const Kdata &query, const int k, const int compareCol, std::vector<struct Match> *matchs);

    struct KdNode *root;
    std::vector<struct Kdata> tmpTrainData;
    struct Kdata mergeData;
    std::vector<int> imgStart;
    int compareCol;
};

#endif  /*__KDTREE_H__*/
