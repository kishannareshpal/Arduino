/*||||||||||||||||||||||||||||||
||    Mareógrafo - TRACCAR    ||
||||||||||||||||||||||||||||||*/

#include <GPRS_Shield_Arduino.h>
#include "DHT.h"
#include <SoftwareSerial.h>

// make sure that the baud rate of SIM900 is 9600!
// you can use the AT Command(AT+IPR=9600) to set it through Serial Monitor and use (AT&W) to save
#define BAUDRATE  9600

//----------------- GPRS SHIELD ------------------//
  /* Library: https://github.com/Seeed-Studio/GPRS_SIM900
     Datasheet: _________________________________________ */
#define PIN_TX 7
#define PIN_RX 8

char buffer[512];

boolean received = false;   // Becomes TRUE when a message is received (default value: false).
String sms_code = "NaN";    // The number of
char phone[14];             // buffer for the phone number.
char http_cmd[] = "";
String numb;                // Stores the phone number which sends the sms_code (such as, when verifying balance)
bool sent;                  // true: when data is sent to server succefully. false: when otherwise.

GPRS gprs(PIN_TX, PIN_RX, BAUDRATE);
//----------------- ----------- ------------------//


//-------------------- DHT11 ---------------------//
  /* Library: https://github.com/adafruit/DHT-sensor-library
     Datasheet: https://akizukidenshi.com/download/ds/aosong/DHT11.pdf */

#define DHTPIN 9 // what digital pin DHT is connected to
#define DHTTYPE DHT11

    /*
        •Pin instructions:
          --- FRONT IS WHERE THE HOLES ARE LOCATED. Pin 1 is on the Left and Pin 4 is on the right.
            - Connect pin 1 (on the left) of the sensor to +5V
            - Connect pin 2 of the sensor to whatever your DHTPIN is
            - Connect pin 4 (on the right) of the sensor to GROUND
            - Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor
    */
float t; // temperature
float h; // Humidity

DHT dht(DHTPIN, DHTTYPE);
//----------------- ----------- ------------------//


//-------------------- BAT ULTRASONIC ---------------------//
  /* Wiki: https://www.elecfreaks.com/wiki/index.php?title=BAT
     Model: EF03141*/

#define TRIGGERPIN 10 // what digital pin TRIGGER PIN is connected to
#define ECHOPIN 11 // what digital pin ECHO PIN is connected to

//----------------- ----------- ------------------------//



////////////////////////////////////////////////
//                CUSTOM FUNC.                //
////////////////////////////////////////////////

// Helper function to check if a String is A Number:
boolean isANumber(String s){
    /*
         • Known issues:
         –– Returns TRUE to "a1" (alpha-numeric). (TODO: Return FALSE)
    */
    for(byte i=0; i<s.length(); i++){
      if(isDigit(s.charAt(i))){
        // If it's a numeric:
        return true;
      }
    }
    // if it cotains any letter:
    return false;
}

// Helper function to convert String to Char*
char* string2char(String s){
  if(s.length()!=0){
      char *p = const_cast<char*>(s.c_str());
      return p;
  }
}

// Blinker:
// void blinkplease(int times, int speed = 1000){
//     // Hint: SUCCESS, use times=2
//     // Hint: ERROR, use times=4
//     for(int i=0; i<times; i++){
//           digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
//           delay(speed);                       // wait for a second
//           digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
//           delay(speed);                      // wait for a second
//     }
// }

// Init the GPRS_Shield:
void init_gprs(){
  // Initialize Shield
  // use DHCP
  while(!gprs.init()){
    delay(1000);
    Serial.print("Init error\r\n");
  }
  delay(3000);

  // attempt DHCP
  while(!gprs.join(F("internet"))){ //gprs.join(F(String apn default: cmnet))
    Serial.println("GPRS error");
    delay(2000);
  }

  // successful DHCP
  Serial.print("IP is: ");
  Serial.println(gprs.getIPAddress());
}

// Test the connection by connecting to traccar:
void testTraccar(){
    char server_ip[] = "197.219.3.62";
    int server_port = 5055;

    if(!gprs.connect(TCP, server_ip, server_port)) {
      Serial.println("Error connecting to TRACCAR\n");
    }else{
      Serial.println("Connection to TRACCAR Estab.\n");
    }
}

