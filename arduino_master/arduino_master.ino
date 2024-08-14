#include <SoftwareSerial.h>



#define RX_PIN 0  // RO connected to D0 (Hardware RX)
#define TX_PIN 1  // DI connected to D1 (Hardware TX)
#define DE_RE_PIN 2  // DE and RE connected to D2

SoftwareSerial rs485Serial(RX_PIN, TX_PIN); // Initialize SoftwareSerial
String command = "";

void setup() {
  // pinMode(DE_RE_PIN, OUTPUT);...................
  digitalWrite(DE_RE_PIN, LOW); // Set DE and RE low to enable receiver
  pinMode(TdsSensorPin,INPUT);

  Serial.begin(9600);  // Serial monitor
  rs485Serial.begin(9600);  // Software serial for RS485
}

void sendCommand(String command) {
  digitalWrite(DE_RE_PIN, HIGH); // Enable transmitter
  delay(10); // Short delay to allow DE/RE to settle
  rs485Serial.println(command);
  rs485Serial.flush();
  digitalWrite(DE_RE_PIN, LOW); // Disable transmitter
}

void loop() {
  if (Serial.available()) {
    command = Serial.readStringUntil('\n'); // Read command from Serial Monitor
    sendCommand(command);

    delay(100); // Wait for a response

    Serial.println(command);
    while (rs485Serial.available()) {
      String response = rs485Serial.readStringUntil('\n');
      Serial.println("Received: " + response);
    }

  }

}

float valueTds(){
  static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT) 
         analogBufferIndex = 0;
   }   
   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > 800U)
   {
      printTimepoint = millis();
      for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
        analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
      tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value

      if(tdsValue != 0){
        tdsValue += 20;
      }

      return tdsValue;
   }
}

int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
      for (i = 0; i < iFilterLen - j - 1; i++) 
          {
        if (bTab[i] > bTab[i + 1]) 
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}
