#ifndef __TOPOSORT_H__
#define __TOPOSORT_H__

#include "node.hpp"

#include <unordered_set>
#include <cstddef>
#include <utility>
#include <memory>
#include <vector>

template <class T>
class Toposort
{

public:

    template <class... Args>
    std::size_t makeNode(Args&&... args)
    {
        if (!_recycleds.empty())
        {
            const auto toReuseId = _recycleds.back();

            using Allocator = typename std::vector<TNode<T>>::allocator_type;

            Allocator allocator = _heap.get_allocator();

            std::allocator_traits<Allocator>::destroy(allocator, 
                                                      _heap.data() + toReuseId);

            std::allocator_traits<Allocator>::construct(allocator, 
                                                        _heap.data() + toReuseId, 
                                                        args...);
            _pendings.insert(toReuseId);
            _recycleds.pop_back();

            return toReuseId;
        }

        _heap.emplace_back(args...);

        const std::size_t id = _heap.size() - 1;

        _pendings.insert(id);

        return id;
    }

    const T& getValue(const std::size_t id) const
    {
        return _heap[id].t;
    }

    T& getValue(const std::size_t id)
    {
        return _heap[id].t;
    }

    void attach(const std::size_t lhs,
                const std::size_t rhs)
    {
        Node& lhsNode = _heap[lhs].node;
        Node& rhsNode = _heap[rhs].node;

        lhsNode.dependers.insert(rhs);
        rhsNode.dependees.insert(lhs);

        const auto& fit = _pendings.find(lhs);

        if (fit != _pendings.end())
        {
            _pendings.erase(fit);
            _blockeds.insert(lhs);
        }
    }

    void detach(const std::size_t lhs,
                const std::size_t rhs)
    {
        Node& lhsNode = _heap[lhs].node;
        Node& rhsNode = _heap[rhs].node;

        lhsNode.dependers.erase(rhs);
        rhsNode.dependees.erase(lhs);

        if (lhsNode.dependers.empty())
        {
            const auto& fit = _blockeds.find(lhs);

            if (fit != _blockeds.end())
            {
                _blockeds.erase(fit);
                _pendings.insert(lhs);
            }
        }
    }

    void detachAll(const std::size_t id)
    {
        for (const auto dependee : _heap[id].node.dependees)
        {
            auto&& dependers = _heap[dependee].node.dependers;

            dependers.erase(id);

            if (dependers.empty())
            {
                _blockeds.erase(dependee);
                _pendings.insert(dependee);
            }
        }
    }


    bool isCyclic()
    {
        return _pendings.empty() && !_blockeds.empty();
    }

    std::vector<std::size_t> computeCycle()
    {
        if (!isCyclic())
        {
            return {};
        }

        std::vector<std::size_t> cycle;

        auto id = *(_blockeds.begin());

        do
        {
            cycle.push_back(id);
            id = *(_heap[id].node.dependers.begin());
        } 
        while (id != cycle.front());

        return cycle;
    }

    std::size_t top() const
    {
        return *(_pendings.begin());
    }

    bool empty() const
    {
        return _pendings.empty();
    }

    void pop()
    {
        const auto& topIt = _pendings.begin();

        const auto top = *topIt;

        detachAll(top);

        _recycleds.push_back(top);

        _pendings.erase(topIt);
    }

private:

    std::unordered_set<std::size_t> _pendings;
    std::unordered_set<std::size_t> _blockeds;

    std::vector<std::size_t> _recycleds;

    std::vector<TNode<T>> _heap;
};

#endif //__TOPOSORT_H__