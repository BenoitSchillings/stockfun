
#include <windows.h> 

#include <windowsx.h>

#include <stdlib.h>	
#include <conio.h>
#include <stdio.h>


#include <windows.h> 
#include <windowsx.h>
#include <QtGui>


#include <string.h>
#include <process.h>	  // for Thread	example

#include "minread.h"
#include "const.h"
#include "resource.h"
#include "minread.h"
#include "resource.h"
#include "ccdctl.h"
#include "interface_board.h"

#include "CCDUNIT.h"
#include "Frame.h"

bool	c1394 = TRUE;

//-------------------------------------------------------------------


HDC hMSDC=NULL;					// global stored measure DC of our window
HWND hMSWND=NULL;				// global stored measure HWND of our window

HANDLE hTHREAD;
HANDLE hPROCESS;
HANDLE h;


//-------------------------------------------------------------------


UINT		ExpTime=100000;
int			init_1394();
void		get1394(float *p);
void		set_1394_exp(float v);

void		set_1394_gain(float v);

//-------------------------------------------------------------------



TC253	*the_cam;


//-------------------------------------------------------------------


void	consumer(void*)
{
	the_cam->Consumer();
	 _endthread();
}


//-------------------------------------------------------------------



TC253::TC253(QObject *parent)
{
	vparent = parent;

	the_cam = this;
	cur = 0;
	v_gain = 0;
	v_amplifier = 0;
	busy = 0;
	new_gain = -1;
	new_exp = -1;
	new_amp = -1;

	if (! CCDDrvInit(DRV)) {
		ErrorMsg(" Can't open CCD driver - use simulation");
		//exit(0);
	};

	if (c1394) {
		init_1394();
	}
	else {
		InitBoard(DRV);
	
		if (_AD16CDS)
			SetupAD();

		InitHRCounter();

		SetBoardVars(DRV,SYM_PULSE, BURSTMODE,_PIXEL,1, FLAG816,PPORTADR,PCLK,2*XCKDELAY);
		HighSlope(DRV);
		V_Off(DRV);
	}

	h = (HANDLE) _beginthread(consumer,0,NULL); // starts read loop 


	SetPriorityClass(h, REALTIME_PRIORITY_CLASS);
	SetThreadPriority(h,THREAD_PRIORITY_TIME_CRITICAL);
}


//-------------------------------------------------------------------

void	TC253::xSetExposure(float t)
{
	if (c1394) {
		set_1394_exp(t);
	}
	else {
		ExpTime = t*1000000.0;
		InitHRCounter();
	}
}


//-------------------------------------------------------------------

void	TC253::SetExposure(float t)
{
	new_exp = t;
}


//-------------------------------------------------------------------

void	TC253::SetGain(int gain)
{
	new_gain = gain;
}

//-------------------------------------------------------------------


void	TC253::xSetGain(int gain)
{
	if (c1394) {
		set_1394_gain(gain);
		v_gain = gain;
	}
	else {

		if (gain>63) gain = 63;
		if (gain<0) gain = 0;

		v_gain = gain;

		SetAD16Default(DRV,16);
		SetADAmpRed(DRV, v_gain);
		SetDA(DRV, v_amplifier, 2);

		SetADOff(DRV, 0, 0);
	}
}

//-------------------------------------------------------------------


void	TC253::SetAmplifier(int gain)
{
	new_amp = gain;
}

//-------------------------------------------------------------------

void	TC253::xSetAmplifier(int amplifier)
{
	if (amplifier>233) amplifier = 233;

	v_amplifier = amplifier;

	SetAD16Default(DRV,16);
	SetADAmpRed(DRV, v_gain);
	SetDA(DRV, v_amplifier, 2);

	SetADOff(DRV, 0, 0);
}


//-------------------------------------------------------------------

extern QApplication *the_app;


//-------------------------------------------------------------------

void	TC253::Consumer()
{
	float		cnt = 0;
	Frame		*f = new Frame();
	Frame		*g = new Frame();


	while(1) {
		if (c1394) {
			get1394((float*)&(cur_frame.data));
		}
		else {
			MeasureMatrix_once(hMSDC, (float*)&(cur_frame.data));
		}


			QEvent *q = new QEvent(QEvent::User);

			the_app->postEvent(vparent, q);
			//Sleep( 30L );

			if (new_gain>=0) {
				xSetGain(new_gain);
				new_gain = -1;
			}
			if (new_exp>=0) {
				xSetExposure(new_exp);
				new_exp = -1;
			}

			if (new_amp>=0) {
				xSetAmplifier(new_amp);
				new_amp = -1;
			}
	}
}