// Listen to sms:
void listen_SMS(int digits = 19){
    char message[digits];
    int messageIndex = gprs.isSMSunread(); // The number of unread sms's available on the SIM
     if (messageIndex > 0){
      // If there's at least one UNREAD SMS, read it.
      gprs.readSMS(messageIndex, message, digits, phone);
      // Then delete the SMS so the SIM Memory stays freed.
      gprs.deleteSMS(messageIndex);
      // Print the message to SerialMonitor
      Serial.print("\nNew SMS: ");
      Serial.println(message);

        // Check if the message sent to you is numeric...
        if(isANumber(String(message)) && (String(message).length() < 3)){
          // If it's numeric procceed sending it the server ...
          sms_code = String(message);
          numb = String(phone);
          Serial.println("sms-coded");
          received = true;

        } else if(!isANumber(String(message))) {
          // If it's a sequence of letters show an error...
          Serial.print("Error. Expects: Number\r\n\n");

        } else {
            gprs.sendSMS(string2char(numb), message);
            Serial.print("SMS Sent");
            received = false;
        }

     } else {
        // Continue listenning to sms...
        Serial.print("Listenning to SMS\n");
        received = false;
        delay(4000);
     }
};

// Sub Helper for storing all sensors data:
String set_http_header(String data_type, String data_value, String temp_value, String hum_value, String dist_value){
    String h_head = "PUT /?id=944425&lat=-23.868312&lon=35.376653";

    // If we happen to send all of the sensor data at once:
    if((data_type=="") || (data_value=="")){
           h_head += "&temp=";
           h_head += temp_value;
           h_head += "&hum=";
           h_head += hum_value;
           h_head += "&dist=";
           h_head += dist_value;
           h_head += " HTTP/1.0\r\n\r\n";
           return h_head;

    } else { // If we want to send data to server individually
           h_head += "&" + data_type + "=";
           h_head += data_value;
           h_head += " HTTP/1.0\r\n\r\n";
           return h_head;
    }
}

// Helper for sending sensor data to traccar
bool sendData(String data_type, String data_value, String temp_value, String hum_value, String dist_value){

    String http_header = set_http_header(data_type, data_value, temp_value, hum_value, dist_value);
    testTraccar();

    char http_cmd[http_header.length()+1];
    delay(2000);
    http_header.toCharArray(http_cmd, sizeof(http_cmd));
    delay(3000);
    Serial.println("fetching...");
    gprs.send(http_cmd, sizeof(http_cmd)-1);
    delay(3000);
    while (true) {
        int ret = gprs.recv(buffer, sizeof(buffer)-1);
        delay(3000);
        if (ret <= 0){
            Serial.println("done!\r\n");
            break;
        }
        buffer[ret] = '\0';
        Serial.print("Recv: ");
        Serial.print(ret);
        Serial.print(" bytes\r\n");
        Serial.println(buffer);
        delay(5000);
        return 1; // Returns 1 so we know that the HTTP_Request was 200 OK, and send a confirming sms.
    }
    // vaciado del puerto serie.
    Serial.flush(); //Sino, al volver a arrancar tiene bits en el bufer y no lee bien los sensores
    received = false;
};

// Check for balance on the SIM
void checkSaldoVodacom(){
  // To check balance on Vodacom we can send ("S" or "SALDO") to ("84162")
  gprs.sendSMS("84162", "s");
  listen_SMS();
  received = false;
}

// Read and Send Temperature to Traccar
bool read_sendTemp(){
   // Clear the buffer.
   Serial.flush();
   // First read temperature from the sensor:
   t = dht.readTemperature();
   // Send the temperature to traccar
   return sendData("temp", String(t), "", "", "");
}

// Read and Send Humidity to Traccar
bool read_sendHum(){
  // First read humidity from the sensor:
  Serial.flush();
  h = dht.readHumidity();
  return sendData("hum", String(h), "", "", "");
}


