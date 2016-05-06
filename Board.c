/*  Board.C				PCI Version       V 2.4
	G. Stresing			5/2005

  all functions for reading driver data through DeviceIoControl

  on WINNT:		LscPCI.sys
  on Win95:		LscPCI.vxd


  NO FIFO - sync Version
	V2.0:	TI sensor flag TI; 
			SendCommand for 16Bit cds
			sym with F_DP


 Win C++version:	rename in board.cpp
					add #include "stdafx.h"
						#include "global.h"
  */

//#include "stdafx.h"		// use in C++
//#include "global.h"		// use in C++

#include <windows.h> 
#include <windowsx.h>
#include <ddraw.h>


#include "minread.h"

#include "resource.h"

#include "ccdctl.h"
#include "board.h"


// use LSCPCI1 on PCI Boards
#define	DRIVERNAME	"\\\\.\\LSCPCI"

// globals within board
// don't change values here - are set within functions SetBoardVars...

// handle array for our drivers
HANDLE ahCCDDRV[5] = {INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE};		

ULONG aWAITS[5] = {0,0,0,0,0} ;	// waitstates
ULONG aFLAG816[5] = {1,1,1,1,1};  //AD-Flag
ULONG aPIXEL[5] = {0,0,0,0,0};	// pixel
ULONG aXCKDelay[5] = {1000,1000,1000,1000,1000};	// sensor specific delay
BOOL aINIT[5] = {FALSE,FALSE,FALSE,FALSE,FALSE};


void ErrorMsg(char ErrMsg[20])
{
  if (MessageBox( GetActiveWindow(), ErrMsg, "ERROR",MB_OK|MB_ICONEXCLAMATION ) == IDOK ) {};
};


BOOL CCDDrvInit(UINT drvno)
{
	BOOL fResult=FALSE;
	char AString [20];
	char num[6] ;
	HANDLE hccddrv = INVALID_HANDLE_VALUE ;

	if ((drvno < 1) || (drvno>4)) return FALSE;
	strcpy(AString,"");
	strcpy(num,"");
	strcat(AString,DRIVERNAME);
	itoa(drvno,num,16);
	strcat(AString,num);
	hccddrv = CreateFile(AString, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
						 NULL, OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL /*FILE_FLAG_DELETE_ON_CLOSE*/, NULL);

	if (hccddrv == (HANDLE) INVALID_HANDLE_VALUE)
		return FALSE;	//test : is driver there ?
	
	//save handle in global array
	ahCCDDRV[drvno]=hccddrv;
	return TRUE;	  // no Error, driver found
}; //CCDDrvInit


void CCDDrvExit(UINT drvno)
{
    CloseHandle(ahCCDDRV[drvno]);	   // close driver
	ahCCDDRV[drvno] = INVALID_HANDLE_VALUE;
	aINIT[drvno] = FALSE;
};


BOOL InitBoard(UINT drvno)
	{		// inits PCI Board and gets all needed addresses
			// and gets Errorcode if any
	ULONG Errorcode = Error_notinitiated;
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	ctrlcode = 1; // Init Board


	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_SetFct,  // read error code
							&ctrlcode,        // Buffer to driver.
                            sizeof(ctrlcode),
							&Errorcode,sizeof(Errorcode),&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("InitBoard failed");};
	
	// these error messages are for inspection of the driver only
	// they can be omitted because here should be no error
	if (Errorcode==NoError)
			{return TRUE; }  // everything went fine

		else   switch (Errorcode)
			{
			case Error_notinitiated: ErrorMsg("CCD Board not initialized");
			break;
			case Error_noregkey: ErrorMsg(" No registry entry found ");
			break;
			case Error_nosubregkey: ErrorMsg(" No registry sub entry found ");
			break;
			case Error_nobufspace: ErrorMsg(" Can't init buffer space ");
			break;
			case Error_nobios: ErrorMsg(" No PCI bios found ");
			break;
			case Error_noboard: ErrorMsg(" Can't find CCD Board ");
			break;
			case Error_noIORegBase: ErrorMsg(" Can't find PCI space ");
			break;
			case Error_Physnotmapped: ErrorMsg(" Can't map PCI space ");
			break;
			case Error_Fktnotimplemented: ErrorMsg(" function not implemented");
			break;
			case Error_Timer: ErrorMsg(" PCI Timer Error ");
			break;

			}
	return FALSE;
	};  // InitBoard

BOOL ActMouse(UINT drvno)
	{		// inits PCI Board and gets all needed addresses
			// and gets Errorcode if any
#ifndef _DEBUG
	ULONG Errorcode = Error_notinitiated;
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	ctrlcode = 3; // Activate mouse


	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_SetFct,  // read error code
							&ctrlcode,        // Buffer to driver.
                            sizeof(ctrlcode),
							&Errorcode,sizeof(Errorcode),&ReturnedLength,NULL);
	if (! fResult)
		ErrorMsg("ActMouse failed");

	if (Errorcode!=NoError)
		{ErrorMsg(" Mouse Error ");
	return FALSE;}

	ShowCursor(TRUE); //					
#endif
	return TRUE;
}//ActMouse

