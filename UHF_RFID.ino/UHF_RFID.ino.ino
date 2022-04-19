#include <M5Stack.h>
#include "RFID_command.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define HALL 36

UHF_RFID RFID;

const char *ssid = "Van den Eede";
const char *password = "a123456789";

String comd = " ";
CardpropertiesInfo card;
ManyInfo cards;
SelectInfo Select;
CardInformationInfo Cardinformation;
QueryInfo Query;
ReadInfo Read;
TestInfo Test;

String RFIDs = "";

void setup() {
    M5.begin();

    RFID._debug = 1;
    Serial2.begin(115200, SERIAL_8N1, 16, 17);//16.17
    if (RFID._debug == 1)Serial.begin(115200, SERIAL_8N1, 21, 22);
    M5.Lcd.fillRect(0, 0, 340, 280, BLACK);

    RFID.Set_transmission_Power(1200);
    RFID.Set_the_Select_mode();
    RFID.Delay(100);
    RFID.Readcallback();
    RFID.clean_data();

    pinMode(HALL, INPUT);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        M5.Lcd.printf("Connecting to WiFi.\n");
        delay(1000);
    }
}

void loop() {
    M5.Lcd.fillCircle(310, 10, 6, GREEN);
    RFID.Delay(150);
    M5.Lcd.fillCircle(310, 10, 6, BLACK);
    RFID.Delay(150);

    if (!digitalRead(HALL)) {
        RFIDs = "RFIDs=[";
        int counter = 0;
        while ((cards = RFID.Multiple_polling_instructions(10)).len == 0 && counter < 5) {
            RFID.Delay(300);
            counter++;
        }

        if (cards.len > 0) {
            for (size_t i = 0; i < cards.len; i++) {
                RFIDs += "\"" + cards.card[i]._EPC + "\",";
                M5.Lcd.drawString(cards.card[i]._RSSI, 200, 5 + i * 15, 2);
                M5.Lcd.drawString(cards.card[i]._PC, 230, 5 + i * 15, 2);
                M5.Lcd.drawString(cards.card[i]._EPC, 0, 5 + i * 15, 2);
                M5.Lcd.drawString(cards.card[i]._CRC, 280, 5 + i * 15, 2);
            }
            RFIDs = RFIDs.substring(0, RFIDs.length() - 1);
            RFIDs += "]";
            M5.Lcd.printf("%s\n", RFIDs.c_str());
            RFID.Delay(1000);
            RFID.clean_data();

            httpRequest();
        }

        //  M5.Lcd.fillRect(0, 0, 340, 280, BLACK);
        while (!digitalRead(HALL)) {
            M5.Lcd.fillCircle(310, 10, 6, RED);
            RFID.Delay(150);
            M5.Lcd.fillCircle(310, 10, 6, BLACK);
            RFID.Delay(150);
        }
        M5.Lcd.fillRect(0, 0, 340, 280, BLACK);

    }
}

void httpRequest() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        http.begin("http://backpack.cvdeede.be/api/set_contents?" + RFIDs);
        int httpCode = http.POST("");

        if (httpCode == 202) {
            M5.Lcd.printf("POST Success!\n");
            http.begin("http://backpack.cvdeede.be/api/missing");
            httpCode = http.GET();
            if (httpCode == 204) {
                M5.Lcd.fillRect(0, 0, 340, 280, GREEN);
            } else {
                M5.Lcd.fillRect(0, 0, 340, 280, RED);
            }
        } else {
            M5.Lcd.printf("POST failed!\n");
        }

        http.end();
    }
}
