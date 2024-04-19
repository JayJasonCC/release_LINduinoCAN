<h1 align="right">EXPERIMENTAL</h1>

<h1 align="center"> LINduinoCAN </h1>

<p align="center">
    Developing an experimental platform for vehicular communcation protocols (CAN & LIN) - aiding the development/ testing of novel security proposals for LIN bus.
</p>

---

<p align="center">
  <strong>Contents</strong>
</p>

<p align="center">
    &bull;
  <a href="#introduction-project-outline-objectivs-and-aims">Introduction: Project Outline, Objectivs, and Aims</a> &bull;
  <a href="#repo-structure">Repo Structure</a> &bull;
  <a href="#key-features-capabilities">Key Features/ Capabilities</a> &bull;
  <a href="#getting-started">Getting Started</a> &bull;
  <a href="#dependencies">Dependencies</a> &bull;
  <a href="#bill-of-materials">Bill Of Materials</a>
</p>

---

# Introduction: Project Outline, Objectivs, and Aims

## Project Overview:

Modern vehicles, now sophisticated cyber-physical systems, require complex on-board connectivity to satisfy the requirements of newfound customer demands for innovative features, safety mechanisms, and ADAS functionality. To facilitate this, protocols suchlike CAN, FlexRay and LIN are depended upon. Although ISO sets clear objectives for OEM's and TIERs to work towards sucure-by-design development, one protocol remains vastly used yet untouced by Cyber-Security controls - LIN bus.

Thus, developing an accessible, deployable, and expandable platform for CAN and LIN aims to provide a gateway to further research regarding LIN security mechanisms and the security of vehicular networks as a whole.
 
## Research Objectives:

1.	Investigate widespread vehicular communication protocols, identifying technologies used within systems of differing criticality and function within an automotive context. 
2.	Identify attacks against vehicular communication protocols and its capability to inadvertently affect the vehicle, subsidised with available mitigations and the utilised platforms. 
3.	Develop an experimental platform for CAN and LIN bus capable of implementing CyS proposals for LIN bus and/ or assist in the development of novel approaches to LIN bus security. 
4.	Evaluate the developed platform against previously identified works, while providing scenario-based testing to review its CAN and LIN communication capabilities. 

 

# Repo Structure
> Description of the repo's file structure/ layout

- **component_CAN&LIN** - This folder contains the final projects source code, designs, and schematics to allow simple replication of the developed platform. This reflects checkpoint-3 in phase-3 of the methodology.

- **component_LIN** - This folder holds the source code, designs, and schematics for just the LIN component of the system. Reflecting checkpoint-1 in phase-3 of the methodology.

- **component_CAN** - This folder holds the source code, designs, and schematics for just the CAN component of the system. Reflecting checkpoint-2 in phase-3 of the methodology.


# Key Features/ Capabilities
> Key features of the developed platform. Refering to the 'component_CAN&LIN'
- [x] LIN Communication
- [x] CAN Communication
- [x] CAN and LIN interconnection
- [x] CAN message monitoring
- [x] LIN message monitoring
- [x] Simple deployment
- [x] Easy expandability
- [x] Accessible and affordable

# Getting Started
>  This includes the basic steps to get the platform up and running.

Based on the component you wish to build, obtain the materials required (details in the bill of materials) and assemble as per the provided schematics.
Open the source code using the Arduino IDE, connect each Arduino and flash it with the correct code. Once completed, provide the LIN circuit with minimum 5.5V and maximum 26V supply voltage. The LIN component CANNOT be powered solely from the Arduinos's, since max output is only 5V. The CAN component however can solely be powered using the Arduinos. Crucially, ensure a common ground between LIN nodes is maintained, otherwise sporadic message readings may occur, and that if using more than 2 CAN nodes, the bus is terminated with 120-Ohm resistance.

# Dependencies
>Library dependencies required for the source code.

| Library       | git                                   | docs                                  |
| ------------- |----------------------------------------| ------------------------------------- |
| `DHT.H`  | [git](https://github.com/adafruit/DHT-sensor-library) | [docs](https://learn.adafruit.com/dht) |
| `NewPing.H`  | [git](https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home) | [docs](https://www.arduino.cc/reference/en/libraries/newping/) |
| `Mcp2515.H & SPI.H`  | [git](https://github.com/autowp/arduino-mcp2515) | [docs](https://www.arduino.cc/en/Reference/SPI) |
| `LiquidCrystal.H`  | [git](https://github.com/arduino-libraries/LiquidCrystal) | [docs](https://www.arduino.cc/en/Reference/LiquidCrystal) |
> View attached docs for specifics


# Bill of Materials
> Find below the bill of materials for 'component_CAN&LIN'.

| Amount | MN (manufacturer number) | MPN (manufacturer part number) | Part Type | Properties |
|--------|--------------------------|--------------------------------|-----------|------------|
| 3 | Can bus module | pins 13; package THT; variant variant 1; pin spacing 0.1in (2.54mm) | MCP2515, TJA1050 | 8MHz |
| 3 | Arduino Mega 2560 (Rev3) | type Arduino MEGA 2560 (Rev3) | n/a | n/a |
| 1 | Humidity and Temperature Sensor DHT11 | u2 0-50°C; output signal Digital Signal; na DHT11; u3 ±5%RH ±2°C; u1 20-95%RH; dc 3.3-5.5V DC; variant variant 1 | n/a | n/a |
| 1 | Diode | package diode-1n4001; variant pth | n/a | n/a |
| 1 | LCD screen | type Character; pins 16 | n/a | n/a |
| 1 | Red (633nm) LED | polarity common anode; current 0.030A; package 5 mm [THT]; pin order rgb; rgb RGB | n/a | n/a |
| 1 | Red (633nm) LED | color Red (633nm); current 0.030A; leg yes; package 5 mm [THT] | n/a | n/a |
| 1 | Green (555nm) LED | color Green (555nm); current 0.030A; leg yes; package 3 mm [THT] | n/a | n/a |
| 1 | Red (633nm) LED | color Red (633nm); current 0.030A; leg yes; package 3 mm [THT] | n/a | n/a |
| 1 | Yellow (595nm) LED | color Yellow (595nm); current 0.030A; leg yes; package 3 mm [THT] | n/a | n/a |
| 1 | Arduino Uno (Rev3) - ICSP | type Arduino UNO (Rev3) - ICSP (w/o icsp2) | n/a | n/a |
| 2 | MCP2003 | chip label MCP2003; true; pins 8; package DIP (Dual Inline) [THT]; hole size 0.7mm,0.508mm; pin spacing 300mil | n/a | n/a |
| 2 | 120Ω Resistor | resistance 120Ω; tolerance ±5%; power 0.25W; bands 4; package THT; pin spacing 400 mil | n/a | n/a |
| 1 | 1kΩ Resistor | resistance 1kΩ; tolerance ±5%; power 0.25W; bands 5; package THT; pin spacing 400 mil | n/a | n/a |
| 1 | US-100 Ultrasonic Distance Sensor | type ultrasonic distance sensor; variant US-100 | n/a | n/a |
| 4 | Pushbutton | contact resistance 0.1Ω; switch status Pressed; package [THT] | n/a | n/a |