BOOL DeactMouse(UINT drvno)
	{		// inits PCI Board and gets all needed addresses
			// and gets Errorcode if any
#ifndef _DEBUG
	ULONG Errorcode = Error_notinitiated;
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	ctrlcode = 2; // Deactivate mouse


	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_SetFct,  // read error code
							&ctrlcode,        // Buffer to driver.
                            sizeof(ctrlcode),
							&Errorcode,sizeof(Errorcode),&ReturnedLength,NULL);
	if (! fResult)
		ErrorMsg("DeactMouse failed");

	if (Errorcode!=NoError)
		{ErrorMsg(" Mouse Error ");
	return FALSE;}

	ShowCursor(FALSE); //		
#endif			
	return TRUE;
	
	}//DeactMouse


BOOL SetBoardVars(UINT drvno, BOOL sym, BOOL burst,ULONG pixel, ULONG waits,ULONG flag816,ULONG pportadr,
				  ULONG pclk, ULONG xckdelay)
{	//pportadr, pclk, xckdelay for compatibility - no function here
	//initiates DMA Registers
	//	pclk -> freq=m_nWaitStates;//0=40MHz;1=20MHz;2=10MHz max 6=0 for FIFO mode
	BYTE data = 0;
	ULONG reg = 0;
	USHORT buswidth = 0x840; //default 8 bit

	if (ahCCDDRV[drvno] == INVALID_HANDLE_VALUE)
		{	return FALSE;		}

	//set startval for CTRLA Reg  +slope, IFC=h, VON=1 
	WriteByteS0(drvno,0x03, 0x04);  //write CTRLA reg in S0



	if (sizeof(ArrayT)==4) {buswidth=0x0843;}; //32 bit
	if (sizeof(ArrayT)==2) {buswidth=0x0841;}; //16 bit


	WriteLongIOPort(drvno,buswidth, 0x080) ;	//DMA Mode Reg, 3waits=84f, 843 sonst
	//843= long +local address const, ready enable, mode C 
	//841= ushort +local address const, ready enable, mode C 
	
	WriteLongIOPort(drvno,0, 0x088) ;			//set local Address(88h)= 0
	WriteLongIOPort(drvno,0x08, 0x090) ;		//set descriptor(90h) = 0x08 local->PCI
	// all DMA Inits completed 
	// buffer address and size is set in IOCTL_GetCCD or ReadFile

	
  /*sym - flag activates divider for symmetrical ND pulse
    max 1F * 66ns + 100ns  waits
    max 1F waits, no F_DP ND has 2.2 mu - with F_DP ND has 4.2 mu
    min 0waits, no F_DP ND has 100ns=10MHz - with F_DP has 170ns=5.8MHz
    SYM=false activates DMAModeRegs waitstategenerator
    0waits=66ns , max F waits =530ns
    double ND pulse on every 12bit read
    Bit7 = F_NDSYM ON, Bit6 = F_DP(Double pulse) ON, Bit5 = DIS_ND OFF*/
	
	if (sym) //compatible mode up to 10 MHz cams
		{
		data = 0x080; //sym pulse 
        if (waits>0x01F)  {data |= 0x1F; }
				else {data |=  (UCHAR)waits;}
		if (flag816==1)  {data |= 0x040; } //for 16Bit set F_DP
			   else {data &= 0x0BF; } //else clear F_DP
		WriteByteS0(drvno,data, 0x05);  //write CTRLB reg in S0
        //deact DMAMode asym waitstate generator
        reg = ReadLongIOPort(drvno,0x080);  // DMAMode Reg
        reg &= 0x0ffffffC3; //clear waitstates
        WriteLongIOPort(drvno,reg, 0x080);  // DMAMode Reg
		}
	   else   // highest speed cams 15MHz
	   {
        data=0;
		if (waits > 0x0F) { data = 0x0F;}  //max f waits
					else {data =  (UCHAR) waits;}
        data = data << 2;
        reg = ReadLongIOPort(drvno,0x080);  // DMAMode Reg
        reg = reg & 0x0ffffffC3; //clear waitstates
        reg = reg | data;
        WriteLongIOPort(drvno,reg, 0x080);  // DMAMode Reg
        // deact SYM generator
		data = 0;
		if (flag816==1)  {data |= 0x040; } //for 16Bit set F_DP
        WriteByteS0(drvno,data, 0x05);  //write CTRLB reg in S0
	   };
	if (burst)
		{//30 MHz cams can use dma burst mode
		reg = ReadLongIOPort(drvno,0x080);  // DMAMode Reg
        reg = reg | 0x100;				//set burst bit
		reg = reg | 0x080;				//en bterm for highest speed
        WriteLongIOPort(drvno,reg, 0x080);  // DMAMode Reg

		// set burst flag for 33MHz ND-freq
		data = ReadByteS0(drvno,0x06);
		data = data | 0x08;
		WriteByteS0(drvno,data, 0x06);  //write CTRLC reg in S0
		}
	  else
	  { // clear EN_BURST Flag
		data = ReadByteS0(drvno,0x06);
		data = data & 0xf7;
		WriteByteS0(drvno,data, 0x06); 
	  };


	  //set global vars if driver is there

	aWAITS[drvno] = waits;
	aFLAG816[drvno] = flag816;

	aPIXEL[drvno] = pixel;
	if (_IR) 	aPIXEL[drvno] =  2 * pixel;
	if (_IRSingleCH) 	aPIXEL[drvno] =  4 * pixel;
	aXCKDelay[drvno] = xckdelay;
	aINIT[drvno] = TRUE;

	return TRUE; //no error
	};  // SetBoardVars


