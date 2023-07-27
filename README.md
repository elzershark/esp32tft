# esp32tft
iobrokeresp32tft

Forum = https://www.elzershark.com/forum/index.php?thread/201

ioborker adapter mqtt server installieren.

Am basteln. Funz aber schon.

bin Datei herunterladen.

https://github.com/elzershark/iobrokeresp32tft -> esp32tft/build/esp32.esp32.esp32/esp32tft.ino.bin

Auf das Gerät aufspielen. https://github.com/esphome/esphome-flasher/releases

Danach einfach sich verbinden mit WLAN -> 192.168.4.1 aufrufen und WLAN, MQTT Daten eingeben.

Fertig.

Code // https://www.barth-dev.de/online/rgb565-color-picker/ gelb = 255,255,0 = 0xFFE0; // https://calculator.name/baseconvert/hexadecimal/decimal/0xFFE0 = 65504; // TFT_YELLOW = 65504 zum testen in den mqtt datenpunkt das hier eingeben.

Code 1;20;0;65504;0;0;2;hallo, was geht

Aufbau: Getrennt wird mit ";"Semikolon . also nicht nutzen als text. 8 Taden Teile gibt es

1; 20; 0; 65504; 0; 0; 2; hallo, was geht

1--2---3----4----5--6--7-------8----------

1: Rotation des Display (0-3)

2: Bildschirmhelligkeit (0-255)

3: Hintergrundfarbe (0 ist z.B. Schwarz) siehe oben die links

4: Textfarbe (65504 ist z.B. Gelb) siehe oben die links

5: Textbeginn Cursor x-Achse

6: Textbeginn Cursor y-Achse

7: Textgröße (0-??) nicht getestet

8: Der Text. Leerzeichen, neue Zeile u.s.w. funzt alles.

Das im Bild ist 1 Text. Mit Leerzeichen, neue Zeile u.s.w. Einfach reinkopiert.

https://amzn.to/472Emls