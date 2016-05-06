#include <windows.h> 
#include <windowsx.h>

#include <QtGui>


#include <stdlib.h>	
#include <conio.h>
#include <stdio.h>

#include "ccdctl.h"
#include "const.h"
#include "global.h"


#include <string.h>
#include <process.h>	  // for Thread	example


#include "interface_board.h"
#include "draw.h"
#include "frame.h"


#define	LI	LARGE_INTEGER

__int64 LargeToInt(LARGE_INTEGER li)
{ //converts Large to Int64
	__int64 res = 0;
	
	res = li.HighPart;
	res = res << 32 ;
	res = res + li.LowPart;
	return res;
} //LargeToInt


BOOL InitHRCounter()
{
// init high resolution counter 
	BOOL	ifcounter;
	__int64 delayus=0;
	LI		freq;


	freq.LowPart  = 0;
	freq.HighPart = 2;

//set global TPS: ticks per second
	ifcounter = QueryPerformanceFrequency(&freq);	
	TPS = LargeToInt(freq); //ticks per second

	if (TPS==0)
		return FALSE; // no counter available

	delayus = ExpTime; //DELAYUS ;
	DELAY = delayus * TPS;
	DELAY = DELAY / 1000000;  //set global delay variable
	
	return TRUE;
} 

__int64 LargetoInt(LI li)
{ //converts Large to Int64
	__int64 res = 0;

	res = li.HighPart;
	res = res << 32 ;
	res = res + li.LowPart;
	return res;
} //LargeToInt


__int64 ticksTimestamp( void )
//reads actual counter value
{	
	LI cntval = {0,0};
	
	QueryPerformanceCounter(&cntval);
	return LargetoInt(cntval);
}

//calc delay in ticks from us
__int64 ustoTicks(ULONG us)
{
// init high resolution counter 
// and calcs DELAYTICKS from m_belPars.m_belDelayMsec

	BOOL	ifcounter;
	__int64 delaytks=0;
	__int64 tps=0; //ticks per second
	LI		freq;

	freq.LowPart  = 0;
	freq.HighPart = 0;

//get tps: ticks per second

	ifcounter = QueryPerformanceFrequency(&freq);	
	tps = LargetoInt(freq); //ticks per second

	if (tps==0)
		return FALSE; // no counter available

	delaytks = us ;
	delaytks = delaytks * tps;
	delaytks = delaytks / 1000000;  
	return delaytks;
} 

long Tickstous(__int64 tks)
{
// init high resolution counter 
 // and returns ms

	BOOL	ifcounter;
	__int64 delay=0;
	__int64 tps=0; //ticks per second
	LI		freq;


	freq.LowPart  = 0;
	freq.HighPart = 0;

//get tps: ticks per second
	ifcounter = QueryPerformanceFrequency(&freq);	
	tps = LargetoInt(freq); //ticks per second

	if (tps==0)
		return 0; // no counter available

	delay = tks * 1000000;
	delay = delay / tps;
	return (long) delay;
} 

//calc delay in ticks from ms
__int64 mstoTicks(long ms)
{
// init high resolution counter 
// and calcs DELAYTICKS from m_belPars.m_belDelayMsec

	BOOL	ifcounter;
	__int64 delaytks=0;
	__int64 tps=0; //ticks per second
	LI		freq ;

	freq.LowPart  = 0;
	freq.HighPart = 0;

//get tps: ticks per second

	ifcounter = QueryPerformanceFrequency(&freq);	
	tps = LargetoInt(freq); //ticks per second

	if (tps==0)
		return FALSE; // no counter available

	delaytks = ms * 1000;
	delaytks = delaytks * tps;
	delaytks = delaytks / 1000000;  

	return delaytks;
} 

long Tickstoms(__int64 tks)
{
// init high resolution counter 
 // and returns ms
	BOOL	ifcounter;
	__int64 delay=0;
	__int64 tps=0; //ticks per second
	LI		freq ;

	freq.LowPart  = 0;
	freq.HighPart = 0;

//get tps: ticks per second
	ifcounter = QueryPerformanceFrequency(&freq);	
	tps = LargetoInt(freq); //ticks per second

	if (tps==0)
		return 0; // no counter available

	delay = tks * 1000000;
	delay = delay / tps;
	delay = delay / 1000;  //set global delay variable
	return (long) delay;
} 


void Delayus(ULONG us)
{
	__int64 t0 = 0;

	t0 = ticksTimestamp();
	while ( (ticksTimestamp() - t0) < ustoTicks(us));
}



