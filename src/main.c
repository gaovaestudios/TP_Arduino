#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>   // <-- agregado

// --- OLED 0.91" ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- LCD I2C ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Keypad 4x4 ---
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {10, 9, 8, 7};
byte colPins[COLS] = {6, 5, 4, 3};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// --- Pines ---
int bpmPin = A0;
int tempPin = A1;
int buzzer = 12;

// --- Sensor BPM ---
#define samp_siz 4
#define rise_threshold 5
bool sensorActivo = false;

// --- Variables del sensor BPM ---
float reads[samp_siz];
float sum, last, reader, before = 0;
bool rising = false;
int rise_count = 0;
float first = 0, second = 0, third = 0;
float print_value = 0, lastBPM = 0;
unsigned long last_beat = 0, lastPrint = 0;
const unsigned long interval = 3000;

// --- Estado del menú ---
enum Estado { MENU, SENSOR, TEMP };
Estado estadoActual = MENU;

// --- FUNCIONES ---
void mostrarMenu();
void manejarMenu(char key);
void iniciarSensor();
void medirBPM();
void apagarSensor();
void beepTresSegundos();
void probarTemperatura();
void mostrarTermometro(float temperatura);

// --- MAPEO CON DECIMALES ---
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup() {
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // --- Inicializar OLED ---
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Error OLED"));
    for (;;);
  }
  oled.clearDisplay();
  oled.display();

  mostrarMenu();
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    Serial.print("Tecla presionada: ");
    Serial.println(key);

    if (estadoActual == MENU) {
      manejarMenu(key);
    } 
    else if (estadoActual == SENSOR) {
      if (key == '0') {
        apagarSensor();
      } 
      else if (key == '3') {
        beepTresSegundos();
        apagarSensor();
      }
    } 
    else if (estadoActual == TEMP && key == '0') {
      mostrarMenu();
    }
  }

  if (estadoActual == SENSOR && sensorActivo) {
    medirBPM();
  }
}

// --- Mostrar menú principal ---
void mostrarMenu() {
  estadoActual = MENU;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1) BPM 2) Temp");
  lcd.setCursor(0, 1);
  lcd.print("0) Volver");

  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(15, 12);
  oled.print("Elija una opcion");
  oled.display();
}

// --- Manejo del menú ---
void manejarMenu(char key) {
  switch (key) {
    case '1': iniciarSensor(); break;
    case '2': probarTemperatura(); break;
    case '3': beepTresSegundos(); break;
    case '0': apagarSensor(); break;
    default:
      lcd.clear();
      lcd.print("Opcion invalida");
      delay(1000);
      mostrarMenu();
      break;
  }
}

// --- Iniciar sensor BPM ---
void iniciarSensor() {
  lcd.clear();
  lcd.print("Iniciando sensor");
  delay(1000);
  lcd.clear();
  lcd.print("Midiendo BPM...");
  sensorActivo = true;
  estadoActual = SENSOR;

  for (int i = 0; i < samp_siz; i++) reads[i] = 0;
  sum = 0; rising = false; rise_count = 0;
  first = second = third = 0;
  before = 0; lastBPM = 0;
  lastPrint = millis();

  // Mostrar mensaje en OLED
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(25, 12);
  oled.print("Midiendo BPM...");
  oled.display();
}

void medirBPM() {
  static int ptr = 0;
  long now, start;
  int n = 0;
  float reader_local;

  start = millis();
  reader_local = 0;
  do {
    reader_local += analogRead(bpmPin);
    n++;
    now = millis();
  } while (now < start + 20);
  reader_local /= n;

  // --- Señal al Plotter Serie ---
  Serial.println(reader_local);

  // --- Cálculo original BPM ---
  sum -= reads[ptr];
  sum += reader_local;
  reads[ptr] = reader_local;
  last = sum / samp_siz;
  ptr++;
  if (ptr >= samp_siz) ptr = 0;

  if (last > before) {
    rise_count++;
    if (!rising && rise_count > rise_threshold) {
      rising = true;
      first = millis() - last_beat;
      last_beat = millis();
      print_value = 60000. / (0.4 * first + 0.3 * second + 0.3 * third);
      third = second;
      second = first;
      if (print_value >= 40 && print_value <= 100) {
        lastBPM = print_value;
      }
    }
  } else {
    rising = false;
    rise_count = 0;
  }

  before = last;

  // --- Actualización del LCD + beep sincronizado ---
  if (millis() - lastPrint >= interval) {
    lcd.clear();
    lcd.setCursor(0, 0);

    if (lastBPM > 0) {
      lcd.print("BPM: ");
      lcd.print(lastBPM, 0);

      // 💓 Corazón late junto al beep
      mostrarCorazon(true);      // corazón grande
      tone(buzzer, 1000, 200);
      delay(200);                // duración del latido
      mostrarCorazon(false);     // vuelve al tamaño normal

    } else {
      lcd.print("Esperando senal");
    }

    lastPrint = millis();
  }
}



