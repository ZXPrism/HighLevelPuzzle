#pragma once

#include <functional>
#include <vector>

struct TimerTask
{
    float _Cycle = 0;
    float _Counter = 0;
    std::function<void()> _Callback;
};

class Timer
{
public:
    void Tick(float dt);
    int DispatchTask(float cycle, const std::function<void()> &callback);
    void RemoveTask(int taskNo);

private:
    std::vector<TimerTask> _Tasks;
};

extern Timer gTimer;
