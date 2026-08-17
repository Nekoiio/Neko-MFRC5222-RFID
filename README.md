# MFRC522 RFID Library

**A layered, specification-driven C++ driver for RFID/NFC hardware — built from the datasheet up, not from an existing library.**

`C++` · `Embedded Systems` · `Protocol Implementation` · `Arduino (portable HAL)` · `Active Development`

---

## What This Project Demonstrates

- **Layered system architecture** — hardware access, protocol logic, and application concerns are fully decoupled. Each layer depends only on the interface below it, never on a concrete implementation.
- **Specification-driven implementation** — built directly from the MFRC522 datasheet and the ISO14443A / MIFARE protocol documentation, rather than adapted from an existing library.
- **Protocol and state-machine design** — the MIFARE card lifecycle (request → anti-collision → selection → authentication → read/write) is implemented as an explicit, ordered sequence with defined transitions and failure states.
- **Interface-driven, testable design** — abstract interfaces and dependency injection (e.g. `SPIBus`) allow each layer to be tested and swapped independently of the others.
- **Data integrity as a first-class concern** — UID/BCC validation and CRC checks are treated as required steps, not afterthoughts.
- **Documentation as part of the build process** — architecture, rationale, and roadmap are written and maintained alongside the code, not reconstructed after the fact.

---

## Overview

This is a ground-up, layered C++ implementation of the **MFRC522 RFID/NFC reader**, built to understand and recreate its full communication stack — from raw SPI transactions up through MIFARE Classic protocol logic — rather than wrapping an existing library.

The project is deliberately built with portability in mind. Hardware-specific code is isolated behind abstraction layers so the core MFRC522 and MIFARE protocol logic can eventually run on platforms beyond Arduino without being rewritten.

## Engineering Approach

Most RFID projects reach for an existing library and call `read()`. This project avoids that shortcut on purpose. Every layer — SPI transport, register access, device driver, protocol logic — is implemented independently against the datasheet and the ISO14443A specification, and verified before the next layer is built on top of it.

That constraint mirrors the discipline needed on larger, integration-heavy systems: correctly interpreting a technical specification, isolating volatile implementation details (a specific chip, platform, or backend) behind a stable interface, and building components that can be tested, replaced, or extended without destabilizing everything built on top of them.

## Project Goals

- Recreate MFRC522 communication from the datasheet.
- Understand the SPI communication protocol at the register level.
- Separate hardware-specific code from device and protocol logic.
- Implement MIFARE Classic communication without relying on an existing RFID library.
- Build a clean public-facing API on top of the lower-level implementation.
- Make the library portable to other microcontrollers and platforms.

---

# Architecture

```text
Application
     │
     ▼
Public API                    Planned
     │
     ▼
MIFARE Protocol Layer         In Progress
     │
     ▼
MFRC522 Driver                Completed / In Progress
     │
     ▼
Register Access Layer         Completed
     │
     ▼
SPI Hardware Abstraction      Completed
     │
     ▼
Platform Implementation       Arduino SPI
```

The project is intentionally built from the bottom up: each layer is fully working and independently tested before the next is started.

---

# Completed

## 1. SPI Hardware Abstraction

An abstract `SPIBus` interface so the MFRC522 driver has no direct dependency on Arduino's SPI implementation.

**Responsibilities**
- Initialize the SPI bus.
- Begin and end SPI transactions.
- Control chip select.
- Transfer bytes over SPI.

**Arduino implementation.** `ArduinoSPIBus` provides the platform-specific implementation using `SPI.begin()`, `SPI.beginTransaction()`, `SPI.transfer()`, `SPI.endTransaction()`, and Arduino GPIO functions for chip select. The MFRC522 driver itself never touches the Arduino SPI library directly — it only knows about `SPIBus`.

---

## 2. MFRC522 Register Access

`RegisterAccess` abstracts register communication away from the higher-level MFRC522 driver.

**Implemented**
- Register writes.
- Single register reads.
- Multi-byte register reads.
- MFRC522 SPI address encoding.

**Write address transformation**
```cpp
(reg << 1) & 0x7E
```

**Read address transformation**
```cpp
((reg << 1) & 0x7E) | 0x80
```

This layer owns the MFRC522-specific SPI addressing rules so the rest of the driver works with register enums instead of raw addresses.

---

## 3. Register Definitions

Registers are represented as an enum rather than scattered raw hex addresses. This makes:

```cpp
registers.readRegister(Register::Version);
```

possible instead of:

