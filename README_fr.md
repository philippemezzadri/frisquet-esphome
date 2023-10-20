# Frisquet Boiler pour ESPHome

Ce composant ESPHome permet la communication entre un appareil ESPHome (ESP8266 ou ESP32) et une chaudière de chauffage [Frisquet](<https://www.frisquet.com/fr/>) (équipée d'un thermostat radio Eco System).

La solution développée est applicable à toutes les chaudières Frisquet commercialisées jusqu'en 2012 et équipées du module Eco Radio System. Les chaudières plus récentes équipées du module Visio ne sont pas compatibles car Frisquet a depuis mis en place un protocole de communication chiffré.

Le composant **Frisquet Boiler** apparaît comme un dispositif de [sortie de flottant](<https://esphome.io/components/output/>) sur ESPHome.

Il est recommandé de le combiner avec le composant **Heating Curve Climate** également fourni dans ce projet. Ce composant [Climate](<https://esphome.io/components/climate/index.html>) offrira un contrôle de la température en utilisant un capteur de température extérieur. Si nécessaire, il est également possible d'utiliser un autre type de composant Climate, tel que le [PID Climate](https://esphome.io/components/climate/pid.html?highlight=pid).

## Références

Ce travail est fortement inspiré de :

- <https://antoinegrall.wordpress.com/decodage-frisquet-ers/>
- <http://wiki.kainhofer.com/hardware/vaillantvrt340f>
- <https://github.com/etimou/frisquet-arduino>

et des discussions menées dans ce fil :

- <https://easydomoticz.com/forum/viewtopic.php?f=17&t=1486sid=d2f41ac68e5bab18fd412a192a21c2c4> (Français)

## Câblage

ESPHome remplace le récepteur HF Eco Radio System d'origine et est connecté à la carte principale de la chaudière via une prise micro-fit 4.

| ESP32                 | Côté chaudière      | Numéro de broche |
| --------------------- | ------------------- |:----------:|
| GND                   | fil noir            | 1          |
| Pin 21 (configurable) | fil jaune           | 2          |
| 5V                    | fil rouge (optionnel) | 3          |

**Connecteur Micro-fit 4 :**

<img src="doc/connector_4pin1.png" alt="Dessin de la configuration de la broche Micro-fit 4" width="80"/>

Direction de vue définie pour la configuration de la broche du connecteur :

- Réceptacle - _vue arrière_
- Connecteur - _vue de face_

_Remarque_: Il a été observé que le courant fourni par la carte principale de la chaudière n'est pas suffisant pour alimenter l'ESP32.

## Installation

**Note :** pour la méthode d'installation précédente (obsolète) basée sur des composants personnalisés, voir [ici](doc/custom_components.md).

Le composant Frisquet ESPHome se compose de deux composants :

- `heat_curve_climate` un composant personnalisé [Climate](<https://esphome.io/components/climate/index.html>) qui contrôlera le point de consigne
