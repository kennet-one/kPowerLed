//************************************************************
// nodeId = 4155616721
//
//************************************************************
#include "painlessMesh.h"
#include "mash_parameter.h"
#include "CRC.h"

Scheduler userScheduler; // to control your personal task
painlessMesh mesh; 

char buttonState = 0;

void handleBody( const String &msg ) {

  if (msg.equals("powled0")) {
    buttonState = 0;
  }

  if (msg.equals("powled1")) {
    buttonState = 1;
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.setSleep(false);

  pinMode(33, OUTPUT);

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
}

void loop() {

  for (uint8_t _i=0; _i<4; ++_i){ String _b; if (!qPop(_b)) break; handleBody(_b); }
  mesh.update();

  if (buttonState == 0) {
    digitalWrite(33, LOW);
  } else {
    digitalWrite(33, HIGH);
  }
}