# Sofle Color ZMK

Private firmware repository for the G2-r02 schematic review based on G1-r04. Hardware/CAD files and BOM stay in their separate local repository.

ZMK and Zephyr are pinned by full commit. Targets are the original nice!nano v2 electrical interface, with left 5x7 and right 5x6 matrices. Right EC11 is 20 detents/20 pulses with push; matrix diodes are col2row. Right RGB gate requires the documented P1.07 manual connection.

G2 minimal firmware covers keyboard scan/wake configuration, BLE split, default QWERTY, fiveway arrows/Enter, right volume/mute, SPI2/gate declarations and the PWM0 resource on P0.20. The encoder A/B inputs use internal pullups with no external filter. It intentionally keeps the LCD and RGB rails off and explicitly drives their signal pins LOW. PWM0 is configured for a1250ns resource probe but never started; this does not validate the mixed LED chain timing. It is not the complete DH06 RGB3-bit/VCOM or PWM0/EasyDMA LED driver. No display, LED voltage, sleep-current, BLE reconnect or physical wake behavior is claimed as tested by a successful compile.

Production firmware must keep the parent VCC on while either child needs power, implement independent LCD/RGB shutdown, complete DMA before dropping signals/power, maintain 500ms VCOM while the DH06 is powered (including HOLD), clear/redraw after OFF and test signal backfeed. Do not enable the stock underglow external-power coupling. The selected LED3.3V route remains unqualified.

The default keymap is provisional for G2; complete keymap review and real hardware acceptance remain later gates. Actions produces left/right minimal UF2 files and settings reset.

## Source

Sofle row/column and EC11 conventions follow the MIT-licensed ZMK Sofle shield. ZMK is MIT licensed. This repository's new files are MIT licensed.
