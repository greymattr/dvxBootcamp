/* Copyright (C) Matthew Fatheree - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential.
 * Written by Matthew Fatheree <matthewfatheree@gmail.com>, 2018
 */

#include <Arduino.h>              // Include for Arduino Functionality
#include <ESP8266WiFi.h>          // https://github.com/esp8266/Arduino
#include <DNSServer.h>            // Include for network functionality
#include <ESP8266WebServer.h>     // Include for web server functionality
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager
#include <WiFiClient.h>           //
#include <SPI.h>                  // basic spi/i2c comunication
#include <Wire.h>                 // basic spi/i2c comunication
#include <Adafruit_GFX.h>         // basic graphics lib for the screen
#include <Adafruit_SSD1306.h>     // ssd1306 is the screen we are using
#include <DHT.h>                  // controls for temp / humidity sensor
#include <DHT_U.h>                // controls for temp / humidity sensor

#define USE_MQTT
#ifdef USE_MQTT
#include <PubSubClient.h>         // for mqtt messaging
#endif

#define RESET_PIN     D6          // reset pin button
#define RESET_TIME    5           // number of seconds for reset
#define DHT11_PIN     D8          // pin for temp / humidity DHT11 sensor

#define DHTTYPE DHT11     // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT_Unified dht( DHT11_PIN, DHTTYPE );

#define DEFAULT_DEVICE_NAME "ESP12E"

/* UUID_PREFIX IN HEX 4d 43 46 31 45 53 50 0a */
#define UUID_PREFIX         "MCF1ESP"
#define DEVICE_TYPE         "SENSOR.TEMP"

/*
 * Connect pin D1 on  ESP12E board to SCK, and pin D2 to SDA
 */
#define OLED_RESET LED_BUILTIN

static WiFiManager wifiManager;
ESP8266WebServer server( 80 );

char dev_uuid[16];                // UUIDs in binary form are 16 bytes long
char uuid_str[33];                // UUID as a string
char dev_mac[6];                  // MAC Address is 6 bytes

static unsigned int cur_temp = 0;

char twirler[] = { '/', '-', '\\', '|' };
unsigned int twirl_counter = 0;

Adafruit_SSD1306 display( OLED_RESET );

#ifdef USE_MQTT
// Update these with values suitable for your network.
const char* ssid = "........";
const char* password = "........";
const char* mqtt_server = "greysic.com";

WiFiClient espClient;
PubSubClient client( espClient );
long lastMsg = 0;
char msg[50];
int value = 0;

void mqtt_callback( char* topic, byte* payload, unsigned int length )
{
  unsigned int i;
  char buf[64];
  Serial.print( "Message arrived [" );
  Serial.print( topic );
  Serial.print( "] " );
  for ( i = 0; i < length; i++ ) {
    Serial.print( ( char )payload[i] );
  }
  Serial.println();

  if( strncmp( ( char * )payload, DEVICE_TYPE, strlen( DEVICE_TYPE ) ) == 0 ) {
    Serial.printf( "recevied message for device type %s\n", DEVICE_TYPE );
    memset( buf, 0, sizeof( buf ) );
    i = length - 32;
    memcpy( buf, ( char * )payload + i, 32 );
    Serial.printf( "recevied message for uuid %s\n", buf );
    if( strncmp( buf, uuid_str, 32 ) == 0 ) {
      Serial.printf( "DEVICE_TYPE & UUID match\n" );
      i = length - 43;
      memset( buf, 0, sizeof( buf ) );
      memcpy( buf, ( char * )payload + i, 10 );
      if ( strncmp( buf, WiFi.hostname().c_str(), 10 ) == 0 ) {
        Serial.printf( "Message is for this unit %s\n", WiFi.hostname().c_str() );
      } else {
        Serial.printf( "%s != %s",WiFi.hostname().c_str(), buf );
      }
    } else {
      Serial.printf( "UUID mismatch\n%s != %s\n", buf, uuid_str );
    }
  }
}


