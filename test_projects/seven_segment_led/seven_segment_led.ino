#include <SoftwareSerial.h>

// Pin 1 on LED MATRIX IS THE BOTTOM LEFT
// Pin 5 on LED MATRIX IS THE BOTTOM RIGHT
// Pin 6 on LED MATRIX IS THE TOP RIGHT
// Pin 10 on LED MATRIX IS THE TOP LEFT

//Arduino Pin 2 to Pin 9 on LED Matrix.
//Arduino Pin 3 to Pin 10.
//Arduino Pin 4 to Pin 4.
//Arduino Pin 5 to Pin 2..
//Arduino Pin 6 to Pin 1.
//Arduino Pin 8 to Pin 7.
//Arduino Pin 9 to Pin 6.

//GND to Pin 3 and Pin 8 each connected with 220 ohm resistors.

int a = 2;  //For displaying segment "a"
int b = 3;  //For displaying segment "b"
int c = 4;  //For displaying segment "c"
int d = 5;  //For displaying segment "d"
int e = 6;  //For displaying segment "e"
int f = 8;  //For displaying segment "f"
int g = 9;  //For displaying segment "g"

void write_this(int i){
  switch(i){
    case 0:
      digitalWrite(a,HIGH);
      digitalWrite(c,HIGH);
      digitalWrite(d,HIGH);
      digitalWrite(e,HIGH);
      digitalWrite(f,HIGH);
      digitalWrite(g,HIGH);
      break;

    case 1:
      digitalWrite(g, HIGH);
      digitalWrite(c, HIGH);
      break;

    case 2:
      digitalWrite(f, HIGH);
      digitalWrite(g, HIGH);
      digitalWrite(b, HIGH);
      digitalWrite(d, HIGH);
      digitalWrite(e, HIGH);
      break;

   case 3:
      digitalWrite(f, HIGH);
      digitalWrite(g, HIGH);
      digitalWrite(b, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(d, HIGH);
      break;

   case 4:
      digitalWrite(a, HIGH);
      digitalWrite(b, HIGH);
      digitalWrite(g, HIGH);
      digitalWrite(c, HIGH);
      break;

   case 5:
      digitalWrite(f, HIGH);
      digitalWrite(a, HIGH);
      digitalWrite(b, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(d, HIGH);
      break;

  case 6:
      digitalWrite(f, HIGH);
      digitalWrite(a, HIGH);
      digitalWrite(b, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(d, HIGH);
      digitalWrite(e, HIGH);
      break;

  case 7:
      digitalWrite(f, HIGH);
      digitalWrite(g, HIGH);
      digitalWrite(c, HIGH);
      break;

  case 8:
      digitalWrite(a,HIGH);
      digitalWrite(b,HIGH);
      digitalWrite(c,HIGH);
      digitalWrite(d,HIGH);
      digitalWrite(e,HIGH);
      digitalWrite(f,HIGH);
      digitalWrite(g,HIGH);
      break;

  case 9:
      digitalWrite(a,HIGH);
      digitalWrite(b,HIGH);
      digitalWrite(c,HIGH);
      digitalWrite(d,HIGH);
      digitalWrite(f,HIGH);
      digitalWrite(g,HIGH);
      break;
  }
}


void setup() {
  Serial.begin(9600);
  pinMode(a, OUTPUT);  //A
  pinMode(b, OUTPUT);  //B
  pinMode(c, OUTPUT);  //C
  pinMode(d, OUTPUT);  //D
  pinMode(e, OUTPUT);  //E
  pinMode(f, OUTPUT);  //F
  pinMode(g, OUTPUT);  //G
}



void turnOff() {
  digitalWrite(a,LOW);
  digitalWrite(b,LOW);
  digitalWrite(c,LOW);
  digitalWrite(d,LOW);
  digitalWrite(e,LOW);
  digitalWrite(f,LOW);
  digitalWrite(g,LOW);
}

void loop() {
  for(int i; i < 10; i++){
    write_this(i);
    delay(999);
    turnOff();
  }
}
