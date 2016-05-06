
//=============================================================================
// System Includes
//=============================================================================
#include <assert.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <memory.h>

//=============================================================================
// Project Includes
//=============================================================================
#include <./pgrinclude/pgrflycapture.h>

//=============================================================================
// Macro Definitions
//=============================================================================
//
// The number of images to grab.
//
#define _IMAGES_TO_GRAB 10

//
// The maximum number of cameras on the bus.
//

#define _MAX_CAMS       32
//
// The index of the camera to grab from.
//
#define _CAMERA_INDEX   0

// 
#define INITIALIZE         0x000
#define CAMERA_POWER       0x610

FlyCaptureError		error;
FlyCaptureContext	context;   
FlyCaptureInfoEx	info;
FlyCaptureImage		image;

FlyCaptureInfoEx  arInfo[ _MAX_CAMS ];
unsigned int	  uiSize = _MAX_CAMS;
