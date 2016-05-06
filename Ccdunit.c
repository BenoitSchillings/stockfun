#include <stdio.h>
#include <math.h>


#include <stdlib.h>	
#include <conio.h>
#include <stdio.h>


#include <string.h>
#include <process.h>	  // for Thread	example


#include "board.h"


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

	if (! SetPriorityClass(hPROCESS,/*HIGH_PRIORITY_CLASS*/ REALTIME_PRIORITY_CLASS)) ErrorMsg(" No Class set ");
	if (! SetThreadPriority(hTHREAD,THREAD_PRIORITY_TIME_CRITICAL)) ErrorMsg(" No Thread set ");
}

void	SetThreadLow(void)
{
	// in WinNT GetAsyncKeyState -> WaitTrigger works only with HIGH_PRIORITY_CLASS
	//for that reason we set thread low
	//HIGH_PRIORITY_CLASS

	if (! SetPriorityClass(hPROCESS,NORMAL_PRIORITY_CLASS)) ErrorMsg(" No Class set ");
	if (! SetThreadPriority(hTHREAD,THREAD_PRIORITY_NORMAL)) ErrorMsg(" No Thread set ");
}

int	min = 100000;
int		max = 0;
int		range = 5;
long	cnt = 0;
float	prange;
long	level;

long get_level()
{
	return level;
}

long	xpredarray[1000];


void ReadData(PFrameBufType pframeline, int line)
{
// read on line 
// Sourearray is readdata of type ArrayT
// Dest is pframeline of type PFrameBufType

	int		i=0;
	int		inc=0;
	long	a=0;
	long	b =0;

	long*	preadarray = (pArrayT) pframeline;
	
	preadarray = &xpredarray[0];


	GETCCD(DRV,preadarray,FFTLINES,FKT,ZADR);

	

//resort if readarray word or long
	
		int val =0;

	
		cnt++;

		min = 6395;
		range = 18500;

		if (line == 150) {
			level = 0;
			for (i = 0; i < _PIXEL; i++)
				level += preadarray[i];
		}
		if (line == 151 || line == 152 || line == 153) {
			for (i = 0; i < _PIXEL; i++)
				level += preadarray[i];
		}
		if (line == 154)
			level /= (_PIXEL*4);

long			*s;
FrameBufType	*d;
		
		s = preadarray;
		d = pframeline;

		for (i=0; i <_PIXEL /2 ; i++) {
			val = *s++ - min;
			if (val < 0) val = 0;
			val = val >> 7;
			if (val > 255) val = 255;
			*d++ =  (FrameBufType) val;

			val = *s++ - min;
			if (val < 0) val = 0;
			val = val >> 7;
			if (val > 255) val = 255;
			*d++ =  (FrameBufType) val;

		}
}

// *********************** read loop **********************




void Measure(HDC aDC)   //sync to counter example
{
	int		i,j = 0;
	BOOL	Abbruch = FALSE;
	BOOL	Space = FALSE;
	__int64 START = 0;
	LI		PERFORMANCECOUNTERVAL = {0,0};

//!	PUCHAR pFrameBuf = pFrameBuf;

	PFrameBufType pframe=pFrameBuf;


//	first time init for start
	if (! EXTTRIGFLAG) {
		QueryPerformanceCounter(&PERFORMANCECOUNTERVAL);
		START = LargeToInt(PERFORMANCECOUNTERVAL);
	}

	do	{	
		GETCCD(1,pframe,0,0,0);	//clear array, needed for online add (addrep>1)
		i=0;

		do  {
			i += 1;

			//we use waittrigger even in internal mode for checking keys
			WaitTrigger(DRV,EXTTRIGFLAG,&Space, &Abbruch);
			//if external trigger wait for delay after trigger
		
			if (EXTTRIGFLAG) {
				QueryPerformanceCounter(&PERFORMANCECOUNTERVAL);
				START = LargeToInt(PERFORMANCECOUNTERVAL);
			}
		
		//OutTrig shows here the delay
		//wait for counter has reached start + needed delay
		
			do {QueryPerformanceCounter(&PERFORMANCECOUNTERVAL);}
			while ((START+ DELAY) >= LargeToInt(PERFORMANCECOUNTERVAL)); 
		
		//set start for next loop
		
			if (! EXTTRIGFLAG) {
				QueryPerformanceCounter(&PERFORMANCECOUNTERVAL);
				START = LargeToInt(PERFORMANCECOUNTERVAL);
			}

			OutTrigHigh(DRV);
			ReadData(pframe, 0);
			OutTrigLow(DRV);
			Display(aDC, 1,PLOTFLAG);
		}
		while((i<ADDREP) && (!Abbruch));
	}
	while (!Space && !Abbruch);
};



void MeasureSleep(HDC aDC)   // use of sleep example
{	
	int		i = 0;	
	ULONG	j = 0;
	BOOL	Abbruch = FALSE;
	BOOL	Space = FALSE;

	do	{
		GETCCD(1,pFrameBuf,0,0,0); //clear array
		i=0;
		
		do  {
			i += 1;
			WaitTrigger(DRV,EXTTRIGFLAG,&Space, &Abbruch);

			Sleep(ExpTime/1000);	// only if sleep !=0 other threads can work also
			OutTrigHigh(DRV);
			ReadData(pFrameBuf, 0);
			OutTrigLow(DRV);

			Display(aDC, 1,PLOTFLAG);
		} while((i<ADDREP) && (!Abbruch));

	} while (!Space && !Abbruch);
};	 // CCD- readloop



