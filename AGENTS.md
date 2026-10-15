# CANgaroo

**CANgaroo** is an open-source CAN bus analyzer and trace tool for Linux and Windows,
written in C++20 on top of **Qt 6** (Widgets). It decodes CAN / CAN-FD / LIN traffic in
real time using DBC and LDF databases, visualizes signals, records and replays traces,
and embeds a Python interpreter (via pybind11) for scripting. This repository is a
community fork of Hubert Denkmair's original cangaroo (further developed by Schildkroet
and others); the code is licensed under the **GPL v2 or later** — every source file
carries the GPL header found in `gpl-header.txt`.

Current version: `0.7.1` (defined in `src/src.pro` via `VERSION = x.y.z`; CI extracts it
from there, not from git tags).

## Repository layout

```
cangaroo.pro      — top-level qmake project (subdirs: src; tests opt-in via CONFIG+=tests)
src/              — the application (qmake entry point: src/src.pro)
  core/           — Backend, BusTrace, BusMessage, Log, MeasurementSetup, PythonEngine,
                    TraceRecorder, TraceFileWriter, TraceLineFormat, PcapNgFormat,
                    ThemeManager, AutosarE2E, ...
  db/             — description databases (wired by db/db.pri):
                      model/ (CanDb*/Lin* object graph), dbc/ (DBC parser),
                      ldf/ (LIN Description File parser, header-only)
  driver/         — BusInterface, BusListener, CanDriver + one subdirectory per driver
                    (SocketCan, SLCAN, GrIP, CANBlaster, CandleApi, LindeApi, Aiode,
                    UsbVendorInterface, PeakCan, Kvaser, Vector, TinyCan, ZsCanFd)
  decoders/       — protocol decoders (UDS, J1939)
  window/         — UI windows, one self-contained directory + .pri each
                    (TraceWindow, SetupDialog, TxGeneratorWindow, ReplayWindow,
                    GatewayWindow, LinControlWindow, GpioControlWindow, GraphWindow,
                    ScriptWindow, SettingsDialog, ...), all included via window/window.pri
  helpers/        — utility code
  translations/   — Qt .ts files (German, Spanish, Chinese)
  mainwindow.*    — application shell, driver registration, menu actions
tests/            — Qt Test unit tests (opt-in build, see below)
examples/         — ready-to-use Python scripting examples + demo.dbc / demo.ldf
firmware/         — STM32G4 reference adapter firmware (gs_usb / lin_usb / aio_usb device
                    side) + libusb SampleApp; standalone STM32CubeIDE project,
                    NOT part of the qmake build
docs/             — manual.tex, usb_interfaces.md (USB wire protocols), view.png
scripts/          — setup_can.sh / setup_vcan.sh helpers, qmake_python_config.py
                    (queries Python paths for the Windows qmake build)
packaging/        — polkit SocketCAN rules consumed by the CI .deb job
canifconfig/      — small standalone C helper tool (own .pro file)
build/            — out-of-source build artifacts (moc/o/rcc/ui); bin/ holds the binary
```

## Technology stack and dependencies

- **C++20, Qt 6** — modules: `core gui widgets xml charts serialport serialbus svg`
  (the top-level `cangaroo.pro` additionally requires `charts`).
- **Python 3 + pybind11** — embedded interpreter for the scripting engine
  (`core/PythonEngine`). On Unix qmake uses `pkg-config python3-embed`; on Windows
  `scripts/qmake_python_config.py` queries the installed interpreter.
- **libnl-3 / libnl-route-3** — Linux only, for SocketCAN interface configuration.
- **libusb-1.0** via pkg-config on Linux/macOS (LindeAPI and aiode drivers are always
  built); Windows uses WinUSB instead.
- Linux dependencies (Ubuntu/Debian): `build-essential qt6-base-dev qt6-charts-dev
  qt6-serialport-dev qt6-serialbus-dev qt6-svg-dev qt6-tools-dev qt6-l10n-tools
  libqt6opengl6-dev libnl-3-dev libnl-route-3-dev libusb-1.0-0-dev python3-dev
  pybind11-dev pkg-config` (see README.md for Fedora/Arch equivalents).

## Build

Build system is **qmake** (`.pro` / `.pri` files) — there is no CMake build; the CI
workflow file is only *named* `cmake.yml`.

```bash
# Linux / from repo root
qmake6 && make -j$(nproc)          # binary lands in bin/cangaroo
# or from src/
cd src && qmake6 && make -j$(nproc)
```