void Resort(UINT drvno, void* dioden)
//		example for resort array for 1 PCI board with 2 slots
//      array type long 8bit vals in +0 port1 and +1 in port2
//		resort for display -> port1 longs in DB1 and port2 longs in DB2 
{
		ULONG i=0;
		ULONG barraylength =0;
		ULONG larraylength =0;

		BOOL gotit = FALSE;


// 1----------------   resort for 1 PCI board and 2 or more slots
//		example for resort array for 1 PCI board with 2 slots
//      array type long 8bit vals in +0 port1 and +1 in port2
//		resort for display -> port1 longs in DB1 and port2 longs in DB2 
/*

  		BYTE* pbtarray;
		BYTE* pbsarray ;

		pbsarray =(BYTE*) calloc(aPIXEL[drvno],sizeof(long)); 
		if (pbsarray==0) 		
			{ErrorMsg("alloc resort Buffer failed");
			return;
			}
		gotit=TRUE;
		larraylength = aPIXEL[drvno];
		barraylength = sizeof(ArrayT)*aPIXEL[drvno];

		pbtarray = (BYTE *)dioden;


		for (i=0;i<barraylength;i++) //copy all relevant data to source
			pbsarray[i] =  pbtarray[i];


		for (i=0;i<larraylength-sizeof(ArrayT);i++)
			{ 
			pbtarray[i*4] = pbsarray[i*4+0];
			pbtarray[i*4+1] = 0; //pbsarray[i*2+1];
			pbtarray[i*4+2] = 0;//pbsarray[i*4+2];
			pbtarray[i*4+3] = 0;//pbsarray[i*4+3];
			//DB2 for display of 2nd camera
			pbtarray[barraylength+i*4] = pbsarray[i*4+1];
			pbtarray[barraylength+i*4+1] = 0; //pbsarray[i*2+1];
			pbtarray[barraylength+i*4+2] = 0;//pbsarray[i*4+2];
			pbtarray[barraylength+i*4+3] = 0;//pbsarray[i*4+3];
			} 
	if (gotit) free(psarray);
*/
// ----------------- END  resort for 1 PCI board and 2 or more slots


// 2---------------- resort for 2 channels IR cams parallel
/*	#if (_IR2) //if IR reads 2 lines at once
		//takes 3 micro sec on amd1000
	
		if (zadr==1) chl = 2; // CH2=1  CH1=2
		if (zadr==2) chl = 1;
		if (zadr==0) { chl =2; chh=1;}; //get both channels

		if (fkt==2)
			{ //add
			for (i=0;i<	aPIXEL[drvno];i++)
				* (pDiodenBase++) +=  * (LPLONG)(pCorArray + i*sizeof(ArrayT) + chl );
		
			if (zadr==0)
				for (i=0;i<	aPIXEL[drvno];i++) //get second array
					* (pDiodenBase2++) += * (LPLONG)(pCorArray + i*sizeof(ArrayT) + chh ); 
			}
		  else
			{// fkt==1 standard read
			for (i=0;i<	aPIXEL[drvno];i++)
				* (pDiodenBase++) =  * (LPLONG)(pCorArray + i*sizeof(ArrayT) + chl );
		
			if (zadr==0)
				for (i=0;i<	aPIXEL[drvno];i++) //get second array
					* (pDiodenBase2++)= * (LPLONG)(pCorArray + i*sizeof(ArrayT) + chh ); 
			}
*/
// ----------------- END resort for 2 channels IR cams parallel

/*
// 3----------------- resort for standard IR camera
//resort for IR camera - only pixel 3 and 4 are valid

	ArrayT* pltarray;
	ArrayT* plsarray ;

	plsarray = (ArrayT *)dioden;
	pltarray = (ArrayT *)dioden;

	
#if (_IRSingleCH) // one channel for HA G92xx with 256 pixel
	larraylength = _PIXEL;
	for (i=0;i<larraylength;i++)
			{ 
			pltarray[i] = plsarray[4*i+2];
			}
#else	// two channel sensors with 512 pixel
	larraylength = _PIXEL / 2;
	for (i=0;i<larraylength;i++)
			{ 
			pltarray[2*i] = plsarray[4*i+1];
			pltarray[2*i+1] = plsarray[4*i+2];
			} 
#endif

//  ----------------- END resort for standard IR camera
*/

// -------- resort hi/lo

	if (aFLAG816[drvno] ==1)
	{	// resort 12/16 bit array
		ULONG i=0;
		BYTE* ptarray;
		BYTE* psarray ;
		psarray = (BYTE*) calloc(_PIXEL*4, sizeof(BYTE));



//		psarray = (UCHAR*) dioden;  //(BYTE*) calloc(PIXEL*4, sizeof(BYTE));
		ptarray = (UCHAR*) dioden;    

		for (i=0;i<_PIXEL*4;i++)
			psarray[i]= ptarray[i];

		if (sizeof(ArrayT)==1) {return ;}; //8 bit not for 12bit

		//resort 12 bit array takes 4ns / pixel on a 3GHz PC
		if (sizeof(ArrayT)==4) //target 32 bit
		{
		for (i=0;i<_PIXEL-5;i++)
			{
		//	 ptarray[i*4] = psarray[i*4+4];		// lo byte
		//	 ptarray[i*4+1] = psarray[i*4+5];  // hi byte
		//	 ptarray[i*4+2] =  0;
		//	 ptarray[i*4+3] =  0;
			}		
		}; //32 bit
		
		if (sizeof(ArrayT)==2) //target 16 bit
		{
		for (i=0;i<_PIXEL;i++)
			{
			 ptarray[i*2] =  psarray[i*2+1];	// lo byte
			 ptarray[i*2+1] =  psarray[i*2];	// hi byte
			}
		}; //16 bit
	free(psarray);
	}
// ___________________________ END resort Hi/Lo




}	//Resort 




