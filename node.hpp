#ifndef __NODE_H__
#define __NODE_H__

#include <unordered_set>
#include <cstddef>

struct Node
{
    std::unordered_set<std::size_t> dependers;
    std::unordered_set<std::size_t> dependees;
};

template <class T>
struct TNode
{
    template <class... Args>
    TNode(Args&&... args) : t{args...} {}

    Node node;
    T    t;
};

#endif //__NODE_H__