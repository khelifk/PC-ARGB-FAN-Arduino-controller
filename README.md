# PC-ARGB-FAN-Arduino-controller
This project is to help you build a PC ARGB Fan controller using an Arduino Nano and a cheap infrared receiver with a small remote control.
Hardware used for this project:
Arduino Nano
Thermalright TL-C12CW-S
Infrared receiver PC838
Ir Remote control YK-001
Hardware Configuration :
ARGB Data Pin: Digital Pin 3

IR Receiver: Digital Pin 5

Fan PWM Control (Optional): Digital Pin 9

Required Libraries
FastLED: For ARGB control

IRremote: For infrared remote handling

Remote Control Layout (YK-001)

Functionality Mapping:

Row 1 - Main Controls: Power Toggle, Animation Mode Change, Mute (Pause).

Row 2 - Playback Controls: Previous (Slower animation), Play/Pause, Next (Faster animation).

Row 3 - Volume/Brightness: Brightness decrease, Brightness increase, Rainbow Effect (EQ).

Row 4 - Special Functions: White color (0), Special Effect 1 (100+), Default Reset (200+).

Row 5 - Primary Colors: Red (1), Green (2), Blue (3).

Row 6 - Secondary Colors: Yellow (4), Cyan (5), Magenta (6).

Row 7 - Tertiary Colors: Orange (7), Purple (8), Pink (9).

Control Functions Descriptions:

togglePower(): Switches the LEDs on/off while maintaining memory of the last state.

adjustBrightness(): Steps the brightness up or down by 25 units (range 10–255).


changeMode(): Cycles through 11 different animation modes (Flash, Strobe, Rainbow, etc.).

startupAnimation(): Runs a quick rainbow cycle and white flash to confirm the system is ready upon boot.
