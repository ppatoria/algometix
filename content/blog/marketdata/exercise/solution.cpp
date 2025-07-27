#include <cstdint>
#include <vector>

#pragma pack(push, 1)

/**
#### **Binary Message Header Format (8 bytes total)**
| Field          | Size (bytes) | Description |
|----------------|--------------|----------------------------------------------------------|
| `MsgSize`      | 2            | Size of the entire message (including header)            |
| `MsgType`      | 2            | Type of the message (e.g., 140 for Quote,220 for Trade)  |
| `SourceTimeNS` | 4            | Nanosecond timestamp                                     |
*/

struct Header {
  uint16_t msgSize;
  uint16_t msgType;
  uint32_t sourceTime;
};

static_assert(sizeof(Header) == (2 + 2 + 4));


/**
#### **Quote Message (MsgType 140)**
| Field          | Size (bytes) | Description                        |
|----------------|--------------|------------------------------------|
| `SymbolIndex`  | 4            | Unique symbol identifier           |
| `SymbolSeqNum` | 4            | Sequence number for symbol updates |
| `AskPrice`     | 8            | Best ask price (scaled in cents)   |
| `AskVolume`    | 4            | Number of shares available at ask  |
| `BidPrice`     | 8            | Best bid price (scaled in cents)   |
| `BidVolume`    | 4            | Number of shares available at bid  |

*/

struct QuoteMessage{
  uint32_t symbolIndex;
  uint32_t symbolSeqNum;
  double askPrice;
  uint32_t askVolume;
  double bidPrice;
  uint32_t bidVolume;
};
static_assert(sizeof(QuoteMessage) == (4+4+8+4+8+4));
/**
#### **Trade Message (MsgType 220)**
| Field          | Size (bytes) | Description                   |
|----------------|--------------|-------------------------------|
| `SymbolIndex`  | 4            | Unique symbol identifier      |
| `SymbolSeqNum` | 4            | Sequence number               |
| `TradeID`      | 4            | Unique trade ID               |
| `Price`        | 8            | Trade price (scaled in cents) |
| `Volume`       | 4            | Shares traded                 |
*/

struct TradeMessage {
  uint32_t symbolIndex;
  uint32_t symbolSeqNum;
  uint32_t tradeID;;
  double price;
  uint32_t volume;
};
static_assert(sizeof(TradeMessage) == (4+4+4+8+4));


/**
#### **Stock Summary Message (MsgType 223)**
| Field         | Size (bytes) | Description                 |
|---------------|--------------|-----------------------------|
| `SymbolIndex` | 4            | Unique symbol identifier    |
| `HighPrice`   | 8            | Highest price in the period |
| `LowPrice`    | 8            | Lowest price in the period  |
| `Open`        | 8            | Opening price               |
| `Close`       | 8            | Closing price               |
| `TotalVolume` | 8            | Total volume traded         |
 */

struct StockSummary{
  uint32_t symbolIndex;
  double highPrice;
  double lowPrice;
  double open;
  double close;
  uint64_t totalVolume;
};
static_assert(sizeof(StockSummary) == (4+8+8+8+8+8));

#pragma pack(pop)

std::vector<uint8_t> partial_binary_stream = {
    0x90, 0x00, 0x8C, 0x00, 0x34, 0x12, 0x56, 0x78, 0xAB, 0xCD, 0xEF, 0x01,
    0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x88, 0x00, 0xDC,
    0x00, 0x78, 0x56, 0x34, 0x12, 0x12, 0x34, 0x56, 0x78, 0x01, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0xA0, 0x86, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0xAA, 0x00, 0xDF, 0x00, 0x90,
    0x78, 0x34, 0x12, 0x02, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x50,
    0x46, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00};

void parse(std::vector<uint8_t> &binary_stream) {
  size_t offset=0;
  while(offset < binary_stream.size()){

  }
}


int main() { return 0; }

