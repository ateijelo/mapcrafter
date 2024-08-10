/*
 * Copyright 2012-2016 Moritz Hilscher
 *
 * This file is part of Mapcrafter.
 *
 * Mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../mapcraftercore/mc/blockstate.h"
// #include "../mapcraftercore/mc/chunk.h"
#include "../mapcraftercore/mc/world.h"
#include "../mapcraftercore/util.h"
#include "../mapcraftercore/version.h"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <cstring>
#include <iostream>
#include <string>

#include <boost/optional.hpp>
#include <locale.h>

#include <optional>
#include <regex>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

// evil, I know
using namespace mapcrafter::mc;
using namespace std;

class Limits {
  public:
    optional<int> min_x = {};
    optional<int> max_x = {};
    optional<int> min_y = {};
    optional<int> max_y = {};
    optional<int> min_z = {};
    optional<int> max_z = {};
};

// void listRegions(const string& world_dir, const Limits& limits) {
// 	World world(world_dir, Dimension::OVERWORLD);
// 	//World world(world_dir, Dimension::NETHER);
// 	if (!world.load()) {
// 		cerr << "error loading world" << endl;
tuple<optional<int>, optional<int>, optional<int>> parseCoords(const string &coords) {
    const regex coords_re("(-?[0-9]+),(-?[0-9]+),(-?[0-9]+)");
    optional<int> x, y, z;
    smatch m;
    if (regex_match(coords, m, coords_re)) {
        try {
            x = stoi(m[1]);
            y = stoi(m[2]);
            z = stoi(m[3]);
        } catch (invalid_argument &e) {
        }
    }
    return make_tuple(x, y, z);
}

void dumpRegion(const RegionFile &region_file) {
    for (auto chunkPos : region_file.getContainingChunks()) {
        const std::vector<uint8_t> &data = region_file.getChunkData(chunkPos);

        nbt::NBTFile nbt;
        nbt.readNBT(reinterpret_cast<const char *>(&data[0]), data.size(), nbt::Compression::ZLIB);
        cout << "chunk " << chunkPos.x << " " << chunkPos.z << "\n";
        nbt.dump(cout);
    }
}

void dumpHeightMaps(const RegionFile &region_file) {
    for (auto chunkPos : region_file.getContainingChunks()) {
        const std::vector<uint8_t> &data = region_file.getChunkData(chunkPos);

        nbt::NBTFile nbt;
        nbt.readNBT(reinterpret_cast<const char *>(&data[0]), data.size(), nbt::Compression::ZLIB);
        // cout << "chunk " << chunkPos.x << " " << chunkPos.z << "\n";

        const auto &heightMaps = nbt.findTag<nbt::TagCompound>("Heightmaps");
        if (heightMaps.hasTag<nbt::TagLongArray>("OCEAN_FLOOR")) {
            const auto &tag = heightMaps.findTag<nbt::TagLongArray>("OCEAN_FLOOR");
            cout << chunkPos.x << " " << chunkPos.z << " ";
            bool first = true;
            for (int i = 0; i < 37; i++) {
                uint64_t l = tag.payload[i];
                for (int j = 0; j < 7; j++) {
                    uint16_t height = (uint16_t)(l >> 9 * j) & 0x1ff;
                    cout << (first ? "" : ",") << height;
                    first = false;
                }
            }
            // tag.write(cout);
            cout << endl;
        }
    }
}

template <std::size_t PALETTE_SIZE>
void readPackedShorts_v116(
    const std::vector<int64_t> &data, std::array<uint16_t, PALETTE_SIZE> &palette
) {
    uint32_t shorts_per_long = (PALETTE_SIZE + data.size() - 1) / data.size();
    uint32_t bits_per_value = 64 / shorts_per_long;
    palette.fill(0);
    uint16_t mask = (1 << bits_per_value) - 1;

    for (uint32_t i = 0; i < shorts_per_long; i++) {
        uint32_t j = 0;
        for (uint32_t k = i; k < PALETTE_SIZE; k += shorts_per_long) {
            assert(j < data.size());
            palette[k] = (uint16_t)(data[j] >> (bits_per_value * i)) & mask;
            j++;
        }
        assert(j <= data.size());
    }
}

void dumpBlockStates(
    const RegionFile &region_file,
    const ChunkPos &chunkPos,
    const nbt::NBTFile &chunk,
    const Limits &limits
) {
    const nbt::TagList &sections = chunk.findTag<nbt::TagList>("sections");
    for (auto it = sections.payload.begin(); it != sections.payload.end(); ++it) {
        const nbt::TagCompound &section = (*it)->cast<nbt::TagCompound>();
        const nbt::TagByte &y_tag = section.findTag<nbt::TagByte>("Y");
        int section_y = int(y_tag.payload);

        if (limits.min_y.has_value() && (section_y * 16 + 15) < limits.min_y)
            continue;
        if (limits.max_y.has_value() && (section_y * 16) > limits.max_y)
            continue;

        if (!section.hasTag<nbt::TagCompound>("block_states"))
            continue;

        const nbt::TagCompound &block_states = section.findTag<nbt::TagCompound>("block_states");

        const nbt::TagList &palette = block_states.findTag<nbt::TagList>("palette");

        cout << "{\"section\": [" << chunkPos.x << ", " << section_y << ", " << chunkPos.z << "], ";
        cout << "\"palette\": [";
        for (size_t i = 0; i < palette.payload.size(); i++) {
            const auto entry = palette.payload.at(i)->cast<nbt::TagCompound>();
            const auto name = entry.findTag<nbt::TagString>("Name").payload;
            cout << "\"" << name << "\"";
            if (i < palette.payload.size() - 1)
                cout << ", ";
        }
        cout << "], ";

        cout << "\"block_states\": [";
        if (block_states.hasTag<nbt::TagLongArray>("data")) {
            const nbt::TagLongArray &block_states_data =
                block_states.findTag<nbt::TagLongArray>("data");
            std::array<uint16_t, 4096L> blocks;
            readPackedShorts_v116(block_states_data.payload, blocks);
            for (int i = 0; i < 4096; i++) {
                cout << blocks.at(i);
                if (i < 4095)
                    cout << ",";
            }
        }
        cout << "]}\n";
    }
}

void dumpContainers(
    const RegionFile &region_file,
    const ChunkPos &chunkPos,
    const nbt::NBTFile &chunk,
    const Limits &limits
) {
    if (!chunk.hasTag<nbt::TagList>("block_entities")) {
        return;
    }

    const nbt::TagList &entities = chunk.findTag<nbt::TagList>("block_entities");

    for (auto it = entities.payload.begin(); it != entities.payload.end(); ++it) {
        const nbt::TagCompound &entity = (*it)->cast<nbt::TagCompound>();
        if (!entity.hasTag<nbt::TagList>("Items"))
            continue;

        string entity_id = entity.findTag<nbt::TagString>("id").payload;
        int x = entity.findTag<nbt::TagInt>("x").payload;
        int y = entity.findTag<nbt::TagInt>("y").payload;
        int z = entity.findTag<nbt::TagInt>("z").payload;

        if (limits.min_x.has_value() && x < limits.min_x)
            continue;
        if (limits.max_x.has_value() && x > limits.max_x)
            continue;
        if (limits.min_y.has_value() && y < limits.min_y)
            continue;
        if (limits.max_y.has_value() && y > limits.max_y)
            continue;
        if (limits.min_z.has_value() && z < limits.min_z)
            continue;
        if (limits.max_z.has_value() && z > limits.max_z)
            continue;

        // entity.dump(cout);

        const nbt::TagList &items = entity.findTag<nbt::TagList>("Items");
        for (auto it = items.payload.begin(); it != items.payload.end(); ++it) {
            const nbt::TagCompound &item = (*it)->cast<nbt::TagCompound>();

            string item_id = item.findTag<nbt::TagString>("id").payload;
            int slot = item.findTag<nbt::TagByte>("Slot").payload;
            int count = item.findTag<nbt::TagByte>("Count").payload;

            if (item.hasTag("tag")) {
                auto item_tag = item.findTag<nbt::TagCompound>("tag");
                if (item_tag.hasTag("BlockEntityTag")) {
                    auto block_entity = item_tag.findTag<nbt::TagCompound>("BlockEntityTag");
                    if (block_entity.hasTag("Items")) {
                        auto subitems = block_entity.findTag<nbt::TagList>("Items");

                        for (auto &ptr : subitems.payload) {
                            auto subitem = ptr->cast<nbt::TagCompound>();
                            string subitem_id = subitem.findTag<nbt::TagString>("id").payload;
                            int subitem_slot = subitem.findTag<nbt::TagByte>("Slot").payload;
                            int subitem_count = subitem.findTag<nbt::TagByte>("Count").payload;
                            cout << entity_id << " " << x << "," << y << "," << z << " ";
                            cout << "item=" << subitem_id << " ";
                            cout << "slot=" << slot << "(" << subitem_slot << ") "
                                 << "count=" << subitem_count << endl;
                        }
                    }
                }
            }

            cout << entity_id << " " << x << "," << y << "," << z << " ";
            cout << "item=" << item_id << " ";
            cout << "slot=" << slot << " count=" << count << endl;
        }
    }
}

void scanRegion(const RegionFile &region_file, const Limits &limits, const string &action) {
    const auto regionPos = region_file.getPos();
    int region_max_x = (regionPos.x + 1) * 32 * 16 - 1;
    int region_min_x = (regionPos.x) * 32 * 16;
    int region_max_z = (regionPos.z + 1) * 32 * 16 - 1;
    int region_min_z = (regionPos.z) * 32 * 16;

    if (limits.max_x.has_value() && region_min_x > limits.max_x)
        return;
    if (limits.min_x.has_value() && region_max_x < limits.min_x)
        return;
    if (limits.max_z.has_value() && region_min_z > limits.max_z)
        return;
    if (limits.min_z.has_value() && region_max_z < limits.min_z)
        return;

    for (auto chunkPos : region_file.getContainingChunks()) {
        int chunk_max_x = (chunkPos.x + 1) * 16 - 1;
        int chunk_min_x = (chunkPos.x) * 16;
        int chunk_max_z = (chunkPos.z + 1) * 16 - 1;
        int chunk_min_z = (chunkPos.z) * 16;

        if (limits.max_x.has_value() && chunk_min_x > limits.max_x)
            continue;
        if (limits.min_x.has_value() && chunk_max_x < limits.min_x)
            continue;
        if (limits.max_z.has_value() && chunk_min_z > limits.max_z)
            continue;
        if (limits.min_z.has_value() && chunk_max_z < limits.min_z)
            continue;

        const std::vector<uint8_t> &data = region_file.getChunkData(chunkPos);

        nbt::NBTFile chunk;
        chunk.readNBT(
            reinterpret_cast<const char *>(&data[0]), data.size(), nbt::Compression::ZLIB
        );

        if (action == "containers") {
            dumpContainers(region_file, chunkPos, chunk, limits);
        } else if (action == "block_states") {
            dumpBlockStates(region_file, chunkPos, chunk, limits);
        }
    }

    // auto chunks = region.getContainingChunks();
    // for (auto chunk_it = chunks.begin(); chunk_it != chunks.end(); ++chunk_it) {
    // 	if (region.getChunkTimestamp(*chunk_it) < timestamp)
    // 		continue;

    // 	this->entities[*region_it][*chunk_it].clear();

    // 	mc::nbt::NBTFile nbt;
    // 	const std::vector<uint8_t>& data = region.getChunkData(*chunk_it);

    // }
}

void scanWorld(
    const string &world_dir, const Limits &limits, const string &dimension, const string &action
) {
    Dimension d = Dimension::OVERWORLD;
    if (dimension == "nether")
        d = Dimension::NETHER;
    if (dimension == "end")
        d = Dimension::END;
    World world(world_dir, d);
    if (!world.load()) {
        cerr << "error loading world" << endl;
    }
    auto regionSet = world.getAvailableRegions();
    for (auto regionPos : regionSet) {
        RegionFile region_file;
        world.getRegion(regionPos, region_file);
        region_file.read();

        scanRegion(region_file, limits, action);
    }
}

int main(int argc, char **argv) {

    // Declare a group of options that will be
    // allowed only on command line

    po::options_description generic("Generic options");
    auto opts = generic.add_options();

    bool block_states = false;

    opts = opts("version,v", "print version string");
    opts = opts("help", "produce help message");
    opts = opts("dimension,d", po::value<string>()->default_value("overworld"), "dimension");
    opts = opts("from,f", po::value<string>(), "from");
    opts = opts("to,t", po::value<string>(), "to");
    opts = opts("region,r", po::value<string>(), "search block entities in region");
    opts = opts("dump", po::value<string>(), "dump region nbt in human readable format");
    opts = opts("sections", po::value<string>(), "write region sections data in raw nbt format");
    opts = opts(
        "block-states", po::bool_switch(&block_states), "write region block data in raw nbt format"
    );
    opts =
        opts("height-maps", po::value<string>(), "write region height-map data in raw nbt format");

    // Hidden options, will be allowed both on command line and
    // in config file, but will not be shown to the user.
    po::options_description hidden("Hidden options");
    hidden.add_options()("world_dir", po::value<string>(), "world directory");

    po::options_description cmdline_options;
    cmdline_options.add(generic).add(hidden);

    po::options_description visible("Allowed options");
    visible.add(generic);

    po::positional_options_description p;
    p.add("world_dir", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << visible;
        return 1;
    }

    Limits limits;

    const regex coords_re("(-?[0-9]),(-?[0-9]),(-?[0-9])");
    optional<int> fx, fy, fz;
    optional<int> tx, ty, tz;
    if (vm.count("from"))
        tie(fx, fy, fz) = parseCoords(vm["from"].as<string>());
    if (vm.count("to"))
        tie(tx, ty, tz) = parseCoords(vm["to"].as<string>());

    limits.min_x = min(fx, tx);
    limits.min_y = min(fy, ty);
    limits.min_z = min(fz, tz);
    limits.max_x = max(fx, tx);
    limits.max_y = max(fy, ty);
    limits.max_z = max(fz, tz);

    // if (vm.count("minimum-x")) limits.min_x = vm["minimum-x"].as<int>();
    // if (vm.count("maximum-x")) limits.max_x = vm["maximum-x"].as<int>();
    // if (vm.count("minimum-y")) limits.min_y = vm["minimum-y"].as<int>();
    // if (vm.count("maximum-y")) limits.max_y = vm["maximum-y"].as<int>();
    // if (vm.count("minimum-z")) limits.min_z = vm["minimum-z"].as<int>();
    // if (vm.count("maximum-z")) limits.max_z = vm["maximum-z"].as<int>();

    if (vm.count("dump")) {
        auto filename = vm["dump"].as<string>();
        RegionFile region_file(filename);
        region_file.read();
        dumpRegion(region_file);
    } else if (block_states) {
        scanWorld(
            vm["world_dir"].as<string>(), limits, vm["dimension"].as<string>(), "block_states"
        );
    } else if (vm.count("height-maps")) {
        auto filename = vm["height-maps"].as<string>();
        RegionFile region_file(filename);
        region_file.read();
        dumpHeightMaps(region_file);
    } else if (vm.count("region")) {
        auto filename = vm["region"].as<string>();
        RegionFile region_file(filename);
        region_file.read();

        scanRegion(region_file, limits, "containers");
    } else {

        if (!vm.count("world_dir")) {
            cout << "Usage: " << visible << endl;
            return 1;
        }

        scanWorld(vm["world_dir"].as<string>(), limits, vm["dimension"].as<string>(), "containers");
    }
}