// --- Apagar sensor ---
void apagarSensor() {
  sensorActivo = false;
  lcd.clear();
  lcd.print("...");
  noTone(buzzer);
  delay(1000);
  mostrarMenu();
}

// --- Beep de 3 segundos ---
void beepTresSegundos() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("No hay pulso,");
  lcd.setCursor(0, 1);
  lcd.print("te moriste :(");

  oled.clearDisplay();
  oled.drawLine(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2, SSD1306_WHITE);
  oled.display();

  tone(buzzer, 1000);
  delay(3000);
  noTone(buzzer);

  mostrarMenu();
}

// --- Probar temperatura ---
void probarTemperatura() {
  estadoActual = TEMP;
  lcd.clear();
  lcd.print("Leyendo Temp...");
  Serial.println("\n--- Prueba de temperatura ---");

  const int numLecturas = 10;
  long suma = 0;

  for (int i = 0; i < numLecturas; i++) {
    suma += analogRead(tempPin);
    delay(100);
  }

  float promedio = suma / (float)numLecturas;
  float temperatura = mapFloat(promedio, 119, 114, 36, 37);

  Serial.print("Promedio analogico: ");
  Serial.print(promedio, 1);
  Serial.print("  |  Temperatura: ");
  Serial.print(temperatura, 2);
  Serial.println(" °C");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp Prom:");
  lcd.setCursor(0, 1);
  lcd.print(temperatura, 2);
  lcd.print(" C");

  mostrarTermometro(temperatura);

  delay(3000);
  mostrarMenu();
}

// --- Mostrar termómetro en OLED ---
void mostrarTermometro(float temperatura) {
  oled.clearDisplay();
  oled.drawCircle(25, 26, 6, SSD1306_WHITE);
  oled.fillCircle(25, 26, 6, SSD1306_WHITE);
  oled.drawRoundRect(22, 4, 6, 18, 3, SSD1306_WHITE);
  for (int i = 0; i < 5; i++) {
    int y = 6 + i * 4;
    oled.drawLine(32, y, 38, y, SSD1306_WHITE);
  }

  float tempMin = 35.0;
  float tempMax = 40.0;
  int nivel = map(temperatura * 10, tempMin * 10, tempMax * 10, 0, 18);
  if (nivel < 0) nivel = 0;
  if (nivel > 18) nivel = 18;
  for (int i = 0; i < nivel; i++) {
    oled.drawFastHLine(23, 22 - i, 4, SSD1306_WHITE);
  }

  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(50, 6);
  oled.print("Temp:");
  oled.setCursor(50, 16);
  oled.print(temperatura, 1);
  oled.print(" C");
  oled.display();
}
void mostrarCorazon(bool grande) {
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);

  int x = SCREEN_WIDTH / 2;
  int y = SCREEN_HEIGHT / 2;

  if (grande) {
    // Corazón grande (latido)
    oled.fillCircle(x - 6, y - 3, 5, SSD1306_WHITE);
    oled.fillCircle(x + 6, y - 3, 5, SSD1306_WHITE);
    oled.fillTriangle(x - 11, y - 1, x + 11, y - 1, x, y + 10, SSD1306_WHITE);
  } else {
    // Corazón pequeño (reposo)
    oled.fillCircle(x - 5, y - 3, 4, SSD1306_WHITE);
    oled.fillCircle(x + 5, y - 3, 4, SSD1306_WHITE);
    oled.fillTriangle(x - 9, y - 1, x + 9, y - 1, x, y + 8, SSD1306_WHITE);
  }

  oled.display();
}
