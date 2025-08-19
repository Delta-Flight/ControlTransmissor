#include <Arduino.h>
#include "EspNowRcLink/Transmitter.h"
#include <Preferences.h>

// Pinos dos potenciômetros
const int PIN_THROTTLE = 34;
const int PIN_YAW = 35;
const int PIN_PITCH = 32;
const int PIN_ROLL = 33;
const int NUM_ANALOG_CHANNELS = 4;
const int potPins[] = {PIN_THROTTLE, PIN_YAW, PIN_PITCH, PIN_ROLL};

// Pinos dos canais auxiliares
const int PIN_AUX1 = 23;
const int PIN_AUX2 = 22;
const int PIN_AUX3a = 18; // Nova chave de 3 posições
const int PIN_AUX3b = 5;
const int PIN_AUX4 = 19; // Nova chave de 2 posições
const int NUM_AUX_CHANNELS = 4;

// Pinos de controle
const int BUZZER_PIN = 12;

// Intervalo de envio
const uint32_t SEND_INTERVAL_MS = 20;
float alpha = 0.4; // Fator de suavização (alpha)

// Armazenamento de valores
Preferences prefs;
int minValues[NUM_ANALOG_CHANNELS];
int maxValues[NUM_ANALOG_CHANNELS];
int centerValues[NUM_ANALOG_CHANNELS];

// Valores filtrados
float filteredValues[NUM_ANALOG_CHANNELS + NUM_AUX_CHANNELS];

EspNowRcLink::Transmitter tx;

// Funções do buzzer e da música do Mario
#define NOTE_B0 31
#define NOTE_C1 33
#define NOTE_CS1 35
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_FS7 2960
#define NOTE_G7 3136
#define NOTE_GS7 3322
#define NOTE_A7 3520
#define NOTE_AS7 3729
#define NOTE_B7 3951
#define NOTE_C8 4186
#define NOTE_CS8 4435
#define NOTE_D8 4699
#define NOTE_DS8 4978

// Trecho da música do Mario
int melody[] = {
  NOTE_E7, NOTE_E7, 0, NOTE_E7,
  0, NOTE_C7, NOTE_E7, 0,
  NOTE_G7, 0, 0, 0,
  NOTE_G6
};

int tempo[] = {
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12
};

