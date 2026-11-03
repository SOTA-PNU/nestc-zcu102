@echo off
rem Keep WSL alive. Waking it once with /bin/true lets it idle-stop and the CI runner goes Offline (2026-11-03).
powershell -NoProfile -WindowStyle Hidden -Command "Start-Process -WindowStyle Hidden -FilePath wsl.exe -ArgumentList '-d','Ubuntu-22.04','-e','sleep','infinity'"