//-------------------------------------------------------------------



//set FALSE if you don't want to use the thread:


#define GLOBAL_VARS 1


BOOL _USETHREAD=TRUE;

PFrameBufType pFrameBuf = NULL;

// camera values for calling GETCCD and InitBoard

// parameter for Loop
 int ADDREP = 1;			 	//addition loop for fkt=2; 1 else
 BOOL EXTTRIGFLAG = FALSE;		// run with external Trigger
									// DELAYMS is here wait after trigger!
 BOOL HISLOPE = TRUE;			// Slope for external Trigger

 BOOL HIAMP = FALSE;			// Amplification for switchable sensors

__int64 TPS = 0;			//ticks per second; is set in InitHRCounter
__int64 DELAY = 0;			//also set in InitHRCounter



// Display data
 BOOL	PLOTFLAG = TRUE;			// TRUE for dense, FALSE for dots
 int	XOFF   = _PIXEL / 600;		// index offset for display	
 int	LOX = 0;//21;				// left  x-corner of plot
 int	LOY = 41;					// left upper y-corner of plot
 





/* multimon stuff
*/
int     MyDevice;
char    MyDeviceName[128];
RECT    MyDeviceRect;




// Prototypes 



//globals

ULONG		LOOKUP[256];
BITMAPINFO	*BMINFO;
int			FRAME=0; //global actdisplay framecount
int			LINE=0; //global actdisplay linecount

//setups
BOOL		ShutterMode = TRUE;
BOOL		DisplayOnce = TRUE;
BOOL		ExtTrig = FALSE;
//AD
int			ADGain =0;
int			ADOfs =-255;
int			SensorGain =0;



void SetupAD(void)
{
	BOOL	pos=FALSE;
	UINT	ofs=0;

	SetAD16Default(DRV,16);
	SetADAmpRed(DRV,ADGain);
	SetDA(DRV, SensorGain, 2);

	pos	= (ADOfs >= 0);

	ofs=abs(ADOfs);

	SetADOff(DRV, ofs, pos);
}



#include "PCIInit.C"



#include "ptgreyjunk.h"

bool	no_cam;
float	rn = 0;

void	get1394(float *p)
{
	int		x, y;
	int		cx,cy;
	ushort	*cp;

	
	if (no_cam)	{
		for (y = 0; y < 768; y++) {
			for (x = 0; x < 1024; x++) {
				p[y*_PIXEL + x] = (x*1.5)+(y*10);
			}
		}
 
		rn += 0.01;
		p[_PIXEL*300 + 200 + (int)(sin(rn)*30)] += 5000;
		p[_PIXEL*300 + 201 + (int)(sin(rn)*30)] += 5000;
		p[_PIXEL*301 + 200 + (int)(sin(rn)*30)] += 5000;
		p[_PIXEL*301 + 201 + (int)(sin(rn)*30)] += 5000;
		Sleep( 50L );
		return;
	}


	error = flycaptureGrabImage2( context, &image);
	if (error != 0) {
		no_cam = TRUE;
		return;
	}

	cx = _PIXEL;
	if (cx > image.iCols)
		cx = image.iCols;

	cy = _MAXLINES;
	if (cy > image.iRows)
		cy = image.iRows;

	cp = (ushort*)(image.pData);

	for (y = 0; y < cy; y++) {
		for (x = 0; x < cx; x++) {
			*(p + x) = 5000 + 0.5*((cp[x]&0xff)<<8 | (cp[x]>>8));
		}
		p += _PIXEL;
		cp += image.iCols;
	}


}


void	set_1394_exp(float v)
{
   float fmin, fmax;
   bool	 b_junk;

	if (no_cam) return;

   

   ::flycaptureGetCameraAbsPropertyRange(
                                         context,
                                         FLYCAPTURE_SHUTTER,
										 &b_junk,
                                         &fmin,
                                         &fmax,
                                         0, 0);

	v = v * 1000.0;
	if (v > fmax) v = fmax;

  flycaptureSetCameraAbsProperty(context,FLYCAPTURE_SHUTTER,v);
}


