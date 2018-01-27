#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "abstract_task.hpp"
#include "toposort.hpp"
#include "future.hpp"
#include "task.hpp"

#include <optional>
#include <vector>
#include <chrono>

class Scheduler
{

public:

    Scheduler() : isRunning{false} {}

    template<class R, class... Args>
    std::size_t makeTask()
    {
        _tasks.emplace_back
        (
            std::make_unique<Task<R, Args...>>()
        );

        return _taskGraph.makeNode(_tasks.back().get());
    }

    template<class R, class... Args>
    Task<R, Args...>& getTask(std::size_t id)
    {
        return reinterpret_cast<Task<R, Args...>&>(*(_taskGraph.getValue(id)));
    }

    void attach(const std::size_t lhs,
                const std::size_t rhs);

    void detach(const std::size_t lhs,
                const std::size_t rhs);

    void run();

    template <class R, class Rep, class Period>
    R attach(const std::size_t id, 
             std::future<R>&& future,
             const std::chrono::duration<Rep, Period>& pollingDelay)
    {
        FutureIdPair* const backPtr = 
            registerFuture<Future<R, Rep, Period>>(id,
                                                   std::move(future),
                                                   pollingDelay); 

        const R& returnValue =  reinterpret_cast<Future<R, Rep, Period>*>(backPtr->future.get())->future.get();

        cleanFutureIdPair(backPtr);

        return returnValue;
    }

    template <class R, class Rep1, class Period1, 
                       class Rep2, class Period2>
    std::optional<R> attach(const std::size_t id, 
                            std::future<R>&& future,
                            const std::chrono::duration<Rep1, Period1>& pollingDelay,
                            const std::chrono::duration<Rep2, Period2>& timeoutDuration)
    {
        FutureIdPair* const backPtr = 
            registerFuture<ScopedFuture<R, Rep1, Period1>>(id, 
                                                           std::move(future), 
                                                           pollingDelay, 
                                                           timeoutDuration);

        auto&& aFuture = reinterpret_cast<ScopedFuture<R, Rep1, Period1>*>(backPtr->future.get())->future;

        using namespace std::chrono_literals;

        const auto& returnValue = (aFuture.wait_for(0s) == std::future_status::ready) ? std::optional<R>{aFuture.get()}
                                                                                      : std::optional<R>{};

        cleanFutureIdPair(backPtr);

        return returnValue;
    }

private:

    bool checkFutures();
    
    void cleanFutureIdPair(FutureIdPair* const backPtr);    
    
    template <class F, class... Args>
    FutureIdPair* registerFuture(const std::size_t id, Args&&... args)
    {
        _futureIdPairs.emplace_back(
            std::make_unique<FutureIdPair>
            (
                std::make_unique<F>(std::move(args)...),
                id
            )
        );

        FutureIdPair* backPtr = _futureIdPairs.back().get();

        attach(id, id); // create a loop waiting for the future

        return backPtr;
    }

    std::vector<std::unique_ptr<AbstractTask>> _tasks;
    std::vector<std::unique_ptr<FutureIdPair>> _futureIdPairs;

    Toposort<AbstractTask*> _taskGraph;

public:

    bool isRunning;
};


#endif //__SCHEDULER_H__