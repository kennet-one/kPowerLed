//************************************************************
// nodeId = 4155616721
//
//************************************************************
#include "painlessMesh.h"
#include "mash_parameter.h"


Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;


char buttonState = 0;

void receivedCallback( uint32_t from, String &msg ) {

  String str1 = msg.c_str();
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

  mesh.update();

  if (buttonState == 0) {
    digitalWrite(33, LOW);
  } else {
    digitalWrite(33, HIGH);
  }
}
