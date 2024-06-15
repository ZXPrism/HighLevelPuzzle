#pragma once

#include <functional>
#include <vector>

enum class TimerTaskType
{
    FINITE,
    LOOP
};

struct TimerTask
{
    float _Cycle = 0;
    float _Counter = 0;
    TimerTaskType _Type;
    int _LoopNum = 0;
    std::function<void()> _Callback;
    std::function<void()> _EndCallback;
};

class Timer
{
public:
    void Tick(float dt);
    int DispatchFiniteTask(float cycle, const std::function<void()> &callback, const std::function<void()> &endCallback, int loopNum);
    int DispatchLoopTask(float cycle, const std::function<void()> &callback);
    void RemoveTask(int taskID);

private:
    std::vector<TimerTask> _Tasks;
};

extern Timer gTimer;
