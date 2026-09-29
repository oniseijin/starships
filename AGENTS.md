# AGENTS.md

This repository contains the source code for "Starships," a recreation of an old starship game, originally prototyped in Processing and ported to OpenFrameworks.

## 🛠️ Key Commands

*   **Build/Deploy:** Use Gradle tasks (`gradlew`) to build and deploy the application.
    *   `gradlew deploy`: Triggers the build and copies files to `/Applications/Starships`.
    *   `gradlew deployLocal`: Copies necessary files to the local Applications directory.

## 📂 Repository Layout & Structure

*   **Source Code:** Core logic resides in `src/` (e.g., `Ship.cpp`, `Lazer.cpp`, `main.cpp`).
*   **Build System:** Gradle scripts (`build.gradle`, `gradlew`) manage the build process, utilizing the `org.openbakery.xcode-plugin`.
*   **Assets:** Media files (images, sounds) are located in `bin/data/`.
*   **Project Files:** Xcode project files are in `Starships.xcodeproj/`.

## 📜 Conventions & Cautions

*   **Active Again:** Revived 2026-09-29 on arm64 Apple Silicon (openFrameworks 0.12.1) — see `BUILD-arm64.md` for the supported make build; the Gradle/Xcode targets are legacy and NOT the supported path.
*   **Web Remake:** `web/` is a rules-faithful 1v1 LAN remake (Python stdlib + websockets server, dependency-free canvas client) — see `web/README.md`.
*   **Development Context:** openFrameworks 0.12.1 lives at `~/lib/of_v0.12.1_osx_release` (NOT `~/.local/...` — oF makefiles drop `/.` paths).
*   **Build Target:** `make` Release (default target).

---
*Last commit referenced: `2020-02-22 fixes #9`*
