# Pick & Place Palletizer — KOSEN 4th Year Project (2025)

An automated palletizing cell built by a student team at KOSEN-KMUTT. The machine picks
boxes off a conveyor with a vacuum head and stacks them onto a pallet at roughly
**72 packs/hour**, checks every placement with machine vision, and reports to a local
dashboard and a cloud dashboard.

I was the **team leader**, and personally responsible for the PLC program, the
OPC UA → Node-RED → NETPIE data path, and the machine-vision check.

---

## System overview

```
        Conveyor + Cartesian (X-Y-Z) robot
                      |
              Siemens PLC  ──OPC UA──►  Node-RED  ──►  local dashboard
                      |                     |          (control + monitoring)
                      |                     |
   Arduino  ◄────Node-RED──────────────────┘     └──►  NETPIE cloud dashboard
 (vacuum head + conveyor I/O)                          └──►  history + Excel export
```

## What is in this repository

| Folder | Contents |
| --- | --- |
| `vision/` | Python + OpenCV placement check |
| `node-red/` | Exported Node-RED flow for the local dashboard |
| `arduino/` | Arduino sketch for the vacuum head and conveyor I/O |

**Not included:** the PLC program (Siemens project file) and the NETPIE cloud flow. Both
lived on the machine's own controllers and were not exported before the project was handed
in. The system overview above describes the complete build; this repository holds the parts
I still have the source for.

---

## Design note — why there is an Arduino in a PLC project

The PLC had **6 spare outputs**, and the Cartesian robot alone used all six. The vacuum
head and the conveyor still needed control, and the project budget did not cover a PLC
expansion module.

The first plan was to wire PLC outputs directly to the Arduino as hard signals. While
studying Node-RED I found it could talk to the Arduino as well, so Node-RED became the
link between the two controllers instead — which needed no extra wiring at all.

**Known limitation:** this puts Node-RED inside the control path. If Node-RED stalls, the
arm stops. A production version should keep the full sequence inside the PLC and leave
Node-RED for monitoring only.

---

## Machine vision

Placement is verified with a camera and OpenCV using colour detection — the boxes and the
pallet are deliberately different colours, and a box is registered when the expected colour
is found at the expected position.

The first version used a fixed threshold and broke whenever the room lights were switched
on or off, because reflections pushed dark pixels out of range. The threshold band was
widened so detection stayed stable across lighting conditions.

**Better fix for next time:** add controlled lighting at the inspection point and shield it
from ambient light, instead of compensating in software.

---

## Built with

Siemens PLC (TIA Portal) · Arduino · Python, OpenCV · Node-RED · NETPIE (MQTT) · OPC UA

---

## Notes

- This repository contains my own work from the project; teammates contributed the
  mechanical build and assembly.
- Credentials, tokens and network addresses have been removed from all files.
- Team 4 people
- https://canva.link/kahg17aeh8h0wgb

