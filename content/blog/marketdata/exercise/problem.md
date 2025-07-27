---
title: "Market Data Parser"
author: "Pralay Patoria"
draft: true
---

**Question:**  
You're designing a high-performance market data parser for the NYSE Pillar feed, which consists of structured binary messages. The feed contains different message types such as Quotes (MsgType 140), Trades (MsgType 220), and Stock Summary messages (MsgType 223). Each message starts with a common header and follows a specific binary format.

### **Task 1: Parsing Market Data Stream**  
Given the structured C++ definitions for the Quote, Trade, and Stock Summary messages, implement a parser that reads a binary data stream and extracts valid messages.

#### **Binary Message Header Format (8 bytes total)**
| Field          | Size (bytes) | Description                                              |
|----------------|--------------|----------------------------------------------------------|
| `MsgSize`      | 2            | Size of the entire message (including header)            |
| `MsgType`      | 2            | Type of the message (e.g., 140 for Quote, 220 for Trade) |
| `SourceTimeNS` | 4            | Nanosecond timestamp                                     |

#### **Quote Message (MsgType 140)**
| Field          | Size (bytes) | Description                        |
|----------------|--------------|------------------------------------|
| `SymbolIndex`  | 4            | Unique symbol identifier           |
| `SymbolSeqNum` | 4            | Sequence number for symbol updates |
| `AskPrice`     | 8            | Best ask price (scaled in cents)   |
| `AskVolume`    | 4            | Number of shares available at ask  |
| `BidPrice`     | 8            | Best bid price (scaled in cents)   |
| `BidVolume`    | 4            | Number of shares available at bid  |

#### **Trade Message (MsgType 220)**
| Field          | Size (bytes) | Description                   |
|----------------|--------------|-------------------------------|
| `SymbolIndex`  | 4            | Unique symbol identifier      |
| `SymbolSeqNum` | 4            | Sequence number               |
| `TradeID`      | 4            | Unique trade ID               |
| `Price`        | 8            | Trade price (scaled in cents) |
| `Volume`       | 4            | Shares traded                 |

#### **Stock Summary Message (MsgType 223)**
| Field         | Size (bytes) | Description                 |
|---------------|--------------|-----------------------------|
| `SymbolIndex` | 4            | Unique symbol identifier    |
| `HighPrice`   | 8            | Highest price in the period |
| `LowPrice`    | 8            | Lowest price in the period  |
| `Open`        | 8            | Opening price               |
| `Close`       | 8            | Closing price               |
| `TotalVolume` | 8            | Total volume traded         |

---

### **Task 2: Handling a Partial Message**  
Below is a **sample binary stream (Little Endian)** containing multiple messages, including a **partial message at the end**.

#### **Example Stream with a Partial Message**
```
90 00 8C 00 34 12 56 78 AB CD EF 01 00 00 00 02 00 00 00 50 00 00 00 00 00 00 00
32 00 00 00 40 00 00 00 00 00 00 00 28 00 00 00 01 00
88 00 DC 00 78 56 34 12 12 34 56 78 01 00 00 00 02 00 00 00 05 00 00 00 A0 86 01 00 00 00 00 00 64 00 00 00 02 00 00 00 01 00 00 00 03 00
AA 00 DF 00 90 78 34 12 02 00 00 00 06 00 00 00 50 46 02 00 00 00 00 00 C8 00 00 00 01 00 00
```
- The **last message** (`AA 00 DF 00 ...`) is incomplete.

```c++
#include <vector>
#include <cstdint>

std::vector<uint8_t> partial_binary_stream = {
    0x90, 0x00, 0x8C, 0x00, 0x34, 0x12, 0x56, 0x78, 0xAB, 0xCD, 0xEF, 0x01, 
    0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x88, 0x00, 0xDC, 0x00, 0x78, 0x56, 0x34, 0x12, 0x12, 0x34, 0x56, 0x78,
    0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
    0xA0, 0x86, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00,
    0xAA, 0x00, 0xDF, 0x00, 0x90, 0x78, 0x34, 0x12, 0x02, 0x00, 0x00, 0x00,
    0x06, 0x00, 0x00, 0x00, 0x50, 0x46, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xC8, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00
};
```


---

### **Task 3: Processing the Completed Stream**  
Below is the **continuation of the binary stream**, which completes the partial message from above and adds new messages.

#### **Completed Binary Stream**
```
00 00 20 00 00 00 75 30 00 00 21 00 00 00 30 00 00 00 60 00 00 00 50 00 00 00 80 00 00 00
92 00 8C 00 56 34 12 78 AB CD EF 01 00 00 00 02 00 00 00 45 00 00 00 00 00 00 00
35 00 00 00 42 00 00 00 00...
```
```c++
#include <vector>
#include <cstdint>

std::vector<uint8_t> complete_binary_stream = {
    0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x75, 0x30, 0x00, 0x00, 0x21, 0x00,
    0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x50, 0x00,
    0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
    0x92, 0x00, 0x8C, 0x00, 0x56, 0x34, 0x12, 0x78, 0xAB, 0xCD, 0xEF, 0x01,
    0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x45, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x35, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x00
    // Add the rest if needed...
};
```
---

### **Requirements**
1. **Write a C++ function** to:
   - Parse the binary stream, extract complete messages, and store them in appropriate data structures.
   - Detect and handle **partial messages** (store and process them when the remaining bytes arrive).
   - Print each parsed message in a human-readable format.
   
2. **Handle corner cases:**
   - Streams ending with a partial message.
   - Messages with different `MsgType` values.
   - Unexpected message sizes.

3. **Optimization Considerations:**
   - The feed is high-frequency. How would you optimize for low-latency parsing?
   - Would you use memory pooling or other techniques to improve efficiency?

---

### **Bonus Questions**
- How would you modify the parser to support **multiple feed handlers** (e.g., multiple TCP streams)?
- If the messages were **compressed** (e.g., using LZ4), how would you modify your approach?
- Can you propose an **alternative design** using zero-copy parsing (e.g., mmap or direct buffer access)?

---

### **Expected Output (Example)**
After processing the binary stream, the output should look like:
```
[Quote] SymbolIndex: 0x12345678, Ask: 50.00, Bid: 40.00, Volume: 32
[Trade] SymbolIndex: 0x56781234, Price: 100.50, Volume: 64, TradeID: 5
[StockSummary] SymbolIndex: 0x78341290, High: 200.00, Low: 150.00, Close: 180.00
```

---

**How would you implement this?** 