On Windows: install Qt 6 (with the Serial Bus component) and Python 3 + pybind11
(`pip install pybind11`), then open `cangaroo.pro` in Qt Creator. Deploy with
`windeployqt --release cangaroo.exe`.

### Optional driver flags

```bash
qmake6 CONFIG+=kvaser CONFIG+=peakcan
```

- `CONFIG+=peakcan` — PEAK PCAN driver (Windows only; needs the PCAN-Basic SDK extracted
  to `src/driver/PeakCanDriver/pcan-basic-api/`, and `PCANBasic.dll` next to the .exe).
- `CONFIG+=kvaser` — Kvaser driver (needs the CANlib SDK / linuxcan; on Windows also set
  `CANLIB_DIR=<path to SDK>` and ship `canlib32.dll`).
- `CONFIG+=zscanfd` — Zilogic USB-to-CAN-FD driver (Windows only; needs `zscanfd.dll`
  and the `qtzscanfdbus.dll` Qt canbus plugin at runtime; download links are in
  `src/src.pro`).

Vector and TinyCAN are always built and go through Qt SerialBus plugins; they need the
respective vendor runtime (Vector XL Driver Library, TinyCAN library) on the target
machine. Candlelight/CANable (gs_usb), SLCAN, CANBlaster (UDP), GrIP, lin_usb and
aio_usb are always built.

## Runtime architecture

### Driver pattern

- `CanDriver` — discovers and owns `BusInterface` instances.
- `BusInterface` (QObject) — abstract send/receive interface; LIN-capable interfaces
  implement `sendLinSleepWakeup()` and advertise `capability_lin_master/slave/monitor`
  flags.
- `BusListener` — runs in a dedicated QThread, calls `readMessage()` in a loop and feeds
  `BusTrace`.
- Driver constructors use `reinterpret_cast<CanDriver*>(driver)`.
- TX messages: appended to a mutex-protected `_txMsgList` in `sendMessage()`, dequeued
  in `readMessage()`.
- Vendor USB interfaces of the composite `1d50:606f` adapter (lin_usb, aio_usb) go
  through `driver/UsbVendorInterface`: libusb on Linux/macOS, WinUSB by
  `DeviceInterfaceGUID` on Windows. Never `libusb_open()` it on Windows: that opens
  every WinUSB interface and collides with `CandleApiDriver` on gs_usb interface 0
  (see `docs/usb_interfaces.md`).
- The lin_usb / aio_usb wire-protocol headers exist twice, byte-for-byte identical:
  `src/driver/{LindeApiDriver/lin_usb_protocol.h, AiodeDriver/aio_usb_protocol.h}` and
  `firmware/STM32G4_TinyUSB_CanLinAio/SampleApp/*_protocol.h`. Edit one, copy it over
  the other — they have silently diverged before, and a divergence here is a
  host/device mismatch.
- Qt SerialBus plugins (Vector, TinyCAN): check
  `QCanBus::instance()->plugins().contains()` before use.
- Drivers with an enable/disable toggle (CANBlaster, TinyCAN): follow the settings
  pattern in `mainwindow.cpp`.

### Key classes

- `Backend` — singleton, owns `BusTrace`, measurement setup, drivers.
- `BusTrace` — thread-safe message store; saves to candump/ASC/MDF4/PCAP/PCAPng via
  `save(QFile&, TraceFileFormat)`. The format mapping lives in `core/TraceFileFormat.h`
  and is shared by the GUI save dialog and the Python `save_trace()` binding.
- `TraceRecorder` — owned by `Backend`; streams every frame to ASC/candump/PCAPng files
  while armed (Measurement → Record, Ctrl+R), independent of `BusTrace`'s in-memory
  size limit. Fed from `BusTrace::enqueueMessage` (listener threads, queue only); writes
  on the main thread in batches. Encoding is shared with the one-shot export via
  `core/TraceLineFormat` and `core/PcapNgFormat`. Takes an interface-name function
  instead of `Backend`, so it is unit-testable.
- `TraceFileWriter` — Backend-free writers for whole export files (candump, ASC, MDF4,
  PCAP, PCAPng); `BusTrace::save()` delegates here. `core/SocketCan.h` maps a
  `BusMessage` to `canid_t` and error classes (`linux/can/error.h`) for every
  binary/candump format.
- `CanMessage` — value type for a single CAN frame (registered as Qt metatype).
- `BusMessage` — unified CAN/LIN message type; `BusMessage::Type` distinguishes
  `CAN` vs `LIN`.

### Adding a new driver

1. Create `src/driver/NewDriver/` with `NewDriver.h/.cpp`, `NewInterface.h/.cpp`,
   `NewDriver.pri`.
