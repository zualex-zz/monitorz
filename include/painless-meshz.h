#ifndef PAINLESSMESHZ_H
#define PAINLESSMESHZ_H

#include "painlessMesh.h"

#define   MESH_PREFIX     "z-mesh"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

// Scheduler userScheduler; // to control your personal task
// painlessMesh  mesh;

class PainlessMeshz {
    // char * ssid;
    // char * password;
  static Scheduler userScheduler; // to control your personal task
  static painlessMesh mesh;
public:

    // PainlessMeshz(char * ssid, char * password) : ssid(ssid), password(password) {}

    // User stub
  // void sendMessage() ; // Prototype so PlatformIO doesn't complain

  // const Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

  void sendMessage(String msg) {
    // String msg = "Hello from node ";
    msg += mesh.getNodeId();
    mesh.sendBroadcast( msg );
    // taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
  }

  // Needed for painless library
  // static void receivedCallback( uint32_t from, String &msg ) {
  //   SERIALZ.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  // }

  static void newConnectionCallback(uint32_t nodeId) {
      SERIALZ.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
  }

  static void changedConnectionCallback() {
    SERIALZ.printf("Changed connections\n");
  }

  static void nodeTimeAdjustedCallback(int32_t offset) {
      SERIALZ.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
  }

  static void init(void (*receivedCallback)(uint32_t, String&)) {
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
    mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

    mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
    // mesh.onReceive(&receivedCallback);
    mesh.onReceive(receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

    // userScheduler.addTask( taskSendMessage );
    // taskSendMessage.enable();
  }

  static void update() {
    // it will run the user scheduler as well
    mesh.update();
  }

};

Scheduler PainlessMeshz::userScheduler; // to control your personal task
painlessMesh PainlessMeshz::mesh;

#endif