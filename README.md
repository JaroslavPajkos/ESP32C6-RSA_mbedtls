# RSA Digital Signature Time Comparison

This project implements an RSA digital signature application using the **mbedtls** library on an ESP32 microcontroller. It focuses on comparing the performance (in CPU cycles) of RSA signature generation and verification for different key sizes (2048 and 3072 bits) using PKCS#1 v1.5 and PSS padding schemes. Additionally, it includes test certificates and keys for cryptographic operations.

## Project Overview

- **Author**: Jaroslav Pajkos
- **Year**: 2025
- **Purpose**: Measure and compare the execution time of RSA digital signature operations (signing and verification) for different key sizes and padding schemes.
- **Platform**: ESP32 with ESP-IDF framework
- **Library**: mbedtls for cryptographic operations
- **Key Features**:
  - Generates RSA keys for 2048 and 3072 bits.
  - Computes SHA-256 hash of a test message.
  - Signs the message using PKCS#1 v1.5 and PSS padding.
  - Verifies signatures and measures execution time in CPU cycles.
  - Includes test certificates and keys (RSA 1024-bit, ED25519, etc.) for cryptographic testing.

## Repository Structure

- `CMakeLists.txt`: Build configuration for the ESP-IDF project.
- `mbedtls_rsa.c`: Main source file implementing RSA signature operations and timing measurements.
- `certs_test.h`: Header file containing test certificates and keys (RSA, DH, DSA, ED25519).
- `README.md`: This file.

## Prerequisites

- **Hardware**: ESP32 development board
- **Software**:
  - [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/) (Espressif IoT Development Framework)
  - [mbedtls](https://tls.mbed.org/) library (included as a dependency in ESP-IDF)
  - CMake and a compatible build tool
- **Tools**: Serial monitor (e.g., minicom or PuTTY) to view console output

## Setup Instructions

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/your-username/rsa-time-comparison.git
   cd rsa-time-comparison
   ```

2. **Set Up ESP-IDF**:
   Follow the [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) to install and configure ESP-IDF.

3. **Configure the Project**:
   Run the following command to configure the project:
   ```bash
   idf.py set-target esp32
   idf.py menuconfig
   ```
   Ensure the mbedtls component is enabled (it is included by default in ESP-IDF).

4. **Build the Project**:
   Compile the project using:
   ```bash
   idf.py build
   ```

5. **Flash to ESP32**:
   Connect your ESP32 board and flash the firmware:
   ```bash
   idf.py -p /dev/ttyUSB0 flash
   ```
   Replace `/dev/ttyUSB0` with the appropriate serial port for your system.

6. **Monitor Output**:
   View the console output using:
   ```bash
   idf.py monitor
   ```

## Usage

- The program runs automatically upon flashing to the ESP32.
- It performs the following steps:
  1. Initializes mbedtls contexts and seeds the random number generator.
  2. Computes the SHA-256 hash of the test message: `"Testovaci text na zasifrovanie"`.
  3. For each key size (2048, 3072 bits):
     - Generates an RSA key pair.
     - Signs the message hash using PKCS#1 v1.5 and PSS padding.
     - Verifies the signatures and measures the CPU cycles for verification.
  4. Prints the results to the console, including:
     - Message hash
     - Signatures (PKCS#1 v1.5 and PSS)
     - Verification times in CPU cycles
- The test certificates in `certs_test.h` are not used in the main program but are included for potential cryptographic testing.

### Sample Output
```
Starting RSA Time Comparison

Message hash: [SHA-256 hash in hex]

=== Testing RSA-2048 ===
Signing message (PKCS#1 v1.5)...
PKCS#1 v1.5 Signature: [Signature in hex]
Signing message (PSS)...
PSS Signature: [Signature in hex]

Measuring RSA-2048 PKCS#1 v1.5 Verify (mbedtls_rsa_pkcs1_verify)...
RSA-2048 PKCS#1 v1.5 Verify (mbedtls_rsa_pkcs1_verify): [X] cycles

Measuring RSA-2048 PSS Verify (mbedtls_rsa_rsassa_pss_verify)...
RSA-2048 PSS Verify (mbedtls_rsa_rsassa_pss_verify): [Y] cycles

=== Testing RSA-3072 ===
...

Time Comparison Completed Successfully!
```

## Notes

- **Performance Measurement**: The `start()` and `end()` functions use RISC-V cycle counters to measure execution time. The reported cycles are divided by 160 for normalization (specific to the ESP32's clock configuration).
- **Certificates**: The `certs_test.h` file includes test certificates and keys from wolfSSL, which can be used for additional cryptographic experiments.
- **Error Handling**: The program includes robust error handling for mbedtls operations, with detailed error messages printed to the console.
- **Limitations**: The project is designed for ESP32 and may require modifications for other platforms.

## Contributing

Contributions are welcome! Please open an issue or submit a pull request for bug fixes, improvements, or new features.

## License

This project is licensed under the [MIT License](LICENSE).

## Acknowledgments

- [mbedtls](https://tls.mbed.org/) for the cryptographic library
- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/) for the development framework
- [wolfSSL](https://www.wolfssl.com/) for providing test certificates