//...................................functions related to BAT ULTRASONIC SENSOR

  // To get the length of the pulse in microseconds:
  long getMicroseconds() {
    digitalWrite(TRIGGERPIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGERPIN, HIGH);                 // pull the Trig pin to high level for more than 10us impulse
    delayMicroseconds(10);
    digitalWrite(TRIGGERPIN, LOW);
    long microseconds = pulseIn(ECHOPIN,HIGH);   // waits for the pin to go HIGH, and returns the length of the pulse in microseconds
    return microseconds;                    // return microseconds
  }

  // To get the distance:
  long getDistance(long microseconds, int unit) {
    /*
      IF UNIT = 1 --> Centimeters (CM)
      IF UNIT = 2 --> Inches (")
    */
    long dist;
    if(1){
      dist = microseconds/29/2  ;     /* Distance_CM  = ((Duration of high level)*(Sonic :340m/s))/2
                                             ||       = ((Duration of high level)/(Sonic :29.4cm/µs))/2
                                      */
    } else{
      dist = microseconds/74/2;      // Distance_Inches
    }
    return dist;
  }

//...................................................................

bool read_sendDist(){
  // Get distance in centimeters:
  long dist_CM = getDistance(getMicroseconds(), 1); // (1=cm OR 2=inches) - Distance(µs, unit)
  // Send the distance to traccar:
  return sendData("dist", String(dist_CM), "", "", "");
}

// Send data from all sensors every 1sec, for 10sec
void read_sendAll(){
    // Now, send temp:
    t = dht.readTemperature();
    h = dht.readHumidity();
    long dist_CM = getDistance(getMicroseconds(), 1);
    sent = sendData("", "", String(t), String(h), String(dist_CM));
}



//============================================//
//             DEFAULT FUNCTIONS.             //
//============================================//
void setup(){
    Serial.begin(BAUDRATE); // Start the SoftwareSerial

    /* BAT ULTRASONIC */
    pinMode(TRIGGERPIN, OUTPUT);    // set TRIGGERPIN as output for trigger
    pinMode(ECHOPIN, INPUT);        // set ECHOPIN as input for echo

    /* GPRS SHIELD */
    init_gprs(); // Init the Shield
    testTraccar(); // Test connection to TRACCAR
} // .setup

void loop(){
    // "http://197.219.3.62:5055/?id=5678901234&lat=-23.868312&lon=35.376653&timestamp=20170403163800&hdop=0.0&altitude=5.5&speed=70.5"

    // Arduino traccarID: 5678901234
    // My iPhone traccarID: 944425

    // Keep listenning to SMS to continue
    listen_SMS(); //listenSMS(*int digits), defaults to 4 digits.

    if(received){  // if sms with a valid number is received:
       if(sms_code == "0"){
         // Get the Saldo:
         checkSaldoVodacom();

       } else if(sms_code == "1"){
         // Send Temperature readings to Traccar:
         sent = read_sendTemp();

         // Notify via sms that it finished sending data
         if(sent){
           // if true
           gprs.sendSMS(string2char(numb), "Temperature of " + String(t) + "ºC sent!");
         } else {
           // otherwise
           gprs.sendSMS(string2char(numb), "Failed to send temperature!");
         }

       } else if(sms_code == "2"){
         // Send Distance reading to Traccar:
         sent = read_sendHum();

         // Notify via sms that it finished sending data
         if(sent){
           // if true
           gprs.sendSMS(string2char(numb), "Humidity of " + String(h) + "% sent!");
         } else {
           // otherwise
           gprs.sendSMS(string2char(numb), "Failed to send humidity!");
         }

       } else if(sms_code == "3"){
         // Send Both (Temp and Dist) to Traccar:
         sent = read_sendDist();

         // Notify via sms that it finished sending data
         if(sent){
           // if true
           gprs.sendSMS(string2char(numb), "Distance of" + String(d) + "cm sent!");
         } else {
           // otherwise
           gprs.sendSMS(string2char(numb), "Failed to send distance!");
         }

       } else if(sms_code == "4"){
         // Send Both (Temp and Dist) to Traccar:
         for(int i = 0; i < 10; i++){
           read_sendAll();
         }

         // Notify via sms that it finished sending data
         if(sent){
           // if true
           gprs.sendSMS(string2char(numb), "10 data readings sent to server!");
         } else {
           // otherwise
           gprs.sendSMS(string2char(numb), "Something wen't wrong while sending data!");
         }
         received = false;
       }
    }
} //.loop
