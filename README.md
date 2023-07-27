# esp32tft
iobrokeresp32tft

https://amzn.to/472Emls


Forum = https://www.elzershark.com/forum/index.php?thread/201

ioborker adapter mqtt server installieren.

Am basteln. Funz aber schon.

bin Datei herunterladen.

esp32tft/build/esp32.esp32.esp32/esp32tft.ino.bin

Auf das Gerät aufspielen. https://github.com/esphome/esphome-flasher/releases

Danach einfach sich verbinden mit WLAN -> 192.168.4.1 aufrufen und WLAN, MQTT Daten eingeben.

Fertig.

Code // https://www.barth-dev.de/online/rgb565-color-picker/ gelb = 255,255,0 = 0xFFE0; // https://calculator.name/baseconvert/hexadecimal/decimal/0xFFE0 = 65504; // TFT_YELLOW = 65504 zum testen in den mqtt datenpunkt das hier eingeben.

Code 0;0;2;hallo, was geht

Aufbau: Getrennt wird mit ";"Semikolon . also nicht nutzen als text. 4 Daten Teile gibt es

0; 0; 2; hallo, was geht

1--2--3-------4


1: Textbeginn Cursor x-Achse

2: Textbeginn Cursor y-Achse

3: Textgröße (0-??) nicht getestet

4: Der Text. Leerzeichen, neue Zeile u.s.w. funzt alles.

mqtt:

Datenpunkt text /text1 /text2 = Siehe oben (0; 0; 2; hallo, was geht)

Datenpunkt rotation = Rotation des Display (0-3)

Datenpunkt Brightness = Bildschirmhelligkeit (0-255)

Datenpunkt fillScreen = Hintergrundfarbe (0 ist z.B. Schwarz) siehe oben die links

Datenpunkt Textfarbe = Textfarbe (65504 ist z.B. Gelb) siehe oben die links

Datenpunkt fillRect = trenne mit , (Komma). 5 Teile z.b. "47,53,92,29,31" 

Erklärung:

  47;53;92;29;31
  
  1--2--3--4--5
  

           53
        ***********
        *         *
      47*       92* 
        *         *
        ***********29 
	       

1 = Abstand von links

2 = Abstand von Oben

3 = Füllbereich pixel von links nach rechts

4 = Füllbereich pixel von oben nach unten

5 = Füllfarbe

Touchscreen Klicks: x,y

           y  y1
       x***********
        *         *
        *x2       *x1
        *         *
        *********** 
	         y2

1= x

2 = 240 - y

3 = x1 - x2

4 = y1 - y2 

https://amzn.to/472Emls
