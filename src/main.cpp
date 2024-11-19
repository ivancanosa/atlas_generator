#include <argparse/argparse.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_set>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include "bin_packing.hpp"

using namespace atlas;

struct Options {
    Vec2 bin_size{1920, 1080};
    std::string atlas_images_path{"atlas_images"};
    std::string atlas_description_path{"atlas.json"};
    std::vector<std::string> image_paths; // List of image paths
};

auto parse_options(int argc, char* argv[]) -> Options {
    argparse::ArgumentParser program("atlas");
    Options options{};

    program.add_argument("--height")
        .scan<'i', int>()
        .default_value(1920)
        .help("height of each atlas bin");

    program.add_argument("--width").scan<'i', int>().default_value(1080).help(
        "width of each atlas bin");

    program.add_argument("-d")
        .default_value("atlas_images")
        .help("output dir of atlas images");

    program.add_argument("-j")
        .default_value("atlas.json")
        .help("output dir of atlas json description");

    program.add_argument("images").remaining().help(
        "list of image paths or directories to include in the atlas");

    try {
        program.parse_args(argc, argv);

        // Populate options with parsed arguments
        options.bin_size.x             = program.get<int>("--width");
        options.bin_size.y             = program.get<int>("--height");
        options.atlas_images_path      = program.get<std::string>("-d");
        options.atlas_description_path = program.get<std::string>("-j");
        options.image_paths = program.get<std::vector<std::string>>("images");
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    return options;
}

void expand_image_paths(const std::vector<std::string>& paths,
                        std::vector<std::string>& expanded_paths) {
    namespace fs = std::filesystem;

    const std::unordered_set<std::string> valid_extensions = {
        ".png", ".jpg", ".jpeg", ".bmp", ".tga"};

    for (const auto& path_str : paths) {
        fs::path path(path_str);
        if (fs::is_regular_file(path)) {
            if (valid_extensions.count(path.extension().string())) {
                expanded_paths.push_back(path.string());
            } else {
                std::cerr << "Skipping non-image file: " << path << std::endl;
            }
        } else if (fs::is_directory(path)) {
            for (const auto& entry : fs::recursive_directory_iterator(path)) {
                if (fs::is_regular_file(entry.path()) &&
                    valid_extensions.count(entry.path().extension().string())) {
                    expanded_paths.push_back(entry.path().string());
                }
            }
        } else {
            std::cerr << "Invalid path: " << path << std::endl;
        }
    }
}

void atlas_package(const Options& options) {
    namespace fs = std::filesystem;

    // Ensure output directories exist
    fs::create_directories(options.atlas_images_path);

    std::vector<atlas::Item> items;
    std::unordered_map<uint32_t, std::string> id_to_filename;

    std::vector<std::string> expanded_image_paths;
    expand_image_paths(options.image_paths, expanded_image_paths);

    // Process provided image paths
    for (const auto& path_str : expanded_image_paths) {
        fs::path path(path_str);

        int width, height, channels;
        auto image_data =
            stbi_load(path.string().c_str(), &width, &height, &channels, 4);
        if (!image_data) {
            std::cerr << "Failed to load image: " << path.string() << std::endl;
            continue;
        }

        atlas::Item item;
        item.rect.size = {static_cast<uint32_t>(width),
                          static_cast<uint32_t>(height)};
        item.id = static_cast<uint32_t>(items.size()); // Unique ID for the item
        items.push_back(item);

        id_to_filename[item.id] = path.string();
        stbi_image_free(image_data);
    }

    // Perform bin packing
    auto bins = atlas::bin_packing(items, options.bin_size);

    // Prepare JSON output
    nlohmann::json atlas_description;

    for (size_t bin_idx = 0; bin_idx < bins.size(); ++bin_idx) {
        const auto& bin = bins[bin_idx];
        auto bin_image  = std::vector<uint8_t>(
            options.bin_size.x * options.bin_size.y * 4, 0);

        for (const auto& item : bin.items) {
            const auto& filename = id_to_filename[item.id];
            int width, height, channels;
            auto image_data = stbi_load(filename.c_str(), &width, &height, &channels, 4);
            if (!image_data) {
                std::cerr << "Failed to load image: " << filename << std::endl;
                continue;
            }

            // Copy image data into the bin image buffer
            for (uint32_t y = 0; y < item.rect.size.y; ++y) {
                for (uint32_t x = 0; x < item.rect.size.x; ++x) {
                    auto src_idx = (y * item.rect.size.x + x) * 4;
                    auto dst_idx =
                        ((item.rect.position.y + y) * options.bin_size.x +
                         (item.rect.position.x + x)) *
                        4;

                    std::copy(image_data + src_idx, image_data + src_idx + 4,
                              bin_image.begin() + dst_idx);
                }
            }
            stbi_image_free(image_data);

            // Update JSON description
            atlas_description["bins"][bin_idx]["items"].push_back({
                {"id", filename},
                {"position", {item.rect.position.x, item.rect.position.y}},
                {"size", {item.rect.size.x, item.rect.size.y}},
            });
        }

        // Write the bin image to file
        std::string output_image_path = options.atlas_images_path + "/bin_" +
                                        std::to_string(bin_idx) + ".png";
        if (!stbi_write_png(output_image_path.c_str(), options.bin_size.x,
                            options.bin_size.y, 4, bin_image.data(),
                            options.bin_size.x * 4)) {
            std::cerr << "Failed to write bin image: " << output_image_path
                      << std::endl;
        }
    }

    // Write the JSON description
    std::ofstream json_file(options.atlas_description_path);
    if (json_file) {
        json_file << atlas_description.dump(4);
    } else {
        std::cerr << "Failed to write atlas description: "
                  << options.atlas_description_path << std::endl;
    }
}

auto main(int argc, char* argv[]) -> int {
    auto options = parse_options(argc, argv);
    atlas_package(options);
    return 0;
}
