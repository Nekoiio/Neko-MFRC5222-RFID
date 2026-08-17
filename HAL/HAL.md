# HAL (Hardware Abstraction Layer)

## Purpose

The HAL layer separates hardware-specific communication from the MFRC522 driver logic.

The MFRC522 driver does not directly communicate with Arduino SPI hardware.
Instead, it communicates through the SPIBus interface.

This allows the library to support different platforms:

- Arduino
- ESP32
- STM32
- Raspberry Pi
- Other SPI controllers

---

# SPIBus

## Description

Abstract interface representing an SPI communication bus.

The MFRC522 requires SPI communication to send register addresses,
commands, and receive responses.

---

## Functions

### begin()

### Purpose
Initializes the SPI hardware.

### Input

None.

### Output

None.

---

### beginTransaction()

### Purpose

Starts an SPI transaction.

This normally:

- Configures SPI speed
- Configures SPI mode
- Pulls chip-select LOW

### Input

None.

### Output

None.

---

### endTransaction()

### Purpose

Ends an SPI transaction.

This normally:

- Releases chip-select HIGH
- Ends SPI communication

### Input

None.

### Output

None.

---

### transfer(uint8_t byte)

### Purpose

Transfers a single byte over SPI.

SPI is full duplex, meaning:

Sending one byte also receives one byte.

### Input

| Parameter | Description |
|-|-|
| byte | Byte sent over MOSI |

### Output

Returns the byte received from MISO.

---

### transfer(uint8_t* buffer, size_t length)

### Purpose

Transfers an entire SPI packet.

The same buffer is used for sending and receiving.

Before:
```
Address | Dummy
0xEE    | 0x00
```

After:
```
Unused | Data
0x00   | 0x92
```

### Input

| Parameter | Description |
|-|-|
| buffer | Packet buffer |
| length | Number of bytes |

### Output

Buffer contents are replaced with received data.

---
