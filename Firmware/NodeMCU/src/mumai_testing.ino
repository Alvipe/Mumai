/* This code is for testing the connection of a NodeMCU (ESP8266-based board) with a LTC2440 (24-bit ADC)
and for testing different ways of sending data wirelessly to improve transmission time.
Data is received at the PC through a PuTTY terminal connected to the IP address of the NodeMCU.*/

#include <ESP8266WiFi.h> // https://github.com/esp8266/Arduino
#include <SPI.h>

// With the union data type, the EMG data is stored as a float and as a byte array simultaneously
static const int dataSize = sizeof(float);
typedef union {
    float floating;
    uint8_t binary[dataSize];
} binaryFloat;

// WiFi parameters
const char* ssid = "yourssid";
const char* password = "yourpassword";
WiFiServer server(80);

// ADC parameters
static const double Vref = 4.54; //ADC external reference voltage
// Set I/O pins used in addition to clock, data in, data out
const byte slaveSelectPin = 5;  // digital pin 5 for /CS
const byte resetPin = 16;  // digital pin 16 for /RESET

/* SpiRead code by John Beale
(taken and adapted from http://www.steveluce.com/24bits/Applications%20of%20the%20LTC%2024%20bit%20ADC.html) */
float SpiRead(void) {
    long result = 0;
    float v = 0;
    byte sig = 0;
    byte b;

    digitalWrite(slaveSelectPin,LOW);   // take the SS pin low to select the chip
    delayMicroseconds(1);              // probably not needed, only need 25 nsec delay

    b = SPI.transfer(0xff); // B3
    if ((b & 0x20) ==0) sig=1; // is input negative?
    b &=0x1f; // discard bits 29, 30, 31
    result = b;
    result <<= 8;
    b = SPI.transfer(0xff); // B2
    result |= b;
    result = result<<8;
    b = SPI.transfer(0xff); // B1
    result |= b;
    result = result<<8;
    b = SPI.transfer(0xff); // B0
    result |= b;

    digitalWrite(slaveSelectPin,HIGH); // take the SS pin high to de-select the chip

    if (sig) result |= 0xf0000000; // if input is negative, insert sign bit (0xf0.. or 0xe0... ?)
    result = result>>4; // truncate lowest 4 bits
    v = result*(Vref/2)/16777216; //Vadc*(Vref/2)/2^24
    return(v);
}

void setup()
{
    Serial.begin(115200);
    pinMode (slaveSelectPin, OUTPUT);
    pinMode (resetPin, OUTPUT);
    digitalWrite(resetPin,HIGH);  // reset is active low

    // Configure SPI
    SPI.begin(); // initialize SPI, covering MOSI,MISO,SCK signals
    SPI.setBitOrder(MSBFIRST);  // data is clocked in MSB first
    SPI.setDataMode(SPI_MODE0);  // SCLK idle low (CPOL=0), MOSI read on rising edge (CPHI=0)
    SPI.setClockDivider(SPI_CLOCK_DIV16);  // SPI_CLOCK_DIV16 gives me a 1.0 MHz SPI clock, with 16 MHz crystal on Arduino

    // Connect to WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    server.begin();
    Serial.println("");
    Serial.println("WiFi connected");

    // Print some information of the ESP8266 module:
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
    Serial.print("ESP8266 ID: ");
    Serial.println(ESP.getChipId());
}

void loop()
{
    binaryFloat data;
    data.floating = 1.2345;
    uint8_t dataBuffer[400];
    for(int j=0;j<=400-dataSize;j=j+dataSize) {
        dataBuffer[j] = data.binary[0];
        dataBuffer[j+1] = data.binary[1];
        dataBuffer[j+2] = data.binary[2];
        dataBuffer[j+3] = data.binary[3];
    }
    unsigned long tictocs = 0;

    WiFiClient client = server.available();
    client.setNoDelay(1);
    while(client) {
        if(client.connected()) {
            Serial.println("Connected to client");
            for(int i=0;i<1000;i++) {
                // data.floating = SpiRead();
                unsigned long tic = micros();
                // client.println(data.floating,4);
                // client.write(&data.binary[0],4);
                client.write(&dataBuffer[0],400);
                unsigned long toc = micros();
                tictocs += toc - tic;
            }
            unsigned long meanTime = tictocs/1000;
            Serial.println(meanTime);
            tictocs = 0;
        }
        else {
            Serial.println("Client disconnected");
            client.stop();
        }
    }
    Serial.println("No client");
}
