#include <cstdlib>
#include <iostream>
#include <future>
#include <chrono>

#include "scheduler.hpp"

int main()
{

    Scheduler scheduler;

    auto ping = scheduler.makeTask<void>();
    auto pong = scheduler.makeTask<void>();
    
    scheduler.getTask<void>(ping).assign
    (
        [&]()
        {
            std::cout << "ping" << std::endl;
            scheduler.detach(pong, ping);
            scheduler.attach(ping, pong);
            std::cout << "ping" << std::endl;
            scheduler.detach(pong, ping);
            scheduler.attach(ping, pong);

            using namespace std::chrono_literals;

            auto&& answer = scheduler.attach(ping,
                                             std::async
                                             (
                                                 []
                                                 {
                                                     std::this_thread::sleep_for(2s);
                                                     return 42;
                                                 }
                                             ),
                                             10us, // polling delay to avoid maxing out CPU
                                             2s    // timeout
            );

            std::cout << (answer ? *answer : -1) << std::endl;
            //std::cout << answer << std::endl;

            std::cout << "ping" << std::endl;
            scheduler.detach(pong, ping);
            scheduler.attach(ping, pong);
            std::cout << "ping" << std::endl;
        }
    );

    scheduler.getTask<void>(pong).assign
    (
        [&]()
        {
            std::cout << "pong" << std::endl;
            scheduler.detach(ping, pong);
            scheduler.attach(pong, ping);
            std::cout << "pong" << std::endl;
            scheduler.detach(ping, pong);
            scheduler.attach(pong, ping);
            std::cout << "pong" << std::endl;
        }
    );

    scheduler.attach(pong, ping);

    scheduler.run();

    return EXIT_SUCCESS;
}


/*#include <unordered_set>
#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>

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


int main()
{
    Toposort<char> graph;

    auto b = graph.makeNode('b');    
    auto c = graph.makeNode('c');
    auto a = graph.makeNode('a');
    auto d = graph.makeNode('d');

    graph.attach(b, a);
    graph.attach(c, b);
    graph.attach(a, c);

    while (!graph.empty() && !graph.isCyclic())
    {
        std::cout << graph.getValue(graph.top()) << std::endl;
        graph.pop();
    }

    if (graph.isCyclic())
    {
        for (const auto cycler : graph.computeCycle())
        {
            std::cout << "cycler: " << graph.getValue(cycler) << std::endl;
        }
    }

    return EXIT_SUCCESS;
}*/