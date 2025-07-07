# ESPHome custom components

My collection of components for ESPHome.

To use these components, use the [external components](https://esphome.io/components/external_components.html).

Example:
```yaml
external_components:
  - source:
      type: git
      url: https://github.com/nattgris/esphome_custom_components
    components: [ ifan ]
```

## My components 

includes;

* [automower_g3](components/automower_g3) - Local control of Husqvarna Gen 3 Automowers
* [ifan](components/ifan) - provides simple support for the fan, buzzer, and light of an sonoff ifan04, cloned from [Christopher P. Yarger's custom_components repo](https://github.com/cpyarger/custom_components).
