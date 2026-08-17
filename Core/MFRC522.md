# MFRC522 lib breakdown
---
**<small>(values a...z are unused parameters)</small>**

## Public
| function | param 1 | param 2 | param 3 | param 4 | param 5 | desc. |
|:---|:---|:---|:---|:---|:---|:---|
| begin | b | c | d | e | f | [jmp2H]("begin()") |
| reset | b | c | d | e | f | [jmp2H]("reset()") |
| getVersion | b | c | d | e | f | [jmp2H]("getVersion()") |
| setAntennaOn | b | c | d | e | f | [jmp2H]("setAntennaOn()") |
| setAntennaOff | b | c | d | e | f | [jmp2H]("setAntennaOff()") |
| transceive | b | c | d | e | f | [jmp2H]("transceive()") |

----
## Private
| function | param 1 | param 2 | param 3 | param 4 | param 5 | desc. |
|:---|:---|:---|:---|:---|:---|:---|
| initRegisters | b | c | d | e | f | g |
| checkVersion | b | c | d | e | f | g |
| clearFIFO | b | c | d | e | f | g |
| clearComIrq | b | c | d | e | f | g |
| writeToFIFO | b | c | d | e | f | g |
| readFROMFIFO | b | c | d | e | f | g |
| waitForCompletion | b | c | d | e | f | g |
| checkForErrors | b | c | d | e | f | g |
| setBiskMask | b | c | d | e | f | g |
| clearBitMask | b | c | d | e | f | g |

---

## begin()

Resets MFRC5222 init registers to recommended presets then checks the version to determine how to communicate with the RFID card. **Returns bool**

---

## Reset()


---

## 