//  call of the read function
BOOL GETCCD(UINT drvno, void* dioden, ULONG fftlines, long fkt, ULONG zadr)	
{	//starts data transfer and gets data in buffer dioden
	//drvno: driver number 1..4; 1 for LSCISA1
	//dioden: pointer to destination array of ULONGs
	//fftlines: vertical lines of FFT sensors: vclks=2 * no of lines
	//fkt:  -1=clrread; 0=init dest array with 0; 1=standard read; 2=add to data
	//zadr: cam address on bus -> for series2000 addressed mode
	//returns true; false on error

  	BOOL fResult=FALSE;
	DWORD   ReturnedLength = 0;
	pArrayT pReadArray = NULL;
	pArrayT pCorArray = NULL;
	pArrayT	pDiodenBase = NULL;
	pArrayT	pDiodenBase2 = NULL;
	ULONG arraylength=0;
	sCCDFkts CCDfkt;
	ULONG i = 0;
	BOOL addalloc=FALSE;
	BOOL coralloc=FALSE;
	BYTE chl = 1;
	BYTE chh = 1;

	if (! aINIT[drvno]) return FALSE;	// return with error if no init

	//set function recs
	CCDfkt.Adrwaits = aXCKDelay[drvno]; // pass wait between XCK and ND
	if (_TI)
		{
		CCDfkt.Waits = 0;  // 6000 only valid for  TI sensors, time of vclks in 0 = 2mu
		CCDfkt.Vclks = fftlines * 4;
		}
	  else
		{
		CCDfkt.Waits = 6000;  // 6000 only valid for  FFT sensors, time of vclks in ns = 6000
		CCDfkt.Vclks = fftlines * 2;
		}
	CCDfkt.Fkt	= fkt;
	CCDfkt.Zadr = zadr;

	pReadArray = (pArrayT)dioden;
//	pReadArray = pReadArray + (db-1) * pixel;
	pDiodenBase = pReadArray;
	arraylength = aPIXEL[drvno] * sizeof(ArrayT); //length in bytes


	
	if (fkt==0) // set array to 0
		{
		for (i=0;i<	_PIXEL;i++)
			*(pReadArray++) = 0;
		return TRUE;
		}


	if  (fkt>2)
		return FALSE;  // function not implemented

	#if (_IR2)
		//alloc array for correction 4 x pixel
		//for IR - need 2. array
		pDiodenBase2 = (pArrayT)dioden + aPIXEL[drvno];
		pCorArray = (pArrayT) calloc(aPIXEL[drvno]*4,sizeof(ArrayT));
		if (pCorArray==0) 		
			{ErrorMsg("alloc Cor Buffer failed");
			return FALSE; }
		coralloc=TRUE;
		pReadArray = pCorArray;
		arraylength *= 4;
		CCDfkt.Fkt = 1;		//make standard read
	#else
		// here normal read
		if (fkt!=1) //read in our local array ladioden - add and clrread
			{
			//alloc local array dioden, so we don't overwrite our DIODEN
			pReadArray = (pArrayT) calloc(aPIXEL[drvno],sizeof(ArrayT)); 
			if (pReadArray==0) 		
				{ErrorMsg("alloc ADD/CLR Buffer failed");
				return FALSE; }
			addalloc=TRUE;
			CCDfkt.Fkt = 1;		//make standard read
			}

	
		if ((_IR) && (!addalloc))
			{
			//alloc local array because it is 2*PIXEL -> has to be resortet later
			pReadArray = (pArrayT) calloc(aPIXEL[drvno],sizeof(ArrayT)); 
			if (pReadArray==0) 		
				{ErrorMsg("alloc IR Buffer failed");
				return FALSE; }
			addalloc=TRUE;
			CCDfkt.Fkt = 1;	
			}

	#endif
	
		
	//read camera data
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_GetCCD,  
		 					&CCDfkt, sizeof(CCDfkt),
							pReadArray,arraylength,
							&ReturnedLength,NULL);
	
	if ((! fResult) || (ReturnedLength!=arraylength))
		{ErrorMsg("Read DMA Buffer failed");
		if (addalloc) free(pReadArray);
		if (coralloc) free(pCorArray);
		return FALSE; }

	//clrread and add: fkt=-1 and 2 could not be implemented with dma
	//so we do it here after reading
	if (fkt==-1)
	{ // return , nothing else to do
		if (addalloc) free(pReadArray);
		if (coralloc) free(pCorArray);
		return TRUE;
		}
 
	if (_RESORT) Resort(drvno,pReadArray);	

	if ((_IR) && (fkt!=2)) // copy back
			{
			pDiodenBase2=pReadArray;
			for (i=0;i<	_PIXEL;i++)
				* (pDiodenBase++) = * (pDiodenBase2++); 
			}

	if (fkt==2) // we must now add our data to DIODEN for online add
			{
			pDiodenBase2=pReadArray;
			for (i=0;i<	_PIXEL;i++)
				* (pDiodenBase++) += * (pDiodenBase2++); 
			}

	if (addalloc) free(pReadArray);
	if (coralloc) free(pCorArray);
	
