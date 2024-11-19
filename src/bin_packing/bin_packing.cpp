#include "bin_packing.hpp"
#include <algorithm>
#include <optional>
#include <stdexcept>

namespace atlas {

auto fits_within_position(const Vec2& position, const Vec2& itemSize,
                          const Vec2& binSize) -> bool {
    return position.x + itemSize.x <= binSize.x &&
           position.y + itemSize.y <= binSize.y;
}

// Helper function to find the bottom-left position for placement
auto find_placement(const Bin& bin,
                    const Vec2& itemSize) -> std::optional<Vec2> {
    std::vector<Rect> occupied;
    for (const auto& item : bin.items) {
        occupied.push_back(item.rect);
    }

    // Check for all possible positions
    for (uint32_t y = 0; y <= bin.size.y - itemSize.y; ++y) {
        for (uint32_t x = 0; x <= bin.size.x - itemSize.x; ++x) {
            Vec2 candidate{x, y};

            // Ensure the candidate position does not overlap any existing items
            bool fits = std::all_of(
                occupied.begin(), occupied.end(), [&](const Rect& rect) {
                    bool noOverlap =
                        candidate.x + itemSize.x <= rect.position.x ||
                        candidate.x >= rect.position.x + rect.size.x ||
                        candidate.y + itemSize.y <= rect.position.y ||
                        candidate.y >= rect.position.y + rect.size.y;
                    return noOverlap;
                });

            if (fits) {
                return candidate;
            }
        }
    }

    // No valid position found
    return std::nullopt;
}

auto bin_packing(const std::vector<Item>& items,
                 Vec2 binSize) -> std::vector<Bin> {
    std::vector<Bin> bins;

    const auto sort_decreasing_height = [&]() {
        auto sorted_items = items;
        std::sort(sorted_items.begin(), sorted_items.end(),
                  [](const Item& a, const Item& b) {
                      return a.rect.size.y > b.rect.size.y ||
                             (a.rect.size.y == b.rect.size.y &&
                              a.rect.size.x > b.rect.size.x);
                  });
        return sorted_items;
    };

    auto sorted_items = sort_decreasing_height();

    for (const auto& item : sorted_items) {
        bool placed = false;

        // Try to place the item in an existing bin
        for (auto& bin : bins) {
            if (auto placement = find_placement(bin, item.rect.size)) {
                // Place item in this bin
                Item placed_item          = item;
                placed_item.rect.position = *placement;
                bin.items.push_back(placed_item);
                placed = true;
                break;
            }
        }

        // If not placed, create a new bin
        if (!placed) {
            Bin new_bin{binSize, {}};
            if (auto placement = find_placement(new_bin, item.rect.size)) {
                Item placed_item          = item;
                placed_item.rect.position = *placement;
                new_bin.items.push_back(placed_item);
                bins.push_back(new_bin);
            } else {
                throw std::runtime_error("Item does not fit in a new bin!");
            }
        }
    }

    return bins;
}

} // namespace atlas
