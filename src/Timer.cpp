#include "Timer.h"

Timer gTimer;

void Timer::Tick(float dt)
{
    for (auto &task : _Tasks)
    {
        task._Counter += dt;
        if (task._Counter >= task._Cycle)
        {
            task._Counter = 0;
            task._Callback();
        }
    }
}

int Timer::DispatchTask(float cycle, const std::function<void()> &callback)
{
    auto &task = _Tasks.emplace_back();
    task._Cycle = cycle;
    task._Callback = callback;
    return _Tasks.size() - 1;
}

void Timer::RemoveTask(int taskNo)
{
    std::swap(_Tasks[taskNo], _Tasks.back());
    _Tasks.pop_back();
}
