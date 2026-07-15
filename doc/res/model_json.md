\page model_json Sending Configuration JSON

\image html model_json.png "A saved DBC-based sending config, as shown in a text editor" width=700px

```
state ::= raw-state | dbc-state | replay-state

raw-state ::= {
    "canId": string,
    "data": string,
    "cyclicEnabled": bool,
    "cyclicIntervalUs": string,
    "manipulationEnabled": bool,
    "manipulations": [ manipulation* ]
}

dbc-state ::= {
    "cyclicEnabled": bool,
    "cyclicIntervalUs": string,
    "dbcFileName": string,
    "selectedSignals": [ signal-ref* ],
    "manipulationEnabled": bool,
    "manipulations": [ manipulation* ],
    "valueFunctions": { (signal-ref: value-function)* }
}

replay-state ::= {
    "speedIndex": int,
    "manipulationEnabled": bool,
    "manipulations": [ manipulation* ],
    "dbcFileName": string
}

value-function ::= { "tree": (expr | null), "bindings": [ binding* ] }

binding ::= { "symbol": char, "typeIndex": int, "configKey": string }

expr ::= { "kind": "value", "value": number }
       | { "kind": "variable", "symbol": char }
       | { "kind": "operator", "operation": op-code,
           "children": [ expr, expr ] }
       | { "kind": "function", "function": fn-code,
           "children": [ expr ] }

signal-ref ::= string   (format "messageId:signalName")

manipulation ::= { "type": "RawManipulation", "value": raw-manip-body }
               | { "type": "DbcManipulation", "value": dbc-manip-body }

raw-manip-body ::= {
    "trigger": [ raw-trigger* ],
    "strategy": raw-strategy,
    "mutation": mutation
}

dbc-manip-body ::= {
    "trigger": [ dbc-trigger* ],
    "strategy": dbc-strategy,
    "mutation": mutation
}

raw-trigger ::= { "type": "IDTrigger", "value": { "id": uint16 } }
              | { "type": "DLCTrigger", "value": { "dlc": uint8 } }
              | { "type": "RandomTrigger", "value": { "probability": float } }

dbc-trigger ::= { "type": "SignalNameTrigger",
                   "value": { "signal_name": string } }
              | { "type": "SignalThresholdTrigger",
                   "value": { "signal_name": string, "threshold": double,
                              "isGreater": bool } }
              | { "type": "RandomTrigger", "value": { "probability": float } }

raw-strategy ::= { "type": "RawEffectStrategy",
                    "value": { "effects": [ raw-effect* ] } }
               | { "type": "DelayedStrategy", "value": { "delayUs": uint32 } }
               | { "type": "DropStrategy", "value": {} }

dbc-strategy ::= { "type": "DbcEffectStrategy",
                    "value": { "effects": [ dbc-effect* ] } }
               | { "type": "DelayedStrategy", "value": { "delayUs": uint32 } }
               | { "type": "DropStrategy", "value": {} }
               | { "type": "DbcInsertStrategy",
                    "value": { "delayUs": uint32,
                               "message": (dbc-message | null) } }

raw-effect ::= { "type": "BitFlipEffect",
                  "value": { "byteIndex": uint8, "bitIndex": uint8 } }
             | { "type": "RandomBitFlipEffect", "value": {} }

dbc-effect ::= { "type": "ValueSetEffect",
                  "value": { "signal_name": string, "value": double } }
             | { "type": "ClampEffect",
                  "value": { "signal_name": string,
                             "min_value": double, "max_value": double } }
             | { "type": "NoiseEffect",
                  "value": { "signal_name": string, "amplitude": double } }

mutation ::= { "type": "NoMutation", "value": {} }
           | { "type": "LatchMutation", "value": {} }

dbc-message ::= { "receiveTimeNs": int64,
                   "signalValues": [ dbc-signal* ],
                   "messageId": uint16 }

dbc-signal ::= { "name": string, "value": double }
```