#include "bin_packing.hpp"
#include <gtest/gtest.h>
#include <unordered_set>

using namespace atlas;

auto no_overlap(const Rect& r1, const Rect& r2) -> bool {
    return r1.position.x + r1.size.x <= r2.position.x || // r1 is to the left of r2
           r2.position.x + r2.size.x <= r1.position.x || // r1 is to the right of r2
           r1.position.y + r1.size.y <= r2.position.y || // r1 is below r2
           r2.position.y + r2.size.y <= r1.position.y;   // r1 is above r2
}

void validate_bin(const Bin& bin) {
    const auto& items = bin.items;

    const auto no_dupplicates = [&](){
        std::unordered_set<uint32_t> seenIds;
        for (const auto& item : items) {
            ASSERT_TRUE(seenIds.insert(item.id).second) << "Duplicate item ID: " << item.id;
        }
    };

    const auto no_overlapping = [&](){
        for (size_t i = 0; i < items.size(); ++i) {
            for (size_t j = i + 1; j < items.size(); ++j) {
                ASSERT_TRUE(no_overlap(items[i].rect, items[j].rect)) 
                    << "Items " << items[i].id << " and " << items[j].id << " overlap!";
            }
        }
    };

    const auto ensure_items_fit = [&](){
        for (const auto& item : items) {
            ASSERT_GE(item.rect.position.x, 0);
            ASSERT_GE(item.rect.position.y, 0);
            ASSERT_LE(item.rect.position.x + item.rect.size.x, bin.size.x);
            ASSERT_LE(item.rect.position.y + item.rect.size.y, bin.size.y);
        }

    };

    no_dupplicates();
    no_overlapping();
    ensure_items_fit();
}

void validate_packing(const std::vector<Item>& original_items, const std::vector<Bin> bins) {
    std::unordered_set<uint32_t> placed_ids;

    for (const auto& bin : bins) {
        for (const auto& item : bin.items) {
            placed_ids.insert(item.id);
        }

        validate_bin(bin);
    }

    ASSERT_EQ(placed_ids.size(), original_items.size()) 
        << "Not all items were placed in bins!";
    for (const auto& item : original_items) {
        ASSERT_TRUE(placed_ids.count(item.id)) 
            << "Item with ID " << item.id << " was not placed!";
    }

}

TEST(BinPackingTest, EmptyItems) {
    Vec2 bin_size{10, 10};
    std::vector<Item> items; // No items to pack

    auto bins = bin_packing(items, bin_size);

    ASSERT_TRUE(bins.empty());
}

TEST(BinPackingTest, ValidConfiguration) {
    Vec2 bin_size{5, 5};

    std::vector<Item> items = {
        {{.size = {2, 2}}, 1},
        {{.size = {3, 3}}, 2},
        {{.size = {1, 4}}, 3},
        {{.size = {2, 1}}, 4}
    };

    auto bins = bin_packing(items, bin_size);

    validate_packing(items, bins);
}

TEST(BinPackingTest, LargeNumberOfItems) {
    // Test case setup for many small items
    Vec2 bin_size{10, 10};
    std::vector<Item> items;
    for (uint32_t i = 0; i < 100; ++i) {
        items.push_back({{.size = {1, 1}}, i});
    }

    // Perform bin packing
    auto bins = bin_packing(items, bin_size);

    validate_packing(items, bins);
}

TEST(BinPackingTest, ItemsTooLarge) {
    Vec2 bin_size{5, 5};

    std::vector<Item> items = {
        {{.size = {6, 6}}, 1}, // Too large for bin
    };

    // Expect runtime error due and item bigger than a bin
    ASSERT_THROW(bin_packing(items, bin_size), std::runtime_error);
}

TEST(BinPackingTest, SingleItemFitsExactly) {
    Vec2 bin_size{5, 5};
    std::vector<Item> items = {
        {{.size = {5, 5}}, 1}, // Fits exactly into the bin
    };

    auto bins = bin_packing(items, bin_size);

    validate_packing(items, bins);
    ASSERT_EQ(bins.size(), 1); // Only one bin should be used
}

TEST(BinPackingTest, MultipleBinsRequired) {
    Vec2 bin_size{5, 5};
    std::vector<Item> items = {
        {{.size = {5, 5}}, 1},
        {{.size = {5, 5}}, 2},
        {{.size = {5, 5}}, 3}, // All require separate bins
    };

    auto bins = bin_packing(items, bin_size);

    validate_packing(items, bins);
    ASSERT_EQ(bins.size(), 3); 
}

TEST(BinPackingTest, IrregularItemSizes) {
    Vec2 bin_size{10, 10};
    std::vector<Item> items = {
        {{.size = {3, 7}}, 1},
        {{.size = {7, 3}}, 2},
        {{.size = {4, 4}}, 3},
        {{.size = {2, 5}}, 4},
    };

    auto bins = bin_packing(items, bin_size);

    validate_packing(items, bins);
}

TEST(BinPackingTest, NonRectangularArrangement) {
    Vec2 bin_size{6, 6};
    std::vector<Item> items = {
        {{.size = {3, 2}}, 1},
        {{.size = {2, 3}}, 2},
        {{.size = {3, 2}}, 3},
        {{.size = {2, 2}}, 4},
    };

    auto bins = bin_packing(items, bin_size);

    validate_packing(items, bins);
    ASSERT_EQ(bins.size(), 1); // All items should fit into a single bin
}

TEST(BinPackingTest, NonSquareBin) {
    Vec2 bin_size{10, 5};
    std::vector<Item> items = {
        {{.size = {10, 2}}, 1},
        {{.size = {10, 2}}, 2},
        {{.size = {10, 1}}, 3},
    };

    auto bins = bin_packing(items, bin_size);

    validate_packing(items, bins);
    ASSERT_EQ(bins.size(), 1);
}

TEST(BinPackingTest, LargeSparseItems) {
    Vec2 bin_size{100, 100};
    std::vector<Item> items = {
        {{.size = {50, 50}}, 1},
        {{.size = {30, 30}}, 2},
        {{.size = {20, 20}}, 3},
    };

    auto bins = bin_packing(items, bin_size);

    validate_packing(items, bins);
    ASSERT_EQ(bins.size(), 1);
}
