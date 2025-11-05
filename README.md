# TP_Arduino

Codeamos el ejercicio 1 para el arduino.

TO DO LIST:
· Modificar readme - pines - datasheets y links que usamos - Piezas que usamos
· Subir codigo como archivo.c
· Lista de componentes
· Agregar librerias utilizadas, fotografias del proyecto

# MEDIDOR DE PULSO Y TEMPERATURA

**Integrantes del grupo:** Gentta Iván, Aguirre Christian, Aira Nicholas, Spataro Franco.
**Docentes y materia:** Pedro Iriso - Matías Gagliardo.
                    **LABORATORIO DE COMPUTACIÓN I**

## Descripción general y funcionalidades
El grupo diseñó un sistema con interfaz de usuario diseñado en arduino, el cual se maneja mediante un teclado matricial 4x4 y una Pantalla LCD 16x2 con módulo I2C que permite al usuaio interactuar con distintas funciones médicas para testear, entre ellas, lectura de temperatura(grados), la medición de frecuencia cardiaca(BPM), y la activacion de un zumbador(Buzzer) para cortar el ritmo cardiaco a modo ilustrativo y un Oled que grafica el pulso.
Se elaboró un prototipo de monitor básico, capaz de medir señales vitales y permite mostrar el resultado por pantalla, combinando lecturas analógicas de sensores, procesamiento digital de señales, y retroalimentación visual y sonora.
El sistema comienza con un menú principal interactivo que muestra en el LCD distintas funciones (1-Iniciar sensor BPM, 2-Medir temperatura, 3-Beep 3 segudos, 0-Volver) la cuales se seleccionan mediante el keypad para que se realice el envío de datos al monitor serial para verificación y depuración de las lecturas.
 1-Medicion frecuencia cardiaca (BPM): Utiliza un sensor analogico de pulso KY-039 y realiza una lectura analógica del sensor de pulso, detección de flancos ascendentes y cálculo del ritmo cardíaco en BPM.
 2-Medicion de temperatura: Utiliza un sensor de temperatura KY-028 para capturar la información y sacar el promedio de 10 lecturas analogicas y su conversion a grados Celsius mediante un mapeo.
 3-Beep: Simula la detención del ritmo cardiaco mediante la emisión de un tono de de tres segundos.
 En conjunto, el proyecto permite al usuario realizar mediciones básicas de salud y comprobar el funcionamiento del sistema de manera práctica y visual.

## Requisitos funcionales cumplidos
.Manejo de interfaz de usuario con teclado matricial: El sistema reconoce las teclas presionadas y ejecuta acciones segun el modo activo.
.Menú en pantalla Led: Se muestra un menú claro e intuitivo que permite acceder a las distintas funcionalidades.
.Medicion de la frecuencia cardiaca(BPM): El sistema lee los valores capturados por el sensor de pulso, detecta flancos ascendentes, calcula los intervalos entre latidos y muestra el valor promedio de BPM en pantalla.
.Medicion de temperatura corporal: El dispositivo obtiene varias lecturas del sensor de temperatura, calcula su promedio y muestra el valor correspondiente en grados Celsius.
.Emisión de alertas (Beep): Se genera un tono continuo de tres segundos simulando la detencion del pulso y sonidos breves con cada detección de pulso válida.
.Gestion del sistema: Se maneja un flujo de trabajo ordenado y modularizado con funciones.
.Visualizacion clara en pantalla: El Oled muestra mensajes informativos y los resultados de cada medición de manera legible.

## Componentes utilizados
**Componente:** Arduino UNO
**Descripción:** Microcontrolador principal, ejecuta el programa y coordina todos los periféricos.

**Componente:** Pantalla LCD 16x2 con módulo I2C y Oled 0.9 pulgadas
**Descripción:** Display alfanumérico, muestra el menú, mensajes y resultados de las mediciones.

**Componente:** Teclado matricial 4x4
**Descripción:** Módulo de entrada, permite la interacción del usuario mediante teclas numéricas y de control.

**Componente:** Sensor de pulso KY-039
**Descripción:** Sensor analógico, detecta variaciones en la señal del pulso para calcular la frecuencia cardíaca.

**Componente:** Sensor de temperatura KY-028
**Descripción:** Sensor analógico lineal, mide la temperatura corporal aproximada del usuario.

