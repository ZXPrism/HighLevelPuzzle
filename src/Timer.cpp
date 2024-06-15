#include "Timer.h"

Timer gTimer;

void Timer::Tick(float dt)
{
    std::vector<int> removedTaskID;
    int taskNum = _Tasks.size();
    for (int i = 0; i < taskNum; i++)
    {
        auto &task = _Tasks[i];
        task._Counter += dt;
        if (task._Counter >= task._Cycle)
        {
            task._Counter = 0;
            task._Callback();
            if (task._Type == TimerTaskType::FINITE)
            {
                --task._LoopNum;
                if (task._LoopNum == 0)
                {
                    task._EndCallback();
                    removedTaskID.push_back(i);
                }
            }
        }
    }

    for (auto id : removedTaskID)
    {
        RemoveTask(id);
    }
}

int Timer::DispatchFiniteTask(float cycle, const std::function<void()> &callback, const std::function<void()> &endCallback, int loopNum)
{
    auto &task = _Tasks.emplace_back();
    task._Cycle = cycle;
    task._Callback = callback;
    task._EndCallback = endCallback;
    task._Type = TimerTaskType::FINITE;
    task._LoopNum = loopNum;
    return _Tasks.size() - 1;
}

int Timer::DispatchLoopTask(float cycle, const std::function<void()> &callback)
{
    auto &task = _Tasks.emplace_back();
    task._Cycle = cycle;
    task._Callback = callback;
    task._Type = TimerTaskType::LOOP;
    return _Tasks.size() - 1;
}

void Timer::RemoveTask(int taskID)
{
    std::swap(_Tasks[taskID], _Tasks.back());
    _Tasks.pop_back();
}
