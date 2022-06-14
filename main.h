#ifndef MAIN_H
#define MAIN_H

struct octree_leaf_type
{
    int r,g,b;
    int x;
    int y;
};

struct octree_node_type
{
    struct octree_leaf_type * leafs;
    int leafs_number;
    int r0,g0,b0;
    int size;
    struct octree_node_type * parent;
    struct octree_node_type * childs[8];
};


#endif // MAIN_H
