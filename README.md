# 🧭 NaviNode: Sistema Inteligente de Asistencia a la Navegación

![Estado](https://img.shields.io/badge/Estado-Prototipo_Funcional-success)
![Hardware](https://img.shields.io/badge/Hardware-ESP32-blue)
![Arquitectura](https://img.shields.io/badge/Arquitectura-Edge%2FCloud_Híbrida-orange)

NaviNode es un prototipo de bastón inteligente diseñado para la navegación urbana de personas con discapacidad visual. A diferencia de las soluciones monolíticas tradicionales, NaviNode utiliza un enfoque de **procesamiento híbrido**: delega la carga computacional pesada (rutas GPS y reconocimiento de voz mediante IA) al dispositivo móvil del usuario, y utiliza el microcontrolador únicamente como un nodo actuador háptico y auditivo de alta eficiencia conectado vía Bluetooth Low Energy (BLE).

---

## 🏗️ Arquitectura del Sistema

El proyecto se divide en dos capas principales:

1. **Capa Lógica y de Ruteo (App Web / Frontend):**
   * Aplicación móvil basada en tecnologías web (PWA).
   * Utiliza la **Web Speech API** para capturar el destino mediante comandos de voz (NLP).
   * Se integra con el **SDK de Google Maps** (Directions y Geocoding API) para generar arreglos de *waypoints* (esquinas y cruces).
   * Implementa ruteo dinámico basado en la **Fórmula de Haversine** con un *hitbox* de 10 metros para actualizar la ruta de forma automática.
   * Utiliza la **Web Bluetooth API** para transmitir comandos de corrección direccional ('I', 'D', 'C') al hardware.

2. **Capa de Actuación (Hardware Embebido - C++):**
   * Procesador central **ESP32** gestionando conectividad BLE.
   * Magnetómetro **QMC5883L (GY-271)** vía bus I2C para calcular el azimut y el vector de orientación en tiempo real.
   * Módulo **DFPlayer Mini** vía UART para decodificar alertas de voz precargadas desde una memoria MicroSD hacia un altavoz local.
   * Actuadores hápticos (motores de vibración) gestionados por señales digitales (PWM/HIGH).

---

## 🔌 Hardware Requerido (Lista de Componentes)

* 1x Microcontrolador ESP32-WROOM-32.
* 1x Módulo Brújula GY-271 (Chip QMC5883L).
* 1x Módulo de audio DFPlayer Mini + Altavoz 8Ω 1W-3W.
* 1x Batería de Litio 18650 (3.7V, 1200mAh mín.).
* 1x Módulo de carga TP4056 (Con protección USB-C).
* 1x Módulo elevador de voltaje (Boost Converter) MT3608.
* 1x Mini interruptor deslizante.
* Actuadores de vibración tipo moneda (Coin Vibration Motors).

> 💡 **Nota sobre el diseño de energía:** El sistema utiliza el módulo MT3608 para elevar los 3.7V de la batería a 5V constantes. Esto evita los reinicios por caídas de tensión (*brownouts*) que sufre el ESP32 cuando el módulo de audio y el Bluetooth demandan picos altos de corriente de forma simultánea.

---

## ⚙️ Instalación y Configuración

### 1. Despliegue del Hardware (Firmware ESP32)
1. Instalar el [IDE de Arduino](https://www.arduino.cc/en/software).
2. Añadir el soporte para la placa ESP32 en el Gestor de Tarjetas.
3. Instalar las siguientes librerías desde el Gestor de Librerías:
   * `QMC5883LCompass` por MRPrograms.
   * `DFRobotDFPlayerMini` por DFRobot.
4. Compilar y subir el archivo principal `NaviNode_BLE.ino` al ESP32.

### 2. Calibración Geomagnética (Hard-Iron) Obligatoria
Antes de usar el bastón en la calle, es crítico inyectar los valores de compensación magnética debido a la interferencia generada por la batería 18650 y los pines del ESP32.
1. Cargar el script de extracción de extremos (incluido en la carpeta `/tools`).
2. Rotar el hardware en forma de "8" en todos los ejes durante 45 segundos.
3. Copiar la matriz resultante desde el Monitor Serie.
4. Pegar los valores en el código principal: `compass.setCalibration(minX, maxX, minY, maxY, minZ, maxZ);`.

### 3. Despliegue de la Aplicación Web
1. Clonar este repositorio.
2. Reemplazar la variable `API_KEY` en el archivo `index.html` (o `app.js`) por tu credencial activa de Google Cloud (con acceso a Maps JavaScript API, Directions API y Geocoding API).
3. **Requisito Crítico:** La aplicación web **debe** estar alojada en un servidor con certificado SSL/TLS (`https://`). Si se ejecuta en local (`http://`), los navegadores modernos bloquearán el acceso al Bluetooth, Micrófono y GPS por políticas de seguridad. (Se recomienda desplegar gratuitamente en Netlify, Vercel o GitHub Pages).

---

## 🚀 Uso del Sistema

1. **Encendido:** Activar el interruptor físico del bastón. El ESP32 iniciará el servidor BLE.
2. **Vinculación:** Abrir la aplicación web alojada mediante HTTPS en el dispositivo móvil. Otorgar permisos de GPS y Micrófono. Pulsar el botón "Conectar Bastón" para emparejar el ESP32 vía Bluetooth.
3. **Navegación:** Tocar el botón de micrófono en la pantalla y dictar el destino (Ej. *"Plazuela Zudañez"*).
4. **Marcha:** El teléfono trazará la ruta. A medida que el usuario camine, el bastón emitirá vibraciones o reproducirá audios (Ej. *"Gire a la izquierda"*) si el azimut de la brújula no coincide con el vector de la calle trazada.

---

## 🐛 Solución de Problemas Frecuentes (Troubleshooting)

* **El ESP32 se congela y la brújula envía puros ceros (0°):** Esto ocurre por pérdida de sincronía en el bus I2C debido a ruido eléctrico. Asegúrese de que la instrucción `Wire.setClockTimeout(3000);` esté presente en el `setup()` del firmware.
* **El navegador no detecta el Bluetooth del ESP32:** Verifique que el servicio de ubicación del celular esté encendido y que la app web se esté ejecutando en un entorno `https://`.
* **El audio del DFPlayer Mini tiene zumbido o ruido de estática:** Verifique que ha colocado la resistencia de 1kΩ en serie entre el pin TX del ESP32 y el pin RX del DFPlayer.

---

## 👨‍💻 Créditos
Proyecto desarrollado para la asignatura de Robótica I / Robótica II - Universidad Mayor, Real y Pontificia de San Francisco Xavier de Chuquisaca (USFX).
