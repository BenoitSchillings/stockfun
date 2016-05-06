//  Board.h				PCI V2.0
//	all functions for managing Interfaceboard
//	with & without Fifo  
//  new: data array ushort


#define BoardType  "PCI"
#define BoardVN  "2.0"

//  same header file for ISA and PCI version
BOOL CCDDrvInit(UINT drvno);	// init the driver -> true if found

void CCDDrvExit(UINT drvno);	// closes the driver
BOOL InitBoard(UINT drvno);	// init the board and alloc mem, call only once !
BOOL SetBoardVars(UINT drvno, BOOL sym, BOOL burst,ULONG pixel, ULONG waits,ULONG flag816,ULONG pportadr,
				  ULONG pclk, ULONG xckdelay);
BOOL ActMouse(UINT drvno);
BOOL DeactMouse(UINT drvno);

void ErrorMsg(char ErrMsg[20]);		// error msg box
ULONG ReadLongIOPort(UINT drvno,ULONG PortOff);// read long from IO runreg
ULONG ReadLongS0(UINT drvno,ULONG PortOff);	// read long from space0
UCHAR ReadByteS0(UINT drvno,ULONG PortOff);	// read byte from space0
void WriteLongIOPort(UINT drvno,ULONG DWData, ULONG PortOff);// write long to IO runreg
void WriteLongS0(UINT drvno,ULONG DWData, ULONG PortOff);// write long to space0
void WriteByteS0(UINT drvno,BYTE DWData, ULONG PortOff); // write byte to space0

BOOL GETCCD(UINT drvno,void* dioden, ULONG fftlines, long fkt, ULONG zadr); 
// camera read function
void ClrRead(UINT drvno, UINT fftlines, UINT zadr, UINT ccdclrcount);
// clear camera with reads
void ClrShCam(UINT drvno, UINT zadr);// clears Shuttercamera with IFC signal

void AboutDrv(UINT drvno);	// displays the version and board ID = test if board is there

//	functions for managing controlbits in CtrlA register
void HighSlope(UINT drvno);		//set input Trigger slope high
void LowSlope(UINT drvno);		//set input Trigger slope low
void OutTrigHigh(UINT drvno);		//set output Trigger signal high
void OutTrigLow(UINT drvno);		//set output Trigger signal low
void OutTrigPulse(UINT drvno,ULONG PulseWidth);	// pulses high output Trigger signal
void WaitTrigger(UINT drvno,BOOL ExtTrigFlag, BOOL *SpaceKey, BOOL *EscapeKey);	
	// waits for trigger input or Key
BOOL CheckFFTrig(UINT drvno);		// trigger sets FF - clear via write CtrlA 0x10

void OpenShutter(UINT drvno);		// set IFC=high
void CloseShutter(UINT drvno);	// set IFC=low
void V_On(UINT drvno);			// set V_On signal low (V = V_Fak)
void V_Off(UINT drvno);			// set V_On signal high (V = 1)

void SetOpto(UINT drvno,BYTE ch);  // set opto channel if output
void RsetOpto(UINT drvno,BYTE ch); // reset opto channel if output
BOOL GetOpto(UINT drvno,BYTE ch);	//read opto channel if input

// new Keyboard read which is not interrupt dependend
// reads OEM scan code directly on port 0x60
UCHAR ReadKeyPort(UINT drvno  );

//old 16bit ADs functions
void CAL16Bit(UINT drvno, UINT zadr);
void SetOvsmpl(UINT drvno, UINT zadr);

//TIs electron multiplier
void SetHiamp(UINT drvno, BOOL hiamp);

//programming interface for 16 bit A/D-cds
void SendCommand(UINT drvno, BYTE adr, BYTE data);
void SetAD(UINT drvno, BYTE adadr, BYTE addata);
// 16 Bit AD with cds
void SetADOff(UINT drvno, BYTE ofs, BOOL pos);
void SetADAmpRed(UINT drvno, BYTE amp);
void SetAD16Default(UINT drvno,UINT res);
void SetDA(UINT drvno, BYTE gain, BYTE ch);

