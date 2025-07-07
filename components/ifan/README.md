# ifan

provides simple support for the fan, buzzer, and light of an Sonoff ifan04, cloned from
[Christopher P. Yarger's custom_components repo](https://github.com/cpyarger/custom_components).

## Example

The following is an example YAML configuration for the ifan component


```yaml
substitutions:
  name: dining-room-fan
  device_description: Sonoff iFan04-L
  friendly_name: Dining Room

esphome:
  name: ${name}
  comment: ${device_description}
  platform: ESP8266
  board: esp01_1m

external_components:
  - source: github://cpyarger/custom_components@master
    #refresh: 0s

# Disable logging on serial as it is used by the remote
logger:
  baud_rate: 0

# Enable Home Assistant API
api:

ota:
  password: !secret ota_password

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  fast_connect: on
captive_portal:

web_server:
  port: 80

binary_sensor:
  - platform: gpio
    id: button_light
    pin: GPIO0
    on_press:
      then:
        - light.toggle: fan_light

output:
  - platform: esp8266_pwm
    id: led_pin
    pin: GPIO13
    inverted: true

light:
  - platform: monochromatic
    id: led1
    output: led_pin
    default_transition_length: 0s
    restore_mode: always off
    
  - platform: ifan
    id: fan_light
    name: "${friendly_name} Light"
    
fan:
  - platform: ifan
    id: the_fan
    name: "${friendly_name} Fan"
    # optional: allows you to disable the buzzer, enabled by default. 
    # buzzer_enable: false 
button:
  - platform: template
    name: ${friendly_name} Cycle Fan
    on_press:
      then:
        - fan.cycle_speed: the_fan


switch:
  - platform: restart
    name: "${friendly_name} Restart"
```
