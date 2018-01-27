#ifndef __ABSTRACT_TASK_H__
#define __ABSTRACT_TASK_H__

#include <boost/context/execution_context.hpp>

class Scheduler;

class AbstractTask
{
    friend Scheduler;

public:

    AbstractTask();

    virtual void run() = 0;

    virtual ~AbstractTask() {}

private:

    using Context = boost::context::execution_context<AbstractTask*>;

    static Context entryPoint(Context&& context, AbstractTask* task);

    Context    _context;
};

#endif //__ABSTRACT_TASK_H__