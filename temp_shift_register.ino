/*
  Web Server - combined with shift-register
 
 */
 
#include <SPI.h>
#include <Ethernet.h>
 
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xF8, 0xF5 };
IPAddress ip(192,168,2, 30);

int sensorPin = 0; //the analog pin the TMP36's Vout (sense) pin is connected to
                        //the resolution is 10 mV / degree centigrade with a
                        //500 mV offset to allow for negative temperatures

float tempCorrection = -4.7; // A little calibrating
 
// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(8101);

int count_requests = 0;


// Shift-register vars here
int data = 2; 
int clock = 3;
int latch = 4;

//Used for single LED manipulation
int ledState = 0;
const int ON = HIGH;
const int OFF = LOW;





void setup()
{
  // PinModes for Shift-Register
  pinMode(data, OUTPUT);
  pinMode(clock, OUTPUT);  
  pinMode(latch, OUTPUT);  
  Serial.begin(9600);
  Serial.println("reset");


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  
  Serial.begin(9600);
}
 
void loop()
{
  // First the Shift-Register part
  int delayTime = 500; //the number of milliseconds to delay between LED updates
  //for(int i = 0; i < 256; i++){
  //   updateLEDs(i);
  //   updateLEDsLong(i);
  //   delay(delayTime); 
  //}

  if (Serial.available() > 0) {
    // ASCII '0' through '9' characters are
    // represented by the values 48 through 57.
    // so if the user types a number from 0 through 9 in ASCII, 
    // you can subtract 48 to get the actual value:
    int bitToSet = Serial.read() - 48;

  // write to the shift register with the correct bit set high:
    registerWrite(bitToSet, HIGH);
  }



  // Next the web-server part
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    count_requests = count_requests + 1;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
           
          //getting the voltage reading from the temperature sensor
          int reading = analogRead(sensorPin);  
 
          // converting that reading to voltage, for 3.3v arduino use 3.3
          float voltage = reading * 5.0;
          voltage /= 1024.0; 
          // now print out the temperature
          float temperatureC = (voltage - 0.5) * 100 + tempCorrection ;  //converting from 10 mv per degree wit 500 mV offset
                                                        //to degrees ((volatge - 500mV) times 100)
          // now convert to Fahrenheight
          float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
          
          // Print some info
          client.print("Request: ");
          client.print(count_requests);
          client.println("<br/>");
          client.print("Reading: ");
          client.print(reading);
          client.println("<br/>");
          client.print("Voltage = reading * 5.0 / 1024.0: ");
          client.print(voltage);
          client.println("<br/>");
          client.print(temperatureC);
          client.print("   Temperature in Celcius = (voltage - 0.5) * 100 + tempCorrection");
          client.println("<br/>");
          client.print(temperatureF);
          client.print("   Temperature in Fahrenheight = (temperatureC * 9.0 / 5.0) + 32.0");
          client.println("<br/>");
          
          
          Serial.print("Request: "); Serial.println(count_requests);
          Serial.print("Reading: "); Serial.println(reading);
          Serial.print("Voltage (= reading * 5.0 / 1024.0): "); Serial.println(voltage);
          Serial.print(temperatureC); Serial.println("  Temperatur in Celcius = (voltage - 0.5) * 100");
          Serial.print(temperatureF); Serial.println("  Temperatur in Fahrenheight = (temperatureC * 9.0 / 5.0) + 32.0");
          Serial.println("---------------");
          
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
        
    // close the connection:
    client.stop();
  }
}


// Shift-Register code here...

void registerWrite(int whichPin, int whichState) {
// the bits you want to send
  byte bitsToSend = 0;

  // turn off the output so the pins don't light up
  // while you're shifting bits:
  digitalWrite(latchPin, LOW);

  // turn on the next highest bit in bitsToSend:
  bitWrite(bitsToSend, whichPin, whichState);

  // shift the bits out:
  shiftOut(dataPin, clockPin, MSBFIRST, bitsToSend);

    // turn on the output so the LEDs can light up:
  digitalWrite(latchPin, HIGH);

}

/*
 * updateLEDs() - sends the LED states set in ledStates to the 74HC595
 * sequence
 */
void updateLEDs(int value){
  digitalWrite(latch, LOW);     //Pulls the chips latch low
  shiftOut(data, clock, MSBFIRST, value); //Shifts out the 8 bits to the shift register
  digitalWrite(latch, HIGH);   //Pulls the latch high displaying the data
}

/*
 * updateLEDsLong() - sends the LED states set in ledStates to the 74HC595
 * sequence. Same as updateLEDs except the shifting out is done in software
 * so you can see what is happening.
 */ 
void updateLEDsLong(int value){
  digitalWrite(latch, LOW);    //Pulls the chips latch low
  for(int i = 0; i < 8; i++){  //Will repeat 8 times (once for each bit)
  int bit = value & B10000000; //We use a "bitmask" to select only the eighth 
                               //bit in our number (the one we are addressing this time through
  value = value << 1;          //we move our number up one bit value so next time bit 7 will be
                               //bit 8 and we will do our math on it
  if(bit == 128){digitalWrite(data, HIGH);} //if bit 8 is set then set our data pin high
  else{digitalWrite(data, LOW);}            //if bit 8 is unset then set the data pin low
  digitalWrite(clock, HIGH);                //the next three lines pulse the clock pin
  delay(1);
  digitalWrite(clock, LOW);
  }
  digitalWrite(latch, HIGH);  //pulls the latch high shifting our data into being displayed
}


//These are used in the bitwise math that we use to change individual LEDs
//For more details http://en.wikipedia.org/wiki/Bitwise_operation
int bits[] = {B00000001, B00000010, B00000100, B00001000, B00010000, B00100000, B01000000, B10000000};
int masks[] = {B11111110, B11111101, B11111011, B11110111, B11101111, B11011111, B10111111, B01111111};
/*
 * changeLED(int led, int state) - changes an individual LED 
 * LEDs are 0 to 7 and state is either 0 - OFF or 1 - ON
 */
 void changeLED(int led, int state){
   ledState = ledState & masks[led];  //clears ledState of the bit we are addressing
   if(state == ON){ledState = ledState | bits[led];} //if the bit is on we will add it to ledState
   updateLEDs(ledState);              //send the new LED state to the shift register
 }
