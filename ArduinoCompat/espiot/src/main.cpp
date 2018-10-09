/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Belkin/Linksys
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Arduino.h>              // Include for Arduino Functionality
#include <ESP8266WiFi.h>          // https://github.com/esp8266/Arduino
#include <DNSServer.h>            // Include for network functionality
#include <ESP8266WebServer.h>     // Include for web server functionality
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager
#include <WiFiClient.h>           // for connection to AP
#include <Ticker.h>               // for LED status indicator

#define SM_BUF_SIZE         128
#define BUF_SIZE            SM_BUF_SIZE*2
#define LG_BUF_SIZE         BUF_SIZE*2

/***************  DEVICE CONFIGURATION BELOW *********************************/
#define BASE_FRIENDLY_NAME  "espiot"
#define WIFI_RESET_PIN     D6          // reset pin button, pull to GND to reset
#define RESET_TIME         5           // number of seconds for reset
#define RESET_USES_TICKER              // set up a ticker task to watch for the reset pin

#define STATUS_LED         LED_BUILTIN  // use the built in LED for status

#define DEFAULT_DEVICE_NAME "ESP12E"           // this is not currently used
/* UUID_PREFIX IN HEX 0x4D 0x43 0x46 0x31 0x45 0x53 0x50 0x0A */
#define UUID_PREFIX         "ESPIOT"          // must be 10 letters or less
#define DEVICE_TYPE         "DEVICE.WIFI"
// #define DEVICE_TYPE         "DEVICE.ONOFF"

char dev_uuid[16];                // UUIDs in binary form are 16 bytes long
char uuid_str[33];                // UUID as a string
char dev_mac[6];                  // MAC Address is 6 bytes

#define USE_MQTT                  // define to use mqtt for status and control

#ifdef USE_MQTT
#include <PubSubClient.h>         // for mqtt messaging
/* Update these with values suitable for your network / MQTT broker */
const char* ssid = "........";
const char* password = "........";
const char* mqtt_server = "greysic.com";

/***************  END OF DEVICE CONFIGURATION  ********************************/

static int init_done = 0;        // set to 1 after connecting to MQTT and sending status message
int dev_status = 0;

char pub_topic[ BUF_SIZE ];
char cmd_topic[ BUF_SIZE ];
char arrive_topic[ BUF_SIZE ];
char pub_msg[ LG_BUF_SIZE ];

WiFiClient espClient;
PubSubClient client( espClient );
long lastMsg = 0;
char msg[50];
int value = 0;
char sub_msg[ BUF_SIZE ];

void run_cmd( char *cmd, char *topic );
void mqtt_callback( char* topic, byte* payload, unsigned int length );
void reconnect();
#endif

Ticker ticker;
static int reset_button_flag = 0;
#ifdef RESET_USES_TICKER
Ticker reset_ticker;
#endif

static WiFiManager wifiManager;

ESP8266WebServer server( 80 );
static unsigned int reset_counter = RESET_TIME;

void led_tick( );
void check_reset( );
void wifi_reset( );
void configModeCallback ( WiFiManager *myWiFiManager );
void send_self_redirect( char *msg, int delay );
void handle_root( );
void handle_404( );

