#pragma once

#include <algorithm>
#include <filesystem>
#include <format>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#ifdef MY_DEBUG
#define DEBUG_SCOPE(x)                                                                                                                     \
    do                                                                                                                                     \
    {                                                                                                                                      \
        x                                                                                                                                  \
    } while (0)
#else
#define DEBUG_SCOPE(x)
#endif

namespace fs = std::filesystem;

void RandPiecewiseDist(std::vector<float> &result, int count, const std::vector<float> &segments, const std::vector<int> &weight);

void TraverseFolder(const std::string &folderPath, const std::function<void(const fs::path &)> &callback);

class DSU // from OI wiki
{
public:
    void Init(int n);
    std::size_t Find(std::size_t x);
    void Unite(std::size_t x, std::size_t y);

private:
    std::vector<std::size_t> pa, size;
};
