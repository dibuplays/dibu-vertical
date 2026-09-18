# Dibu Vertical for OBS Studio

Dibu Vertical adds a true secondary vertical canvas to OBS Studio for portrait
streaming and recording. It is a GPL-2.0 fork of the open-source
[Aitum Vertical](https://github.com/Aitum/obs-vertical-canvas) project, with a
separate plugin identity and Dibu Plays branding.

## Features

- Smart Focus character tracking: draw a box around a gameplay character and
  Dibu Vertical automatically pans the selected source on the portrait canvas
- A purpose-built Dibu Studio interface with a branded header, dedicated
  tracking controls, labeled outputs, and a clearer editing hierarchy
- Independent vertical scenes, sources, transitions, and preview
- Source move, resize, crop, ordering, visibility, grouping, and filters
- Vertical recording and streaming outputs
- Vertical virtual camera
- Backtrack/replay clips
- Linked horizontal and vertical scenes
- Hotkeys, projectors, and OBS WebSocket requests
- Multiple portrait canvas resolutions

## Smart Focus (v0.2)

1. Put the gameplay source in a Dibu vertical scene and size it so there is
   extra picture outside the portrait frame for Smart Focus to pan through.
2. Click **Draw Target Box**, then drag a tight box around the player character
   in the vertical preview.
3. Tracking begins automatically. Use **Camera smoothness** to choose between a
   cinematic slow follow and a faster response.
4. Click **Pause** to hold the current framing, or **Clear** before choosing a
   new character.

Smart Focus analyses only rendered video frames. It does not read game memory,
inject code into a game, or affect the normal horizontal OBS canvas. The tracker
uses lighting-resistant visual matching, motion prediction, anti-drift checks,
and automatic full-frame reacquisition. A target may still need to be selected
again after a cutscene, respawn, major costume change, or long disappearance.

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
