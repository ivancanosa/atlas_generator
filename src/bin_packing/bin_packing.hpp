#pragma once

#include <cstdint>
#include <vector>

namespace atlas {

struct Vec2 {
    uint32_t x, y;
};

struct Rect {
    Vec2 position{0, 0};
    Vec2 size{1, 1};
};

struct Item {
    Rect rect{};
    uint32_t id{0};
};

struct Bin {
    Vec2 size{1, 1};
    std::vector<Item> items{};
};

// Performs a greedy 2D bin packing algorithm
auto bin_packing(const std::vector<Item>& items, Vec2 bin_size) -> std::vector<Bin>;

}
