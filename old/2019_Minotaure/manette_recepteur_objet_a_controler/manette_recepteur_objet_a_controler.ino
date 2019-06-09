#define BUFFER_SIZE 8
#define A 7 //addr 0
#define B 4 //addr 0
#define X 6 //addr 0
#define Y 5 //addr 0
#define WEST 4 //addr 1
#define EAST 6 //addr 1
#define NORTH 5 //addr 1
#define SOUTH 7 //addr 1
#define LB 2 //addr 0
#define RB 3 //addr 0
#define LS 4 //addr 1
#define RS 3 //addr 1
#define START 1 //addr 0
#define SELECT 0 //addr 0

uint8_t yolo=0;
byte dataBuffer[BUFFER_SIZE];

void setup() {
  // put your setup code here, to run once:
  //pinMode(3,INPUT_PULLUP);
  //pinMode(2,OUTPUT);
  //Serial.begin(115200);
  Serial3.begin(115200,SERIAL_8N1);
  //Serial2.begin(115200);
  pinMode(2, OUTPUT);
  pinMode(5,OUTPUT);
  //digitalWrite(2,HIGH);
}

void loop() {
  yolo++;
  // put your main code here, to run repeatedly:
  uint8_t dataBufferWrite[2] = {127, yolo}; // two feedback bytes, as per the protocol
  Serial3.readBytes(dataBuffer,BUFFER_SIZE); //controller input data
  for (int i = 0; i < BUFFER_SIZE; i++) {
    //Serial.print(dataBuffer[i]); //prints controller data
    //Serial.print(" ");
  }
  if (bitRead(dataBuffer[0], A)||bitRead(dataBuffer[1], NORTH)) {
    digitalWrite(2, HIGH); //lights the LED if the button assigned to number 2 is pressed
  }
  else {
    digitalWrite(2, LOW);
  }
  if(bitRead(dataBuffer[0],LS)){
    digitalWrite(5,HIGH);
  }
  
  ////Serial.print(millis());
  //Serial.println("");
  Serial3.write(dataBufferWrite, 2);
  for (int i = 0; i<2; i++) {
    //Serial.println(dataBufferWrite[i]);
  }
  /*if (!digitalRead(3)) {
    reset();
  }//*/
}

/*void reset() {
  digitalWrite(2,LOW);
  delay(100);
  digitalWrite(2,HIGH);
}//*/
