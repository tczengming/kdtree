// Last Update:2018-12-05 18:13:25
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
#include <stack>
#include <algorithm>
#include <functional>

#include "kdtree.h"

using std::placeholders::_1;
using std::placeholders::_2;

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

    memcpy(ptr, buf, DataSize());
}

Kdata::Kdata(const Kdata &r)
{
    if (this == &r)
        return;

    cols = r.cols;
    rows = r.rows;

    ptr = (float*)malloc(DataSize());
    if (ptr == NULL)
    {
        printf( "%s %d malloc failed!\n", __func__, __LINE__ );
        return;
    }

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

    ptr = (float*)malloc(DataSize());
    if (ptr == NULL)
    {
        printf( "%s %d malloc failed!\n", __func__, __LINE__ );
        return *this;
    }

    for ( int i = 0; i < cols*rows; ++i )
    {
        ptr[i] = r.ptr[i];
    }

    return *this;
}

Kdata::~Kdata()
{
    free(ptr);
    ptr = NULL;
}

int Kdata::DataSize()
{
    return rows * cols * sizeof(float);
}

KdNode::KdNode()
    :compareCol(0),
    imgIdx(0),
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

KdTree::KdTree()
    :root(NULL)
{
}

KdTree::~KdTree()
{
    if (root != NULL)
        delete root;

    tmpTrainData.clear();
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

bool KdTree::Cmp(int a, int b)
{
    return mergeData.ptr[a * mergeData.cols + compareCol] < mergeData.ptr[b * mergeData.cols + compareCol];
}

KdNode *KdTree::Build(std::vector<int> &inds, int start, int size, int compareCol)
{
    if (inds.size() <= 0)
        return NULL;

    KdNode *tmpNode = new KdNode();
    tmpNode->compareCol = compareCol;

    if (start == (int)(inds.size() - 1) || size <= 1)
    {
        RowInMergeData2TrainIdxImgIdx(inds[start], &(tmpNode->trainIdx), &(tmpNode->imgIdx));
        return tmpNode;
    }
    else if (start == 0 && size == inds.size())
    {
        assert(compareCol == 0);
    }

    this->compareCol = compareCol;
    auto boundCmp = std::bind(&KdTree::Cmp, this, _1, _2);
    std::sort(inds.begin() + start, inds.begin() + start + size, boundCmp);

    // Determine a point to divide
    int median = start + (size / 2);
    RowInMergeData2TrainIdxImgIdx(inds[median], &(tmpNode->trainIdx), &(tmpNode->imgIdx));

    // Create left node
    tmpNode->left = Build(inds, start, median - start, compareCol + 1);

    // Create right node
    if (size + start - median - 1 > 0)
        tmpNode->right = Build(inds, median + 1, size + start - median - 1, compareCol + 1);

    return tmpNode;
}

bool KdTree::RowInMergeData2TrainIdxImgIdx(int rowInMergeData, int *trainIdx, int *imgIdx)
{
    for ( int i = 0; i < imgStart.size() - 1; ++i )
    {
        if (rowInMergeData >= imgStart[i] && rowInMergeData < imgStart[i+1])
        {
            *imgIdx = i;
            *trainIdx = rowInMergeData - imgStart[i];
            return true;
        }
    }

    if (imgStart.size() > 0)
    {
        int pos = imgStart.size()-1;
        if (rowInMergeData >= imgStart[pos] && rowInMergeData < mergeData.rows)
        {
            *imgIdx = pos;
            *trainIdx = rowInMergeData - imgStart[pos];
            return true;
        }
        else
        {
            printf( "%s %d find row failed\n", __func__, __LINE__ );
        }
    }

    printf( "%d find failed\n", rowInMergeData );
    return false;
}

int KdTree::RowIndex(int trainIdx, int imgIdx)
{
    return imgStart[imgIdx] + trainIdx;
}

Kdata KdTree::GetData(int imgIdx,int trainIdx)
{
    int row = RowIndex(trainIdx, imgIdx);
    Kdata ret(mergeData.ptr + row*mergeData.cols, 1, mergeData.cols);
    return ret;
}

Kdata KdTree::GetData(const struct KdNode &node)
{
    return GetData(node.imgIdx, node.trainIdx);
}

void KdTree::Add(const Kdata &data)
{
    tmpTrainData.push_back(data);
}

void KdTree::Train()
{
    mergeData.rows = 0;
    free(mergeData.ptr);
    mergeData.ptr = NULL;

    if (tmpTrainData.empty())
        return;

    mergeData.cols = tmpTrainData[0].cols;

    for ( int i = 0; i < tmpTrainData.size(); ++i )
    {
        mergeData.rows += tmpTrainData[i].rows;
        assert(mergeData.cols == tmpTrainData[i].cols);
    }

    mergeData.ptr = (float*)malloc(mergeData.DataSize());
    if (mergeData.ptr == NULL)
    {
        mergeData.rows = 0;
        printf( "%s %d malloc failed\n", __func__, __LINE__ );
        return;
    }

    int pos = 0;
    int start = 0;

    for ( int i = 0; i < tmpTrainData.size(); ++i )
    {
        memcpy(((char*)mergeData.ptr) + pos, tmpTrainData[i].ptr, tmpTrainData[i].DataSize());
        pos += tmpTrainData[i].DataSize();

        imgStart.push_back(start);
        start += tmpTrainData[i].rows;
    }

    tmpTrainData.clear();

    std::vector<int> inds;
    for ( int i = 0; i < mergeData.rows; ++i )
    {
        inds.push_back(i);
    }

    root = Build(inds, 0, inds.size(), 0);
}

void KdTree::Print(const KdNode *node, int level)
{
    int rowStart = imgStart[node->imgIdx];
    int rowCount;
    if (imgStart.size() == 1 || node->imgIdx == imgStart.size()-1)
        rowCount = mergeData.rows - rowStart;
    else if (imgStart.size() > 1)
        rowCount = imgStart[node->imgIdx + 1] - rowStart;
    else
        return;

    printf("level %d [ ", level);
    for ( int r = rowStart; r < rowStart+rowCount; ++r )
    {
        for ( int c = 0; c < mergeData.cols; ++c )
            printf(" %f,", mergeData.ptr[r*mergeData.cols + c]);
    }
    printf(" ]\n");

    if (node->left == NULL && node->right == NULL)
        return;

    Print(node->left, level + 1);
    Print(node->right, level + 1);
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
            ret += powl(b.ptr[r*a.cols + c] - a.ptr[r*a.cols + c], 2);
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

        Kdata data = GetData(*pSearch);
        if (target.ptr[pSearch->compareCol] <= data.ptr[pSearch->compareCol])
        {
            pSearch = pSearch->left;
        }
        else
        {
            pSearch = pSearch->right;
        }
    }

    // pop
    nearest = GetData(*(searchPath.top()));
    searchPath.pop();
    dist = Distance(nearest, target);

    struct KdNode* pBack;

    while (searchPath.size() != 0)
    {
        pBack = searchPath.top();
        searchPath.pop();

        Kdata data = GetData(*pBack);
        if (pBack->left == NULL && pBack->right == NULL) // is leaf
        {
            if ( Distance(nearest, target) > Distance(data, target) )
            {
                nearest = data;
                dist = Distance(data, target);
            }
        }
        else
        {
            // if the circle which radius is dist include pBack
            if (fabs(data.ptr[pBack->compareCol] - target.ptr[pBack->compareCol]) < dist)
            {
                if ( Distance(nearest, target) > Distance(data, target) )
                {
                    nearest = data;
                    dist = Distance(data, target);
                }

                 //if target in then left of pBack search in right area otherwise search in left area
                if (target.ptr[pBack->compareCol] <= data.ptr[pBack->compareCol])
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

void KdTree::Knearest(const KdNode *tree, const Kdata &query, const int k, const int compareCol, std::vector<struct Match> *matchs)
{
    if (tree == NULL)
        return;

    double maxDistance;
    if (matchs->size() > 0)
    {
        struct Match &topMatch = matchs->at(matchs->size() - 1);
        maxDistance = Distance(GetData(topMatch.imgIdx, topMatch.trainIdx), query);
    }
    else
    {
        maxDistance = 0;
    }

    // Calculate distance between the query and self along one direction (if depth is even, the direction is horizontal)
    Kdata curData = GetData(*tree);
    double sub = query.ptr[compareCol] - curData.ptr[compareCol];
    KdNode *first;
    KdNode *second;

    // Find the nearest node
    if (sub < 0)
    {
        first = tree->left;
        second = tree->right;
    }
    else
    {
        second = tree->left;
        first = tree->right;
    }

    Knearest(first, query, k, compareCol + 1, matchs);

    // Update max distance
    if (matchs->size() > 0)
    {
        struct Match &topMatch = matchs->at(matchs->size() - 1);
        maxDistance = Distance(GetData(topMatch.imgIdx, topMatch.trainIdx), query);
    }
    else
    {
        maxDistance = 0;
    }

    double distance = fabs(sub);
    if (second && (matchs->size() < k || distance < maxDistance))
    {
        Knearest(second, query, k, compareCol + 1, matchs);
    }

    struct Match match;
    match.distance = Distance(GetData(*tree), query);
    match.imgIdx = tree->imgIdx;
    match.trainIdx = tree->trainIdx;

    if (matchs->size() == 0)
    {
        // Add self
        matchs->push_back(match);
    }
    else
    {
        // Add self with insertion sort
        for (std::vector<struct Match>::iterator it = matchs->begin(); it <= matchs->end(); ++it)
        {
            if (it == matchs->end())
            {
                matchs->push_back(match);
                break;
            }

            if (match.distance < Distance(GetData(it->imgIdx, it->trainIdx), query))
            {
                matchs->insert(it, match);
                break;
            }
        }

        if (matchs->size() > k)
        {
            matchs->pop_back();
        }
    }
}

void KdTree::Knearest(const Kdata &query, const int k, std::vector<struct Match> *matchs)
{
    Knearest(root, query, k, 0, matchs);
}

