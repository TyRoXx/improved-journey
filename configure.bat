pushd vcpkg || exit /B 1
:: disable annoying message about metrics (we don't really care about metrics, but there seems to be no other way to make the message go away)
call bootstrap-vcpkg.bat -disableMetrics || exit /B 1
:: vcpkg install is missing a way to pass a triplet, so we have to set it for every package individually:
.\vcpkg.exe install --recurse --disable-metrics fmt:x64-windows-static imgui[sdl2-binding,sdl2-renderer-binding]:x64-windows-static imgui-sfml:x64-windows-static catch2:x64-windows-static sqlite3:x64-windows-static sdl2:x64-windows-static sdl2-image:x64-windows-static sdl2-ttf:x64-windows-static || exit 1
.\vcpkg.exe upgrade --no-dry-run || exit 1
popd || exit 1
.\vcpkg\downloads\tools\cmake-3.25.1-windows\cmake-3.25.1-windows-i386\bin\cmake.exe -B ..\improved_journey_build -S . -G "Visual Studio 17 2022" -A x64 || exit 1