#if _IS_C4350
//special for Ha Detector head C4350
	V_On(drvno);  //starts new read sequence to Fifo
	V_Off(drvno);
//special end
#endif



	return TRUE; // no Error, all went fine
	};  // GETCCD


ULONG ReadLongIOPort(UINT drvno,ULONG PortOff)
	{		// reads long of PCIruntime register LCR
			// PortOff: Reg Offset from BaseAdress - in bytes
	ULONG DWData = 0;
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	PortOffset;

	PortOffset = PortOff;
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_ReadLongIORunReg,  
							&PortOffset,        
                            sizeof(PortOffset),
							&DWData,sizeof(DWData),&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("Read IORunReg failed");};
	return DWData;
	};  // ReadLongIOPort


ULONG ReadLongS0(UINT drvno,ULONG PortOff)
	{		// reads long on space0 area
			// PortOff: Offset from BaseAdress - in Bytes !
	ULONG DWData = 0;
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	PortOffset;

	PortOffset = PortOff;
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_ReadLongS0,  // read one byte
							&PortOffset,        // Buffer to driver.
                            sizeof(PortOffset),
							&DWData,sizeof(DWData),&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("Read in space0 failed");};
	return DWData;
	};  // ReadLongS0



UCHAR ReadByteS0(UINT drvno,ULONG PortOff)
	{		// reads long on space0 area
			// PortOff: Offset from BaseAdress - in Bytes !
	UCHAR DWData = 0;
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	PortOffset;

	PortOffset = PortOff;
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_ReadByteS0,  // read one byte
							&PortOffset,        // Buffer to driver.
                            sizeof(PortOffset),
							&DWData,sizeof(DWData),&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("Read byte in space0 failed");};
	return DWData;
	};  // ReadByteS0




void WriteLongIOPort(UINT drvno,ULONG DWData, ULONG PortOff)
	{	// writes long to PCIruntime register
		// PortOff: Reg Offset from BaseAdress - in bytes
	BOOL fResult = FALSE;
	sDLDATA WriteData;
	ULONG	DataLength;
	DWORD   ReturnedLength;

	WriteData.POff	= PortOff;
	WriteData.Data	= DWData;
	DataLength		= 8; 
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_WriteLongIORunReg, 
							&WriteData,      
                            DataLength,
							NULL,0,&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("WriteLongIOPort failed");}
	};  // WriteLongIOPort


void WriteLongS0(UINT drvno,ULONG DWData, ULONG PortOff)
	{	// writes long to space0 register
		// PortOff: Reg Offset from BaseAdress - in longs
	BOOL fResult = FALSE;
	sDLDATA WriteData;
	ULONG	DataLength;
	DWORD   ReturnedLength;

	WriteData.POff= PortOff;
	WriteData.Data = DWData;
	DataLength = 8; 
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_WriteLongS0,  // 
							&WriteData,      
                            DataLength,
							NULL,0,&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("WriteLongS0 failed");}
	};  // WriteLongS0


void WriteByteS0(UINT drvno,BYTE DWData, ULONG PortOff)
	{	// writes byte to space0 register
		// PortOff: Reg Offset from BaseAdress - in longs
	BOOL fResult = FALSE;
	sDLDATA WriteData;
	ULONG	DataLength;
	DWORD   ReturnedLength;

	WriteData.POff= PortOff;
	WriteData.Data = DWData;
	DataLength = sizeof(WriteData); 
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_WriteByteS0,  // 
							&WriteData,      
                            DataLength,
							NULL,0,&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("WriteByteS0 failed");}
	};  // WriteByteS0



