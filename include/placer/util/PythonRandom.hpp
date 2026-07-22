#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace placer
{
class PythonRandom
{
public:
    explicit PythonRandom(std::uint64_t seed);
    [[nodiscard]] std::uint32_t getRandBits32();
    [[nodiscard]] std::uint64_t getRandBits(int bits);
    [[nodiscard]] std::size_t randBelow(std::size_t n);

    template <typename T>
    void shuffle(std::vector<T>& values)
    {
        for (std::size_t i = values.size(); i > 1; --i) {
            const std::size_t j = randBelow(i);
            std::swap(values[i - 1], values[j]);
        }
    }

private:
    void initGenRand(std::uint32_t seed);
    void initByArray(const std::vector<std::uint32_t>& key);
    std::uint32_t state_[624]{};
    int index_{624};
};
}