void	SetThreadHigh(void)
{
	// in WinNT GetAsyncKeyState -> WaitTrigger works only with HIGH_PRIORITY_CLASS
	//for that reason we use our own GetKey, which is part of board.c

	SetPriorityClass(hPROCESS,/*HIGH_PRIORITY_CLASS*/ REALTIME_PRIORITY_CLASS);
	SetThreadPriority(hTHREAD,THREAD_PRIORITY_TIME_CRITICAL);
}

void	SetThreadLow(void)
{
	// in WinNT GetAsyncKeyState -> WaitTrigger works only with HIGH_PRIORITY_CLASS
	//for that reason we set thread low
	//HIGH_PRIORITY_CLASS

	SetPriorityClass(hPROCESS,NORMAL_PRIORITY_CLASS);
	SetThreadPriority(hTHREAD,THREAD_PRIORITY_NORMAL);
}



long						xpredarray[1000];


int	NN = 0;

void ReadData(float *int_ptr, int line)
{
// read on line 
// Sourearray is readdata of type ArrayT
// Dest is pframeline of type PFrameBufType

	int		i=0;
	int		inc=0;
	long	a=0;
	long	b =0;


	long*	preadarray;
	
	preadarray = &xpredarray[0];

	GETCCD(DRV,preadarray,FFTLINES,FKT,ZADR);

	
	unsigned short	*ff;
	long			*s;

		
	s = preadarray;

		
	s = preadarray;

	NN++;

	float	avg;

	avg = 0;
	for (i = _PIXEL - 360; i < _PIXEL; i++) {
		avg += *(s+i);
	}
	avg /= 720.0;

	for (i=0; i <_PIXEL ; i++) {
		float	val = *s++ / 2.0;
		*int_ptr++ = val - avg + 5000.0;
	}
}

// *********************** read loop **********************



void MeasureMatrix_once(HDC aDC, float *p)
//read of matrix camera and display picture
//for FFTs with no binning
{
	int		j = 0;
	int		val=0;
	int		frame=0;
	int		line=0;
	BOOL	Space = FALSE;
	BOOL	trig=FALSE;
	__int64 START = 0;
	__int64 act=0;
	LI		PERFORMANCECOUNTERVAL = {0,0};
	PFrameBufType actline=NULL;
	__int64 tdisp = 0;


	SetThreadHigh();
	//	clear prev triggers
	CheckFFTrig(DRV);

	CheckFFTrig(DRV);


	OutTrigHigh(DRV); // tigger for osci

	QueryPerformanceCounter(&PERFORMANCECOUNTERVAL);
	OutTrigLow(DRV);

	if (ShutterMode) {
		CloseShutter(DRV);					
		Delayus(10);
		OpenShutter(DRV); // reset ODB 
		Delayus(10);
		START = PERFORMANCECOUNTERVAL.QuadPart;
	}

	// wait exposuretime
	//wait for counter has reached start + needed delay

	do {
			QueryPerformanceCounter(&PERFORMANCECOUNTERVAL);
	} while ((START+ DELAY) >= PERFORMANCECOUNTERVAL.QuadPart); 


	V_On(DRV);		//VON = high

	Delayus(200);	// delay for generating vclks


	for (line=0; line<_MAXLINES;line++) {	
		actline = pFrameBuf;
		actline += (frame * _MAXLINES * _PIXEL + line * _PIXEL);
		ReadData(p, line);
		p += _PIXEL;
	}

	V_Off(DRV);   //VON = lo 

	SetThreadLow();
}



void Contimess(void *dummy)
{
	DWORD oldpriclass;
	int j=0;

	DeactMouse(DRV);


	// paint rectangle


	// init board first
	SetBoardVars(DRV,SYM_PULSE, BURSTMODE,_PIXEL,1, FLAG816,PPORTADR,PCLK,1*XCKDELAY);

		// set slope for ext. trigger

	HighSlope(DRV);

			// set amplification if switchable

	V_Off(DRV);


	// if thread is wanted ...
	if  (_USETHREAD) {
		hPROCESS = GetCurrentProcess();
		oldpriclass = GetPriorityClass(hPROCESS); //keep old val
		hTHREAD = GetCurrentThread();
	}


//	MeasureMatrix(hMSDC) ;  // only area

	if (_USETHREAD) {
		// we need to reset the Class
		if (! SetPriorityClass(hPROCESS, oldpriclass)) ErrorMsg(" No Class reset ");	
	}
}//Contimess