void AboutDrv(UINT drvno)
{	USHORT version = 0;
	ULONG S0Data = 0;
	BOOL fResult = FALSE;
	ULONG PortNumber = 0;		// must be 0
	DWORD   ReturnedLength=0;  // Number of bytes returned
	char pstring[22]="";
	char wstring[16]="";
	char astring[3]="";

	HWND hWnd = GetActiveWindow();
	HDC aDC = GetDC(hWnd); 
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_GetVersion,
					&PortNumber,        // Buffer to driver.
                    4,&version,sizeof(version),&ReturnedLength,NULL);
	if (fResult)
		{ // read driver version via DevIoCtl
		strcpy(wstring,"");
		strcat(wstring,"Driver LSCPCI");
		strcpy(astring,"");
		itoa(drvno,astring,16);
		strcat(wstring,astring); 
		strcpy(astring,"   ");
		itoa(version,astring,16);
		strcpy(pstring,"");
		strcat(pstring, " version: 0x");
		strcat(pstring,astring); 
		if (MessageBox( hWnd, pstring, wstring,MB_OK|MB_ICONEXCLAMATION ) == IDOK ) {};
		}
	else
		{ErrorMsg("About DeviceIo failed");};
	
	// read ISA Id from S0Base+7
	S0Data=	ReadLongS0(drvno,4); // Board ID =5053
	S0Data = S0Data>>16;

	//or
	//S0Data = (UCHAR)ReadByteS0(8); // ID=53
	strcpy(wstring,"");
	itoa(S0Data,wstring,16);
	strcpy(pstring,"     ID = 0x");	
	strcat(pstring,wstring);
	if (MessageBox( hWnd, pstring," Board ID ",MB_OK|MB_ICONEXCLAMATION ) == IDOK ) {};
	ReleaseDC(hWnd,aDC);
};



/* functions for managing controlbits in CtrlA register                         		   
                                                                           
   the bits of CtrlA register have these functions:            
  							                   
	DB5		DB4		DB3			DB2		DB1		DB0                  
	Slope	-	    TrigOut		XCK		IFC		V_ON                 
    1: pos	1: on   1: high		1: high	1: high	1: high	V=1       
    0: neg	0: off  0: low		0: low	0: low	0: low	V=VFAK        
  									   
	D7, D8 have no function
  
	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
  void LowSlope(UINT drvno)
  	{// clear bit D5
	BYTE CtrlA;
	CtrlA = ReadByteS0(drvno,4)& 0x0df;	
	WriteByteS0(drvno,CtrlA ,4);
	}; //LowSlope

  void HighSlope(UINT drvno)
  	{// set bit D5
	BYTE CtrlA;
	CtrlA = ReadByteS0(drvno,4) | 0x20;	
	WriteByteS0(drvno,CtrlA,4);
	}; //HighSlope

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Ausgabe eines High-Signals an Pin 17                                      */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void OutTrigLow(UINT drvno)
	{
	BYTE CtrlA;
	CtrlA = ReadByteS0(drvno,4) & 0xf7;
	WriteByteS0(drvno,CtrlA,4);
	};						//OutTrigLow

/*---------------------------------------------------------------------------*/


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Ausgabe eines Low-Signals an Pin 17                                       */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void OutTrigHigh(UINT drvno)
	{
	BYTE CtrlA;
	CtrlA = ReadByteS0(drvno,4) | 0x08;
	WriteByteS0(drvno,CtrlA,4);
	}; //OutTrigHigh


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Ausgabe eines PulseWidth breiten Rechteckpulses an Pin 17                 */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void OutTrigPulse (UINT drvno,ULONG PulseWidth)
  {
	OutTrigHigh(drvno);
	Sleep (PulseWidth);
	OutTrigLow(drvno);
  };

/*---------------------------------------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                                           
   Wait for raising edge of Pin #17 SubD = D6 in CtrlA register
   ReturnKey is 0 if trigger, else keycode (except space )              
   if keycode is space, the loop is not canceled          
                                                                             
	D6 depends on Slope (D5)
	HighSlope = TRUE  : pos. edge        
	HighSlope = FALSE : neg. edge         
                                                                      
	if ExtTrigFlag=FALSE this function is used to get the keyboard input                                                                           

  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void WaitTrigger(UINT drvno,BOOL ExtTrigFlag,BOOL *SpaceKey, BOOL *AbrKey)
	// returns if Trigger or Key
	{ 
	BOOL FirstLo = FALSE;
	BOOL HiEdge = FALSE;
	BOOL Abbr = FALSE;
	BOOL Space = FALSE;
	UCHAR ReturnKey =0;
	BYTE ReadTrigPin = 0;

	do 
		{
		if (ExtTrigFlag)
			{ 
			ReadTrigPin = ReadByteS0(drvno,4) & 0x040;
			if (ReadTrigPin == 0) FirstLo = TRUE; //first look for lo
			if (FirstLo) {if (ReadTrigPin > 0) HiEdge = TRUE;}; // then look for hi
			}
			else HiEdge = TRUE;

//#if _PS2KEYBOARD  //with PS2 keyboard
		ReturnKey = ReadKeyPort(drvno);
		if (ReturnKey==_ScanCode_Cancel) Abbr = TRUE;
		if (ReturnKey==_ScanCode_End) Space = TRUE;
//#else	//other keyboard -> do not use highest priority thread
		// or use Sleep to serve the interrupt
		if (GetAsyncKeyState(VK_ESCAPE))  Abbr = TRUE;
		if (GetAsyncKeyState(VK_SPACE))  Space = TRUE;
//#endif
		}
	while ((!HiEdge) && (! Abbr));
	if (Abbr) *AbrKey = TRUE;	//stops immediately
	if (Space) *SpaceKey = TRUE;	//stops after next trigger
  };// WaitTrigger


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

  void CloseShutter(UINT drvno)   // IFC = low
	{
	UCHAR CtrlA;
	CtrlA = ReadByteS0(drvno,4)& 0x0fd;	/* $FD = 1111 1101 */
	WriteByteS0(drvno,CtrlA ,4);
	}; //CloseShutter

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

  void OpenShutter(UINT drvno)   // IFC = high
	{
	UCHAR CtrlA;
	CtrlA = ReadByteS0(drvno,4)| 0x02 ;	
	WriteByteS0(drvno,CtrlA,4);
	}; //OpenShutter


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* set V_ON low (V = V_Fak)                                               */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

  void V_On(UINT drvno)
	{
	UCHAR CtrlA;
	CtrlA = ReadByteS0(drvno,4)| 0x01;
	WriteByteS0(drvno,CtrlA ,4);
	}; //V_On

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* set V_ON high (V = 1)                                                  */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

  void V_Off(UINT drvno)
	{
	UCHAR CtrlA;
	CtrlA = ReadByteS0(drvno,4) & 0xfe;	// $FE = 1111 1110 
	WriteByteS0(drvno,CtrlA,4);
	}; //V_Off


