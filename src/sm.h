#ifndef SM_H
#define SM_H

#include <windows.h>
#include <iostream>
#include "util.h"

class Camera;
// Scene Manager singleton
namespace SM {

// Update the window size
extern void updateWindow(int nw, int nh);
// Update the global delta value on each frame.
extern void updateDelta();
// How long the program has been running
extern float getGlobalTime();
// Update the delta mouse positions
extern void updateMouse(int nx, int ny);
// Launch timer lasting `m` seconds
extern void triggerTimer(int m);
// Run timer
extern void runTimer();
// Acknowledge and reset timer
extern void ackTimer();
// Deactivate timer
extern void endTimer();
// Increase the program tick counter
extern void updateTick();
// Switch 3D perspective between first- and third-person camera
extern void switchFirstAndThirdCam();
// Toggle the free camera on or off
extern void toggleFreeCam();

// delta time
extern float delta;

// window settings
extern int o_width; // Original window width
extern int o_height; // Original window height
extern float o_aspect; // Original window aspect ratio
extern int width; // Window width
extern int height; // Window height
extern int offset_w; // Window width offset (width/2)
extern int offset_h; // Window height offset (height/2)
extern float aspect; // Window aspect ratio
extern float comp_aspect; // Ratio of the current window size to the original value
extern float w_ratio;
extern float h_ratio;

// Mouse position
extern glm::vec2 mouse; // Mouse position
extern glm::vec2 mouseRest; // Position the mouse is warped to in camera mode
extern float mouseDX;  // mouse delta x
extern float mouseDY;  // mouse delta y

// start time of the program
extern DWORD startTime;
// Times the `update()` function has been run. Essentially the current frame of the program
extern DWORD tick;

// scene camera
extern Camera *camera;

extern bool isFreeCam;      // in free cam, the player model is loaded and updates but does not move with the camera
extern bool isFirstPerson;  // in first person, the player model is not updated but the camera follows it anyway
extern bool isThirdPerson;  // in third person, the player model is loaded and the camera moves with it

// timer variables
extern float timer_c;
extern float timer_max;
extern bool timer_started;
extern bool timer_finished;

// Mesh variables
extern int unnamedMeshCount;
extern int unnamedStaticMeshCount;
extern int unnamedBoneMeshCount;
extern int unnamedVariantMeshCount;
extern int MAX_NUM_INSTANCES;

extern bool debug;

};  // namespace SM

#endif /* SM_H */
