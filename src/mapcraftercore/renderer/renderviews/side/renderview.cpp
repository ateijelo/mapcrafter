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

#include "renderview.h"

#include "../../../mc/blockstate.h"
#include "../../rendermode.h"
#include "../../rendermodes/overlay.h"
#include "tilerenderer.h"
#include "tileset.h"

namespace mapcrafter {
namespace renderer {

BlockImages *SideRenderView::createBlockImages(mc::BlockStateRegistry &block_registry) const {
    return new RenderedBlockImages(block_registry);
}

TileSet *SideRenderView::createTileSet(int tile_width) const { return new SideTileSet(tile_width); }

TileRenderer *SideRenderView::createTileRenderer(mc::BlockStateRegistry &block_registry,
                                                 BlockImages *images, int tile_width,
                                                 mc::WorldCache *world,
                                                 RenderMode *render_mode) const {
    return new SideTileRenderer(this, block_registry, images, tile_width, world, render_mode);
}

void SideRenderView::configureBlockImages(BlockImages *images,
                                          const config::WorldSection &world_config,
                                          const config::MapSection &map_config) const {}

void SideRenderView::configureTileRenderer(TileRenderer *tile_renderer,
                                           const config::WorldSection &world_config,
                                           const config::MapSection &map_config) const {
    assert(tile_renderer != nullptr);
    RenderView::configureTileRenderer(tile_renderer, world_config, map_config);

    tile_renderer->setShadowEdges({2, 1, 2, 2, 2});
}

} /* namespace renderer */
} /* namespace mapcrafter */