// optional Opto Couplers
  void SetOpto(UINT drvno,BYTE ch)
	{//sets signal=low
	BYTE ctrlc;
	ctrlc = ReadByteS0(drvno,6); 
	if (ch==2) {ctrlc |= 0x04;}
	else ctrlc |= 0x02;
	WriteByteS0(drvno,ctrlc,6);
	}; //SetOpto


  void RsetOpto(UINT drvno,BYTE ch)
  { //sets signal=high
	BYTE ctrlc;
	ctrlc = ReadByteS0(drvno,6); 
	if (ch==2) {ctrlc &= 0xfb;}
	else ctrlc &= 0xfd;
	WriteByteS0(drvno,ctrlc,6);
	}; //RsetOpto


  BOOL GetOpto(UINT drvno,BYTE ch)
	{//no input or low -> high / high input -> low 
	BYTE ctrlc;
	ctrlc = ReadByteS0(drvno,6); 
	if (ch==2) {ctrlc &= 0x04;}
	else ctrlc &= 0x02;
	if (ctrlc>0) return TRUE;
	return FALSE;
	}; //GetOpto


  void ClrRead(UINT drvno, UINT fftlines, UINT zadr, UINT ccdclrcount) 
	  //normal clear for Kamera is a complete read out
	  //most cams needs up to 10 complete reads for resetting the sensor
	  //depends how much it was overexposured
	{
	pArrayT dummy=NULL;
	UINT i;
    for (i=0;i<ccdclrcount;i++) 
				{
				//OutTrigHigh(drvno);
				GETCCD(drvno,dummy,fftlines,-1,zadr); 
				//OutTrigLow(drvno);
				};
	}; //ClrRead



  void ClrShCam(UINT drvno, UINT zadr) //clear for Shutter cameras
	{
	pArrayT dummy = NULL;
	CloseShutter(drvno);              //IFC=low
	Sleep(5);
	GETCCD(drvno,dummy,0,-1,zadr);
	Sleep(5);
	OpenShutter(drvno);               //IFC=High
	Sleep(5);
	}; //ClrShCam


UCHAR ReadKeyPort(UINT drvno)
	{		//Reads PS2 Key directly -> very low jitter
	// !!! works with PS2 keyboard only !!!!!!!!
	// on WINNT, getasynckeystate does not work with highest priority

	UCHAR Data = 0;
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	PortOffset = 0; //has no function

	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_ReadKey,  // read one byte
							&PortOffset,        // Buffer to driver.
                            sizeof(PortOffset),
							&Data,sizeof(Data),&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("Read Key Ioctl failed");
		exit(0); };
	return Data;
	};  // ReadKeyPort


void CAL16Bit(UINT drvno, UINT zadr)
//for ADC16061
{// calibrate 16 bit A/D converter
	//takes 272800 ND cycles
	pArrayT pReadArray = (pArrayT) calloc(aPIXEL[drvno]*4,sizeof(ArrayT)); 
	zadr = zadr|0x80;
	GETCCD(drvno,pReadArray,0,1,zadr);
	free(pReadArray);
}//CAL16Bit

void SetOvsmpl(UINT drvno, UINT zadr)
//for ADC16061
{// set Oversample / reset with cal16bit
	 // by calling GETCCD with adr 0x10 set
	pArrayT pReadArray = (pArrayT) calloc(aPIXEL[drvno]*4,sizeof(ArrayT)); 
	zadr = zadr|0x10;
	GETCCD(drvno,pReadArray,0,1,zadr);
	free(pReadArray);
}//SetOvsmpl

