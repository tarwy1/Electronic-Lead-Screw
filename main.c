#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "stdio.h"

long DelayTime = 4000; // delay constant between data pulses

__interrupt void PinOne(void);

void WriteDataToDisplay(uint16_t t);

__interrupt void PinTwo(void);

// keep lathe in A position (ratio, 1)


//////////////////////////////////
// communication connections:   red-blue   black - green   brown-yellow   51 31 39
//////////////////////////////////

float EncoderCount = 0; // step count for encoder - decimal count with ratio
float ScalingFactor = 0; // factor to convert encoder steps to stepper steps
float ThreadPitch = 1.75f; // threading pitch (mm)
float FeedSpeed = 0.03f; // feed speed (mm/rev)
float EncoderRatio = 2.0f;
char ThreadBool = 'F'; // threading or not - 'F' = feeding 'T' = threading
int StepBuffer = 0; // buffer for steps, for use with scaling factors over 1
int RotationDirection = 0; // rotation direction 1 = clockwise 0 = anti
int Microsteps = 800; // number of pulses per stepper motor rotation
int StepperEnabled = 0;
int EncoderRPMCount = 0;
int RPM = 0;

///////////////////////////////////////////////
//  LED1 - 6500  thread/feed
//  LED3 - 6502  fwd/rev
//  LED5 - 6504  mm/in
///////////////////////////////////////////////

main(void){

    ScalingFactor = (Microsteps*FeedSpeed*EncoderRatio)/(3*1024); // initializing scaling factor to be a 0.04mm/rev feed

    Device_init();

    Device_initGPIO();

    Board_init();

    Interrupt_initModule();

    Interrupt_initVectorTable();


    // setting up cpu interrupt for encoder pin 1
    /////////////////////////////////////////////////
    GPIO_setInterruptType(GPIO_INT_XINT1, GPIO_INT_TYPE_RISING_EDGE);

    GPIO_setInterruptPin(EncoderSignalOne, GPIO_INT_XINT1);

    GPIO_enableInterrupt(GPIO_INT_XINT1);

    Interrupt_register(INT_XINT1, &PinOne);

    Interrupt_enable(INT_XINT1);
    /////////////////////////////////////////////////



    // setting up cpu interrupt for encoder pin 2
    /////////////////////////////////////////////////
    GPIO_setInterruptType(GPIO_INT_XINT2, GPIO_INT_TYPE_RISING_EDGE);

    GPIO_setInterruptPin(EncoderSignalTwo, GPIO_INT_XINT2);

    GPIO_enableInterrupt(GPIO_INT_XINT2);

    Interrupt_register(INT_XINT2, &PinTwo);

    Interrupt_enable(INT_XINT2);
    /////////////////////////////////////////////////


    Interrupt_enableMaster();

    EINT;
    ERTM;

    GPIO_writePin(DisplayDataPin, 0); // default data to 0

    GPIO_writePin(DisplayClockPin, 0); // default clock to 0

    GPIO_writePin(DisplayDataEnable, 0); // default dataenable to 0

    GPIO_writePin(StepperPin, 0); // initialize pulse pin to 0

    GPIO_writePin(StepperDirection, 0); // initialize direction pin to 0 (clockwise)

    DEVICE_DELAY_US(3000000); // startup delay

    WriteDataToDisplay(6500);

    WriteDataToDisplay(FeedSpeed*100);

    int ThreadHigh = 0;

    int PlusHigh = 0;

    int MinusHigh = 0;

    int StepperEnableInputHigh = 0;

    int DirectionHigh = 0;

    //int UnitHigh = 0;

    int PreviousEncoderCount = 0;

    int PreviousRPM = 0;

    int RPMUpdateCount = 0;

    GPIO_writePin(StepperEnableOutput, StepperEnabled);

    while(1) // forever loop running while  not interrupt
    {
        //////////////////////////////////////////////
        if(GPIO_readPin(ThreadTogglePin)==1 && ThreadHigh == 0)
        {
            ThreadHigh = 1;
            if(ThreadBool == 'F'){
                ThreadBool = 'T';
                ScalingFactor = (Microsteps*ThreadPitch*EncoderRatio)/(3*1024);
                WriteDataToDisplay(ThreadPitch*100.0f+0.005f);
                WriteDataToDisplay(6500);
            }
            else{
                ThreadBool = 'F';
                ThreadPitch = 1.75f;
                ScalingFactor = (Microsteps*FeedSpeed*EncoderRatio)/(3*1024);
                WriteDataToDisplay(FeedSpeed*100.0f);
                WriteDataToDisplay(6500);
            }

        }
        else if(GPIO_readPin(ThreadTogglePin)==0){
            ThreadHigh = 0;
        }


        //////////////////////////////////////////////
        if(GPIO_readPin(PlusPin)==1 && PlusHigh == 0){
            PlusHigh = 1;
            if(ThreadBool == 'F'){
                FeedSpeed+=0.01f;
                ScalingFactor = (Microsteps*FeedSpeed*EncoderRatio)/(3*1024);
                WriteDataToDisplay(FeedSpeed*100.0f);
            }
            else{
                ThreadPitch+=0.05f;
                ScalingFactor = (Microsteps*ThreadPitch*EncoderRatio)/(3*1024);
                WriteDataToDisplay(ThreadPitch*100.0f+0.005f);
            }
        }
        else if(GPIO_readPin(PlusPin)==0){
            PlusHigh = 0;
        }


        //////////////////////////////////////////////
        if(GPIO_readPin(MinusPin) == 1 && MinusHigh == 0){
            MinusHigh = 1;
            if(ThreadBool == 'F'){
                FeedSpeed-=0.01f;
                ScalingFactor = (Microsteps*FeedSpeed*EncoderRatio)/(3*1024);
                WriteDataToDisplay(FeedSpeed*100.0f);
            }
            else{
                ThreadPitch-=0.05f;
                ScalingFactor = (Microsteps*ThreadPitch*EncoderRatio)/(3*1024);
                WriteDataToDisplay(ThreadPitch*100.0f+0.005f);
            }
        }
        else if(GPIO_readPin(MinusPin)==0){
            MinusHigh = 0;
        }

        //////////////////////////////////////////////
        if(GPIO_readPin(StepperEnableInput) == 1 && StepperEnableInputHigh == 0){
            StepperEnableInputHigh = 1;
            StepperEnabled = ~StepperEnabled & 0x0001;
            GPIO_writePin(StepperEnableOutput, StepperEnabled);
        }
        else if(GPIO_readPin(StepperEnableInput)==0){
            StepperEnableInputHigh = 0;
        }

        ////////////////////////////////////////////////
        if(GPIO_readPin(DirButtonPin) == 1 && DirectionHigh == 0){
            DirectionHigh = 1;
            RotationDirection = ~RotationDirection & 0x0001;
            WriteDataToDisplay(6502);
        }
        else if(GPIO_readPin(DirButtonPin)==0){
            DirectionHigh = 0;
        }

        PreviousRPM = RPM;
        PreviousEncoderCount = EncoderRPMCount;
        DEVICE_DELAY_US(25000);
        RPM = (EncoderRPMCount-PreviousEncoderCount)*2.34375*EncoderRatio;
        RPMUpdateCount++;
        DEVICE_DELAY_US(5000);
        if(RPM!=PreviousRPM && RPMUpdateCount>5){
            WriteDataToDisplay(RPM+10000);
            RPMUpdateCount = 0;
        }

        if(EncoderRPMCount > 50000){
            EncoderRPMCount = 0;
        }
        if(RPM * ThreadPitch > 3000&&PreviousRPM*ThreadPitch>3000&&ThreadBool=='T'){
            StepperEnabled = 1;
            GPIO_writePin(StepperEnableOutput, StepperEnabled);
        }
    }
}


