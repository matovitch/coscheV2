#include "scheduler.hpp"

#include "abstract_task.hpp"

void Scheduler::attach(const std::size_t lhs,
                       const std::size_t rhs)
{
    _taskGraph.attach(lhs,
                      rhs);

    if (isRunning)
    {
        // Remark: we could do something better here avoiding
        // unneeded context switch if lhs can still run.
        AbstractTask* lhsTask = _taskGraph.getValue(lhs);

        lhsTask->_context = std::move(std::get<0>(lhsTask->_context(lhsTask)));
    }
}

void Scheduler::detach(const std::size_t lhs,
                       const std::size_t rhs)
{
    _taskGraph.detach(lhs,
                      rhs);
}

void Scheduler::run()
{
    isRunning = true;

    while (!_taskGraph.empty() || checkFutures())
    {
        if (_taskGraph.empty())
        {
            continue;
        }

        const std::size_t top =  _taskGraph.top();

        AbstractTask* abstractTaskPtr = _taskGraph.getValue(top);

        abstractTaskPtr->_context = std::move(std::get<0>(abstractTaskPtr->_context(abstractTaskPtr)));

        if (!_taskGraph.empty() && top == _taskGraph.top())
        {
            _taskGraph.pop();
        }
    }

    isRunning = false;
}


bool Scheduler::checkFutures()
{
    bool returnValue = !_futureIdPairs.empty();

    for (const auto& futureIdPair : _futureIdPairs)
    {
        if (futureIdPair->future->ready())
        {
            const std::size_t id = futureIdPair->id;

            detach(id, id);
        }
    }

    return returnValue;
}

void Scheduler::cleanFutureIdPair(FutureIdPair* const backPtr)
{
    const auto& back = _futureIdPairs.back();
    
    backPtr->future = std::move(back->future);
    backPtr->id     =           back->id;

    _futureIdPairs.pop_back();
}