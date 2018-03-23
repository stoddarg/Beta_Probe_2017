/******************************************************************************
*
* Main Dev Kit Application
* Revision 1.0
*
* Notes:
* Initial Software package that is developed for the Dev Kit.  07/06/16 - EBJ
*
******************************************************************************/

#include "LApp.h"

//struct definitions
struct adc_event{
	unsigned int total_events;
	unsigned int event_number;
	unsigned int baseline_int;
	unsigned int short_int;
	unsigned int long_int;
	unsigned int full_int;
	unsigned int footer;
	unsigned int adc_ch_0;
	unsigned int adc_ch_1;
	unsigned int adc_ch_2;
	unsigned int adc_ch_3;
	unsigned int adc_ch_4;
	unsigned int adc_ch_5;
	unsigned int adc_ch_6;
	unsigned int adc_ch_7;
	unsigned int adc_ch_8;
	unsigned int adc_ch_9;
	unsigned int adc_ch_10;
	unsigned int adc_ch_11;
	unsigned int adc_ch_12;
	unsigned int adc_ch_13;
	unsigned int adc_ch_14;
	unsigned int adc_ch_15;
};

//////////////////////////// MAIN //////////////////// MAIN //////////////
int main()
{
	// Initialize System
    init_platform();  		// This initializes the platform, which is ...
	ps7_post_config();
	Xil_DCacheDisable();	//
	InitializeAXIDma();		// Initialize the AXI DMA Transfer Interface
	Xil_Out32 (XPAR_AXI_GPIO_16_BASEADDR, 16384);
	Xil_Out32 (XPAR_AXI_GPIO_17_BASEADDR , 1);
	InitializeInterruptSystem(XPAR_PS7_SCUGIC_0_DEVICE_ID);

	// Initialize buffers
	memset(RecvBuffer, '0', 32);	// Clear RecvBuffer Variable
	memset(SendBuffer, '0', 32);	// Clear SendBuffer Variable

	//*******************Setup the UART **********************//
	XUartPs_Config *Config = XUartPs_LookupConfig(UART_DEVICEID);
	if (NULL == Config) { return 1;}
	Status = XUartPs_CfgInitialize(&Uart_PS, Config, Config->BaseAddress);
	if (Status != 0){ xil_printf("XUartPS did not CfgInit properly.\n");	}

//	/* Conduct a Selftest for the UART */
//	Status = XUartPs_SelfTest(&Uart_PS);
//	if (Status != 0) { xil_printf("XUartPS failed self test.\n"); }			//handle error checks here better

	/* Set to normal mode. */
	XUartPs_SetOperMode(&Uart_PS, XUARTPS_OPER_MODE_NORMAL);
	//*******************Setup the UART **********************//

	//*******************Receive and Process Packets **********************//
	Xil_Out32 (XPAR_AXI_GPIO_0_BASEADDR, 38);
	Xil_Out32 (XPAR_AXI_GPIO_1_BASEADDR, 73);
	Xil_Out32 (XPAR_AXI_GPIO_2_BASEADDR, 169);
	Xil_Out32 (XPAR_AXI_GPIO_3_BASEADDR, 1500);
/*	Xil_Out32 (XPAR_AXI_GPIO_4_BASEADDR, 12);
	Xil_Out32 (XPAR_AXI_GPIO_5_BASEADDR, 75);
	Xil_Out32 (XPAR_AXI_GPIO_6_BASEADDR, 75);
	Xil_Out32 (XPAR_AXI_GPIO_7_BASEADDR, 5);
	Xil_Out32 (XPAR_AXI_GPIO_8_BASEADDR, 25);*/
	//*******************enable the system **********************//
	Xil_Out32(XPAR_AXI_GPIO_18_BASEADDR, 1);
	//*******************Receive and Process Packets **********************//

	// *********** Setup the Hardware Reset GPIO ****************//
	GPIOConfigPtr = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
	Status = XGpioPs_CfgInitialize(&Gpio, GPIOConfigPtr, GPIOConfigPtr ->BaseAddr);
	if (Status != XST_SUCCESS) { return XST_FAILURE; }
	XGpioPs_SetDirectionPin(&Gpio, SW_BREAK_GPIO, 1);
	// *********** Setup the Hardware Reset MIO ****************//

	// *********** Mount SD Card and Initialize Variables ****************//
	if( doMount == 0 ){			//Initialize the SD card here
		ffs_res = f_mount(0, "", 0);
		ffs_res = f_mount(&fatfs, "", 0);
		doMount = 1;
	}

	// *********** Mount SD Card and Initialize Variables ****************//

	//set variables
	int index = 0;
	int ipollReturn = 0;
	int bytesSent = 0;

	// ******************* POLLING LOOP *******************//
	while(1){
		XUartPs_SetOptions(&Uart_PS,XUARTPS_OPTION_RESET_RX);	// Clear UART Read Buffer
		memset(RecvBuffer, '0', 32);							// Clear RecvBuffer Variable

		//sleep(0.5);  // Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 0.5 s
		xil_printf("\n\rDevkit version 2.25 \n\r");
		xil_printf("MAIN MENU \n\r");
		xil_printf("******\n\r");
		xil_printf(" 0) Set Mode of Operation\n\r");
		xil_printf(" 1) Enable or Disable the system\n\r");
		xil_printf(" 2) Stream Processed Data\n\r");
		xil_printf("\n\r***Setup Parameters *** \n\r");
		xil_printf(" 3) Set Trigger Threshold\n\r");
		xil_printf(" 4) Set Integration Times (Number of Clock Cycles * 4ns) \n\r");
		xil_printf("******\n\r");

		ReadCommandPoll();
		menusel = 99999;
		sscanf((char *)RecvBuffer,"%02u",&menusel);
		if ( menusel < 0 || menusel > 4 ) {
			xil_printf(" Invalid Command: Enter 0-4 \n\r");
			sleep(1);
		}

		switch (menusel) { // Switch-Case Menu Select

		case 0: //Set Mode of Operation
			mode = 99; //Detector->GetMode();
			xil_printf("\n\r Waveform Data: \t Enter 0 <return>\n\r");
			xil_printf(" LPF Waveform Data: \t Enter 1 <return>\n\r");
			xil_printf(" DFF Waveform Data: \t Enter 2 <return>\n\r");
			xil_printf(" TRG Waveform Data: \t Enter 3 <return>\n\r");
			xil_printf(" Processed Data: \t Enter 4 <return>\n\r");
			ReadCommandPoll();
			sscanf((char *)RecvBuffer,"%01u",&mode);

			if (mode < 0 || mode > 4 ) { xil_printf("Invalid Command\n\r"); break; }
			Xil_Out32 (XPAR_AXI_GPIO_14_BASEADDR, ((u32)mode));
			// Register 14
			if ( mode == 0 ) { xil_printf("Transfer AA Waveforms\n\r"); }
			if ( mode == 1 ) { xil_printf("Transfer LPF Waveforms\n\r"); }
			if ( mode == 2 ) { xil_printf("Transfer DFF Waveforms\n\r"); }
			if ( mode == 3 ) { xil_printf("Transfer TRG Waveforms\n\r"); }
			if ( mode == 4 ) { xil_printf("Transfer Processed Data\n\r"); }
			sleep(1);

			break;

		case 1: //Enable or disable the system
			xil_printf("\n\r Disable: Enter 0 <return>\n\r");
			xil_printf(" Enable: Enter 1 <return>\n\r");
			ReadCommandPoll();
			sscanf((char *)RecvBuffer,"%01u",&enable_state);
			if (enable_state != 0 && enable_state != 1) { xil_printf("Invalid Command\n\r"); break; }
			Xil_Out32(XPAR_AXI_GPIO_18_BASEADDR, ((u32)enable_state));
			// Register 18 Out enabled, In Disabled
			if ( enable_state == 1 ) { xil_printf("DAQ Enabled\n\r"); }
			if ( enable_state == 0 ) { xil_printf("DAQ Disabled\n\r"); }
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s

			dTime += 1;
			iSprintfReturn = snprintf(cWriteToLogFile, LOG_FILE_BUFF_SIZE, "Enable system %d %f ", enable_state, dTime);

			ffs_res = f_open(&logFile, cLogFile, FA_OPEN_ALWAYS | FA_WRITE);
			if(ffs_res){
				xil_printf("Could not open file %d", ffs_res);
				break;
			}
			ffs_res = f_lseek(&logFile, filptr_clogFile);
			ffs_res = f_write(&logFile, cWriteToLogFile, iSprintfReturn, &numBytesWritten);
			filptr_clogFile += numBytesWritten;												// Add the number of bytes written to the file pointer
			snprintf(cWriteToLogFile, 10, "%d", filptr_clogFile);							// Write that to a string for saving
			ffs_res = f_lseek(&logFile, (10 - LNumDigits(filptr_clogFile)));				// Seek to the beginning of the file skipping the leading zeroes
			ffs_res = f_write(&logFile, cWriteToLogFile, LNumDigits(filptr_clogFile), &numBytesWritten); // Write the new file pointer
			ffs_res = f_close(&logFile);

			break;

		case 2: //Continuously Read of Processed Data
			// the user needs to set the mode before coming here; see case 0
			enable_state = 1;
			Xil_Out32 (XPAR_AXI_GPIO_18_BASEADDR, ((u32)enable_state));	//set enable device

			ipollReturn = 0;
			bytesSent = 0;
			index = 0;

			while(ipollReturn != 113)	//loop until getting a 'q' in the RecvBuffer
			{
				ipollReturn = PollUart(RecvBuffer, &Uart_PS);	//return value of 97(a), 113(q), or 0(else)
				if(Xil_In32(XPAR_AXI_GPIO_11_BASEADDR) > 0)	//if the buffer is full, read it to the screen
				{
					DAQ();
					ipollReturn = PrintData();
				}
			}

			break;

		case 3: //Set Threshold
			iThreshold0 = Xil_In32(XPAR_AXI_GPIO_10_BASEADDR);
			xil_printf("\n\r Existing Threshold = %d \n\r",Xil_In32(XPAR_AXI_GPIO_10_BASEADDR));
			xil_printf(" Enter Threshold (0 to 10240) <return> \n\r");
			ReadCommandPoll();
			sscanf((char *)RecvBuffer,"%05u",&iThreshold1);
			if((iThreshold1 < 0) || (iThreshold1 > 10240)) { xil_printf("Invalid command\r\n"); break; }
			Xil_Out32(XPAR_AXI_GPIO_10_BASEADDR, ((u32)iThreshold1));
			xil_printf("New Threshold = %d \n\r",Xil_In32(XPAR_AXI_GPIO_10_BASEADDR));
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			break;
		case 4: //Set Integration Times
			setIntsArray[4] = -52 + ((int)Xil_In32(XPAR_AXI_GPIO_0_BASEADDR))*4;
			setIntsArray[5] = -52 + ((int)Xil_In32(XPAR_AXI_GPIO_1_BASEADDR))*4;
			setIntsArray[6] = -52 + ((int)Xil_In32(XPAR_AXI_GPIO_2_BASEADDR))*4;
			setIntsArray[7] = -52 + ((int)Xil_In32(XPAR_AXI_GPIO_3_BASEADDR))*4;

			xil_printf("\n\r Existing Integration Times \n\r");
			xil_printf(" Time = 0 ns is when the Pulse Crossed Threshold \n\r");
			xil_printf(" Baseline Integral Window \t [-200ns,%dns] \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_0_BASEADDR))*4 );
			xil_printf(" Short Integral Window \t [-200ns,%dns] \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_1_BASEADDR))*4 );
			xil_printf(" Long Integral Window  \t [-200ns,%dns] \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_2_BASEADDR))*4 );
			xil_printf(" Full Integral Window  \t [-200ns,%dns] \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_3_BASEADDR))*4 );
			xil_printf(" Change: (Y)es (N)o <return>\n\r");
			ReadCommandPoll();
			sscanf((char *)RecvBuffer,"%c",&updateint);

			if (updateint == 'N' || updateint == 'n')
				break;
			if (updateint == 'Y' || updateint == 'y')
				SetIntegrationTimes(&setIntsArray, &setSamples);
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			break;

		case 5: //Perform a DMA transfer
			xil_printf("\n\r Perform DMA Transfer of Waveform Data\n\r");
			xil_printf(" Press 'q' to Exit Transfer  \n\r");
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000);
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			PrintData();		// Display data to console.
			sw = 0;   // broke out of the read loop, stop swith reset to 0
			break;

		case 6: //Perform a DMA transfer of Processed data
			xil_printf("\n\r ********Perform DMA Transfer of Processed Data \n\r");
			xil_printf(" Press 'q' to Exit Transfer  \n\r");
			//Xil_Out32 (XPAR_AXI_GPIO_18_BASEADDR, 0);	// Disable : GPIO Reg Capture Module
			Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 1);	// Enable: GPIO Reg to Readout Data MUX
			//sleep(1);				// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000); // Transfer from BRAM to DRAM, start address 0xa000000, 16-bit length
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 0); 	// Disable: GPIO Reg turn off Readout Data MUX
			ClearBuffers();
			PrintData();								// Display data to console.
			//Xil_Out32 (XPAR_AXI_GPIO_18_BASEADDR, 1);	// Enable : GPIO Reg Capture Module
			sw = 0;   // broke out of the read loop, stop swith reset to 0
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			break;

		case 7: //Check the Size of the Data Buffers
			databuff = Xil_In32 (XPAR_AXI_GPIO_11_BASEADDR);
			xil_printf("\n\r BRAM Data Buffer Size = %d \n\r",databuff);
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			break;

		case 8: //Clear the processed data buffers
			xil_printf("\n\r Clear the Data Buffers\n\r");
			ClearBuffers();
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			break;

		case 9: //Print DMA Data
			xil_printf("\n\r Print Data\n\r");
			PrintData();
			break;
		case 10: //Transfer data over serial port	// Skips setting integrals, threshold	//Difference is this doesn't save to sd, it prints out the data for us
			//xil_printf("321");	//echo back to the gui that the request was received
			mode = 0;
			Xil_Out32 (XPAR_AXI_GPIO_14_BASEADDR, ((u32)mode));	//sets mode to transfer AA waveforms
			sleep(1);
			enable_state = 1;
			Xil_Out32(XPAR_AXI_GPIO_18_BASEADDR, ((u32)enable_state));	//enables the system
			sleep(1);
			getWFDAQ();
			break;
		case 11: //change threshold over the serial connection
			xil_printf("Enter the new threshold: <enter>\n");
			ReadCommandPoll();
			sscanf((char *)RecvBuffer,"%04u",&iThreshold1);
			Xil_Out32(XPAR_AXI_GPIO_10_BASEADDR, ((u32)iThreshold1));
			xil_printf("New Threshold = %d \n\r",Xil_In32(XPAR_AXI_GPIO_10_BASEADDR));
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			break;
		case 12: //change integrals over the serial connection
			xil_printf("Enter each integral time followed by <enter>");
			ReadCommandPoll();
			sscanf((char *)RecvBuffer,"%04u",&setBL);
			ReadCommandPoll();
			sscanf((char *)RecvBuffer,"%04u",&setSI);
			ReadCommandPoll();
			sscanf((char *)RecvBuffer,"%04u",&setLI);
			ReadCommandPoll();
			sscanf((char *)RecvBuffer,"%04u",&setFI);

			Xil_Out32 (XPAR_AXI_GPIO_0_BASEADDR, ((u32)((setBL+52)/4)));	//set baseline int time
			Xil_Out32 (XPAR_AXI_GPIO_1_BASEADDR, ((u32)((setSI+52)/4)));	//set short int time
			Xil_Out32 (XPAR_AXI_GPIO_2_BASEADDR, ((u32)((setLI+52)/4)));	//set long int time
			Xil_Out32 (XPAR_AXI_GPIO_3_BASEADDR, ((u32)((setFI+52)/4)));	//set full int time

			//echo back the changed values //for checking
			xil_printf("  Inputs Rounded to the Nearest 4 ns : Number of Samples\n\r");
			xil_printf("  Baseline Integral Window  [-200ns,%dns]: %d \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_0_BASEADDR))*4, 38+Xil_In32(XPAR_AXI_GPIO_0_BASEADDR) );
			xil_printf("  Short Integral Window 	  [-200ns,%dns]: %d \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_1_BASEADDR))*4, 38+Xil_In32(XPAR_AXI_GPIO_1_BASEADDR));
			xil_printf("  Long Integral Window      [-200ns,%dns]: %d \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_2_BASEADDR))*4, 38+Xil_In32(XPAR_AXI_GPIO_2_BASEADDR));
			xil_printf("  Full Integral Window      [-200ns,%dns]: %d \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_3_BASEADDR))*4, 38+Xil_In32(XPAR_AXI_GPIO_3_BASEADDR));
			break;
		case 13: //Transfer processed data over the serial port
			mode = 4;
			Xil_Out32 (XPAR_AXI_GPIO_14_BASEADDR, ((u32)mode));	//sets mode to processed data
			sleep(1);
			enable_state = 1;
			Xil_Out32(XPAR_AXI_GPIO_18_BASEADDR, ((u32)enable_state)); //enables the system
			sleep(1);
			getWFDAQ();
			break;
		case 14: //HV and temp control

			break;
		case 15:
			ffs_res = f_open(&directoryLogFile, cDirectoryLogFile0, FA_READ);
			dirSize = file_size(&directoryLogFile) - 10;
			filptr_cDIRFile = 10;
			dirFileContents = malloc(1 * dirSize + 1);
			ffs_res = f_lseek(&directoryLogFile, 10);
			ffs_res = f_read(&directoryLogFile, dirFileContents, dirSize, &numBytesRead);
			ffs_res = f_close(&directoryLogFile);
			snprintf(dirFileContents, dirSize + 1, dirFileContents + '\0');
			xil_printf(dirFileContents);
			free(dirFileContents);
			break;
		case 16:
			//DAQ minimal function
			//need to loop over this function
			// utilize DAQ function below (as opposed to WFDAQ)
			break;

		default :
			break;
		} // End Switch-Case Menu Select

	}	// ******************* POLLING LOOP *******************//

    cleanup_platform();
    return 0;
}
//////////////////////////// MAIN //////////////////// MAIN //////////////

//////////////////////////// InitializeAXIDma////////////////////////////////
// Sets up the AXI DMA
int InitializeAXIDma(void) {
	u32 tmpVal_0 = 0;

	tmpVal_0 = Xil_In32(XPAR_AXI_DMA_0_BASEADDR + 0x30);

	tmpVal_0 = tmpVal_0 | 0x1001; //<allow DMA to produce interrupts> 0 0 <run/stop>

	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x30, tmpVal_0);
	Xil_In32(XPAR_AXI_DMA_0_BASEADDR + 0x30);	//what does the return value give us? What do we do with it?

	return 0;
}
//////////////////////////// InitializeAXIDma////////////////////////////////

//////////////////////////// InitializeInterruptSystem////////////////////////////////
int InitializeInterruptSystem(u16 deviceID) {
	int Status;

	GicConfig = XScuGic_LookupConfig (deviceID);

	if(NULL == GicConfig) {

		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, GicConfig, GicConfig->CpuBaseAddress);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;

	}

	Status = SetUpInterruptSystem(&InterruptController);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;

	}

	Status = XScuGic_Connect (&InterruptController,
			XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR,
			(Xil_ExceptionHandler) InterruptHandler, NULL);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;

	}

	XScuGic_Enable(&InterruptController, XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR );

	return XST_SUCCESS;

}
//////////////////////////// InitializeInterruptSystem////////////////////////////////


//////////////////////////// Interrupt Handler////////////////////////////////
void InterruptHandler (void ) {

	u32 tmpValue;
	tmpValue = Xil_In32(XPAR_AXI_DMA_0_BASEADDR + 0x34);
	tmpValue = tmpValue | 0x1000;
	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x34, tmpValue);

	global_frame_counter++;
}
//////////////////////////// Interrupt Handler////////////////////////////////


