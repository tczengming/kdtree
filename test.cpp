// Last Update:2018-12-05 16:52:30
/**
 * @file test.cpp
 * @brief
 * @author tczengming@163.com www.benewtech.cn
 * @version 0.1.00
 * @date 2018-11-29
 */

#include <unistd.h>
#include "kdtree.h"

    int
main( int argc, char **argv )
{
    struct KdTree tree;
    float m[7][2] = {{6, 2}, {6, 3}, {3, 5}, {5, 0}, {1, 2}, {4, 9}, {8, 1}};

    for ( size_t i = 0; i < sizeof(m) / sizeof(m[0]); ++i )
    {
        struct Kdata data(&(m[i][0]), 1, sizeof(m[0]) / sizeof(m[0][0]));
        tree.Add(data);
    }

    tree.Train();
    tree.Print();

    float query[] = {5, 6};
    struct Kdata data(query, 1, 2);
    struct Kdata nearest;
    double dist;
    if (tree.NearestSearch(data, &nearest, &dist))
    {
        printf( "dist;%lf\n", dist );
        tree.PrintData(nearest);
    }

    std::vector<struct Match> matchs;
    tree.Knearest(data, 2, &matchs);
    for ( int i = 0; i < matchs.size(); ++i )
    {
        printf( "i %d imgidx;%d trainidx:%d distance:%f\n", i, matchs[i].imgIdx, matchs[i].trainIdx, matchs[i].distance );
    }

    return 0;
}