```cpp
readRegister(0x37);
```

The enum also serves as a single, central reference for the register map, instead of that documentation living only in comments or the datasheet.

---

## 4. MFRC522 Core Driver

The initial MFRC522 device driver.

**Implemented**
- Driver construction through dependency injection.
- Device initialization.
- Version register detection.
- Software reset.
- Register initialization.
- Antenna control.
- FIFO management.
- Interrupt handling.
- Bit-mask register manipulation.
- Basic transceive operation.

**Initialization sequence**
1. Check whether the device is already initialized.
2. Perform a software reset.
3. Configure required registers.
4. Enable the antenna.
5. Read the Version register.
6. Confirm communication with the MFRC522.

---

## 5. Antenna Control

Software control of the RF antenna through `TxControlReg`, using read-modify-write so unrelated bits in the register are never disturbed.

- **Antenna ON** — the two least-significant bits of `TxControlReg` are set.
- **Antenna OFF** — those bits are cleared, leaving the rest of the register untouched.

---

## 6. FIFO Management

Helpers for the MFRC522's FIFO buffer:

```cpp
clearFIFO()
writeToFIFO()
readFromFIFO()
```

The MFRC522 automatically advances its FIFO pointer, so sequential transfers through `FIFODataReg` work without manual offset tracking.

---

## 7. Interrupt Handling

Polling of `ComIrqReg`, currently monitoring the timer interrupt, error interrupt, and receive/transmit completion interrupts.

This uses polling rather than the physical MFRC522 IRQ pin for now, which keeps the core driver platform-independent — the IRQ pin is hardware-specific wiring, polling isn't.

---

## 8. Transceive Layer

The core MFRC522 transceive sequence:

```text
Idle
  │
  ▼
Clear FIFO
  │
  ▼
Clear interrupts
  │
  ▼
Write data to FIFO
  │
  ▼
Start Transceive command
  │
  ▼
Configure BitFramingReg
  │
  ▼
Wait for completion
  │
  ├── Timeout/Error
  │
  └── Success
         │
         ▼
      Read FIFO
```

The transceive function also supports specifying the number of valid bits in the final transmitted byte — required for commands like REQA and WUPA, which use **7 valid bits** rather than a full byte.

---

# MIFARE Protocol Layer

Sits above the MFRC522 device driver. Its job is to translate MIFARE protocol operations into MFRC522 transceive operations.

## Currently Implemented

**REQA** — MIFARE Request command (`0x26`), 7 valid bits, expects a 2-byte ATQA response.

**WUPA** — MIFARE Wake-Up command (`0x52`), also 7 valid bits, also expects an ATQA response.

## Request Abstraction

REQA and WUPA share the same request/response procedure, so both call a shared `sendRequest()` with the appropriate command instead of duplicating the transaction logic:

```text
REQA()  ──► sendRequest(REQA)
WUPA()  ──► sendRequest(WUPA)
```

This pattern will be reused anywhere multiple MIFARE commands share the same transaction shape.

---

# MIFARE Card Communication Flow

The planned MIFARE Classic communication sequence:

```text
REQA / WUPA
      │
      ▼
Anti-Collision
      │
      ▼
UID
      │
      ▼
Select
      │
      ▼
SAK
      │
      ▼
Authenticate Sector
      │
      ▼
Read / Write Block
```

**Important distinction:** REQA/WUPA do **not** return the UID — the UID is only obtained during anti-collision, then reused for card selection and later for MIFARE Classic authentication.

This is effectively a state machine: each stage has a defined precondition, an expected response, and explicit failure/timeout handling before the next stage is allowed to run.

---

# Planned MIFARE Features

**Anti-Collision** — implement the ISO14443A anti-collision procedure: detect UID length, run cascade levels when required, retrieve UID bytes, validate the BCC, and store the UID internally. Expected first cascade-level command: `93 20`.

**Card Selection** — send UID + BCC, calculate/send CRC_A, receive SAK, determine card characteristics, and handle UID cascade levels.

**Authentication** — MIFARE Classic authentication supporting Key A, Key B, sector/block selection, and authentication state tracking. This will reuse the UID obtained during detection/selection rather than requiring it to be passed manually for every call.

**Block Reading** — the MIFARE Classic READ command (`30 <block>`), expecting 16 bytes back, with response validation before the data is exposed through the MIFARE layer.

**Block Writing** — the MIFARE Classic WRITE command (`A0 <block>`) followed by the 16-byte payload, handling the full MFRC522 transceive sequence the write operation requires.

