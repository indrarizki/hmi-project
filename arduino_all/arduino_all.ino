#include <SoftwareSerial.h>
#include <MQUnifiedsensor.h>

#define RX_PIN 0  // RO connected to D0 (Hardware RX)
#define TX_PIN 1  // DI connected to D1 (Hardware TX)
#define DE_RE_PIN 2  // DE and RE connected to D2


// MQ4
#define         Board                   ("Arduino UNO")
#define         Pin                     (A1)  
#define         Type                    ("MQ-4")
#define         Voltage_Resolution      (5)
#define         ADC_Bit_Resolution      (10) 
#define         RatioMQ4CleanAir        (4.4)

// TDS Sensor
#define TdsSensorPin A2
#define VREF 5.0
#define SCOUNT  30
int analogBuffer[SCOUNT];  
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25;
// END TDS

float tds=0,mq4=0,ph=0,mq2=0,turbidity=0;

MQUnifiedsensor MQ4(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);

SoftwareSerial rs485Serial(RX_PIN, TX_PIN);

String command = "";

void setup() {
  Serial.begin(9600); 
  rs485Serial.begin(9600);

  digitalWrite(DE_RE_PIN, LOW);
  pinMode(TdsSensorPin,INPUT);
  MQ4.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ4.init();

  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ4.update();
    calcR0 += MQ4.calibrate(RatioMQ4CleanAir);
    Serial.print(".");
  }
  MQ4.setR0(calcR0/10);

  if(isinf(calcR0)) {Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);} 

}

void sendCommand(String command) {
  digitalWrite(DE_RE_PIN, HIGH);
  delay(10);
  rs485Serial.println(command);
  rs485Serial.flush();
  digitalWrite(DE_RE_PIN, LOW); 
}

void loop() {
  tds = valueTds();
  mq4 = valueMq4();
  command = "";
  sendCommand(command);
  // if (Serial.available()) {
  //   command = Serial.readStringUntil('\n'); // Read command from Serial Monitor

    // delay(100); // Wait for a response

    // Serial.println(command);
    // while (rs485Serial.available()) {
    //   String response = rs485Serial.readStringUntil('\n');
    //   Serial.println("Received: " + response);
    // }

  }

float valueMq4(){
  MQ4.update(); 
  MQ4.setA(60000000000); MQ4.setB(-14.01);
  float Alcohol = MQ4.readSensor();

  return alcohol
}

float valueTds(){
  static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 40U)
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);
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
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0;
      float compensationCoefficient=1.0+0.02*(temperature-25.0);
      float compensationVolatge=averageVoltage/compensationCoefficient;
      tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5;

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
