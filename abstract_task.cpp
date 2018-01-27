#include "abstract_task.hpp"

AbstractTask::Context AbstractTask::entryPoint(AbstractTask::Context&& context, 
                                               AbstractTask* task)
{
    task->_context = std::move(context);

    task->run();

    context = std::move(task->_context);

    return std::move(context);
}

AbstractTask::AbstractTask() :
    _context{entryPoint}
{}