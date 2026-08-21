# BOOX Remote

Firmware that turns an M5Stack CoreS3 into a Bluetooth page-turn remote for
BOOX readers. It uses PlatformIO with native ESP-IDF; Arduino is not required.

![BOOX Remote interface](docs/ui-preview.png)

This preview is rendered from the same LVGL layout and fonts used by the
device.

The screen has two touch targets:

- **PREV** sends the Left Arrow key.
- **NEXT** sends the Right Arrow key.

Each tap sends one key press. Holding the screen does not repeat it.

## What you need

- M5Stack CoreS3
- USB-C data cable
- A BOOX reader with Bluetooth and an app that supports arrow-key page turns
- [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html)

## Install

Clone the repository, connect the CoreS3, and run:

```sh
pio run --target upload
pio device monitor
```

The first build downloads the ESP-IDF toolchain and M5Stack CoreS3 components.

On the BOOX reader:

1. Open Bluetooth settings and pair with **BOOX Remote**.
2. Open a book in NeoReader.
3. Tap either half of the CoreS3 screen to change pages.

If pairing stops working after reflashing or erasing the device, forget
**BOOX Remote** on the reader and pair it again.

## Display and status

- A filled green dot means Bluetooth is connected; a gray outline means it is
  disconnected or advertising.
- The display dims after 5 minutes and turns off after 30 minutes without a
  tap. Bluetooth remains active.
- A tap wakes the display and turns one page.
- `LOW` appears at 20% battery or below, `CHG` while charging, and `LOW+` when
  both apply.

## Development

Build without flashing:

```sh
pio run
```

The firmware entry point is [`src/main.c`](src/main.c). The shared device and
preview layout is in [`src/ui_screen.c`](src/ui_screen.c). Build settings are in
[`platformio.ini`](platformio.ini), and component versions are declared in
[`src/idf_component.yml`](src/idf_component.yml) and locked in
[`dependencies.lock`](dependencies.lock).

The Bluetooth name is in [`src/config.h`](src/config.h). Brightness, idle
timeouts, and the low-battery threshold are near the top of
[`src/ui.c`](src/ui.c).

After changing the UI, update the README image with:

```sh
sh scripts/render_ui_preview.sh
```

A host-side LVGL build for the same preview lives in
[`test/host`](test/host); it renders the UI without touching the device and
is what the script above runs.

## License

[MIT](LICENSE)
