#ifndef __TASK_H__
#define __TASK_H__

#include "abstract_task.hpp"

#include <type_traits>
#include <functional>
#include <future>

template<class R, class... Args>
class Task : public AbstractTask
{

public:

    Task() : AbstractTask() {}

    void assign(std::function<R(Args...)>&& function)
    {
        _functionPtr = std::make_unique<std::function<R(Args...)>>(function);
    }

    void run()
    {
        // Check if the task was assigned
        if (!_functionPtr)
        {
            return;
        }

        _promise.set_value((*_functionPtr)());
    }

    std::future<R> getFuture()
    {
        return _promise.getFuture();
    }

    ~Task() {}

private:

    std::unique_ptr<std::function<R(Args...)>> _functionPtr;
    std::promise<R>                            _promise;
};

template<class... Args>
class Task<void, Args...> : public AbstractTask
{

public:

    Task() : AbstractTask() {}

    void assign(std::function<void(Args...)>&& function)
    {
        _functionPtr = std::make_unique<std::function<void(Args...)>>(function);
    }

    void run()
    {
        // Check if the task was assigned
        if (!_functionPtr)
        {
            return;
        }

        (*_functionPtr)();
    }
    
    ~Task() {}

private:

    std::unique_ptr<std::function<void(Args...)>> _functionPtr;
};

#endif //__TASK_H__