---

# Planned Public API

The current classes are intentionally low-level. A final public-facing API will sit on top of the internal layers so consumers don't need to understand MFRC522 registers, SPI address encoding, FIFO management, IRQ handling, `BitFramingReg`, MIFARE command framing, anti-collision details, or authentication internals to use the library.

Target usage:

```cpp
RFID reader;

reader.begin();

if (reader.detectCard())
{
    reader.authenticate(...);

    reader.readBlock(...);
}
```

instead of requiring callers to construct low-level transceive operations by hand.

# Planned Architecture

```text
┌───────────────────────────────┐
│         Public API            │
│                               │
│ detectCard()                  │
│ getUID()                      │
│ authenticate()                │
│ readBlock()                   │
│ writeBlock()                  │
└───────────────┬───────────────┘
                │
┌───────────────▼───────────────┐
│       MIFARE Protocol         │
│                               │
│ REQA / WUPA                   │
│ Anti-Collision                │
│ Select                        │
│ Authentication                │
│ Read / Write                  │
└───────────────┬───────────────┘
                │
┌───────────────▼───────────────┐
│        MFRC522 Driver         │
│                               │
│ Commands                      │
│ FIFO                          │
│ IRQ                           │
│ Antenna                       │
│ Transceive                    │
└───────────────┬───────────────┘
                │
┌───────────────▼───────────────┐
│       Register Access         │
│                               │
│ Register Reads/Writes         │
│ SPI Address Encoding          │
└───────────────┬───────────────┘
                │
┌───────────────▼───────────────┐
│          SPIBus               │
│                               │
│ Platform-independent HAL      │
└───────────────┬───────────────┘
                │
                ▼
        Arduino SPI / Other
        Platform Implementations
```

---

# Testing

Currently tested on **Arduino Uno + MFRC522** hardware, covering:

- SPI initialization.
- MFRC522 Version register reads.
- Register read/write verification.
- MFRC522 communication detection.
- Basic transceive functionality.

The register layer was tested independently before the higher-level driver was built on top of it — deliberately, so a failure is always localized to the layer that caused it. It also means a layer (e.g. the SPI HAL) can later be swapped for a new platform without having to re-validate the entire stack above it.

---

# Engineering Principles

**Hardware independence.** The core driver has no direct dependency on Arduino APIs — hardware-specific code lives in the HAL and nowhere else.

**Layer separation.** Each layer has exactly one responsibility:

```text
SPIBus → RegisterAccess → MFRC522 → MIFARE → Public API
```

**Datasheet-driven implementation.** Protocol behavior comes from the MFRC522 documentation and the MIFARE/ISO14443A protocol, not from copying an existing RFID library — so the resulting code reflects an understanding of *why* it works, not just *that* it works.

**Explicit data ownership.** Buffers, lengths, register values, and protocol responses are passed explicitly between layers rather than relying on shared or global state.

**Portability.** Arduino is the first platform implementation, not a requirement baked into the core library — the design goal is that a new platform only ever requires a new `SPIBus` implementation, nothing else.

---

# Roadmap

**Foundation — Complete**
- [x] SPI abstraction + Arduino implementation
- [x] Register definitions + access layer (independently tested)
- [x] MFRC522 reset, initialization, version detection
- [x] Antenna control, FIFO management
- [x] Interrupt polling
- [x] Transceive layer with variable `TxLastBits` support

**MIFARE Protocol — In Progress**
- [x] Request abstraction (REQA / WUPA)
- [ ] Anti-collision + UID handling *(next milestone)*
- [ ] Card selection + SAK parsing
- [ ] MIFARE Classic authentication (Key A / Key B)
- [ ] Block reading / writing
- [ ] Multi-sector handling

**Polish & Platform — Planned**
- [ ] Physical IRQ support
- [ ] Additional platform SPI implementations
- [ ] Error/status refinement
- [ ] Final public-facing API
- [ ] Automated/integration testing
- [ ] Documentation and usage examples
- [ ] Release-ready library structure

---

# Current Status

The project has progressed from raw SPI communication to a functional, layered MFRC522 driver capable of communicating with the chip, accessing registers, configuring the device, managing its FIFO and interrupts, and performing MIFARE request operations.

The next milestone is **anti-collision and card selection**, which will let the library obtain and retain a card's UID — after which the project moves into MIFARE Classic authentication and block-level read/write. The final stage is a public API that hides all internal implementation behind a small, simple interface.