//////////////////////////// SetUp Interrupt System////////////////////////////////
int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr) {
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, XScuGicInstancePtr);
	Xil_ExceptionEnable();
	return XST_SUCCESS;

}
//////////////////////////// SetUp Interrupt System////////////////////////////////

//////////////////////////// ReadCommandPoll////////////////////////////////
// Function used to clear the read buffer
// Read In new command, expecting a <return>
// Returns buffer size read
int ReadCommandPoll() {
	u32 rbuff = 0;			// read buffer size returned

	XUartPs_SetOptions(&Uart_PS,XUARTPS_OPTION_RESET_RX);	// Clear UART Read Buffer
	memset(RecvBuffer, '0', 32);	// Clear RecvBuffer Variable
	while (!(RecvBuffer[rbuff-1] == '\n' || RecvBuffer[rbuff-1] == '\r'))
	{
		rbuff += XUartPs_Recv(&Uart_PS, &RecvBuffer[rbuff],(32 - rbuff));
		sleep(0.1);			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 0.1 s
	}
	return rbuff;
}
//////////////////////////// ReadCommandPoll////////////////////////////////


//////////////////////////// Set Integration Times ////////////////////////////////
// wfid = 	0 for AA
//			1 for DFF
//			2 for LPF
// At the moment, the software is expecting a 5 char buffer.  This needs to be fixed.
void SetIntegrationTimes(int * setIntsArray, u32 * setSamples){
	//int setIntsArray[8] = {};
	//u32 setsamples[8] = {};

	xil_printf("Enter Baseline Stop Time in ns: -52 to 0 <return> \r\n");
	ReadCommandPoll();
	sscanf((char *)RecvBuffer,"%d",&setIntsArray[0]);

	xil_printf("\n\rEnter Short Integral Stop Time in ns: -52 to 8000 <return> \n\r");
	ReadCommandPoll();
	sscanf((char *)RecvBuffer,"%d",&setIntsArray[1]);

	xil_printf("\n\rEnter Long Integral Stop Time in ns: -52 to 8000 <return> \n\r");
	ReadCommandPoll();
	sscanf((char *)RecvBuffer,"%d",&setIntsArray[2]);

	xil_printf("\n\rEnter Full Integral Stop Time in ns: -52 to 8000 <return> \n\r");
	ReadCommandPoll();
	sscanf((char *)RecvBuffer,"%d",&setIntsArray[3]);

	setSamples[0] = ((u32)((setIntsArray[0]+52)/4));
	setSamples[1] = ((u32)((setIntsArray[1]+52)/4));
	setSamples[2] = ((u32)((setIntsArray[2]+52)/4));
	setSamples[3] = ((u32)((setIntsArray[3]+52)/4));

	Xil_Out32 (XPAR_AXI_GPIO_0_BASEADDR, setSamples[0]);
	Xil_Out32 (XPAR_AXI_GPIO_1_BASEADDR, setSamples[1]);
	Xil_Out32 (XPAR_AXI_GPIO_2_BASEADDR, setSamples[2]);
	Xil_Out32 (XPAR_AXI_GPIO_3_BASEADDR, setSamples[3]);

	xil_printf("\n\rInputs Rounded to the Nearest 4 ns : Number of Samples\n\r");
	xil_printf("Baseline Integral Window  [-200ns,%dns]: %d \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_0_BASEADDR))*4, 38+Xil_In32(XPAR_AXI_GPIO_0_BASEADDR) );
	xil_printf("Short Integral Window 	  [-200ns,%dns]: %d \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_1_BASEADDR))*4, 38+Xil_In32(XPAR_AXI_GPIO_1_BASEADDR));
	xil_printf("Long Integral Window      [-200ns,%dns]: %d \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_2_BASEADDR))*4, 38+Xil_In32(XPAR_AXI_GPIO_2_BASEADDR));
	xil_printf("Full Integral Window      [-200ns,%dns]: %d \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_3_BASEADDR))*4, 38+Xil_In32(XPAR_AXI_GPIO_3_BASEADDR));

	xil_printf("value of gpio 3: %d\r\n", Xil_In32(XPAR_AXI_GPIO_3_BASEADDR));

	return;
}
//////////////////////////// Set Integration Times ////////////////////////////////

//////////////////////////// PrintData ////////////////////////////////
int PrintData( ){
	int ipollReturn = 0;
	unsigned int data;
	int bytes_sent = 0;
	int dram_addr;
	int dram_base = 0xa000000;
	int dram_ceiling = 0xA004000; //read out just adjacent average (0xA004000 - 0xa000000 = 16384)

	struct adc_event adc_event = {0,0,0,0,0,0,0,0,43,42,43,44,45,46,47,48,49,40,41,42,43,44,45};
	struct adc_event * a = &adc_event;

	Xil_DCacheInvalidateRange(0x00000000, 65536);

	for (dram_addr = dram_base; dram_addr <= dram_ceiling; dram_addr+=4)
	{
		data = Xil_In32(dram_addr);
		printf("%u\r\n",data);
		ipollReturn = PollUart(RecvBuffer, &Uart_PS);	//return value of 97(a), 113(q), or 0(else)
		if(ipollReturn == 113)
			break;

		//work on the code to sort input from the ADC into a struct then print the struct using XUartPs_Send
		if(data==111111)	//we have an event, sort it into the struct then print out the struct
		{
//			adc_event.total_events = Xil_In32(dram_addr + 4);
			printf("%u\r\n",(unsigned int)Xil_In32(dram_addr + 4));
//			adc_event.event_number = Xil_In32(dram_addr + 8);
			printf("%u\r\n",(unsigned int)Xil_In32(dram_addr + 8));
//			adc_event.baseline_int = Xil_In32(dram_addr + 12);
			printf("%u\r\n",(unsigned int)Xil_In32(dram_addr + 12));
//			adc_event.short_int = Xil_In32(dram_addr + 16);
			printf("%u\r\n",(unsigned int)Xil_In32(dram_addr + 16));
//			adc_event.long_int = Xil_In32(dram_addr + 20);
			printf("%u\r\n",(unsigned int)Xil_In32(dram_addr + 20));
//			adc_event.full_int = Xil_In32(dram_addr + 24);
			printf("%u\r\n",(unsigned int)Xil_In32(dram_addr + 24));
//			adc_event.footer = Xil_In32(dram_addr + 28);
			printf("%u\r\n",(unsigned int)Xil_In32(dram_addr + 28));

			for(int i = 0; i < 16; i++)
			{
				printf("%d\r\n",i);
			}
			dram_addr += 28;
		}
	}

	return ipollReturn;
}
//////////////////////////// PrintData ////////////////////////////////

//////////////////////////// Clear Processed Data Buffers ////////////////////////////////
void ClearBuffers() {
	Xil_Out32(XPAR_AXI_GPIO_9_BASEADDR,1);
	usleep(1);
	Xil_Out32(XPAR_AXI_GPIO_9_BASEADDR,0);
}
//////////////////////////// Clear Processed Data Buffers ////////////////////////////////

//////////////////////////// DAQ ////////////////////////////////
int DAQ() {
	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000); 		// DMA Transfer Step 1
	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);			// DMA Transfer Step 2

	Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 1);
	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000);
	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);
	usleep(54);
	Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 0);

	ClearBuffers();

	return 0;
}
//////////////////////////// DAQ ////////////////////////////////