void playMarioTune() {
  Serial.println("Tocando a melodia do Mario...");
  int size = sizeof(melody) / sizeof(int);
  for (int thisNote = 0; thisNote < size; thisNote++) {
    int noteDuration = 1000 / tempo[thisNote];
    tone(BUZZER_PIN, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(BUZZER_PIN);
  }
}

// Função para fazer um bipe
void beep(int duration, int count) {
  for (int i = 0; i < count; i++) {
    tone(BUZZER_PIN, 1000, duration);
    delay(duration + 50);
  }
}

// Rotina de calibração
void calibrate() {
  Serial.println("INICIANDO CALIBRACAO AUTOMATICA...");
  beep(100, 1);
  delay(1000);

  Serial.println("FASE 1: Mova os sticks para o MINIMO e MAXIMO por 10 segundos.");
  
  // Reseta os valores
  for(int i = 0; i < NUM_ANALOG_CHANNELS; i++) {
    minValues[i] = 4095;
    maxValues[i] = 0;
  }
  
  unsigned long calibrationTime = millis();
  while(millis() - calibrationTime < 10000) {
    for (int i = 0; i < NUM_ANALOG_CHANNELS; i++) {
      int potValue = analogRead(potPins[i]);
      if (potValue > maxValues[i]) maxValues[i] = potValue;
      if (potValue < minValues[i]) minValues[i] = potValue;
    }
    delay(5);
  }

  // Final da primeira fase
  beep(100, 1);
  delay(1000);

  Serial.println("FASE 2: Mantenha os sticks na posicao CENTRAL por 5 segundos.");
  
  unsigned long centerCalibrationTime = millis();
  long centerReadings[NUM_ANALOG_CHANNELS] = {0};
  int readingCount = 0;

  while(millis() - centerCalibrationTime < 5000) {
    for (int i = 0; i < NUM_ANALOG_CHANNELS; i++) {
      centerReadings[i] += analogRead(potPins[i]);
    }
    readingCount++;
    delay(5);
  }

  // Calcula a média para o centro
  for (int i = 0; i < NUM_ANALOG_CHANNELS; i++) {
    centerValues[i] = centerReadings[i] / readingCount;
  }
  
  // Sinaliza o fim da calibração com dois bipes
  Serial.println("Calibracao Concluida.");
  beep(100, 2);

  // Salva na memoria flash
  prefs.begin("calib", false);
  for(int i = 0; i < NUM_ANALOG_CHANNELS; i++) {
    prefs.putInt(("min" + String(i)).c_str(), minValues[i]);
    prefs.putInt(("max" + String(i)).c_str(), maxValues[i]);
    prefs.putInt(("center" + String(i)).c_str(), centerValues[i]);
  }
  prefs.putBool("calibrated", true);
  prefs.end();
}

void apagarflash(){
  prefs.begin("calib", false); // Abre o namespace 'calib' em modo de escrita
  prefs.clear();             // Limpa todos os dados de calibração
  prefs.end();               // Fecha a sessão da preferência
  Serial.println("Calibracao anterior apagada da memoria flash.");
  delay(2000); // Aguarda para que a mensagem seja visível
}

void setup() {
  Serial.begin(115200);

  // Define os pinos dos potenciômetros como entrada
  for (int i=0; i < NUM_ANALOG_CHANNELS; i++) {
    pinMode(potPins[i], INPUT);
  }

  // Define os pinos auxiliares como entrada
  pinMode(PIN_AUX1, INPUT_PULLUP);
  pinMode(PIN_AUX2, INPUT_PULLUP);
  pinMode(PIN_AUX3a, INPUT_PULLUP);
  pinMode(PIN_AUX3b, INPUT_PULLUP);
  pinMode(PIN_AUX4, INPUT_PULLUP);

  // Configura o pino do buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  //apagarflash();
  tx.begin(true);

  // Verifica se já existe uma calibração salva
  prefs.begin("calib", true);
  bool isCalibrated = prefs.getBool("calibrated", false);
  prefs.end();

  if (isCalibrated) {
    Serial.println("Calibracao carregada da Flash.");
    prefs.begin("calib", true);
    for (int i = 0; i < NUM_ANALOG_CHANNELS; i++) {
      minValues[i] = prefs.getInt(("min" + String(i)).c_str());
      maxValues[i] = prefs.getInt(("max" + String(i)).c_str());
      centerValues[i] = prefs.getInt(("center" + String(i)).c_str());
    }
    prefs.end();
    playMarioTune();
  } else {
    // Executa a calibração se não for a primeira vez
    calibrate();
  }

  // Inicializa os valores filtrados com o centro
  for (int i = 0; i < NUM_ANALOG_CHANNELS + NUM_AUX_CHANNELS; i++) {
    filteredValues[i] = 1500;
  }
}

void loop() {
  uint32_t now = millis();
  static uint32_t lastSent = 0;

  // Envia os canais em um intervalo fixo
  if (now - lastSent >= SEND_INTERVAL_MS) {
    lastSent = now;
    
    // Leitura e mapeamento dos canais analógicos
    for (int i = 0; i < NUM_ANALOG_CHANNELS; i++) {
      int rawValue = analogRead(potPins[i]);
      
      int mappedValue;
      if (rawValue < centerValues[i]) {
        mappedValue = map(rawValue, minValues[i], centerValues[i], 1000, 1500);
      } else {
        mappedValue = map(rawValue, centerValues[i], maxValues[i], 1500, 2000);
      }

      // Aplica o filtro exponencial
      filteredValues[i] = alpha * mappedValue + (1 - alpha) * filteredValues[i];

      // Garante que o valor esteja no range correto e envia
      int finalValue = constrain(round(filteredValues[i]), 1000, 2000);
      tx.setChannel(i, finalValue);
    }

    // Leitura e mapeamento dos canais auxiliares
    int aux1Raw = digitalRead(PIN_AUX1);
    int aux2Raw = !digitalRead(PIN_AUX2);
    int aux3a = digitalRead(PIN_AUX3a);
    int aux3b = digitalRead(PIN_AUX3b);
    int aux4Raw = digitalRead(PIN_AUX4);
    
    int auxValues[NUM_AUX_CHANNELS];
    auxValues[0] = aux1Raw == 0 ? 1000 : 2000;
    auxValues[1] = aux2Raw == 0 ? 1000 : 2000;
    
    // Mapeamento da chave de 3 posições (AUX3)
    if (aux3a == 0 && aux3b == 1) {
      auxValues[2] = 1000;
    } else if (aux3a == 0 && aux3b == 0) {
      auxValues[2] = 1500;
    } else if (aux3a == 1 && aux3b == 0) {
      auxValues[2] = 2000;
    } else {
      auxValues[2] = 1500; // Valor fallback
    }

    // Mapeamento da chave de 2 posições (AUX4)
    auxValues[3] = aux4Raw == 0 ? 1000 : 2000;

    for (int i = 0; i < NUM_AUX_CHANNELS; i++) {
      filteredValues[NUM_ANALOG_CHANNELS + i] = alpha * auxValues[i] + (1 - alpha) * filteredValues[NUM_ANALOG_CHANNELS + i];
      int finalValue = constrain(round(filteredValues[NUM_ANALOG_CHANNELS + i]), 1000, 2000);
      tx.setChannel(NUM_ANALOG_CHANNELS + i, finalValue);
    }
    
    // Confirma e envia os canais
    tx.commit();
  }

  tx.update();
}