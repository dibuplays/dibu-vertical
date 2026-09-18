# Dibu Vertical for OBS Studio

Dibu Vertical adds a true secondary vertical canvas to OBS Studio for portrait
streaming and recording. It is a GPL-2.0 fork of the open-source
[Aitum Vertical](https://github.com/Aitum/obs-vertical-canvas) project, with a
separate plugin identity and Dibu Plays branding.

## Features

- 3D Full View: fit a complete widescreen gameplay source into portrait using
  curved edge compression instead of cropping away the sides
- A purpose-built Dibu Studio interface with a branded header, dedicated
  3D Full View controls, labeled outputs, and a clearer editing hierarchy
- Independent vertical scenes, sources, transitions, and preview
- Source move, resize, crop, ordering, visibility, grouping, and filters
- Vertical recording and streaming outputs
- Vertical virtual camera
- Backtrack/replay clips
- Linked horizontal and vertical scenes
- Hotkeys, projectors, and OBS WebSocket requests
- Multiple portrait canvas resolutions

## 3D Full View (v0.3)

1. Select one rectangular gameplay or video source in the vertical preview.
2. Click **Enable 3D Full View**. Dibu Vertical creates an isolated copy for the
   portrait canvas, stretches it to the full canvas, and applies the projection.
3. Adjust **3D curve** to control the cylindrical look and **Edge squeeze** to
   decide how much of the width is moved into the portrait edges.
4. Click **Restore Normal View** to remove the copy and restore the source's
   original position, crop, size, order, visibility, lock, and blend settings.

The complete left and right sides stay visible. The normal horizontal OBS source
is not filtered or resized. Sources that explicitly forbid duplication are left
untouched so the feature cannot accidentally alter the horizontal layout.

## Requirements

- OBS Studio 31.1 or newer
- Windows, macOS, or Ubuntu

## Important compatibility note

Dibu Vertical uses its own module, dock, source, output, hotkey, WebSocket, and
installer identifiers. Do not use it at the same time as another vertical
canvas plugin until compatibility has been tested with your OBS scene
collection. Back up your OBS profile and scene collection before testing.

## Build

For an out-of-tree build, use the supplied CMake presets and GitHub Actions
workflows. The workflows package installers for Windows, macOS, and Ubuntu.

## License and credit

Licensed under GPL-2.0. See [LICENSE](LICENSE) and [NOTICE.md](NOTICE.md).
This project preserves credit to Aitum and all upstream contributors.
