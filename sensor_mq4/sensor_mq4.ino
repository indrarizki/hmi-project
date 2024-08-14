#include <MQUnifiedsensor.h>

#define         Board                   ("Arduino UNO")
#define         Pin                     (A1)  
#define         Type                    ("MQ-4")
#define         Voltage_Resolution      (5)
#define         ADC_Bit_Resolution      (10) 
#define         RatioMQ4CleanAir        (4.4) 

MQUnifiedsensor MQ4(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);

void setup() {

  Serial.begin(9600); //Init serial port
  MQ4.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ4.init(); 

  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ4.update();
    calcR0 += MQ4.calibrate(RatioMQ4CleanAir);
    Serial.print(".");
  }
  MQ4.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 

  Serial.println("*************** Values from MQ-4 **********************");
  Serial.println("|    LPG   |  CH4 |   CO    |    Alcohol    |   Smoke    |");  
}

void loop() {
  MQ4.update(); 
  
  /*
    Exponential regression:
  Gas    | a      | b
  LPG    | 3811.9 | -3.113
  CH4    | 1012.7 | -2.786
  CO     | 200000000000000 | -19.05
  Alcohol| 60000000000 | -14.01
  smoke  | 30000000 | -8.308
  */
  MQ4.setA(3811.9); MQ4.setB(-3.113); // Configure the equation to to calculate CH4 concentration
  float LPG = MQ4.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  
  MQ4.setA(1012.7); MQ4.setB(-2.786); // Configure the equation to to calculate CH4 concentration
  float CH4 = MQ4.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  
  MQ4.setA(200000000000000); MQ4.setB(-19.05); // Configure the equation to to calculate CH4 concentration
  float CO = MQ4.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  
  MQ4.setA(60000000000); MQ4.setB(-14.01); // Configure the equation to to calculate CH4 concentration
  float Alcohol = MQ4.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  
  MQ4.setA(30000000); MQ4.setB(-8.308); // Configure the equation to to calculate CH4 concentration
  float Smoke = MQ4.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  
  Serial.print("|    "); Serial.print(LPG);
  Serial.print("    |    "); Serial.print(CH4);
  Serial.print("    |    "); Serial.print(CO);
  Serial.print("    |    "); Serial.print(Alcohol);
  Serial.print("    |    "); Serial.print(Smoke);
  Serial.println("    |");

  delay(1000); //Sampling frequency
}