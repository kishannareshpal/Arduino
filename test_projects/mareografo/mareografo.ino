/*||||||||||||||||||||||||||||||
||    Mareógrafo - TRACCAR    ||
||||||||||||||||||||||||||||||*/

#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>
// #include <Wire.h>
// #include <LowPower.h>

#define PIN_TX 7
#define PIN_RX 8
// make sure that the baud rate of SIM900 is 9600!
// you can use the AT Command(AT+IPR=9600) to set it through Serial Monitor and use (AT&W) to save
#define BAUDRATE  9600
/* #define LED 4 */

char buffer[512];

boolean received = false; // default value: false.
String sms_code = "NaN";     // default temperature value: NaN (NotANumber).
char saldo;
char phone[14];
char http_cmd[] = "";
char ph;

GPRS gprs(PIN_TX, PIN_RX, BAUDRATE);



//--------------------------------------------//
//                CUSTOM FUNC.                //
//--------------------------------------------//

boolean isANumber(String str){
    for(byte i=0; i<str.length(); i++){
      if(isDigit(str.charAt(i))) {
        // If it's a numeric:
        return true;
      }
    }
    // if it cotains any letter:
    return false;
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
  while(!gprs.join(F("cmnet"))){ //gprs.join(F(String apn default: cmnet))
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
void listen_SMS(int digits = 22){
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
        if(isANumber(String(message))){
          // If it's numeric procceed sending it the server ...
          sms_code = String(message);
          ph = phone;
          received = true;

        } else if(String(message).indexOf("MB") > 0){
          // Special: Received Saldo:
          saldo = message;
          received = true;

        } else {
          // If it's a sequence of letters show an error...
          Serial.print("Error. Expects: Number\r\n\n");
        }
     } else {
      // Continue listenning to sms...
      Serial.print("Listenning to SMS\n");
      received = false;
      delay(4000);
     }
};

// void sendData(){ // temp e dist
//
//     String http_header =  "PUT /?id=944425";
//            http_header += "&lat=-23.868312";
//            http_header += "&lon=35.376653";
//            http_header += "&hdop=90";
//            http_header += "&altitude=23";
//            http_header += "&temp=";
//            http_header += tempp;
//            http_header += " HTTP/1.0\r\n\r\n";
//            //  Serial.println(http_header);
//
//     char http_cmd[http_header.length()+1];
//     delay(2000);
//     http_header.toCharArray(http_cmd, sizeof(http_cmd));
//     delay(3000);
//     Serial.println("fetching...");
//     gprs.send(http_cmd, sizeof(http_cmd)-1);
//     delay(3000);
//     while (true) {
//         int ret = gprs.recv(buffer, sizeof(buffer)-1);
//         delay(3000);
//         if (ret <= 0){
//             Serial.println("done!\r\n");
//             break;
//         }
//         buffer[ret] = '\0';
//         Serial.print("Recv: ");
//         Serial.print(ret);
//         Serial.print(" bytes\r\n");
//         Serial.println(buffer);
//
//         delay(5000);
//     }
//     // vaciado del puerto serie.
//     Serial.flush(); //Sino, al volver a arrancar tiene bits en el bufer y no lee bien los sensores
//     received = false;
// };

// Check for balance on the SIM
void checkSaldoVodacom(){
  // To check balance on Vodacom we can send ("S" or "SALDO") to ("84162")
  bool t = gprs.sendSMS("84162", "s");
  if(t){
    delay(11000);
    listen_SMS();
    Serial.println(ph);
    Serial.println(phone);

    gprs.sendSMS(ph, saldo);
    Serial.println("SMS SENT");
  } else {
    Serial.println("SMS FAILED");
  }
}

void sendTemp(){}
void sendDist(){}
void sendTemp_Dist(){}

//============================================//
//             DEFAULT FUNCTIONS.             //
//============================================//
void setup(){
    Serial.begin(BAUDRATE); // Start the SoftwareSerial
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
         sendTemp();

       } else if(sms_code == "2"){
         // Send Distance reading to Traccar:
         sendDist();

       } else if(sms_code == "3"){
         // Send Both (Temp and Dist) to Traccar:
         sendTemp_Dist();
       }
    }
} //.loop