//////////////////////////// getWFDAQ ////////////////////////////////

int getWFDAQ(){
	int buffsize; 	//BRAM buffer size
	//int dram_addr;	// DRAM Address

	XUartPs_SetOptions(&Uart_PS,XUARTPS_OPTION_RESET_RX);

	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000); 		// DMA Transfer Step 1
	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);			// DMA Transfer Step 2
	sleep(1);						// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
	ClearBuffers();
	//for (dram_addr = 0xa000000; dram_addr <= 0xA004000; dram_addr+=4){Xil_In32(dram_addr);}

	while(1){
		if (!sw) { sw = XGpioPs_ReadPin(&Gpio, SW_BREAK_GPIO); } //read pin
		XUartPs_Recv(&Uart_PS, (u8 *)RecvBuffer, 32);
		if ( RecvBuffer[0] == 'q' ) { sw = 1; }
		if(sw) { return sw;	}

		buffsize = Xil_In32 (XPAR_AXI_GPIO_11_BASEADDR);	// AA write pointer // tells how far the system has read in the AA module
		if(buffsize >= 4095)
		{
			Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 1);				// init mux to transfer data between integrater modules to DMA
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000);
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);
			sleep(1); 												// this will change
			Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 0);

			ClearBuffers();
			PrintData();
		}
	}
	return sw;
}

//////////////////////////// getWFDAQ ////////////////////////////////