2. In the `.pri`: list HEADERS/SOURCES, add `QT += serialbus` if using a Qt plugin.
3. Include the `.pri` from `src/src.pro` (use a `win32:` or `unix:` prefix if
   platform-specific).
4. Register the driver in `mainwindow.cpp` (optionally with a settings-based
   enable/disable toggle).
5. TX reporting: add `QMutex _txMutex` + `QList<CanMessage> _txMsgList` to the
   interface.

### Theming

- `ThemeManager` (`core/`) applies the app's Light/Dark palette + QSS
  (`assets/{light,dark}_theme.qss`).
- Setting `ui/nativeStyling` (bool, default **true**, toggled via Settings → Appearance)
  makes `ThemeManager::applyTheme()` skip the custom palette/QSS and defer to the active
  `QStyle` + desktop platform theme — for a native GNOME/Adwaita look. The non-native
  path keeps the bundled Fusion-style Light/Dark theme.
- Start/Stop accent buttons are styled per-widget
  (`MainWindow::applyControlButtonStyles`), not via the global QSS, so the accent
  survives native mode.
- Toolbar/menu icons use `QIcon::fromTheme(name, fallback)`
  (`MainWindow::applyActionIcons`) to match the system icon theme.
- On GNOME/Wayland, `main.cpp` sets `QT_QPA_PLATFORMTHEME=gnome` and
  `QT_WAYLAND_DECORATION=adwaita` when unset. The full native look needs a Qt6 Adwaita
  style, the `qadwaitadecorations` plugin, and a GNOME Qt platform theme installed.
- `MainWindow` follows live desktop Light/Dark switches via
  `QStyleHints::colorSchemeChanged` (Qt 6.5+).

## Testing

```bash
cd tests && qmake6 && make -j$(nproc) && make check
```

Needs Qt Test (`qt6-base-dev`). Also reachable from the top level via
`qmake6 CONFIG+=tests` — opt-in so the normal build has no testlib dependency.
Run by the `unit-tests` CI job. Binaries land in `build/tests/`.

One binary per area, each linking only the sources it exercises (never `Backend`,
which would drag in drivers and the Python engine). `tests/common.pri` holds the shared
config; `tests/support/LogStub.cpp` satisfies `core/Log` without `Backend`.

| Directory | Covers |
|-----------|--------|
| `bus_message_signal/` | signal packing: `extractRawSignal` / `injectRawSignal` |
| `bus_message_frame/`  | DLC, identifier/flag packing, accessor bounds |
| `can_db_signal/`      | raw ↔ physical, sign extension, multiplexer gate |
| `dbc_parser/`         | DBC text to `CanDb`, incl. Motorola start-bit conversion |
| `can_db/`             | `CanDb::updateFrom` (DBC reload), muxer wiring |
| `trace_file_format/`  | export format resolution from extension / name |
| `trace_line_format/`  | ASC / candump lines, CAN FD DLC table, ASC CAN FD parsing (vectors from python-can) |
| `pcapng_format/`      | pcapng SHB / IDB / EPB bytes for SocketCAN frames (from the pcapng spec + `linux/can.h`) |
| `trace_file_writer/`  | whole export files in every format (`TraceFileWriter`), incl. MDF4 block structure |
| `trace_recorder/`     | `TraceRecorder` arming, naming, splitting, per-part file structure |
| `decoders/`           | UDS and J1939 |
| `autosar_e2e/`        | E2E Profile 2 CRC-8H2F |
| `ldf_parser/`         | LIN Description File parsing |
| `slcan_codec/`        | SLCAN ASCII frame decoding, incl. malformed input |

**Expected values come from outside cangaroo.** The big-endian bug in issue #34 survived
because extract and inject were wrong in mutually cancelling ways, so round-trip
assertions written against cangaroo alone passed. DBC signal vectors are generated with
**cantools**; the E2E CRC is anchored on the AUTOSAR spec check value plus a bitwise
reimplementation. Never generate vectors from cangaroo output.

**Trace file compatibility** is checked against independent readers by
`tests/format_compat/` (CI job `format-compat`, not part of `make check`):
`trace_format_samples` writes every export format plus split recorder output,
`validate.py` reads them with python-can (candump, ASC), scapy (pcap, pcapng) and
asammdf (MDF4). Frames are defined in both files on purpose; reader warnings fail.

```bash
cd tests/format_compat && qmake6 && make && cd ../..
python3 -m venv .venv && .venv/bin/pip install -r tests/format_compat/requirements.txt
build/tests/trace_format_samples build/format_compat
.venv/bin/python tests/format_compat/validate.py build/format_compat
```

