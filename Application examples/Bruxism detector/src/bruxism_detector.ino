#include <MsTimer2.h>

#define EMG_CH1 0
#define buzzerPin 9
#define ledPin 13
#define sampleTime 1
#define adcRef 5.00
#define adcRes 1023

volatile int emg = 0;
float emgMean = 0.00;
float emgMovav = 0.00;
float emgMvc = 0.00;
float adcConv = adcRef/adcRes;
volatile int sOK = 0;
int mode = 0;
unsigned int i = 0, k = 0;

void sampling()
{
    emg = analogRead(EMG_CH1);
    sOK = 1;
}

void meanCalc(unsigned int meanSamples)
{
    while(i<meanSamples)
    {
        delayMicroseconds(50);
        if(sOK==1)
        {
            sOK = 0;
            i++;
            emgMean = emgMean + emg*adcConv;
        }
    }
    i = 0;
    emgMean = emgMean/meanSamples;
}

void movAv()
{
    float emgZero = emg*adcConv - emgMean;
    emgMovav = emgMovav*0.99 + abs(emgZero)*0.01;
}

void mvcCalc(unsigned int mvcSamples)
{
    while(i<mvcSamples)
    {
        delayMicroseconds(50);
        if(sOK==1)
        {
            sOK = 0;
            i++;
            movAv();
            if(emgMovav>emgMvc)  emgMvc = emgMovav;
        }
    }
    i = 0;
    emgMovav = 0.00;
}

void blinkLED(unsigned int repeat,unsigned int bTime)
{
    for(i=0;i<repeat;i++)
    {
        digitalWrite(ledPin,HIGH);
        delay(bTime);
        digitalWrite(ledPin,LOW);
        delay(bTime);
    }
}

void setup() {
    pinMode(EMG_CH1,INPUT);
    pinMode(ledPin,OUTPUT);
    pinMode(buzzerPin,OUTPUT);
    MsTimer2::set(sampleTime, sampling);
    MsTimer2::start();
    Serial.begin(250000);
    delay(5000);

    // System calibration
    blinkLED(1,500);
    meanCalc(30000);
    blinkLED(1,500);

    blinkLED(2,500);
    mvcCalc(5000);
    blinkLED(2,500);
}

void loop() {
    delay(2);
    movAv();
    if(emgMovav > 0.35*emgMvc) {
        digitalWrite(ledPin, HIGH);
        analogWrite(buzzerPin,500);
        delayMicroseconds(1275);
    }
    else {
        digitalWrite(ledPin, LOW);
        analogWrite(buzzerPin,0);
        delayMicroseconds(1275);
    }
    //Serial.println(emgMovav);

}
