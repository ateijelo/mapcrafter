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

#ifndef WORLDCROP_H_
#define WORLDCROP_H_

#include "pos.h"

#include <bitset>
#include <memory>
#include <string>
#include <vector>

namespace mapcrafter {
namespace mc {

/**
 * A 1-dimensional boundary.
 * Has either two limits, one limit (minimum or maximum) or no limit.
 */
template <typename T> class Bounds {
  public:
    Bounds();
    ~Bounds();

    /**
     * Sets the minimum/maximum limit.
     */
    void setMin(T min);
    void setMax(T max);

    /**
     * Resets the minimum/maximum, i.e. sets it to infinity (or -infinity for minimum).
     */
    void resetMin();
    void resetMax();

    /**
     * Returns whether a specific value is within in the bounds.
     */
    bool contains(T value) const;

  private:
    // minimum, maximum
    T min, max;
    // whether minimum, maximum is set to infinity (or -infinity for minimum)
    bool min_set, max_set;
};

/**
 * Data structure to hold information about which blocks should be hidden/shown.
 */
class BlockMask {
  public:
    /**
     * Describes the state of a block with a specific id.
     */
    enum BlockState {
        // all blocks with this id are hidden, independent of the block data
        COMPLETELY_HIDDEN,
        // all blocks with this id are shown, also independent of the block data
        COMPLETELY_SHOWN,
        // some block with this id are hidden, some are shown, depending of the block data
        PARTIALLY_HIDDEN_SHOWN
    };

    BlockMask();
    ~BlockMask();

    /**
     * Sets all blocks with a specific id to be hidden/shown;
     */
    void set(uint16_t id, bool shown);

    /**
     * Sets a block with a specific id/data to be hidden/shown;
     */
    void set(uint16_t id, uint8_t data, bool shown);

    /**
     * Sets all blocks with specific id and data values to be hidden/shown.
     * Using a bitmask all blocks with data values (block data value & bitmask) == specified data
     * are set to be shown/hidden.
     */
    void set(uint16_t id, uint8_t data, uint8_t bitmask, bool shown);

    /**
     * Sets all blocks from id1 to id2 to be hidden/shown.
     */
    void setRange(uint16_t id1, uint16_t id2, bool shown);

    /**
     * Sets all blocks to be hidden or shown.
     */
    void setAll(bool shown);

    /**
     * Loads the block mask from a string definition. Throws a std::invalid_argument
     * exception if the supplied block mask definition is invalid.
     *
     * The block mask definition is a space separated list of block groups you want to
     * hide/show. If a '!' precedes a block group, all blocks of this block group are
     * hidden, otherwise they are shown. Per default, all blocks are shown.
     *
     * Possible block groups are:
     * - All blocks:
     *     '*'
     * - A single block (independent of block data):
     *     '[blockid]'
     * - A single block with specific block data:
     *     '[blockid]:[blockdata]'
     * - A range of blocks:
     *     '[blockid1]-[blockid2]'
     * - All block with a specific id and (block data & bitmask) == specified data:
     *     '[blockid]:[blockdata]b[bitmask]'
     *
     * For example:
     * - Hide all blocks except blocks with id 1,7,8,9 or id 3 / data 2:
     *     '!* 1 3:2 7-9'
     * - Show all blocks except jungle wood and jungle leaves:
     *     '!17:3b3 !18:3b3' (Jungle wood and jungle leaves have id 17 and 18
     *                        and use data value 3 for first two bits (bitmask 3 = 0b11),
     *                        other bits are used otherwise -> ignoring all those bits)
     *
     * TL;DR (except the space at the end): (!?(\*|\d+(:\d+(b\d+)?)?|\d+-\d+) )+
     */
    void loadFromStringDefinition(const std::string &definition);

    /**
     * Returns whether all, none, or some blocks with a specific id are hidden/shown.
     */
    const BlockMask::BlockState &getBlockState(uint16_t id) const;

    /**
     * Returns whether a block
     */
    bool isHidden(uint16_t id, uint8_t data) const;

  private:
    std::vector<BlockMask::BlockState> block_states;

    // the actual block mask
    // 65536 entries for the 16 bit long block id
    // * 16 entries for the 4 bit block data
    std::bitset<65536 * 16> block_mask;

