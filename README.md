# GW2 FPS Unlocker (Nexus Addon)

**AI Disclaimer**: AI was used to check the addon against older versions of the game to make sure it doesnt break each update AND for creating a better pattern to always find the correct setter even after game patches/hotfixes (Opus 5.5 high)

A lightweight open-source addon for **Guild Wars 2 (64-bit)** that bypasses the hard 240 FPS engine cap by patching the game's native frametime calculation. It features a fully integrated **ImGui configuration menu** right inside Nexus.

![Nexus Menu](https://img.shields.io/badge/Nexus-API%20v6-blue) ![C++](https://img.shields.io/badge/C%2B%2B-20-orange) ![License](https://img.shields.io/badge/License-MIT-green)

---

## Disclaimer 
* Modifying Guild Wars 2 through any third party software is not supported by ArenaNet nor by any of its partners.
* Your using this Addon at your own risk.
* to any person working at Anet you can reach out to me via "issues" to ask for full source code or takedown requests of this addon
* Portions or the entirety of the codebase remain closed-source in compliance with requests by ArenaNet

## Features

* **Bypasses the 240 FPS Cap:** Unlocks high refresh rate monitors (300Hz, 360Hz+) by letting the game compute custom target frametimes natively.
* **In-Game ImGui Control Panel:** Seamlessly integrated into the Nexus Options tab with an optional standalone floating window.
* **Live Runtime Adjustment:** Change your target FPS dynamically on the fly (from 30 up to 10000 FPS) without restarting the game.
* **Zero Performance Overhead:** Uses a clean, single-point engine instruction patch instead of heavy polling loops.


## Usage

1. Open the Nexus panel in-game.
2. Go to **Addons** ➔ **FPS Unlocker**.
3. Toggle the unlocker on and use the slider or preset buttons (**60 FPS**, **144 FPS**, **240 FPS**, **360 FPS**, **500 FPS**, **1000 FPS**) to adjust your limit.
4. *Tip:* For best results, ensure **V-Sync** is disabled in your in-game graphics settings. If you play in *Windowed Fullscreen*, Windows DWM might cap your frame rate to your desktop monitor's refresh rate—switch to true *Fullscreen* if you want unlimited uncapped frames.

## Credits
* Special thanks to Delta for creating Nexus and for the invaluable guidance during development. 
* theraytheone for testing a early version of this addon to check if the fps beyond 500 fps works without issues