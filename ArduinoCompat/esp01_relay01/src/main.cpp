/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Belkin/Linksys <dvxbootcamp@gmail.com>
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
#include <WiFiClient.h>

#define USE_MQTT
#ifdef USE_MQTT
#include <PubSubClient.h>
#endif

#define DEFAULT_DEVICE_NAME "ESP8266-01"
#define DEVICE_TYPE         "DEVICE.ONOFF"

#define SWITCH_BUTTON     0          // pin switch button

#define UUID_PREFIX         "MCF1ESP"
/* UUID_PREFIX IN HEX 4d 43 46 31 45 53 50 0a */

static WiFiManager wifiManager;
ESP8266WebServer server( 80 );

char dev_uuid[16];                // UUIDs in binary form are 16 bytes long
char uuid_str[33];                // UUID as a string
char dev_mac[6];                  // MAC Address is 6 bytes

static char relay_state = 0;

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
#endif

void relay_close( )
{
  if( relay_state != 0 ) {
    Serial.printf( "%c%c%c%c", 0xa0, 0x01, 0x01, 0xa2 );
    relay_state = 0;
  }
}

void relay_open( )
{
  if( relay_state != 1 ) {
    Serial.printf( "%c%c%c%c", 0xa0, 0x01, 0x00, 0xa1 );
    relay_state = 1;
  }
}


#ifdef USE_MQTT
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
        if ( ( char )payload[ 13 ] == '1' ) {
          relay_open();
        } else if ( ( char )payload[ 13 ] == '0' ) {
          relay_close();
        } else if ( ( char )payload[ 13 ] == '?' ) {
          memset( buf, 0, sizeof( buf ) );
          sprintf( buf, "%s,%d,%s,%s", DEVICE_TYPE, relay_state, WiFi.hostname().c_str(), uuid_str );
          client.publish( "espiot", buf );
        } else {
          Serial.printf( "Invalid relay state %c\n\r",( char )payload[13] );
        }
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

}

void setup( )
{
  Serial.begin( 9600 );         // open the serial port 9600 to talk with relay
  WiFi.macAddress( ( byte * )dev_mac ); // fill in the devices mac address
  pinMode( SWITCH_BUTTON, INPUT_PULLUP );      // sets the D6 pin as input
  memset( dev_uuid, 0, sizeof( dev_uuid ) ); // create the device UUID
  memcpy( dev_uuid, UUID_PREFIX, 7 );
  memcpy( dev_uuid+10, dev_mac, 6 );
  memset( uuid_str, 0, sizeof( uuid_str ) );
  sprintf( uuid_str, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", \
           dev_uuid[0],dev_uuid[1],dev_uuid[2],dev_uuid[3],dev_uuid[4],dev_uuid[5],dev_uuid[6],dev_uuid[7],dev_uuid[8],dev_uuid[9],dev_uuid[10],dev_uuid[11],dev_uuid[12],dev_uuid[13],dev_uuid[14],dev_uuid[15] );
}



void send_self_redirect( int status, int delay )
{
  char buf[512];
  memset( buf, 0, sizeof( buf ) );
  if ( status == 1 ) {
    sprintf( buf, "<HTML><HEAD><meta http-equiv=\"refresh\" content=\"%i; URL='/'\"/></HEAD><BODY>On</BODY></HTML>", delay );
  } else {
    sprintf( buf, "<HTML><HEAD><meta http-equiv=\"refresh\" content=\"%i; URL='/'\"/></HEAD><BODY>Off</BODY></HTML>", delay );
  }
  server.send( 200, "text/html", buf );
}

void http_on_page( )
{
  relay_open();
  send_self_redirect( 1, 2 );
  return;
}

void http_off_page( )
{
  relay_close();
  send_self_redirect( 0, 2 );
  return;
}

void handle_root( )
{
  char buf[512];
  memset( buf, 0, sizeof( buf ) );
  if ( relay_state == 1 ) {
    sprintf( buf, "<HTML><HEAD></HEAD><BODY><h3>espiot DEVICE.ONOFF switch %s</h3> <br><a href=\"/off\">turn switch off</a></BODY></HTML>\n\n", WiFi.hostname().c_str() );
  } else {
    sprintf( buf, "<HTML><HEAD></HEAD><BODY><h3>espiot DEVICE.ONOFF switch %s</h3> <br><a href=\"/on\">turn switch on</a></BODY></HTML>\n\n", WiFi.hostname().c_str() );
  }
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

/************  MAIN PROGRAM LOOP ***************/
void loop( )
{
  int last_state = 0;
  int i = 0;
  int last_button_state = 1;
  char buf[512];

  relay_open();

  wifiManager.setAPCallback( configModeCallback );
  Serial.printf( "starting Auto connect wifi (%s)\n\r", WiFi.hostname().c_str() );
  if ( !wifiManager.autoConnect( WiFi.hostname().c_str() ) ) {
    Serial.println( "failed to connect and hit timeout" );
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
  server.on( "/on", http_on_page );
  server.on( "/off", http_off_page );
  server.onNotFound( handle_404 );
  server.begin();

  Serial.printf( "local ip: " );
  Serial.println( WiFi.localIP() );
  Serial.printf( "  hostname: " );
  Serial.println( WiFi.hostname() );

  while( 1 ) {
    delay( 150 );
    server.handleClient();
#ifdef USE_MQTT
    if ( !client.connected() ) {
      reconnect();
    }
    client.loop();
    if( relay_state != last_state ) {
      memset( buf, 0, sizeof( buf ) );
      sprintf( buf, "%s,%d,%s,%s", DEVICE_TYPE, relay_state, WiFi.hostname().c_str(), uuid_str );
      client.publish( "espiot", buf );
      last_state = relay_state;
    }
#endif

    i = digitalRead( SWITCH_BUTTON );   // read the input pin
    if ( i ==  LOW ) {
      if ( i != last_button_state ) {   // holding down the button shouldn't toggle the state
        if( relay_state == 1 ) {
          relay_close();
        } else {
          relay_open();
        }
        last_button_state = i;
      }
    } else {
      last_button_state = 1;
    }

  }
  server.close();
  Serial.end();
  return;
}
