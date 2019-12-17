#include <ESP32_Servo.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "Perceptron.h"

#define FIREBASE_HOST "kien31051999-firebase.firebaseio.com" //Do not include https:// in FIREBASE_HOST
#define FIREBASE_AUTH "Zp2lZjOs8F1KKOzc1M04yvuTaAxIYeSoXUS72ZOD"
#define WIFI_SSID "Maker Hanoi"
#define WIFI_PASSWORD "makerhanoi@123456"

FirebaseData firebaseData;
FirebaseJson json;

TaskHandle_t TaskNN;
TaskHandle_t TaskFirebase;
// LED pins
const int led1 = 2;
const int led2 = 4;

#define LEFT 19
#define RIGHT 18

Servo myservo;

// Possible ADC pins on the ESP32: 0,2,4,12-15,32-39; 34-39 are recommended for analog input
int ADC_Max = 4096;

perceptron pneuPerceptron(3); //fourth is for bias
float guess;
float lastGuess = 0;

void setup() {
  Serial.begin(115200); 
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(LEFT, INPUT);
  pinMode(RIGHT, INPUT);

  randomSeed(37);
  //weight initialization
  pneuPerceptron.randomize();
  trainingData();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  long offline = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
    if (millis() - offline > 3000)
    {
      Serial.println(" OFFLINE");
      break;
    }
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");

  myservo.attach(23, 500, 2400);   // attaches the servo on pin 18 to the servo object
                                         // using SG90 servo min/max of 500us and 2400us
                                         // for MG995 large servo, use 1000us and 2000us,
                                         // which are the defaults, so this line could be
                                         // "myservo.attach(servoPin);"
  myservo.write(91);
  delay(200);
  
  //create a task that will be executed in the TaskNNcode() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    TaskNNcode,   /* Task function. */
                    "TaskNN",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    2,           /* priority of the task */
                    &TaskNN,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 
  //create a task that will be executed in the TaskFirebasecode() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    TaskFirebasecode,   /* Task function. */
                    "TaskFirebase",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &TaskFirebase,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500);

}
//TaskNNcode: NN
void TaskNNcode( void * pvParameters ){
  Serial.print("TaskNN running on core ");
  Serial.println(xPortGetCoreID());
  for(;;){
    Serial.print("Loop running on core ");
    Serial.println(xPortGetCoreID());
    int left = digitalRead(LEFT);
    int right = digitalRead(RIGHT);
    Serial.print(left);
    Serial.print(",");
    Serial.println(right);
    if (not right && not left)
    {
      guess = 0;
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
  } 
}
//TaskFirebasecode: blinks an LED every 700 ms
void TaskFirebasecode( void * pvParameters ){
  Serial.print("TaskFirebase running on core ");
  Serial.println(xPortGetCoreID());
  for(;;){
    if (lastGuess != guess)
    {
      String path = "/Spoon";
      Firebase.pushInt(firebaseData, path, guess);
      delay(50);
    }
    lastGuess = guess;
  }
}

void loop() {

}

void trainingData()
{
  Serial.println();
  long lasttime = millis();
  Serial.print("Tranning data........");

  for (int i = 0; i < 10000; i++)
  {
    pneuPerceptron.inputs[0] = 0;
    pneuPerceptron.inputs[1] = 1; 
    guess = pneuPerceptron.feedForward();
    pneuPerceptron.train(1, guess);
  }

  for (int i = 0; i < 10000; i++)
  {
    pneuPerceptron.inputs[0] = 1;
    pneuPerceptron.inputs[1] = 0; 
    guess = pneuPerceptron.feedForward();
    pneuPerceptron.train(-1, guess);
  }

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
