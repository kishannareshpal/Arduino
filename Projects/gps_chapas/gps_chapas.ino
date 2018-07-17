#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>

// default fona pins
#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

// Initialize SoftwareSerial
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

// FONA 808 reference (works for FONA 800 aswell)
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);


/* -------------------- VARIABLES --------------------- */
char replybuffer[255]; // buffer for replies (sms or ussd)
char smsBuffer[90];
float latitude, longitude, speed_kph, heading; // gps related
char fonaNotificationBuffer[64]; //for notifications from the FONA
char callerIDbuffer[32];  // SMS sender's number
bool received = false;  // Becomes TRUE when a message is received (default value: false).
int slot = 0; // this will be the slot number of the received SMS
String sms_code;


/* ------------------ UTILITY FUNCTIONS ------------------ */
char* string2char(String s){
  if(s.length()!=0){
      char *p = const_cast<char*>(s.c_str());
      return p;
  }
}

/* ------------------ CUSTOM METHODS ------------------ */
void init_fona(){
  while (!Serial);
  Serial.begin(115200);
  fonaSerial->begin(4800);

  if (!fona.begin(*fonaSerial)) {
    // If fona init fails
    Serial.println(F("Fona was not found."));
    while(1);
  }

  Serial.println(F("Init Success!")); // Init success
}

bool send_traccar(float lat, float lon, float speed, float heading){
  uint16_t statuscode;
  int16_t length;
  // String url="197.219.3.62:5055/?id=865067024978911";
  String url="demo.traccar.org:5055/?id=865067024978911";

  url += "&lat=";
  url += String(lat,6);
  url += "&lon=";
  url += String(lon,6);
  url += "&hdop=";
  url += String(heading);
  url += "&speed=";
  url += String(speed, 2); // in Kilometers/hour (Km/h)

  //flushSerial();
  Serial.println(url);

  char url_char[120];
  char data[120];
  url.toCharArray(url_char, url.length());
  url.toCharArray(data, url.length());

  //char data[80]=" "; //197.219.3.62:5055/?id=874359200&lat=-25.278402&lon=57.501012&timestamp=20170221144438&hdop=1.4&altitude=1.7&speed=0.74";

  Serial.print("Http POST: http://");
  Serial.println(url_char);

  if (!fona.HTTP_POST_start(url_char, F("text/plain"), (uint8_t *) data, strlen(data), &statuscode, (uint16_t *)&length)){
    Serial.println("Failed!");
    return 1;

  } else {
    while (length > 0){
      while (fona.available()){
        char c = fona.read();
        loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
        UDR0 = c;
        Serial.write(c);
        length--;
        if (!length){
          break;
        }
      }
    }
    return 0;
  }

  fona.HTTP_POST_end();
}

void send_data(){
  bool gps_success = fona.getGPS(&latitude, &longitude, &speed_kph, &heading);

  if(gps_success){
    fona.enableGPRS(true);
    bool sent_traccar = send_traccar(latitude, longitude, speed_kph, heading);

    if(sent_traccar){
      // If success
      Serial.println(F("DATA SENT SUCCESSFULLY"));
    } else {
      Serial.println(F("DATA SENDING FAILED. TRYING AGAIN"));
    }

  } else {
    Serial.println(F("GPS 3D NOT WORKING, fixing..."));
  }
}

int listen_SMS(){ // returns the message slot #

  char* bufPtr = fonaNotificationBuffer; //handy buffer pointer
  if (fona.available()){
    // any data available from the FONA?
    int charCount = 0;
    // Read the notification into fonaInBuffer
    do {
      *bufPtr = fona.read();
      Serial.write(*bufPtr);
      delay(1);
    } while ((*bufPtr++ != '\n') && (fona.available()) && (++charCount < (sizeof(fonaNotificationBuffer)-1)));

    //Add a terminal NULL to the notification string
    *bufPtr = 0;

    //Scan the notification string for an SMS received notification.
    //  If it's an SMS message, we'll get the slot number in 'slot'
    if (1 == sscanf(fonaNotificationBuffer, "+CMTI: " FONA_PREF_SMS_STORAGE ",%d", &slot)) {
      // Retrieve SMS sender address/phone number.
      if (!fona.getSMSSender(slot, callerIDbuffer, 31)) {
        Serial.println("Didn't find SMS message in slot!");
      }

      // Retrieve the slot so i can read the sms.
      received = true;
      return slot;
    }
  }
}

String read_SMS(int slot_number){ // returns the received sms_code
  uint16_t smslen;
  if (!fona.readSMS(slot_number, smsBuffer, 250, &smslen)) { // pass in buffer and max len!
    Serial.println(F("Failed to retrieve sms!"));
  }

  /*
    Delete the received msg after it is processed
    or else we would fill up all of the slots and then
    receiving sms would not be possible.
  */
  if (fona.deleteSMS(slot_number)) {
    Serial.println(F("OK!"));
  } else {
    Serial.print(F("Couldn't delete SMS in slot ")); Serial.println(slot_number);
    fona.print(F("AT+CMGD=?\r\n"));
  }

  // retrieve the received message:
  return String(smsBuffer);
}

boolean send_SMS(char *msg){ // returns 0 if success; 1 if failed
  if (!fona.sendSMS(callerIDbuffer, msg)) {
    return 1; // if message fails to send
  } else {
    return 0; // if message sends successfully
  }
}

bool getSaldo(char *ussd_){ // returns 0 if success; 1 if failed
  // send a USSD
  uint16_t ussdlen;
  if (!fona.sendUSSD(ussd_, replybuffer, 250, &ussdlen)) { // pass in buffer and max len!
    Serial.println(F("Failed"));
    return 1;
  } else {
    return 0;
  }
}


/* -------------------- DEFAULT METHODS --------------------- */
void setup() {
  init_fona();
  fona.enableGPS(true);
}


void loop() {
  // Send data to traccar
  send_data();

  // Listen and Manage SMS
  slot = listen_SMS();
  // Serial.println("slot number ");
  // Serial.print(slot);


  if(received){  // if sms is received:
    sms_code = read_SMS(slot);
    if(sms_code == "0"){
      // Get the Saldo:
      getSaldo("*100*03#");
      send_SMS(replybuffer);
    }
    sms_code = "";
    received = false;
  }

  // Serial.println(received);
  delay(2000);


}
