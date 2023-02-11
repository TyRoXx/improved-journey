#!/usr/bin/env bash
pushd vcpkg || exit 1
# disable annoying message about metrics (we don't really care about metrics, but there seems to be no other way to make the message go away)
./bootstrap-vcpkg.sh -disableMetrics || exit 1
# vcpkg install is missing a way to pass a triplet, so we have to set it for every package individually:
./vcpkg install --disable-metrics fmt:x64-linux imgui-sfml:x64-linux catch2:x64-linux sqlite3:x64-linux || exit 1
./vcpkg upgrade --no-dry-run || exit 1
popd || exit 1
./vcpkg/downloads/tools/cmake-3.25.0-linux/cmake-3.25.0-linux-x86_64/bin/cmake -B ../improved_journey_build -S . -G "CodeBlocks - Ninja" || exit 1
