#include "Utils.h"

#include <algorithm>
#include <numeric>
#include <random>

#include "Logger.h"

void RandPiecewiseDist(std::vector<float> &result, int count, const std::vector<float> &segments, const std::vector<int> &weight)
{
    static std::random_device rd;
    static std::mt19937_64 engine(rd());
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    int nWeight = weight.size();
    std::vector<int> preWeight(nWeight + 1);
    for (int i = 0; i < nWeight; i++)
    {
        preWeight[i + 1] = preWeight[i] + weight[i];
    }

    for (int i = 0; i < count; i++)
    {
        float x = preWeight[nWeight] * dist(engine);
        int segNo = std::lower_bound(preWeight.begin(), preWeight.end(), x) - preWeight.begin();
        result[i] = dist(engine) * (segments[segNo + 1] - segments[segNo]) + segments[segNo];
    }
}

void TraverseFolder(const std::string &folderPath, const std::function<void(const fs::path &)> &callback)
{
    if (!fs::exists(folderPath))
    {
        LOG_WARNING("The folder %s does not exist!", folderPath.c_str());
        return;
    }

    for (const auto &entry : fs::directory_iterator(folderPath))
    {
        if (!entry.is_directory())
        {
            callback(entry.path());
        }
    }
}

void DSU::Init(int n)
{
    size.resize(n);
    std::fill(size.begin(), size.end(), 1);
    pa.resize(n);
    std::iota(pa.begin(), pa.end(), 0);
}

std::size_t DSU::Find(std::size_t x)
{
    return pa[x] == x ? x : pa[x] = Find(pa[x]);
}

void DSU::Unite(std::size_t x, std::size_t y)
{
    x = Find(x), y = Find(y);
    if (x == y)
        return;
    if (size[x] < size[y])
        std::swap(x, y);
    pa[y] = x;
    size[x] += size[y];
}