void setup()
{
  Serial.begin( 9600 );                         // start the serial port
  pinMode( WIFI_RESET_PIN, INPUT_PULLUP );      // sets the D6 pin as input
  pinMode( STATUS_LED, OUTPUT );                // set pin for led STATUS
  WiFi.macAddress( ( byte * )dev_mac );         // fill in the devices mac address
  memset( dev_uuid, 0, sizeof( dev_uuid ) );    // create the device UUID
  memcpy( dev_uuid, UUID_PREFIX, strlen( UUID_PREFIX ) );
  memcpy( dev_uuid+10, dev_mac, sizeof( dev_mac ) );
  memset( uuid_str, 0, sizeof( uuid_str ) );
  sprintf( uuid_str, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", \
           dev_uuid[0],dev_uuid[1],dev_uuid[2],dev_uuid[3],dev_uuid[4],dev_uuid[5],dev_uuid[6], dev_uuid[7], \
           dev_uuid[8],dev_uuid[9],dev_uuid[10],dev_uuid[11],dev_uuid[12],dev_uuid[13],dev_uuid[14],dev_uuid[15] );
  delay( 1000 );
  ticker.attach( 0.2, led_tick );       // start the LED blink task
#ifdef RESET_USES_TICKER
  reset_ticker.attach( 1, check_reset ); // start the reset task
#endif

}

void loop()
{
  char buf[ BUF_SIZE ];

  delay( 500 );
  memset( buf, 0, sizeof( buf ) );
  sprintf( buf, "ESP_%02X%02X%02X", dev_mac[3], dev_mac[4], dev_mac[5] );
  Serial.printf( "%s - START, attempting to connect to wifi\n\r", buf );

  WiFi.hostname( buf );
  wifiManager.setAPCallback( configModeCallback );
  if ( !wifiManager.autoConnect( buf ) ) {
    Serial.printf( "Failed to connect and hit timeout\n\r" );
  }
  // past here the ESP unit has connected to the configured WiFi AP
  ticker.detach();
  ticker.attach( 1, led_tick );
  delay( 100 );

  // start the web server
  server.on( "/", handle_root );
  server.onNotFound( handle_404 );
  server.begin();

  Serial.printf( "SSID: %s\n\r", WiFi.SSID().c_str() );
  Serial.printf( "  IP: %s\n\r", WiFi.localIP().toString().c_str() );
  Serial.printf( "NAME: %s\n\r", WiFi.hostname().c_str() );
  Serial.printf( "TYPE: %s\n\r", DEVICE_TYPE );
  Serial.printf( "UUID: %s\n\r", uuid_str );

#ifdef USE_MQTT
  memset( pub_topic, 0, sizeof( pub_topic ) );
  memset( cmd_topic, 0, sizeof( cmd_topic ) );
  sprintf( pub_topic, "%s/%s/%s/%s/status",BASE_FRIENDLY_NAME, DEVICE_TYPE, WiFi.hostname().c_str(), uuid_str );
  sprintf( cmd_topic, "%s/%s/%s/%s/cmd", BASE_FRIENDLY_NAME, DEVICE_TYPE, WiFi.hostname().c_str(), uuid_str );

  client.setServer( mqtt_server, 1883 );
  client.setCallback( mqtt_callback );
  Serial.printf( "MQTT\n\r" );
  Serial.printf( "HOST: %s\n\r", mqtt_server );
  Serial.printf( " PUB: %s\n\r", pub_topic );
  Serial.printf( " CMD: %s\n\r", cmd_topic );
  Serial.printf( " FMT: [STATUS/CMD](options)\n\r" );
#endif

  while( 1 ) {
    delay( 250 );
#ifdef USE_MQTT
    if ( !client.connected() ) {
      reconnect();
    }
    client.loop();
    if( init_done == 0 ) {
      client.publish( pub_topic, "300,ONLINE" );
      //run_cmd((char *)"HELP?", cmd_topic);
      init_done = 1;
    }
    if( sub_msg[0] != '\0' ) {
      //Serial.printf("recieved MQTT Message on topic %s\n\r", cmd_topic);
      Serial.printf( "MQTT Message: %s\n\r", sub_msg );
      run_cmd( sub_msg, arrive_topic );
      memset( sub_msg, 0, sizeof( sub_msg ) );
      memset( arrive_topic, 0, sizeof( arrive_topic ) );
    }
    if ( reset_button_flag == 1 ) {
      client.publish( pub_topic, "300,RESET_BUTTON_PRESS" );
      reset_button_flag = 0;
    }
#endif
    server.handleClient();
#ifndef RESET_USES_TICKER
    check_reset();
#endif
  }
}

void led_tick()
{
  // toggle Status LED state
  int state = digitalRead( STATUS_LED ); // get the current state of GPIO1 pin
  digitalWrite( STATUS_LED, !state );   // set pin to the opposite state
}

void check_reset()
{
  unsigned int i;
#ifdef USE_MQTT
  char buf[ BUF_SIZE ];
#endif
  i = digitalRead( WIFI_RESET_PIN );   // read the input pin
  if ( i ==  LOW ) {
    if( reset_counter == RESET_TIME ) {
      ticker.detach();
    }
    ticker.attach( 0.2, led_tick );
    Serial.printf( "reset in %02d\n\r", reset_counter );
    reset_counter--;
#ifndef RESET_USES_TICKER
    delay( 1000 );
#endif
    reset_button_flag = 1;
  } else {
    if( reset_counter != RESET_TIME ) {
      ticker.detach();
      ticker.attach( 1, led_tick );
      reset_counter = RESET_TIME;
    }
  }
  if( reset_counter <= 0 ) {
    Serial.printf( "resetting unit now..." );
#ifndef RESET_USES_TICKER
    delay( 1000 );
#endif
    reset_counter = RESET_TIME;
#ifdef USE_MQTT
    memset( buf, 0, sizeof( buf ) );
    sprintf( buf, "RESET" );
    client.publish( pub_topic, buf );
#endif
    wifi_reset();
  }
}

void wifi_reset()
{
  ticker.detach();
  digitalWrite( STATUS_LED, LOW );
  Serial.println( "wifi factory reset" );
  server.close();
  WiFi.disconnect();
  wifiManager.resetSettings();
  delay( 1000 );
  Serial.println( "please push RST" );
  ESP.restart();
  delay( 1000 );
}

// configModeCallback - gets called when WiFiManager enters configuration mode
void configModeCallback ( WiFiManager *myWiFiManager )
{
  Serial.println( "Entered config mode" );
  Serial.println( WiFi.softAPIP() );
  // print the SSID of the config portal
  Serial.println( myWiFiManager->getConfigPortalSSID() );
  // set the LED to fast blink
  ticker.attach( 0.2, led_tick );
}

void send_self_redirect( char *msg, int delay )
{
  char buf[ LG_BUF_SIZE ];
  memset( buf, 0, sizeof( buf ) );
  sprintf( buf, "<HTML><HEAD><meta http-equiv=\"refresh\" content=\"%i; URL='/'\"/></HEAD><BODY>%s</BODY></HTML>", delay, msg );
  server.send( 200, "text/html", buf );
}

void handle_root( )
{
  char buf[ LG_BUF_SIZE * 4 ];
  memset( buf, 0, sizeof( buf ) );
  sprintf( buf, "<HTML><HEAD></HEAD><BODY><pre>" );
  sprintf( buf+strlen( buf ), "<h3>%s - %s</h3>", DEVICE_TYPE, WiFi.hostname().c_str() );
  sprintf( buf+strlen( buf ), "<table cellpadding=3 cellspacing=0 border=0>" );
#ifdef USE_MQTT
  sprintf( buf+strlen( buf ), "<tr><td>mqtt server</td><td>%s</td></tr>", mqtt_server );
  sprintf( buf+strlen( buf ), "<tr><td>publish topic</td><td>%s</td></tr>", pub_topic );
  sprintf( buf+strlen( buf ), "<tr><td>command topic</td><td>%s</td></tr>", cmd_topic );
#endif
  sprintf( buf+strlen( buf ), "<tr><td>UUID</td><td>%s</td></tr>", uuid_str );
  sprintf( buf+strlen( buf ), "<tr><td>Hostname</td><td>%s</td></tr>", WiFi.hostname().c_str() );
  sprintf( buf+strlen( buf ), "<tr><td>Device Type</td><td>%s</td></tr>", DEVICE_TYPE );

  sprintf( buf+strlen( buf ), "</pre></BODY></HTML>" );

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

#ifdef USE_MQTT

void run_cmd( char *cmd, char *topic )
{
  char buf[ BUF_SIZE ];
  memset( buf, 0, sizeof( buf ) );
//  Serial.printf("run_cmd:\n\rtopic: %s\n\rcmd: %s\n\r", topic, cmd);
//  Serial.printf("publish response to topic: %s\n\r", pub_topic);
  if( strncmp( cmd, "REPORT", strlen( "REPORT" ) ) == 0 ) {
    sprintf( buf, "300,REPORT,%s,%s,%s,ONLINE", DEVICE_TYPE, WiFi.hostname().c_str(), uuid_str );
    client.publish( pub_topic, buf );
  } else if( strncmp( cmd, "IP", strlen( "IP" ) ) == 0 ) {
    sprintf( buf, "200,%s,%s", cmd, WiFi.localIP().toString().c_str() );
    client.publish( pub_topic, buf );
  } else if( strncmp( cmd, "SSID", strlen( "SSID" ) ) == 0 ) {
    sprintf( buf, "200,%s,%s", cmd, WiFi.SSID().c_str() );
    client.publish( pub_topic, buf );
  } else if( strncmp( cmd, "RESET", strlen( "RESET" ) ) == 0 ) {
    sprintf( buf, "200,%s", cmd );
    client.publish( pub_topic, buf );
    wifi_reset();
  } else if( strncmp( cmd, "LED-ON", strlen( "LED-ON" ) ) == 0 ) {
    sprintf( buf, "200,%s", cmd );
    ticker.detach();
    digitalWrite( STATUS_LED, LOW );
    client.publish( pub_topic, buf );
  } else if( strncmp( cmd, "LED-OFF", strlen( "LED-OFF" ) ) == 0 ) {
    sprintf( buf, "200,%s", cmd );
    ticker.detach();
    digitalWrite( STATUS_LED, HIGH );
    client.publish( pub_topic, buf );
  } else if( strncmp( cmd, "LED-BLINK", strlen( "LED-BLINK" ) ) == 0 ) {
    sprintf( buf, "200,%s", cmd );
    ticker.attach( 2, led_tick );
    client.publish( pub_topic, buf );
  } else if( strncmp( cmd, "HELP", strlen( "HELP" ) ) == 0 ) {
    client.publish( pub_topic, "200,HELP,[ REPORT,RESET,LED-ON,LED-OFF,LED-BLINK,IP,SSID,HELP ]" );
  } else {
    sprintf( buf, "100,%s,CMD-ERROR", cmd );
    client.publish( pub_topic, buf );
  }
}

void mqtt_callback( char* topic, byte* payload, unsigned int length )
{
//  unsigned int i;
  Serial.printf( "MQTT Message arrived topic: [ %s ]\n\r", topic );
//  for ( i = 0; i < length; i++ ) {
//    Serial.print( ( char )payload[i] );
//  }
//  Serial.println();

  /* copy message for main loop */
  memset( sub_msg, 0, sizeof( sub_msg ) );
  memset( arrive_topic, 0, sizeof( arrive_topic ) );
  if( length > ( sizeof( sub_msg ) - 1 ) ) {
    memcpy( sub_msg, payload, sizeof( sub_msg ) - 1 );
  } else {
    memcpy( sub_msg, payload, length );
  }
  if( strlen( topic ) > ( sizeof( arrive_topic ) - 1 ) ) {
    memcpy( arrive_topic, topic, sizeof( arrive_topic ) - 1 );
  } else {
    memcpy( arrive_topic, topic, strlen( topic ) );
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
      client.subscribe( cmd_topic );
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
