@echo off
set IP=192.168.68.155
set PSW=Password
goto NORMAL
:NORMAL
cls
echo                 Zugangsdaten
echo      ---------------------------------
echo              IP= %IP%
echo              Passwort= %PSW% 
echo      =================================
echo.
echo.
echo Diese Datei kann man mit einem Editor bearbeiten,
echo zum setzen der IP Adresse und des Passwortes.
echo Gibt man nichts ein, werden die Daten uebernommen
echo.
echo Ansonsten, jetzt hier die Daten eintragen
echo.
set /p IP="Die IP vom Display: "
set /p PSW="Das Passwort von MQTT Benutzername: "
echo "Am Display Boot gedrueckt lassen. Dann hier Enter: "
pause
goto END

:END
start cmd /k espota.exe -r -i %IP% -p 3232 --auth=%PSW% --timeout=10000 -f esp32tft.ino.bin
goto NORMAL