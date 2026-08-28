
#include <arduino_lmic.h>
#include <arduino_lmic_hal_boards.h>
#include <arduino_lmic_hal_configuration.h>
#include <arduino_lmic_lorawan_compliance.h>
#include <arduino_lmic_user_configuration.h>


/*
 * ============================================================
 * Heltec WiFi LoRa 32 V2
 * ESP32 + SX1276
 *
 * LoRaWAN OTAA
 * Region: EU868
 * Class: A
 *
 * TEST 1:
 *   Heltec -> Join Request -> wAP LR8 -> ChirpStack
 *
 * Ora utilizza la UART del KC868.
 * ============================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <lmic.h>
#include <hal/hal.h>


// Display Oled

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define VEXT_PIN 21
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define OLED_ADDR 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64


Adafruit_SSD1306 display(
  OLED_WIDTH,
  OLED_HEIGHT,
  &Wire,
  OLED_RST
);


// ============================================================
// LoRaWAN OTAA
// ============================================================

// DevEUI ChirpStack:
// 60 07 27 bc ca 89 0c d5
//
// LMIC: byte order invertito
//static const u1_t PROGMEM DEVEUI[8] = {
//  0xD5, 0x0C, 0x89, 0xCA,
//  0xBC, 0x27, 0x07, 0x60
//};
// 8d f0 76 41 92 dd 29 73

static const u1_t PROGMEM DEVEUI[8] = {
  0x73, 0x29, 0xDD, 0x92,
  0x41, 0x76, 0xF0, 0x8D
};

// JoinEUI ChirpStack:
// d8 6b 08 5d a4 45 41 4a
//
// LMIC: byte order invertito
//static const u1_t PROGMEM APPEUI[8] = {
//  0x4A, 0x41, 0x45, 0xA4,
//  0x5D, 0x08, 0x6B, 0xD8
//};

// 88 27 37 ca f2 e4 a8 d4

static const u1_t PROGMEM APPEUI[8] = {
  0xD4, 0xA8, 0xE4, 0xF2,
  0xCA, 0x37, 0x27, 0x88
};

// ============================================================
// APPKEY
// ============================================================
//
// INSERIRE QUI LA CHIAVE GENERATA DA CHIRPSTACK.
//
// Esempio:
//
// static const u1_t PROGMEM APPKEY[16] = {
//   0x01, 0x02, 0x03, 0x04,
//   0x05, 0x06, 0x07, 0x08,
//   0x09, 0x0A, 0x0B, 0x0C,
//   0x0D, 0x0E, 0x0F, 0x10
// };
//
// NON invertire i byte dell'AppKey.
//

//static const u1_t PROGMEM APPKEY[16] = {

  // >>> INSERIRE QUI I 16 BYTE DELLA APPKEY <<<

//  0x2C, 0xCD, 0x67, 0x96,
//  0x46, 0xFD, 0x3D, 0xF2,
//  0x11, 0xE0, 0xEB, 0xF5,
//  0x3C, 0x72, 0x67, 0xBA
//};

static const u1_t PROGMEM APPKEY[16] = {

  // >>> INSERIRE QUI I 16 BYTE DELLA APPKEY <<<

// f5 c1 2f e7 ad 7a 42 8e d0 e1 66 d9 f4 6c de 4b

  0xF5, 0xC1, 0x2F, 0xE7,
  0xAD, 0x7A, 0x42, 0x8E,
  0xD0, 0xE1, 0x66, 0xD9,
  0xF4, 0x6C, 0xDE, 0x4B
};

HardwareSerial KC868Serial(2);

uint32_t uplinkCount = 0;
uint32_t downlinkCount = 0;
int lastRSSI = 0;
int lastSNR = 0;
bool oledEnabled = true;
unsigned long oledLastActivity = 0;
const unsigned long OLED_TIMEOUT = 60000UL;  // 60 secondi

// ============================================================
// LMIC callbacks
// ============================================================


void os_getArtEui(u1_t *buf)
{
  memcpy_P(buf, APPEUI, 8);
}


void os_getDevEui(u1_t *buf)
{
  memcpy_P(buf, DEVEUI, 8);
}


void os_getDevKey(u1_t *buf)
{
  memcpy_P(buf, APPKEY, 16);
}


// ============================================================
// Heltec WiFi LoRa 32 V2
//
// SX1276
// ============================================================

const lmic_pinmap lmic_pins = {
  .nss = 18,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 14,
  .dio = {26, 35, 34}
};
//const lmic_pinmap lmic_pins = {
//    .nss = 18,
//    .rxtx = LMIC_UNUSED_PIN,
//    .rst = 14,
//    .dio = {26, LMIC_UNUSED_PIN, LMIC_UNUSED_PIN}
//};

// ============================================================
// Application
// ============================================================

static osjob_t sendjob;

const unsigned TX_INTERVAL = 60; // 0 = Disable


// ============================================================
// Test payload
// ============================================================

void do_send(osjob_t *j)
{
  if (LMIC.opmode & OP_TXRXPEND) {

    Serial.println("LMIC: TX/RX ancora in corso");

    return;
  }


  // Payload di test
  static uint8_t payload[] = {
    0x48, 0x45, 0x4C, 0x4C, 0x4F
  };

  Serial.println("LMIC: invio uplink");


  LMIC_setTxData2(
    1,                  // FPort
    payload,
    sizeof(payload),
    0                   // Unconfirmed
  );


  Serial.println("LMIC: uplink accodato");
}

void oledMainScreen()
{
    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("LoRaWAN: ONLINE");

    display.setCursor(0, 16);
    display.print("UP: ");
    display.print(uplinkCount);

    display.print("  DOWN: ");
    display.println(downlinkCount);

    display.setCursor(0, 32);
    display.print("RSSI: ");
    display.print(lastRSSI);
    display.println(" dBm");

    display.setCursor(0, 48);
    display.print("SNR: ");
    display.print(lastSNR);
    display.println(" dB");

    display.display();
}

void oledMessage(const char* line1, const char* line2 = nullptr)
{
    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println(line1);

    if (line2 != nullptr) {
        display.setCursor(0, 16);
        display.println(line2);
    }

    display.display();
}

void oledOn() {

    if (!oledEnabled) {

        oledEnabled = true;

        display.ssd1306_command(SSD1306_DISPLAYON);

        Serial.println("OLED ON");
    }

    oledLastActivity = millis();
}

void oledOff() {

    if (oledEnabled) {

        oledEnabled = false;

        display.ssd1306_command(SSD1306_DISPLAYOFF);

        Serial.println("OLED OFF");
    }
}

// ============================================================
// LMIC events
// ============================================================

void onEvent(ev_t ev)
{
  Serial.print("LMIC event: ");
  Serial.print((int)ev);
  Serial.print("  ");

  switch (ev) {

    case EV_SCAN_TIMEOUT:

      Serial.println("EV_SCAN_TIMEOUT");

      break;


    case EV_BEACON_FOUND:

      Serial.println("EV_BEACON_FOUND");

      break;


    case EV_BEACON_MISSED:

      Serial.println("EV_BEACON_MISSED");

      break;


    case EV_BEACON_TRACKED:

      Serial.println("EV_BEACON_TRACKED");

      break;


    case EV_JOINING:

      Serial.println("EV_JOINING");

      break;


    case EV_JOINED:

      Serial.println();
      Serial.println("--------------------------------");
      Serial.println("L O R A W A N   J O I N E D");
      Serial.println("--------------------------------");
      Serial.println();
      oledMessage(
        "LoRaWAN",
        "ONLINE"
      );

      // Durante il test non ci interessa
      // il Link Check.
      LMIC_setLinkCheckMode(0);

      // Primo uplink
      do_send(&sendjob);

      break;


    case EV_JOIN_FAILED:

      Serial.println("EV_JOIN_FAILED");

      break;


    case EV_REJOIN_FAILED:

      Serial.println("EV_REJOIN_FAILED");

      break;
    
    
    case EV_TXCOMPLETE:

    Serial.println("LMIC event: 10  EV_TXCOMPLETE");

    // --------------------------------------------------
    // Aggiorna informazioni radio
    // --------------------------------------------------

    uplinkCount++;

    lastRSSI = LMIC.rssi;
    lastSNR  = LMIC.snr;

    Serial.print("LMIC.dataLen = ");
    Serial.println(LMIC.dataLen);

    Serial.print("LMIC.seqnoUp = ");
    Serial.println(LMIC.seqnoUp);

    Serial.print("LMIC.rssi = ");
    Serial.println(LMIC.rssi);

    Serial.print("LMIC.snr = ");
    Serial.println(LMIC.snr);


    // --------------------------------------------------
    // DOWNLINK
    // --------------------------------------------------

    if (LMIC.dataLen > 0) {

        downlinkCount++;

        uint8_t fport = LMIC.frame[LMIC.dataBeg - 1];

        Serial.println();
        Serial.println("--------------------------------");
        Serial.println("DOWNLINK RICEVUTO");
        Serial.println("--------------------------------");

        Serial.print("FPort: ");
        Serial.println(fport);

        Serial.print("Lunghezza: ");
        Serial.print(LMIC.dataLen);
        Serial.println(" byte");

        Serial.print("HEX: ");

        for (uint8_t i = 0; i < LMIC.dataLen; i++) {

            uint8_t b = LMIC.frame[LMIC.dataBeg + i];

            if (b < 0x10)
                Serial.print("0");

            Serial.print(b, HEX);
            Serial.print(" ");
        }

        Serial.println();


        // --------------------------------------------------
        // COMANDO IRRIGAZIONE
        // FPort 10
        // Frame: comando, uscita, tempo MSB, tempo LSB
        // --------------------------------------------------

        if (fport == 10 && LMIC.dataLen == 4) {

            uint8_t comando =
                LMIC.frame[LMIC.dataBeg + 0];

            uint8_t uscita =
                LMIC.frame[LMIC.dataBeg + 1];

            uint16_t tempo =
                ((uint16_t)LMIC.frame[LMIC.dataBeg + 2] << 8) |
                LMIC.frame[LMIC.dataBeg + 3];


            Serial.println();
            Serial.println("COMANDO IRRIGAZIONE");

            Serial.print("Comando: 0x");

            if (comando < 0x10)
                Serial.print("0");

            Serial.println(comando, HEX);

            Serial.print("Uscita: ");
            Serial.println(uscita);

            Serial.print("Tempo: ");
            Serial.print(tempo);
            Serial.println(" secondi");


            // --------------------------------------------------
            // DISPLAY DOWNLINK
            // --------------------------------------------------

            display.clearDisplay();

            display.setTextColor(SSD1306_WHITE);
            display.setTextSize(1);

            display.setCursor(0, 0);
            display.print("DOWNLINK F");
            display.println(fport);

            display.setCursor(0, 16);
            display.print("OUT: ");
            display.println(uscita);

            display.setCursor(0, 32);

            if (comando == 0x01) {
                display.println("CMD: ON");
            }
            else if (comando == 0x02) {
                display.println("CMD: ON PERM");
            }
            else if (comando == 0x03) {
                display.println("CMD: OFF");
            }
            else {
                display.print("CMD: 0x");

                if (comando < 0x10)
                    display.print("0");

                display.println(comando, HEX);
            }

            display.setCursor(0, 48);
            display.print("TIME: ");
            display.print(tempo);
            display.println("s");

            display.display();


            // --------------------------------------------------
            // Inviamo il comando al KC868
            // --------------------------------------------------

            sendToKC868(comando, uscita, tempo);
        }


        // --------------------------------------------------
        // DOWNLINK NON VALIDO
        // --------------------------------------------------

        else {

            Serial.println("Downlink non valido per il protocollo irrigazione.");

        }
    }


    // --------------------------------------------------
    // Se non c'e' downlink:
    // mostra schermata principale
    // --------------------------------------------------

    else {

        oledMainScreen();

    }


    // --------------------------------------------------
    // Programma il prossimo uplink
    // --------------------------------------------------
    
    if (TX_INTERVAL > 0) {
       os_setTimedCallback(
           &sendjob,
           os_getTime() + sec2osticks(TX_INTERVAL),
           do_send
       );
    }

    break;

    case EV_LOST_TSYNC:

      Serial.println("EV_LOST_TSYNC");

      break;


    case EV_RESET:

      Serial.println("EV_RESET");

      break;


    case EV_RXCOMPLETE:

      Serial.println("EV_RXCOMPLETE");

      break;


    case EV_LINK_DEAD:

      Serial.println("EV_LINK_DEAD");

      break;


    case EV_LINK_ALIVE:

      Serial.println("EV_LINK_ALIVE");

      break;


    default:

      Serial.print("evento ");

      Serial.println((unsigned)ev);

      break;
  }
}

void sendToKC868(
    uint8_t comando,
    uint8_t uscita,
    uint16_t tempo
) {
    uint8_t frame[4];

    frame[0] = comando;
    frame[1] = uscita;
    frame[2] = (tempo >> 8) & 0xFF;
    frame[3] = tempo & 0xFF;

    Serial.println();
    Serial.println("Invio comando al KC868:");

    Serial.print("UART HEX: ");

    for (uint8_t i = 0; i < 4; i++) {

        if (frame[i] < 0x10)
            Serial.print("0");

        Serial.print(frame[i], HEX);
        Serial.print(" ");
    }

    Serial.println();

    // Invio reale al KC868
    KC868Serial.write(frame, sizeof(frame));
    KC868Serial.flush();

    Serial.println("Frame UART inviato.");

}

// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);

  delay(3600);


  Serial.println();
  Serial.println();
  Serial.println("========================================");
  Serial.println(" Heltec WiFi LoRa 32 V2");
  Serial.println(" LoRaWAN OTAA");
  Serial.println(" Region: EU868");
  Serial.println(" SX1276");
  Serial.println("========================================");


  // ----------------------------------------------------------
  // SPI Heltec V2
  // ----------------------------------------------------------

  SPI.begin(
    5,      // SCK
    19,     // MISO
    27,     // MOSI
    18      // CS
  );


  Serial.println("SPI OK");

// Porta seriale verso KC868
KC868Serial.begin(
    9600,
    SERIAL_8N1,
    36,   // RX Heltec
    13    // TX Heltec
);

pinMode(0, INPUT_PULLUP); //pusante Prog
pinMode(VEXT_PIN, OUTPUT);
digitalWrite(VEXT_PIN, LOW);

delay(100);

pinMode(OLED_RST, OUTPUT);
digitalWrite(OLED_RST, LOW);
delay(20);
digitalWrite(OLED_RST, HIGH);
delay(20);

Wire.begin(OLED_SDA, OLED_SCL);


if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  Serial.println("OLED non trovato!");
  while (true);
} else {
  Serial.println("OLED OK");
   display.clearDisplay();
   display.setTextColor(SSD1306_WHITE);
   display.setTextSize(1);
   display.setCursor(0, 0);
   display.println("LoRa32 Irrigazione");
   display.setCursor(0, 16);
   display.println("LoRaWAN OTAA");
   display.setCursor(0, 32);
   display.println("EU868");
   display.display();
   // Inizia il conteggio dei 60 secondi
   oledLastActivity = millis();
   oledEnabled = true;
   delay(1500);
}


  // ----------------------------------------------------------
  // LMIC
  // ----------------------------------------------------------

  os_init();

  Serial.println("LMIC inizializzato");


  // Reset dello stato LMIC
  LMIC_reset();

  //LMIC_setClockError(MAX_CLOCK_ERROR * 10 / 100);

  // ADR
  LMIC_setAdrMode(1);


  // Durante il test disabilitiamo LinkCheck
  LMIC_setLinkCheckMode(0);


  // Impostazione iniziale:
  //
  // SF7 / 125 kHz
  // 14 dBm
  //
  // Successivamente lasceremo ADR
  // ottimizzare il datarate.

  //LMIC_setDrTxpow(
  //  DR_SF7,
  //  14
  //);

  Serial.print("LMIC datarate: ");
  Serial.println(LMIC.datarate);

  Serial.println("Configurazione EU868 completata");


  // ----------------------------------------------------------
  // OTAA
  // ----------------------------------------------------------

  Serial.println();
  Serial.println("Avvio OTAA...");
  Serial.println("Attesa Join Request / Join Accept...");
  Serial.println();
  oledMessage(
    "LoRaWAN OTAA",
    "JOINING..."
);


  LMIC_startJoining();
}


// ============================================================
// LOOP
// ============================================================

void loop() {

  os_runloop_once();

  static uint8_t rxBuffer[3];
  static uint8_t rxIndex = 0;

  while (KC868Serial.available()) {

    uint8_t b = KC868Serial.read();

    Serial.printf(
        "KC868 RX: %02X\n",
        b
    );

    rxBuffer[rxIndex++] = b;

    if (rxIndex == 3) {

        Serial.printf(
            "KC868 FRAME: %02X %02X %02X\n",
            rxBuffer[0],
            rxBuffer[1],
            rxBuffer[2]
        );

        // ------------------------------------------
        // ACK stato uscite KC868
        // ------------------------------------------

        if (rxBuffer[0] == 0xA5 &&
            rxBuffer[1] == 0x02) {

            uint8_t stato = rxBuffer[2];

            Serial.printf(
                "STATO KC868: 0x%02X\n",
                stato
            );

            Serial.printf(
                "OUT1=%s OUT2=%s OUT3=%s OUT4=%s\n",
                (stato & 0x01) ? "ON" : "OFF",
                (stato & 0x02) ? "ON" : "OFF",
                (stato & 0x04) ? "ON" : "OFF",
                (stato & 0x08) ? "ON" : "OFF"
            );

            Serial.printf(
                "OUT5=%s OUT6=%s OUT7=%s OUT8=%s\n",
                (stato & 0x10) ? "ON" : "OFF",
                (stato & 0x20) ? "ON" : "OFF",
                (stato & 0x40) ? "ON" : "OFF",
                (stato & 0x80) ? "ON" : "OFF"
            );

            // ------------------------------------------
            // UPLINK LoRaWAN
            // ------------------------------------------

            uint8_t payload[3];

            payload[0] = 0xA5;
            payload[1] = 0x02;
            payload[2] = stato;
            Serial.println();
            
            Serial.println("--------------------------------");
            Serial.println("UPLINK STATO KC868");
            Serial.println("--------------------------------");

            Serial.printf(
               "FPort: 10\n"
            );

            Serial.printf(
                "PAYLOAD: %02X %02X %02X\n",
                payload[0],
                payload[1],
                payload[2]
            );

            Serial.println("--------------------------------");

            LMIC_setTxData2(
               10,
               payload,
               3,
               0
            );
        }

        rxIndex = 0;
    }
  }

  // --------------------------------------------------
  // Pulsante PROG
  // --------------------------------------------------

  static bool progPrevious = HIGH;

  bool progState = digitalRead(0);

  if (progPrevious == HIGH && progState == LOW) {

    Serial.println("PROG premuto");

    oledOn();
  }

  progPrevious = progState;


  // --------------------------------------------------
  // Timeout OLED
  // --------------------------------------------------

  if (oledEnabled &&
    (millis() - oledLastActivity >= OLED_TIMEOUT)) {

    oledOff();
  }
}

