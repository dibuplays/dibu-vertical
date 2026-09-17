# Dibu Vertical for OBS Studio

Dibu Vertical adds a true secondary vertical canvas to OBS Studio for portrait
streaming and recording. It is a GPL-2.0 fork of the open-source
[Aitum Vertical](https://github.com/Aitum/obs-vertical-canvas) project, with a
separate plugin identity and Dibu Plays branding.

## Features

- Independent vertical scenes, sources, transitions, and preview
- Source move, resize, crop, ordering, visibility, grouping, and filters
- Vertical recording and streaming outputs
- Vertical virtual camera
- Backtrack/replay clips
- Linked horizontal and vertical scenes
- Hotkeys, projectors, and OBS WebSocket requests
- Multiple portrait canvas resolutions

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
