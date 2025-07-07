# ESPHome component for Husqvarna Automower G3

This is an ESPHome component for fully local control of Gen 3 Automowers
(330X, 430X, ...). It speaks the mower's reverse-engineered UART protocol
that is used between internal boards (display, etc.) in the mower, via any
of the several available extension connectors on the mower motherboard.

An ESPHome capable board is required to be mounted inside the mower with
some additional electrical circuitry and wiring necessary for full
functionality, such as to be able to keep power to the ESPHome board via
the charger voltage when the mower is asleep in the dock, and to wake it
from sleep via the STOP button. A reference hardware impementation is
available at https://github.com/nattgris/remowte.

## Example configuration

``` yaml
uart:
  - id: u1
    tx_pin: 19
    rx_pin: 18
    baud_rate: 115200

automower_g3:
  uart_id: u1

binary_sensor:
  - platform: automower_g3
    enabled:
      name: "Enabled"

sensor:
  - platform: automower_g3
    num_sat:
      name: "Number of satellites"
    latitude:
      name: "Latitude"
    longitude:
      name: "Longitude"
    battery_1_voltage:
      name: "Battery 1 voltage"
    battery_1_level:
      name: "Battery 1"
    battery_1_current:
      name: "Battery 1 current"
    battery_1_temperature:
      name: "Battery 1 temperature"
    battery_2_voltage:
      name: "Battery 2 voltage"
    battery_2_level:
      name: "Battery 2"
    battery_2_current:
      name: "Battery 2 current"
    battery_2_temperature:
      name: "Battery 2 temperature"
    battery_level:
      name: "Battery"
    status:
      name: "Status"
    substatus:
      name: "Substatus"
    next_start:
      name: "Next start"

button:
  platform: automower_g3
  park:
    name: "Park"
  auto:
    name: "Auto"
  man:
    name: "Man"
```
