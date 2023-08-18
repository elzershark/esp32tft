@echo off
set COMPORT=COM3
goto END
:END
cls
echo                 Zugangsdaten
echo      ---------------------------------
echo              COMPort= %COMPORT%
echo      =================================
echo.
echo.
echo.
echo Diese Datei kann man mit einem Editor bearbeiten,
echo zum setzen des COMPorts.
echo Gibt man nichts ein, werden die Daten uebernommen
echo.
echo Ansonsten, jetzt hier die Daten eintragen
echo.
set /p COMPORT="Den COM Port komplett eintragen(z.B. COM3): "
echo   [1] Ohne resetten (Bei updates)
echo   [2] Mit resetten. Alles wird geloescht
echo   [3] COM Ports anzeigen
echo.
set asw=0
set /p asw="Bitte Auswahl eingeben: "
if %asw%==1 goto NORMAL
if %asw%==2 goto RESET
if %asw%==3 goto COMP
goto END
:RESET
echo "Am Display Boot gedrueckt lassen. Dann hier Enter: "
pause
start cmd /k esptool.exe --chip esp32 --port %COMPORT% --baud 921600  --before default_reset --after hard_reset write_flash -e -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x1000 esp32tft.ino.bootloader.bin 0x8000 esp32tft.ino.partitions.bin 0xe000 boot_app0.bin 0x10000 esp32tft.ino.bin
goto END
:NORMAL
echo "Am Display Boot gedrueckt lassen. Dann hier Enter: "
pause
start cmd /k esptool.exe --chip esp32 --port %COMPORT% --baud 921600  --before default_reset --after hard_reset write_flash  -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x1000 esp32tft.ino.bootloader.bin 0x8000 esp32tft.ino.partitions.bin 0xe000 boot_app0.bin 0x10000 esp32tft.ino.bin
goto END
:COMP
start cmd /k chgport
goto END