    /**
     * Updates whether all, none, or some blocks with a specific id are shown/hidden.
     */
    void updateBlockState(uint16_t id);
};

/**
 * Boundaries to crop a Minecraft World.
 */
class WorldCrop {
  public:
    // different types of boundaries -- either rectangular or circular
    const static int RECTANGULAR = 1;
    const static int CIRCULAR = 2;

    WorldCrop();
    ~WorldCrop();

    /**
     * Returns the type of the boundaries.
     */
    int getType() const;

    /**
     * Sets the minimum/maximum values for y-coordinates.
     */
    void setMinY(int value);
    void setMaxY(int value);

    /**
     * Sets the limits (in block coordinates) of the rectangular boundaries.
     */
    void setMinX(int value);
    void setMaxX(int value);
    void setMinZ(int value);
    void setMaxZ(int value);

    /**
     * Sets the limits (in block coordinates) of the circular boundaries.
     */
    void setCenter(const BlockPos &pos);
    void setRadius(long radius);

    /**
     * Returns whether a specific region is contained.
     */
    bool isRegionContained(const mc::RegionPos &region) const;

    /**
     * Returns whether a specific chunk is contained.
     */
    bool isChunkContained(const mc::ChunkPos &chunk) const;

    /**
     * Returns whether a specific chunk is completely contained. With completely is
     * meant that all x- and z-coordinates are included, only some y-coordinates may not
     * be included.
     */
    bool isChunkCompletelyContained(const mc::ChunkPos &chunk) const;

    /**
     * Returns whether a block is contained regarding x- and z-coordinates.
     * Used if a chunk is only partly contained.
     */
    bool isBlockContainedXZ(const mc::BlockPos &block) const;

    /**
     * Returns whether a block is contained regarding its y-coordinate.
     */
    bool isBlockContainedY(const mc::BlockPos &block) const;

    /**
     * Returns/sets whether unpopulated chunks should be cropped.
     */
    bool hasCropUnpopulatedChunks() const;
    void setCropUnpopulatedChunks(bool crop);

    /**
     * Returns whether this world has a block mask.
     */
    bool hasBlockMask() const;

    /**
     * Returns the block mask of this world. Returns a nullptr if !hasBlockMask().
     */
    const BlockMask *getBlockMask() const;

    /**
     * Loads the block mask from a string definition. See BlockMask::loadFromString.
     */
    void loadBlockMask(const std::string &definition);

  private:
    // type of world boundaries -- either RECTANGULAR or CIRCULAR
    int type;

    // usable for both rectangular and circular
    Bounds<int> bounds_y;

    // rectangular limits:
    // in block coordinates
    Bounds<int> bounds_x, bounds_z;
    // in chunk coordinates
    Bounds<int> bounds_chunk_x, bounds_chunk_z;
    // in region coordinates
    Bounds<int> bounds_region_x, bounds_region_z;

    // circular limits
    BlockPos center;
    long radius;

    // whether to hide unpopulated chunks
    bool crop_unpopulated_chunks;

    // block mask
    std::shared_ptr<BlockMask> block_mask;
};

template <typename T>
Bounds<T>::Bounds()
    : min_set(false),
      max_set(false) {}

template <typename T> Bounds<T>::~Bounds() {}

template <typename T> void Bounds<T>::setMin(T min) {
    this->min = min;
    min_set = true;
}

template <typename T> void Bounds<T>::setMax(T max) {
    this->max = max;
    max_set = true;
}

template <typename T> void Bounds<T>::resetMin() { min_set = false; }

template <typename T> void Bounds<T>::resetMax() { max_set = false; }

template <typename T> bool Bounds<T>::contains(T value) const {
    // case 1: no limits
    // value is definitely included
    if (!min_set && !max_set)
        return true;

    // case 2: only a minimum limit
    // value is included if value >= minimum
    if (min_set && !max_set)
        return value >= min;

    // case 3: only a maximum limits
    // value is included if value <= maximum
    if (max_set && !min_set)
        return value <= max;

    // case 3: two limits
    // value is included if value >= minimum and value <= maximum
    return min <= value && value <= max;
}

} // namespace mc
} // namespace mapcrafter

#endif /* WORLDCROP_H_ */
