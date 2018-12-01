// Last Update:2018-12-01 11:02:03
/**
 * @file kdtree.cpp
 * @brief
 * @author tczengming@163.com www.benewtech.cn
 * @version 0.1.00
 * @date 2018-11-28
 */

#include <assert.h>
#include <cstring>
#include <math.h>

#include <iostream>
#include <algorithm>
#include <stack>

#include "kdtree.h"

struct CompareIndex
{
public:
    CompareIndex() : row(0), col(0) {}
    int row;
    int col;
};

static CompareIndex s_compareIndex;

bool CmpData(const struct Kdata&a, const struct Kdata&b)
{
    int pos = s_compareIndex.row * s_compareIndex.col + s_compareIndex.col;
    return a.ptr[pos] < b.ptr[pos];
}

static void AddCompareIndex(int maxRows, int maxCols)
{
    ++s_compareIndex.col;
    if (s_compareIndex.col >= maxCols)
    {
        s_compareIndex.col = 0;
        ++s_compareIndex.row;

        if (s_compareIndex.row >= maxRows)
            s_compareIndex.row = 0;
    }
}

Kdata::Kdata()
    :cols(0),
    rows(0),
    ptr(NULL)
{
}

Kdata::Kdata(float *buf, int row, int col)
{
    ptr = new float[row * col];
    rows = row;
    cols = col;

    memcpy(ptr, buf, sizeof(buf[0]) * (row*col));
}

Kdata::Kdata(const Kdata &r)
{
    if (this == &r)
        return;

    cols = r.cols;
    rows = r.rows;

    ptr = new float[cols * rows];

    for ( int i = 0; i < cols*rows; ++i )
    {
        ptr[i] = r.ptr[i];
    }
}

Kdata & Kdata::operator= (const Kdata &r)
{
    if (this == &r)
        return *this;

    cols = r.cols;
    rows = r.rows;

    if (ptr)
    {
        delete [] ptr;
        ptr = NULL;
    }

    ptr = new float[cols * rows];

    for ( int i = 0; i < cols*rows; ++i )
    {
        ptr[i] = r.ptr[i];
    }

    return *this;
}

Kdata::~Kdata()
{
    delete [] ptr;
    ptr = NULL;
}

KdNode::KdNode()
    :dirIndex(0),
    trainIdx(0),
    left(NULL),
    right(NULL)
{
}

KdNode::~KdNode()
{
    if (left != NULL)
        delete left;
    if (right != NULL)
        delete right;
}

void KdTree::PrintData(const Kdata *data)
{
    printf("[ ");
    for ( int r = 0; r < data->rows; ++r )
    {
        for ( int c = 0; c < data->cols; ++c )
        {
            printf(" %f,", data->ptr[r*c + c]);
        }
    }
    printf(" ] \n");
}

KdNode *KdTree::Build(std::vector<Kdata> &datas, int start, int size)
{
    if (datas.size() <= 0)
        return NULL;
    KdNode *tmpNode = new KdNode();

    if (start == (int)(datas.size() - 1) || size <= 1)
    {
        //tmpNode->data = datas[start];
        std::swap(tmpNode->data, datas[start]);
        tmpNode->dirIndex = s_compareIndex.row * s_compareIndex.col;
        tmpNode->trainIdx = start;
        return tmpNode;
    }
    else if (start == 0 && size == datas.size())
    {
        assert(s_compareIndex.row == s_compareIndex.col && s_compareIndex.row == 0);
    }

    tmpNode->dirIndex = s_compareIndex.row * s_compareIndex.col + s_compareIndex.col;
    std::sort(datas.begin() + start, datas.begin() + start + size, CmpData);

    AddCompareIndex(datas[0].rows, datas[0].cols);

    // Determine a point to divide
    int median = start + (size / 2);
    //tmpNode->data = datas[median];
    std::swap(tmpNode->data, datas[median]);
    tmpNode->trainIdx = median;

    // Create left node
    tmpNode->left = Build(datas, start, median - start);

    // Create right node
    if (size + start - median - 1 > 0)
        tmpNode->right = Build(datas, median + 1, size + start - median - 1);

    return tmpNode;
}

void KdTree::Add(const Kdata &data)
{
    rawData.push_back(data);
}

void KdTree::Train()
{
    s_compareIndex.row = s_compareIndex.col = 0;
    root = Build(rawData, 0, rawData.size());
    rawData.clear();
}

void KdTree::Print(const KdNode *node, int level)
{
    printf(" level %d [ ", level);
    for ( int r = 0; r < node->data.rows; ++r )
    {
        for ( int c = 0; c < node->data.cols; ++c )
        {
            printf(" %f,", node->data.ptr[r*c + c]);
        }
    }
    printf(" ] ");

    if (node->left == NULL && node->right == NULL)
        return;

    Print(node->left, level + 1);
    Print(node->right, level + 1);

    printf("\n");
}

void KdTree::Print()
{
    Print(root, 0);
}

static double Distance(const Kdata &a, const Kdata &b)
{
    assert(a.cols == b.cols && a.rows == b.rows);
    double ret = 0;
    for ( int r = 0; r < a.rows; ++r )
    {
        for ( int c = 0; c < a.cols; ++c )
        {
            ret += powl(b.ptr[r*c + c] - a.ptr[r*c + c], 2);
        }
    }

    ret = sqrt(ret);

    return ret;
}

bool KdTree::NearestSearch(const struct Kdata &target, struct Kdata *out, double *distance)
{
    std::stack<struct KdNode*> searchPath;
    struct KdNode* pSearch = root;
    Kdata nearest;
    double dist = 0.0;

    // push
    while (pSearch != NULL)
    {
        searchPath.push(pSearch);

        if (target.ptr[pSearch->dirIndex] <= pSearch->data.ptr[pSearch->dirIndex])
        {
            pSearch = pSearch->left;
        }
        else
        {
            pSearch = pSearch->right;
        }
    }

    // pop
    nearest = searchPath.top()->data;
    searchPath.pop();
    dist = Distance(nearest, target);

    struct KdNode* pBack;

    while (searchPath.size() != 0)
    {
        pBack = searchPath.top();
        searchPath.pop();

        if (pBack->left == NULL && pBack->right == NULL) // is leaf
        {
            if ( Distance(nearest, target) > Distance(pBack->data, target) )
            {
                nearest = pBack->data;
                dist = Distance(pBack->data, target);
            }
        }
        else
        {
            // if the circle which radius is dist include pBack
            if (fabs(pBack->data.ptr[pBack->dirIndex] - target.ptr[pBack->dirIndex]) < dist)
            {
                if ( Distance(nearest, target) > Distance(pBack->data, target) )
                {
                    nearest = pBack->data;
                    dist = Distance(pBack->data, target);
                }

                 //if target in then left of pBack search in right area otherwise search in left area
                if (target.ptr[pBack->dirIndex] <= pBack->data.ptr[pBack->dirIndex])
                    pSearch = pBack->right;
                else
                    pSearch = pBack->left;
                if (pSearch != NULL)
                    searchPath.push(pSearch);
            }
        }
    }

    *out = nearest;

    if (nearest.ptr == NULL)
    {
        return false;
    }
    else
    {
        *distance = dist;
        return true;
    }
}
