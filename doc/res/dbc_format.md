\page dbc_format Supported DBC Format

CANcept decodes CAN traffic using [DBC](https://www.csselectronics.com/pages/can-dbc-file-database-intro)
(Vector CANdb++) database files. The parser is a hand-rolled recursive-descent parser,
`CanHandler::DbcParser` (`src/can_handler/dbc_handler/dbc_parser.hpp` / `.cpp`), driven by
`DbcParser::parseDbc()`. It parses a **subset** of the full DBC grammar; sections outside that
subset are either dropped without being interpreted, or make parsing fail if content is present.

## Reference example

The following DBC file exercises every section this parser actually understands
(`VERSION`, `NS_`, `BS_`, `BU_`, `BO_`/`SG_`, `VAL_`) and parses cleanly:

```
VERSION "final"

NS_ :
    NS_DESC_
    CM_
    BA_DEF_
    BA_
    VAL_
    CAT_DEF_
    CAT_
    FILTER
    BA_DEF_DEF_
    EV_DATA_
    ENVVAR_DATA_
    SG_MUL_VAL_

BS_:

BU_: CAN_BUS_BUS_ENGINE CAN_BUS_BUS_CTRLPANEL CAN_BUS_BUS_PARKINGSENSOR

BO_ 256 INDICATOR_STATES: 8 CAN_BUS_BUS_CTRLPANEL
 SG_ LEFT_INDICATOR : 0|1@1+ (1,0) [0|1] ""  CAN_BUS_BUS_CTRLPANEL
 SG_ RIGHT_INDICATOR : 1|1@1+ (1,0) [0|1] ""  CAN_BUS_BUS_CTRLPANEL

BO_ 257 BREAK_STATE: 8 CAN_BUS_BUS_CTRLPANEL
 SG_ BREAK_LIGHT_STATE : 0|1@1+ (1,0) [0|1] ""  CAN_BUS_BUS_CTRLPANEL

BO_ 258 ENGINE_DATA: 8 CAN_BUS_BUS_CTRLPANEL
 SG_ ENGINE_SPEED : 0|16@1+ (1,0) [0|10000] "rpm"  CAN_BUS_BUS_CTRLPANEL
 SG_ ENGINE_TEMP : 16|8@1+ (1,0) [0|200] "°C"  CAN_BUS_BUS_CTRLPANEL

BO_ 259 PARKING_SENSOR: 8 CAN_BUS_BUS_CTRLPANEL
 SG_ DISTANCE_BACK : 0|8@1+ (1,0) [0|255] "cm"  CAN_BUS_BUS_PARKINGSENSOR

BO_ 304 POWERTRAIN_STATUS: 8 CAN_BUS_BUS_PWR
 SG_ CURRENT_GEAR : 0|4@1+ (1,0) [0|15] ""  CAN_BUS_BUS_PWR
 SG_ DRIVE_MODE : 4|3@1+ (1,0) [0|7] ""  CAN_BUS_BUS_PWR

BO_ 1200 BATTERY_INFO: 8 CAN_BUS_BUS_BTY
 SG_ BATTERY_VOLTAGE : 0|8@1+ (0.1,0) [0|25.5] "V"  CAN_BUS_BUS_BTY
 SG_ BATTERY_SOC : 8|8@1+ (0.5,0) [0|100] "%"  CAN_BUS_BUS_BTY

VAL_ 256 LEFT_INDICATOR 1 "on" 0 "off" ;
VAL_ 256 RIGHT_INDICATOR 1 "on" 0 "off" ;
VAL_ 257 BREAK_LIGHT_STATE 1 "on" 0 "off" ;
VAL_ 288 OIL_PRESSURE_LOW 1 "ALARM" 0 "OK" ;
VAL_ 304 CURRENT_GEAR 0 "Park" 1 "Reverse" 2 "Neutral" 3 "Drive" 4 "Low" ;
VAL_ 304 DRIVE_MODE 0 "Eco" 1 "Normal" 2 "Sport" 3 "Offroad" ;
```