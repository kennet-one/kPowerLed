//************************************************************
// nodeId = 4155616721
//
//************************************************************
#include "painlessMesh.h"
#include "mash_parameter.h"
#include "CRCMASH.h"


Scheduler userScheduler; // to control your personal task
// (disabled) painlessMesh mesh; // now provided by CRCMASH.h
char buttonState = 0;

void handleBodyFrom( uint32_t from, const String &body ) {

  String str1 = body.c_str();
  String str2 = "powled0";
  String str3 = "powled1";

  if (str1.equals(str2)) {
    buttonState = 0;
  }

  if (str1.equals(str3)) {
    buttonState = 1;
  }
}


void setup() {
  Serial.begin(115200);

  pinMode(33, OUTPUT);

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);

}

void loop() {

  // === CRCMASH Variant B: (from,body) queue ===
  for (uint8_t _i=0; _i<4; ++_i){ uint32_t _from; String _b; if (!qPop2(_from, _b)) break; handleBodyFrom(_from, _b); }
  mesh.update();

  if (buttonState == 0) {
    digitalWrite(33, LOW);
  } else {
    digitalWrite(33, HIGH);
  }
}