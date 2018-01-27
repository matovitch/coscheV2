#ifndef __FUTURE_H__
#define __FUTURE_H__

#include <future>
#include <chrono>

struct AbstractFuture
{
    virtual bool ready() const = 0;
};

template <class T, class Rep, class Period>
struct Future : AbstractFuture
{
    Future(std::future<T>&& aFuture,
           const std::chrono::duration<Rep, Period>& aPollingDelay) : 
        future{std::move(aFuture)},
        pollingDelay{aPollingDelay} 
    {}

    bool ready() const
    {
        using namespace std::chrono_literals;
        
        return future.wait_for(pollingDelay) == std::future_status::ready;
    }

    std::future<T>                           future;
    const std::chrono::duration<Rep, Period> pollingDelay;
};

template <class T, class Rep1, class Period1>
struct ScopedFuture : AbstractFuture
{
    template <class Rep2, class Period2>
    ScopedFuture(std::future<T>&& future,
                 const std::chrono::duration<Rep2, Period2>& timeoutDuration,
                 const std::chrono::duration<Rep1, Period1>& aPollingDelay) :
        future{std::move(future)},
        timeout{std::chrono::steady_clock::now() + timeoutDuration},
        pollingDelay{aPollingDelay}
    {}

    bool ready() const
    {
        return future.wait_for(pollingDelay) == std::future_status::ready ||
               std::chrono::steady_clock::now() > timeout;
    }

    std::future<T>                              future;
    const std::chrono::steady_clock::time_point timeout;
    const std::chrono::duration<Rep1, Period1>  pollingDelay;
};

struct FutureIdPair
{
    FutureIdPair(std::unique_ptr<AbstractFuture>&& aFuture,
                 const std::size_t anId) :
        future{std::move(aFuture)},
        id{anId}
    {}

    std::unique_ptr<AbstractFuture> future;
    std::size_t                     id;
};

#endif // __FUTURE_H__