void reconnect()
{
  // Loop until we're reconnected
  while ( !client.connected() ) {
    Serial.print( "Attempting MQTT connection..." );
    // Attempt to connect
    // if (client.connect("ESP8266Client")) {
    if ( client.connect( WiFi.hostname().c_str() ) ) {
      Serial.println( "connected" );
      // ... and resubscribe
      client.subscribe( "espiot/cmd" );
    } else {
      Serial.print( "failed, rc=" );
      Serial.print( client.state() );
      Serial.println( " try again in 5 seconds" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}
#endif


// configModeCallback - gets called when WiFiManager enters configuration mode
void configModeCallback ( WiFiManager *myWiFiManager )
{
  display.clearDisplay();
  display.setTextSize( 0 );
  display.setTextColor( WHITE );
  display.setCursor( 0,0 );
  display.printf( "-CONFIG MODE-" );
  display.setCursor( 0,9 );
  display.setTextSize( 0 );
  display.printf( "SSID: %s", myWiFiManager->getConfigPortalSSID().c_str() );
  display.setCursor( 0,18 );
  display.printf( "  IP: 192.168.4.1" );
  display.display();
}

void send_self_redirect( char *msg, int delay )
{
  char buf[1024];
  memset( buf, 0, sizeof( buf ) );
  sprintf( buf, "<HTML><HEAD><meta http-equiv=\"refresh\" content=\"%i; URL='/'\"/></HEAD><BODY>%s</BODY></HTML>", delay, msg );
  server.send( 200, "text/html", buf );
}

void handle_root( )
{
  char buf[512];
  memset( buf, 0, sizeof( buf ) );
  sprintf( buf, "<HTML><HEAD></HEAD><BODY><h3>espiot SENSOR.TEMP device %s</h3> <br>Current temp: %d</BODY></HTML>\n\n", WiFi.hostname().c_str(), cur_temp );
  server.send( 200, "text/html", buf );
  return;
}

void handle_404( )
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET )?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for ( uint8_t i=0; i<server.args(); i++ ) {
    message += " " + server.argName( i ) + ": " + server.arg( i ) + "\n";
  }
  server.send( 404, "text/plain", message );
  return;
}

void wifi_reset()
{
  display.clearDisplay();
  display.setTextSize( 0 );
  display.setTextColor( WHITE );
  display.setCursor( 0,0 );
  display.println( "wifi factory reset" );
  display.display();
  delay( 3000 );
  wifiManager.resetSettings();
  display.setCursor( 0,9 );
  display.println( "reboot now" );
  display.display();
  server.close();
  ESP.restart();
}


void setup()
{

  Serial.begin( 9600 );
  pinMode( RESET_PIN, INPUT_PULLUP );      // sets the D6 pin as input
  display.begin( SSD1306_SWITCHCAPVCC, 0x3C ); // initialize with the I2C addr 0x3C (for the 128x32)

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  // text display tests
  display.setTextSize( 0 );
  display.setTextColor( WHITE );
  display.setCursor( 0,0 );
  display.println( "Starting..." );
  display.display();
  WiFi.macAddress( ( byte * )dev_mac );  // fill in the devices mac address
  memset( dev_uuid, 0, sizeof( dev_uuid ) ); // create the device UUID
  memcpy( dev_uuid, UUID_PREFIX, 7 );
  memcpy( dev_uuid+10, dev_mac, 6 );
  memset( uuid_str, 0, sizeof( uuid_str ) );
  sprintf( uuid_str, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", \
           dev_uuid[0],dev_uuid[1],dev_uuid[2],dev_uuid[3],dev_uuid[4],dev_uuid[5],dev_uuid[6],dev_uuid[7],dev_uuid[8],dev_uuid[9],dev_uuid[10],dev_uuid[11],dev_uuid[12],dev_uuid[13],dev_uuid[14],dev_uuid[15] );
}

void update_screen( void )
{
  display.clearDisplay();
  display.setTextSize( 1 );
  display.setTextColor( WHITE );
  display.setCursor( 0,0 );
  display.printf( "SSID: %s\n\r", WiFi.SSID().c_str() );
  display.setCursor( 0,9 );
  display.printf( "  IP: " );
  display.println( WiFi.localIP() );
  display.setCursor( 0,18 );
  display.printf( "NAME: " );
  display.println( WiFi.hostname().c_str() );
  display.fillRect( 100, 24, 30, 10, BLACK );
  display.setCursor( 100,24 );
  display.setTextColor( WHITE );
  display.print( cur_temp );
  display.setCursor( 110, 0 );
  display.printf( "%c", twirler[twirl_counter++] );
  if( twirl_counter >= sizeof( twirler ) ) {
    twirl_counter = 0;
  }
  display.display();
  delay( 200 );
}

void loop()
{

  char buf[128];
  unsigned int i = 0;
  unsigned int last_temp = 0;
  unsigned int reset_counter = RESET_TIME;
  sensors_event_t event;


  memset( buf, 0, sizeof( buf ) );
  sprintf( buf, "ESP_%02X%02X%02X", dev_mac[3], dev_mac[4], dev_mac[5] );
  WiFi.hostname( buf );

  display.clearDisplay();
  display.display();
  display.clearDisplay();
  display.setTextSize( 0 );
  display.setTextColor( WHITE );
  display.setCursor( 0,0 );
  display.printf( "starting AP..." );
  display.display();
  display.setCursor( 0,9 );
  display.printf( "SSID: %s", buf );
  display.display();
  display.setCursor( 0,19 );
  display.printf( "  IP: 192.168.4.1" );
  display.display();
  delay( 1000 );
  wifiManager.setAPCallback( configModeCallback );

  dht.begin();

  if ( !wifiManager.autoConnect( buf ) ) {
    display.clearDisplay();
    display.setCursor( 0,0 );
    display.println( "failed to connect and hit timeout" );
    display.display();
    delay( 10000 );
  }
  // if you get here the ESP unit has connected to the WiFi

#ifdef USE_MQTT
  client.setServer( mqtt_server, 1883 );
  client.setCallback( mqtt_callback );
  if ( !client.connected() ) {
    reconnect();
  }
  client.loop();
#endif

  // start the web server
  server.on( "/", handle_root );
  server.onNotFound( handle_404 );
  server.begin();

  display.clearDisplay();
  display.display();
  display.setTextSize( 1 );
  display.setTextColor( WHITE );

  display.setCursor( 0,0 );
  display.printf( "SSID: %s\n\r", WiFi.SSID().c_str() );

  display.setCursor( 0,9 );
  display.printf( "  IP: " );
  display.println( WiFi.localIP() );

  display.setCursor( 0,18 );
  display.printf( "NAME: " );
  display.println( WiFi.hostname().c_str() );

  display.display();

  while( 1 ) {
    delay( 100 );
#ifdef USE_MQTT
    if ( !client.connected() ) {
      reconnect();
    }
    client.loop();
#endif
    server.handleClient();
    i = digitalRead( RESET_PIN );   // read the input pin
    if ( i ==  LOW ) {
      display.fillRect( 100, 24, 20, 8, BLACK );
      display.display();
      display.setCursor( 100,24 );
      display.setTextColor( WHITE );
      display.printf( "%02d", reset_counter );
      display.display();
      reset_counter--;
      delay( 1000 );
      last_temp = 0;   // this is needed to show the temp again after the button is pressed
    } else {
      reset_counter = RESET_TIME;
      //display.fillRect(100, 24, 20, 8, BLACK);
      //display.display();
    }
    if( reset_counter <= 0 ) {
      display.clearDisplay();
      display.display();
      display.setTextSize( 1 );
      display.setCursor( 0,0 );
      display.printf( "resetting unit now..." );
      display.display();
      delay( 3000 );
      reset_counter = RESET_TIME;
      wifi_reset();
    }

    dht.temperature().getEvent( &event );
    if ( isnan( event.temperature ) ) {
#if 0
      display.clearDisplay();
      display.setTextSize( 1 );
      display.setCursor( 0,0 );
      display.printf( "Error reading temperature!" );
      display.display();
#endif
      delay( 100 );
    }
    if ( reset_counter == RESET_TIME ) {
      update_screen();
      if ( ! isnan( event.temperature ) ) {
        cur_temp = ( int )round( 1.8*event.temperature+32 );
        // cur_temp = (int)round(1.7*event.temperature+32);
        if( cur_temp != last_temp ) {
          // only update if there is a change in temperature
          display.fillRect( 100, 24, 30, 10, BLACK );
          display.display();
          display.setCursor( 100,24 );
          display.setTextColor( WHITE );
          display.print( cur_temp );
          display.display();
          delay( 100 );
          last_temp = cur_temp;
#ifdef USE_MQTT
          memset( buf, 0, sizeof( buf ) );
          sprintf( buf, "%s,%d,f,%s,%s", DEVICE_TYPE, cur_temp, WiFi.hostname().c_str(), uuid_str );
          //          sprintf(buf, "%d", cur_temp);
          client.publish( "espiot", buf );
#endif
        }
      }
    }
  }
}
