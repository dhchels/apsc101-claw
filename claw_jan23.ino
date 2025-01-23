#include <NewPing.h>
#include <Servo.h>

// Cases 
#define INITIAL 0
#define PICKUP 1
#define DROP 3 


// Time in ms for certain delays
#define PICKUP_BUFFER 5000
#define DROP_BUFFER 5000

// Angles in degrees for the servo - Change as needed
#define OPEN 180
#define CLOSE 0

// Pin setup for sonar
#define VCC_PIN 13         
#define TRIGGER_PIN 12    
#define ECHO_PIN 11      
#define GROUND_PIN 10

// Pin setup for servo
#define SERVO 9

// Distances
#define DISTANCE_SIZE 6

// Claw distance thresholds - Change as needed
#define PICKUP_THRESHOLD 14
#define DROP_THRESHOLD 25
#define MAX_DISTANCE 200
#define MIN_DISTANCE 0 

// More sonar and servo setup
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
Servo myservo;

// Global variables
int distances[DISTANCE_SIZE] = {0};
int state = INITIAL;
unsigned long thresholdTime = 0;
int initialCount = 0;

void setup() {
  Serial.begin(9600);

  // Sonar & servo setup
  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(GROUND_PIN, OUTPUT);
  pinMode(VCC_PIN, OUTPUT);
  digitalWrite(GROUND_PIN,LOW);
  digitalWrite(VCC_PIN, HIGH);
  myservo.attach(SERVO);
  
  // Set claw to be open initially (if it isn't already)
  myservo.write(OPEN);
}

void loop() {
  // Get current distance and time
  int currentDistance = moving_avg(sonar.ping_cm());
  Serial.println(currentDistance);
  unsigned long currentTime = millis();

  // Goes through conditions if buffer time has passed
  if(currentTime >= thresholdTime){
    switch(state){
      // Fills array initially for moving avg function to be set up 
      case INITIAL:
        initialCount++;
        if(initialCount >= DISTANCE_SIZE){
          state = PICKUP;
          Serial.println("Done calibrating");
        }
        break;

      // Picks up the object if within a certain distance to the ground
      case PICKUP:
        if(currentDistance <= PICKUP_THRESHOLD){
          myservo.write(CLOSE);
          thresholdTime = currentTime + PICKUP_BUFFER;
          state = DROP;
          Serial.println("Picking up");
        }
        break;

      // Drops the object if within a certain distance to the ground
      case DROP:
        if(currentDistance <= DROP_THRESHOLD){
          myservo.write(OPEN);
          thresholdTime = currentTime + DROP_BUFFER;
          state = PICKUP;
          Serial.println("Dropping");
        }
        break;
    }
  }
  
  // tiny delay to make code not run every literal ms
  delay(50);
}

// Moving average function, prevents false positives & reduce noise
int moving_avg(int val){
  
  // If the distance is 0, that means it is out of the servo's range: set to its max distance
  if(val == 0){
    val = 200;
  }
  Serial.println(val);

  // Get the sum of DISTANCE_SIZE samples. 
  int average = val;

  for(int i = 1; i < DISTANCE_SIZE; i++){
    average += distances[i];
    distances[i-1] = distances[i];
  }

  distances[DISTANCE_SIZE - 1] = val;

  return (float) average / DISTANCE_SIZE;
}
