
# Registers

## Purpose

Contains all MFRC522 register definitions.

Registers are stored as enum values to avoid using raw addresses.

Example:

Instead of:

```cpp
readRegister(0x37);
```
Use
```cpp
readRegister(Register::Version);
```

# **Same for the other enums in this foler**