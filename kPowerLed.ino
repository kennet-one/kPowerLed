//************************************************************
// nodeId = 4155616721
//
//************************************************************
#include "painlessMesh.h"
#include "mash_parameter.h"


Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;


char buttonState = 1;

void receivedCallback( uint32_t from, String &msg ) {

  String str1 = msg.c_str();
  String str2 = "powled";
  //String str3 = "garland_echo";

  if (str1.equals(str2)) {

    if (buttonState == 1) {
      buttonState = 0;
      //mesh.sendSingle(2661345693,"garland_off");
      //mesh.sendSingle(624409705,"garland_off");
    } else {
      buttonState++;
      //mesh.sendSingle(2661345693,"garland_on");
      //mesh.sendSingle(624409705,"garland_on");
    } 
  }

  // if (str1.equals(str3)) {
  //   if (buttonState == 0) {
  //     mesh.sendSingle(2661345693,"garland_off");
  //     mesh.sendSingle(624409705,"garland_off");
  //   } else {
  //     mesh.sendSingle(2661345693,"garland_on");
  //     mesh.sendSingle(624409705,"garland_on");
  //   }
  // }
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