void MeasureMatrix(HDC aDC)
//read of matrix camera and display picture
//for FFTs with no binning
{
	int		j = 0;
	int		val=0;
	int		frame=0;
	int		line=0;
	BOOL	Abbruch = FALSE;
	BOOL	Space = FALSE;
	BOOL	trig=FALSE;
	__int64 START = 0;
	__int64 act=0;
	LI		PERFORMANCECOUNTERVAL = {0,0};
	PFrameBufType actline=NULL;
	__int64 tdisp = 0;
	__int64 treadframe = 0;
	__int64 texp = 0;
	__int64 texpstore = 0;
	__int64 treadallpix = 0;


	SetThreadHigh();
	//	clear prev triggers
	CheckFFTrig(DRV);

	do { 
		FRAME=frame;
		//	clear prev triggers
		CheckFFTrig(DRV);


		OutTrigHigh(DRV); // tigger for osci
		//	first time init for start
		QueryPerformanceCounter(&PERFORMANCECOUNTERVAL);
		OutTrigLow(DRV);

		if (ShutterMode) {
			//  clearpulse
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

		if(!ShutterMode) {
			//set start for next loop
			START = PERFORMANCECOUNTERVAL.QuadPart;	
			texpstore = ticksTimestamp()-texp ;
			texp = ticksTimestamp();
		}
		else
			texpstore = DELAY ;	

		//read complete frame starts here
		treadframe = ticksTimestamp();

		V_On(DRV); //VON = high

		Delayus(200);  // delay for generating vclks


		for (line=0; line<_MAXLINES;line++) {	
			//we use waittrigger only for checking keys
			
			if ((line % 10) == 0)
				WaitTrigger(DRV,FALSE,&Space, &Abbruch);

			if(Abbruch) break;
			//OutTrig shows here the delay

			actline = pFrameBuf;
			actline += (frame * _MAXLINES * _PIXEL + line * _PIXEL);

			ReadData(actline, line);
		}

		// end of frame
		V_Off(DRV);   //VON = lo 


		treadframe = ticksTimestamp()-treadframe;

		tdisp = ticksTimestamp();
		if (DispXY)
		{	Display(aDC, _MAXLINES/2 ,TRUE); }
		else
		{
			DisplayOneFrame(hMSWND,aDC, frame);
		}

		tdisp = ticksTimestamp()-tdisp;

		frame+=1;
		if (frame>=_MAXFRAMES) {
			frame=0; //wrap frame count
		}

		if ((frame==0) && (Space))
			Abbruch=TRUE;
	}
	while (!Abbruch);

	SetThreadLow();
	TReadFrameus = Tickstous(treadframe);
	TDispus = Tickstous(tdisp); // in us
	TExpus = Tickstous(texpstore); // in us
	TReaduSort = Tickstous(treadallpix );
	TReaduSort *= 1000;
	TReaduSort /= _PIXEL;
}//MeasureMatrix


void TestReadTime(void)
{
	__int64 treadpix=0;

	if (!pFrameBuf)
		return;

	treadpix = ticksTimestamp();
	GETCCD(DRV,pFrameBuf,FFTLINES,FKT,ZADR);
	treadpix = ticksTimestamp()-treadpix;
	TReadPix = Tickstous(treadpix );
	TReadPix *= 1000;
	TReadPix /= _PIXEL;
}


void TestTrig(HDC aDC)
{
	BOOL Abbruch = FALSE;
	BOOL Space = FALSE;
	BYTE ReadCtrlA =0;

	do
	{
		WaitTrigger(DRV,FALSE,&Space, &Abbruch);

		ReadCtrlA = ReadByteS0(DRV,4);
		WriteByteS0(DRV,ReadCtrlA,4);

		OutTrigHigh(DRV);
		OutTrigLow(DRV);
	} while (!Space && !Abbruch);
}


void	TestKey(HWND hDlg)
{	
	char	header[260];
	int		j=0;
	int		i=0;
	UCHAR	key=0;

	for (i=0;i<10000;i++) 
	{
		j=sprintf(header," Keytest: 0x%02x ", key);
		TextOut(hMSDC,200,LOY-15,header,j);
		Sleep(1);
	}

}



void Contimess(void *dummy)
{
	DWORD oldpriclass;
	int j=0;
	char header[260];

	DeactMouse(DRV);


	// paint rectangle

	j=sprintf(header," Online Loop - Cancel with ESC, End with space- key  " );
	TextOut(hMSDC,200,LOY-30,header,j);

	// init board first
	SetBoardVars(DRV,SYM_PULSE, BURSTMODE,_PIXEL,WAITS, FLAG816,PPORTADR,PCLK,XCKDELAY);

		// set slope for ext. trigger

	HighSlope(DRV);

			// set amplification if switchable

	V_Off(DRV);

	TestReadTime();

	// if thread is wanted ...
	if  (_USETHREAD) {
		hPROCESS = GetCurrentProcess();
		oldpriclass = GetPriorityClass(hPROCESS); //keep old val
		hTHREAD = GetCurrentThread();
	}


	MeasureMatrix(hMSDC) ;  // only area

	TextOut(hMSDC,200,LOY-30," Measurement stopped !                                                ",70);
	RedrawWindow(hMSWND,NULL,NULL,RDW_INVALIDATE);

	ActMouse(DRV);

	if (_USETHREAD) {
		// we need to reset the Class
		if (! SetPriorityClass(hPROCESS, oldpriclass)) ErrorMsg(" No Class reset ");	
	}
}//Contimess




