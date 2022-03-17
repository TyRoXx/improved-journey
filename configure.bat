pushd vcpkg || exit /B 1
:: disable annoying message about metrics (we don't really care about metrics, but there seems to be no other way to make the message go away)
call bootstrap-vcpkg.bat -disableMetrics || exit /B 1
:: vcpkg install is missing a way to pass a triplet, so we have to set it for every package individually:
.\vcpkg.exe install --disable-metrics fmt:x64-windows-static imgui-sfml:x64-windows-static catch2:x64-windows-static sqlite3:x64-windows-static || exit 1
popd || exit 1
.\vcpkg\downloads\tools\cmake-3.22.2-windows\cmake-3.22.2-windows-i386\bin\cmake.exe -B ..\improved_journey_build -S . -G "Visual Studio 16 2019" -A x64 || exit 1