// function will send a 16 bit integer to arduino seven segment display, add 9999 to display to second 7 segment
void WriteDataToDisplay(uint16_t t)
{

    int j;

    GPIO_writePin(DisplayDataEnable, 1); // begin the data transfer

    for (j = 0; j<16; j++) {  // iterate through every bit in the integer

        DEVICE_DELAY_US(DelayTime);  // delay

        // bitwise & the int against 32768 and bitshift 15 right, bringing the last digit to the start and setting all others to 0
        GPIO_writePin(DisplayDataPin, (t & 0x8000) >> 15);

        t <<= 1;  // shift t 1 left

        GPIO_writePin(DisplayClockPin, 1); // enable clock

        DEVICE_DELAY_US(DelayTime); // delay

        GPIO_writePin(DisplayClockPin, 0);  // set clock to 0 after every pulse

    }

    DEVICE_DELAY_US(5000); // delay

    GPIO_writePin(DisplayDataEnable, 0); // end data transfer

    DEVICE_DELAY_US(5000); // delay

}

// encoder signal pin one interrupt handler
__interrupt void PinOne(void)
{
    EncoderCount+=ScalingFactor; // add scaling factor to count

    // go through encodercount and remove 1 every time and add stepcounter
    for(;EncoderCount>1;EncoderCount-=1)
    {
        StepBuffer+=1;
    }

    // iterate through step buffer and step the stepper for every step in the buffer
    while(StepBuffer>1)
    {
        GPIO_writePin(StepperPin, 1);
        DEVICE_DELAY_US(5);
        GPIO_writePin(StepperPin, 0);
        DEVICE_DELAY_US(5);
        StepBuffer-=1;
    }

    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1); // clear interrupt
}

// encoder signal pin one interrupt handler
__interrupt void PinTwo(void)
{
    EncoderRPMCount += 1;
    // check if encoder signal one is 0, if it is, rotation is clockwise
    if(GPIO_readPin(EncoderSignalOne) == 0)
    {
        GPIO_writePin(StepperDirection, RotationDirection); // clock
    }
    else
    {
        GPIO_writePin(StepperDirection, !RotationDirection); // anticlock
    }
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1); // clear interrupt
}








