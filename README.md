# Mapcrafter [![Build Status](https://travis-ci.org/mapcrafter/mapcrafter.svg?branch=master)](https://travis-ci.org/mapcrafter/mapcrafter)

by Moritz Hilscher

Mapcrafter is a high performance Minecraft map renderer written in C++. It renders
Minecraft worlds to a bunch of tiles (images) which are viewable as a map in any browser
using Leaflet.js.

<p align="center">
  <img src="https://i.imgur.com/WyKXrgO.png" alt="Example map">
</p>

<!-- TOC -->

- [Mapcrafter ![Build Status](https://travis-ci.org/mapcrafter/mapcrafter)](#mapcrafter-)
  - [Supported Minecraft versions](#supported-minecraft-versions)
  - [Features](#features)
  - [Requirements](#requirements)
  - [Example maps](#example-maps)
  - [Help](#help)
    - [Documentation](#documentation)
    - [Reporting bugs and feature requests](#reporting-bugs-and-feature-requests)
    - [Community help](#community-help)
  - [Acknowledgements](#acknowledgements)
  - [License](#license)
  - [Version history](#version-history)
  - [Building](#building)

<!-- /TOC -->

## Supported Minecraft versions

The current version supports Minecraft versions 1.13 through 1.16. Newer versions may partially work.
For Minecraft 1.12 (or older) please use [Mapcrafter 2.4](https://github.com/mapcrafter/mapcrafter/tree/v.2.4).

## Features

- **Web output:** Render your Minecraft worlds to maps viewable in any web browser!
- **Different render views:** Choose between different perspectives to render your world
  from! A 2D topdown and a 3D isometric render view are available!
- **Different rotations:** Choose from four different rotations to render your worlds from!
- **Different render modes:** Choose between different render modes like day, night and cave
  for your maps!
- **Different overlays:** Show additional information on your map! For example: Where can
  slimes spawn? Where can monsters spawn at night?
- **Configuration files:** Highly-customizable which worlds are rendered with which render
  view and other render parameters!
- **Markers:** Automatically generated markers from your Minecraft world data!
- **Other stuff:** Biome colors, incremental rendering, multithreading

## Requirements

- A Linux-based or Unix-like operating system like Mac OS
- A decent C++ compiler (preferable `gcc` >= 4.4, or `clang`), CMake and make if you want
  to build Mapcrafter from source
- Some libraries:
  - `libpng`
  - `libjpeg` (but you may use libjpeg-turbo as drop in replacement)
  - `libboost-iostreams`
  - `libboost-system`
  - `libboost-filesystem` (>= 1.42)
  - `libboost-program-options`
  - `libboost-test` (if you want to use the tests)
- For your Minecraft worlds:
  - Anvil world format
  - Minecraft resource packs

## Example maps

There are a few example maps of the renderer on the
[GitHub Wiki](https://github.com/mapcrafter/mapcrafter/wiki/Example-maps).
Please feel free to add your own maps to this list.

## Help

### Documentation

You can read the current documentation on [docs.mapcrafter.org](http://docs.mapcrafter.org)
and you can also [download other builds](https://readthedocs.org/projects/mapcrafter/downloads/)
from there. The documentation source is in the `docs/` directory and you can build it
yourself with Sphinx.

### Reporting bugs and feature requests

If you find bugs or problems when using Mapcrafter or if you have ideas for new
features, then please feel free to add an issue to the [GitHub issue
tracker](https://github.com/mapcrafter/mapcrafter/issues).

### Community help

[Join us on Discord](https://discord.gg/QNe8jXT), where you can meet other Mapcrafter users
and ask for help. Please be respectful with everyone and have patience with people who are
offering their time to help!

## Acknowledgements

Thanks to pigmap and Minecraft Overviewer, whose documentations and source code
were very helpful. I also used the alpha blending code of pigmap and some maps
stuff of the template from Minecraft Overviewer.

## License

Mapcrafter is free software and available under the GPL license.  You can
access the latest source code of Mapcrafter on GitHub:
http://github.com/mapcrafter/mapcrafter

## Version history

See the [CHANGELOG](./CHANGELOG.md) for details.

## Building

(disorganized notes, might clean up at some point)

### Generating textures with Blockcrafter

Blockcrafter is a separate tool from Mapcrafter [http://github.com/mapcrafter/blockcrafter], it uses the jar files from minecraft, or other asset directories to generate the textures for all blocks so Mapcrafter can render the map. This step is necessary when new Minecraft versions come out, but if you're compiling a version of Mapcrafter that already includes those, skip this step and go to (next section)[###building-mapcrafter]

Until I figure out how to let Docker use my GPU, using the docker container with --osmesa is not an option. It would take ages.

Refresh your python3.8 installation, just in case:

```
pyenv install 3.8.6
```

I don't like pyenv's manipulations, so I explicitly add the Python I want to my path:

export PATH="$HOME/.pyenv/versions/3.8.6/bin/:$PATH"

Create a venv, activate it and install it.

```sh
python -m venv env
. env/bin/active
python setup.py install
```

Get the assets from your launcher. I use PrismLauncher and get my assets like this:

```bash
unar  ~/.local/share/PrismLauncher/libraries/com/mojang/minecraft/1.20.6/minecraft-1.20.6-client.jar
```

Now this should work:

```bash
echo -n -e "" "-r"{0,1,2,3}" -v"{isometric,side,topdown}" -t"{12,16}"\n" |
    xargs -IOPTS -P16 sh -c 'blockcrafter-export \
    -a blockcrafter/custom_assets/ \
    -a minecraft-1.16.5-client/ \
    -a minecraft-1.16.5-client/assets/ \
    -a minecraft-1.17.1-client/ \
    -a minecraft-1.17.1-client/assets/ \
    -a minecraft-1.18.1-client/ \
    -a minecraft-1.18.1-client/assets/ \
    -a minecraft-1.19-client/ \
    -a minecraft-1.19-client/assets/ \
    -a minecraft-1.20.6-client/ \
    -a minecraft-1.20.6-client/assets/ \
    OPTS -o blocks/'
```

Change -P16 to how many tasks you want to run in parallel. The more the better, but more than your CPU's actual concurrency won't make a difference.

I started my world in Minecraft 1.15. Sometimes, a chunk has block ids that are old, and have been replaced by different IDs in more recent versions. To deal with that, I pass assets from all the versions I've used, as you'll see above. If you started your world more recently, you can skip older assets.

Bring the generated images and txt files to Mapcrafter's src/data/blocks directory.

Generate texture files:

```
./src/tools/mapcrafter_textures.py \
    -f ~/.local/share/multimc/libraries/com/mojang/minecraft/1.19/minecraft-1.19-client.jar \
    ./src/data/textures/
```

and then texture code:

```
./src/tools/gen_texture_code.sh
```

### Building Mapcrafter

Run this once:

```bash
cmake -S . -B build
```

And this every time you want to recompile:

```bash
cmake --build build -j16
```

Again, change -j16 to match how many threads to use.