void	set_1394_gain(float v)
{
   float fmin, fmax;
   bool	 b_junk;

   
	if (no_cam) return;


   ::flycaptureGetCameraAbsPropertyRange(
                                         context,
                                         FLYCAPTURE_GAIN,
										 &b_junk,
                                         &fmin,
                                         &fmax,
                                         0, 0);

	if (v > fmax) v = fmax;

  flycaptureSetCameraAbsProperty(context,FLYCAPTURE_GAIN,v);
}

int	init_1394()
{
	no_cam = FALSE;


   error = flycaptureBusEnumerateCamerasEx( arInfo, &uiSize );

   if (error != 0) {no_cam = TRUE;return -1;}

   for( unsigned int uiBusIndex = 0; uiBusIndex < uiSize; uiBusIndex++ ) {
		FlyCaptureInfoEx* pinfo = &arInfo[ uiBusIndex ];
   }

   error = flycaptureCreateContext( &context );
   error = flycaptureInitialize( context, _CAMERA_INDEX );

   if (error != 0) {no_cam = TRUE;return -1;}

	error = flycaptureSetCameraRegister( context, INITIALIZE, 0x80000000 );
   error = flycaptureSetCameraRegister( context, CAMERA_POWER, 0x80000000 );
   error = flycaptureGetCameraInfo( context, &info );
   //error = flycaptureStart( context, FLYCAPTURE_VIDEOMODE_ANY, FLYCAPTURE_FRAMERATE_ANY );
   error = flycaptureStart( context, FLYCAPTURE_VIDEOMODE_1024x768Y16, FLYCAPTURE_FRAMERATE_1_875);

   if (error != 0) {no_cam = TRUE;return -1;}

   memset( &image, 0x0, sizeof( FlyCaptureImage ) );


   bool	 bAuto;
   int	 iValueA;
   int	 iValueB;
   long  lValueA;
   long  lValueB;

   float fmin, fmax;
   bool	 b_junk;

   

   error = ::flycaptureGetCameraAbsPropertyRange(
                                          context,
                                          FLYCAPTURE_GAIN,
										  &b_junk,
                                          &fmin,
                                          &fmax,
                                          0, 0);

   error = ::flycaptureGetCameraProperty(
                                          context,
                                          FLYCAPTURE_SHUTTER,
                                          &lValueA,
                                          &lValueB,
                                          &bAuto );

   error = ::flycaptureSetCameraProperty( 
                                          context,
                                          FLYCAPTURE_SHUTTER,
                                          lValueA,
                                          lValueB,
                                          false );

   error = ::flycaptureGetCameraProperty(
                                          context,
                                          FLYCAPTURE_GAIN,
                                          &lValueA,
                                          &lValueB,
                                          &bAuto );

   error = ::flycaptureSetCameraProperty( 
                                          context,
                                          FLYCAPTURE_GAIN,
                                          lValueA,
                                          lValueB,
                                          false );

   error = ::flycaptureGetCameraProperty(
                                          context,
                                          FLYCAPTURE_GAMMA,
                                          &lValueA,
                                          &lValueB,
                                          &bAuto );

   error = ::flycaptureSetCameraProperty( 
                                          context,
                                          FLYCAPTURE_GAMMA,
                                          lValueA,
                                          lValueB,
                                          false );

  flycaptureSetCameraAbsProperty(context,FLYCAPTURE_GAMMA,1.0f);

  

  error = ::flycaptureGetCameraProperty(
                                          context,
                                          FLYCAPTURE_AUTO_EXPOSURE,
                                          &lValueA,
                                          &lValueB,
                                          &bAuto );

   error = ::flycaptureSetCameraProperty( 
                                          context,
                                          FLYCAPTURE_AUTO_EXPOSURE,
                                          lValueA,
                                          lValueB,
                                          false );
   error = ::flycaptureSetCameraAbsProperty(context,FLYCAPTURE_AUTO_EXPOSURE,1.0f);

   error = ::flycaptureSetCameraProperty( 
                                          context,
                                          FLYCAPTURE_BRIGHTNESS,
                                          lValueA,
                                          lValueB,
                                          false );
   error = ::flycaptureSetCameraAbsProperty(context,FLYCAPTURE_BRIGHTNESS, 0.0f);

   return 0;
}