Test-writing pitfalls:

- Don't use C++14 digit separators (`1'000`) in test `.cpp` files with `Q_OBJECT`:
  qmake's moc scanner reads `'` as a character literal, misses `Q_OBJECT`, and the build
  fails with a missing `<Name>.moc`.
- Known-bad behaviour is marked with `QEXPECT_FAIL` plus a comment explaining the fix,
  never asserted as if correct. When the bug is fixed the test reports "expected failure
  but passed" — drop the marker and assert the real behaviour.

## Code conventions

- Modern C++ (C++20 and above):
  - `auto` and structured bindings where they improve readability
  - range-based for loops, `std::as_const()` for const iteration
  - `constexpr` and `if constexpr` where applicable
  - smart pointers (`std::unique_ptr`, `std::shared_ptr`) over raw owning pointers
  - `std::chrono` for all time-related code
  - concepts and constraints for template interfaces
  - designated initializers for aggregate types
  - `std::optional`, `std::variant`, `std::string_view` where appropriate
  - `[[nodiscard]]`, `[[maybe_unused]]` attributes
  - `noexcept` on move constructors, destructors, and non-throwing functions
  - scoped enums (`enum class`) over unscoped enums
- Allman brace style (opening brace on its own line).
- Qt6 API only (no deprecated Qt5 patterns):
  - `override` instead of `Q_DECL_OVERRIDE`
  - `QString::arg()` / `QString::number()` instead of `QString::asprintf()`
  - `static_cast` / `reinterpret_cast` instead of C-style casts
  - `QProcess::start()` + `waitForFinished()` instead of `QProcess::execute()`
- Include order: own header, C++ standard library, Qt, project, platform. Separate
  include groups with blank lines.
- New source files get the GPL header from `gpl-header.txt`.
- Comments and documentation are written in English.

## CI and deployment

GitHub Actions workflow: `.github/workflows/cmake.yml` (despite the name, it drives
qmake builds). Jobs:

- `unit-tests` — Ubuntu, builds and runs the Qt Test suite (`make check`).
- `format-compat` — trace files validated against python-can / scapy / asammdf.
- `build-ubuntu` — builds a `.deb` package (consumes `packaging/`).
- `build-appimage` — builds an AppImage via linuxdeploy (bundles Wayland platform
  plugin and libpython).
- `build-windows` — MSYS2/MinGW + Qt6, downloads the PCAN-Basic and Kvaser SDKs, deploys
  with `windeployqt6`, bundles MinGW runtime DLLs, the Python runtime and the example
  scripts.

## Security considerations

- CANgaroo runs `ip link` (via `QProcess`) to configure SocketCAN interfaces
  (bitrate, sample point, CAN FD), which requires `CAP_NET_ADMIN`. The README documents
  a targeted sudoers rule (NOPASSWD only for the specific `ip link set` invocations) or
  `setcap cap_net_admin+ep /sbin/ip`. If the interface is set to *"Configured by OS"*,
  no elevated privileges are needed. When touching this code, never broaden the
  privilege surface beyond those exact commands.
- USB devices accessed via libusb (gs_usb / Candlelight, lin_usb, aio_usb) rely on the
  udev rules documented in README.md; the application itself never requests root.
- The embedded Python engine executes user scripts with the full privileges of the
  application — treat any code that feeds scripts or exposes new bindings accordingly.
- Vendor SDK blobs (`PCANBasic.dll`, `canlib32.dll`, `zscanfd.dll`, Vector XL library)
  are proprietary and loaded at runtime; they must never be committed to the repository.
- Driver code talks directly to hardware and parses wire protocols (SLCAN ASCII, gs_usb,
  lin_usb/aio_usb vendor requests); validate lengths and bounds on everything coming off
  a device — see the malformed-input cases in `tests/slcan_codec/`.

## Reference material

- `docs/usb_interfaces.md` — wire protocols for the gs_usb / lin_usb / aio_usb USB
  interfaces (host side) and the composite `1d50:606f` adapter.
- `firmware/STM32G4_TinyUSB_CanLinAio/` — reference device-side firmware implementing
  those protocols (STM32CubeIDE project, TinyUSB), with a libusb `SampleApp/` that
  exercises every request.
- `docs/manual.tex` — user manual; `docs/can_signal_byteorder.ods` — signal byte-order
  reference; `examples/` — Python scripting examples (send/receive, DBC decode, LIN,
  measurement control, `save_trace`).