void SendCommand(UINT drvno, BYTE adr, BYTE data)
//for programming of seriell port for AD98xx
{//before calling IFC has to be set to low
	BYTE regorg=0;
	BYTE reg=0;
//	Sleep(1);
	WriteByteS0(drvno,adr,0); // write address to bus

	regorg = ReadByteS0(drvno,5);// ND pulse
	reg = regorg | 0x20;
	WriteByteS0(drvno,reg,5); // write address to bus
	WriteByteS0(drvno,regorg,5); // write address to bus

	WriteByteS0(drvno,data,0);	// write data to bus
	WriteByteS0(drvno, reg ,5); // ND pulse
	Sleep(1);
	WriteByteS0(drvno,regorg,5);	// write address to bus
//	Sleep(1);
}//SendCommand


void SetHiamp(UINT drvno, BOOL hiamp)
{
	if (_TI)
		{
		CloseShutter(drvno);// IFC=lo
		Sleep(1);
		
		if  (hiamp) {
			SendCommand( drvno, 0x99, 1);
		} //1
		else {
			SendCommand( drvno, 0x99, 1); } //0
			Sleep(1);
			OpenShutter(drvno);		// IFC=hi
			return;
		}

	if (_IR) CloseShutter(drvno);// IR uses #11 or #14
	if (hiamp) {V_On(drvno);}	//standard use #11 VON
	else {V_Off(drvno);}
}//SetHiamp


void SetAD(UINT drvno, BYTE adadr, BYTE addata)
//for AD98xx
{	BYTE reg=0;
	CloseShutter(drvno);// IFC=lo
	Sleep(1);	
	reg = 0;
	reg = adadr<<4;
	if ((adadr&0x80)==0x80) //neg
		reg |=0x01; //D8
  
	SendCommand( drvno, 0xA1, reg); //send adr
 	SendCommand( drvno, 0xB2, addata); //send data
 	SendCommand( drvno, 0xC3, 1); //load SH
 	SendCommand( drvno, 0xC3, 2); // SH send to AD
	Sleep(1);
	OpenShutter(drvno);		// IFC=hi
	Sleep(1);
}//SetAD



void SetADOff(UINT drvno, BYTE ofs, BOOL pos)
//for AD98xx
{ //0=1x .. 64=6x
	BYTE sign_adr = 5;
    if (pos) {}
		else sign_adr |= 0x80; //add sign bit in highest bit
	SetAD(drvno,sign_adr,ofs);
}//SetADOff


void SetADAmpRed(UINT drvno, BYTE amp)
//for AD98xx
{ //0=1x .. 64=6x
	if (amp>=64) amp=0x3F;
	SetAD(drvno,2,amp);
}//SetADAmpRed


void SetAD16Default(UINT drvno,UINT res)
{// for AD98xx
	UCHAR db;
	// SHA mode
	//db = 0xc8 ;  //1100 1000; 
	
	// cds mode Vin=4 0xD8
	// cds Vin=2 0x58
	// cds 
	db = 0xD8;  // cds mode
//	db = 0xC8;  // sha mode
//	if (res==8 )	db |= 01;  //set to 8 bit mode

	SetAD(drvno,0,db);  
	SetADAmpRed(1,0x0);
	SetADOff(drvno, 0,TRUE);
	SetDA(drvno, 0, 2);
}//SetAD16Default

void SetDA(UINT drvno, BYTE gain, BYTE ch)
//AD98xx setup
{ // ch=1->A  ,  CH=2->B
	BYTE lb = 0;
	BYTE hb = 0x40; // buffered
	BYTE g=0;

	if (ch==2)
		hb |= 0x80;  //chb=2

 
	g = gain>>4;
	hb |= g;
	g= gain<<4;
	lb |= g;

	CloseShutter(drvno);// IFC=lo
	Sleep(1);	
	SendCommand( drvno, 0xA1, hb); //send hi byte
 	SendCommand( drvno, 0xB2, lb); //send lo byte
 	SendCommand( drvno, 0xC3, 1); //load SH
 	SendCommand( drvno, 0xC3, 4); // SH send to DA
	Sleep(1);
	OpenShutter(drvno);		// IFC=hi
	Sleep(1);
}//SetDA


BOOL CheckFFTrig(UINT drvno) 
	//trigger FF for short pulses
{	// CtrlA register Bit 6 reads trigger FF
	// if CtrlA bit4 was set FF is activated, write 0 to bit4 clears FF
	// bit7 sets input to ff triggered mode
	BYTE ReadCtrlA =0;

	ReadCtrlA = ReadByteS0(drvno,4);
	ReadCtrlA |= 0x080; //set to triggered input
	ReadCtrlA |= 0x010; //en TrigFF
	WriteByteS0(drvno,ReadCtrlA,4);
	if ((ReadCtrlA&0x40)==0x040)  {// D6 high, recognize pulse
		WriteByteS0(drvno,ReadCtrlA&0xEF,4);//clears Trigger FF	
		WriteByteS0(drvno,ReadCtrlA|0x10,4);//activate again
		return TRUE;
	}
return FALSE;
}//CheckFFTrig