**Componente:** Buzzer
**Descripción:** Elemento sonoro, emite tonos de alerta y señales sonoras.

**Componente:** Cables varios
**Descripción:** Elementos de conexión, permiten interconectar todos los componentes del circuito.

## Esquema Eléctrico


## Máquina de estados
╔════════════════════════════════════════════════════════════════════════╗
║                         ESTADO: MENU                                   ║
║────────────────────────────────────────────────────────────────────────║
║ - Muestra en el LCD:                                                   ║
║   "1) BPM 2) Temp"                                                     ║
║   "3) Beep 0)Off"                                                      ║
║                                                                        ║
║ - Espera una tecla del Keypad:                                         ║
║                                                                        ║
║   Tecla '1' → transición a ESTADO SENSOR                               ║
║        Acción: iniciarSensor()                                         ║
║                                                                        ║
║   Tecla '2' → transición a ESTADO TEMP                                 ║
║        Acción: probarTemperatura()                                     ║
║                                                                        ║
║   Tecla '3' → permanece en MENU                                        ║
║        Acción: beepTresSegundos()                                      ║
║                                                                        ║
║   Tecla '0' → se asegura que todo esté apagado                         ║
║        Acción: apagarSensor() (limpia y vuelve al menú)                ║
╚════════════════════════════════════════════════════════════════════════╝
                      │
                      │
         ┌────────────┴────────────────┐
         │                             │
         ▼                             ▼
╔═════════════════════════════════╗   ╔══════════════════════════════════╗
║        ESTADO: SENSOR (BPM)     ║   ║       ESTADO: TEMP (Temperatura) ║
║─────────────────────────────────║   ║──────────────────────────────────║
║ - Se activa el sensor KY-039     ║   ║ - Lee el sensor KY-028          ║
║   y comienza la medición de BPM. ║   ║ - Promedia 10 lecturas          ║
║                                  ║   ║ - Convierte valor a °C          ║
║ - Muestra BPM cada 3 segundos    ║   ║ - Muestra en LCD:               ║
║   si la señal es válida.         ║   ║   "Temp Prom: XX.X C"           ║
║                                  ║   ║ - Espera 3 seg y vuelve al MENU ║
║ - Hace sonar el buzzer brevemente║   ║                                  ║
║   con cada pulso detectado.      ║   ║                                  ║
║                                  ║   ║ Tecla '0' → transición a MENU   ║
║ Tecla '0' → transición a MENU    ║   ║   Acción: mostrarMenu()          ║
║   Acción: apagarSensor()         ║   ║                                  ║
╚═════════════════════════════════╝   ╚══════════════════════════════════╝
                      │                             │
                      └────────────┬────────────────┘
                                   ▼
                   ╔════════════════════════════════════════╗
                   ║       Retorna al ESTADO: MENU          ║
                   ║ (LCD vuelve a mostrar las opciones)    ║
                   ╚════════════════════════════════════════╝


## Instrucciones de uso
Al tener el medidor de pulso y temperatura conectado y con el código subido al Arduino se mostrara
por la pantalla LCD las distintas opciones y el botón correspondiente para activar sus funciones.

**Medición del ritmo cardiaco (BPM)**

1- Oprima el botón 1 de la botonera para empezar la medición.

2- Coloque el dedo indice entre el medidor infrarojo y el fototransistor.

3- Asegurese que el dispositivo medidor se encuentre dentro de la cubertura proporcionada para resguardarlo lo mas posible de la luz exterior para una mejor medición.

4- Mantenga el dedo quieto por 10 segundos para una lectura mas estable.

5- Lea por la pantalla LCD su ritmo cardiaco.

6- Oprima el botón 0 de la botonera para finalizar la lectura, o el botón 3 para un pequeño easter egg.


**Medición de temperatura**

1- Mantenga apretado de antemano entre 2 dedos el sensor de medición de temperatura por 10 segundos para una mejor lectura.

2- Oprima el botón 2 de la botonera para empezar la medición.

3- Lea por la pantalla LCD su temperatura.


##Imágenes o video demostrativo

### :receipt: Licencia
Este proyecto está licenciado bajo los términos de la **Licencia MIT**.  
Podés ver el texto completo en el archivo [LICENSE](LICENSE).


