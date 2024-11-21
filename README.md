# Blitzortung ESP32 Project

This project is part of the 7th semester studies and involves using an ESP32 microcontroller to detect lightning strikes using the AS3935 lightning sensor.

## Project Structure

- **src/**: Contains the main source code for the project.
- **include/**: Contains header files for the project.
- **lib/**: Contains project-specific libraries.
- **test/**: Contains unit tests for the project.
- **.vscode/**: Contains VSCode configuration files.
- **platformio.ini**: PlatformIO project configuration file.

## Pinout

| ESP32 Pin | AS3935 Pin | Description          |
|-----------|------------|----------------------|
| GPIO 21   | SDA        | I2C Data             |
| GPIO 22   | SCL        | I2C Clock            |
| GPIO 4    | IRQ        | Interrupt Request    |
| GND       | GND        | Ground               |
| 3.3V      | VCC        | Power Supply         |

## Getting Started

1. **Clone the repository**:
    ```sh
    git clone <repository-url>
    cd BlitzortungAS3935
    ```

2. **Open the project in VSCode**:
    ```sh
    code .
    ```

3. **Build and upload the project**:
    ```sh
    pio run --target upload
    ```

4. **Monitor the serial output**:
    ```sh
    pio device monitor
    ```

## Dependencies

- **PlatformIO**: A professional collaborative platform for embedded development.
- **ESP32 Arduino Core**: The Arduino core for the ESP32.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgements

- Special thanks to the professors and colleagues who provided guidance and support throughout this project.

For more information, refer to the official documentation and resources provided in the project directory.
