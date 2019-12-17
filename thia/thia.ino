#include <ESP32_Servo.h>
#include "Perceptron.h"

#define LEFT 19
#define RIGHT 18

Servo myservo;

// Possible ADC pins on the ESP32: 0,2,4,12-15,32-39; 34-39 are recommended for analog input
int ADC_Max = 4096;

perceptron pneuPerceptron(3); //fourth is for bias
float guess;
  

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LEFT, INPUT);
  pinMode(RIGHT, INPUT);
  Serial.begin(115200);

  randomSeed(37);
  //weight initialization
  pneuPerceptron.randomize();
  trainingData();
  
  myservo.attach(23, 500, 2400);   // attaches the servo on pin 18 to the servo object
                                         // using SG90 servo min/max of 500us and 2400us
                                         // for MG995 large servo, use 1000us and 2000us,
                                         // which are the defaults, so this line could be
                                         // "myservo.attach(servoPin);"
  myservo.write(91);
  delay(200);
  delay(1000);
}

void loop() {
  int left = digitalRead(LEFT);
  int right = digitalRead(RIGHT);
  Serial.print(left);
  Serial.print(",");
  Serial.println(right);
  if (not right && not left)
  {
    myservo.write(91);
    delay(500);
  }
  else
  {
    pneuPerceptron.inputs[0] = left;
    pneuPerceptron.inputs[1] = right; 
    guess = pneuPerceptron.feedForward();
    Serial.println(guess);
    if (guess == 1)
    {
      myservo.write(75);
    }
    else if (guess == -1)
    {
      myservo.write(98);
    }
    delay(500);
  }
  
//  if (not right && not left)
//  {
//    myservo.write(91);
//  }
//  if (right && not left)
//  {
//    myservo.write(75);
//  }
//  if (left && not right)
//  {
//    myservo.write(98);
//  }
//  delay(500);

}

void trainingData()
{
  Serial.println();
  long lasttime = millis();
  Serial.print("Tranning data........");

  for (int i = 0; i < 20000; i++)
  {
    pneuPerceptron.inputs[0] = 0;
    pneuPerceptron.inputs[1] = 1; 
    guess = pneuPerceptron.feedForward();
    pneuPerceptron.train(1, guess);
  }

  for (int i = 0; i < 20000; i++)
  {
    pneuPerceptron.inputs[0] = 1;
    pneuPerceptron.inputs[1] = 0; 
    guess = pneuPerceptron.feedForward();
    pneuPerceptron.train(-1, guess);
  }

//  for (int i = 0; i < 20000; i++)
//  {
//    pneuPerceptron.inputs[0] = 0;
//    pneuPerceptron.inputs[1] = 0; 
//    guess = pneuPerceptron.feedForward();
//    pneuPerceptron.train(0, guess);
//  }

  Serial.println("Done");
  Serial.print("Training time: ");
  Serial.print(millis() - lasttime);
  Serial.println(" ms");
  Serial.print("weights[0]: ");
  Serial.println(pneuPerceptron.weights[0]);
  Serial.print("weights[1]: ");
  Serial.println(pneuPerceptron.weights[1]);
  Serial.println("------------------------------------------");
}

