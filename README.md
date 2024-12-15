# MSP430 Dual-Channel Sampling Project

## Overview
This embedded project implements a dual-channel data sampling system using the MSP430 microcontroller. It samples values from two analog channels, processes the data, and transmits it via UART. The system is designed to demonstrate efficient multi-channel sampling, data buffering, and user interaction through button controls and status LEDs.

## Features
- **Dual-Channel Sampling:** Simultaneous sampling from two analog input channels (Channel 1 and Channel 2).
- **Configurable Sampling Rate:** Samples 10 values per channel, with a configurable rate of 3 samples per second.
- **UART Communication:** Transmits sampled data to a PC via UART at 9600 baud.
- **Button-Controlled Operation:**
  - **Start Sampling (SW1):** Initiates sampling for both channels sequentially.
  - **Stop Sampling (SW2):** Halts the sampling process.
- **Status LEDs:**
  - **Channel 1 LED (Blue):** Indicates activity during Channel 1 sampling.
  - **Channel 2 LED (Red):** Indicates activity during Channel 2 sampling.

## System Details
- **Hardware Configuration:**
  - **Analog Inputs:** P6.0 (Channel 1) and P6.1 (Channel 2).
  - **UART Pins:** P4.4 (TX) and P4.5 (RX).
  - **Control Buttons:** P1.4 (SW1) and P1.5 (SW2).
  - **Status LEDs:** P2.4 (Channel 1) and P2.5 (Channel 2).
- **ADC Configuration:**
  - Utilizes ADC12 in a repeat-single-channel mode for efficient sampling.
  - Configured for 8-bit resolution by using the 8 most significant bits (MSBs) of ADC results.
- **Timers:**
  - **Timer A0:** Manages sampling frequency.
  - **Timer A1:** Debounces the control buttons.

## Code Description
- **Main Program:**
  - Initializes peripherals (ADC, UART, Timers, GPIO).
  - Continuously monitors flags to sample and transmit data.
- **Interrupt Handlers:**
  - **ADC ISR:** Handles conversion results for both channels, storing data in respective buffers.
  - **Timer ISR:** Polls buttons for start and stop commands, ensuring reliable operation.
- **Data Transmission:**
  - Sends channel-specific messages (`"Prvi kanal:"`, `"Drugi kanal:"`) followed by sampled data via UART.

## How It Works
1. **Start Sampling:**
   - Press SW1 to begin sampling on Channel 1.
   - The system alternates between Channel 1 and Channel 2 until stopped.
2. **Stop Sampling:**
   - Press SW2 to halt the process. The system completes any ongoing sampling before stopping.
3. **Data Transmission:**
   - Once sampling for a channel is complete, the data is sent to the PC through UART.

## Usage Instructions
1. Connect the MSP430 to your PC via UART.
2. Compile and load the program onto the MSP430 using your preferred IDE or toolchain.
3. Open a serial terminal on your PC (e.g., PuTTY, Tera Term) at 9600 baud to receive data.
4. Press SW1 to start sampling and monitor the data in the terminal.
5. Press SW2 to stop the sampling process.

## Dependencies
- MSP430 compiler (e.g., GCC for MSP430 or Code Composer Studio).
- UART terminal software for data visualization.

## Notes
- Sampling behavior can be modified by adjusting the `NUM_OF_SAMPLE` or Timer A0 configurations.
- Ensure pull-up resistors are enabled for buttons to avoid unintended triggers.
- The UART message format and structure can be customized as needed.

## License
This project is provided as-is, with no warranties or guarantees. Use it for educational purposes or modify it to suit your needs.

---

Happy coding with MSP430!

