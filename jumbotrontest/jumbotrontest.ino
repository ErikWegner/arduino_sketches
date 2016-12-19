  /*
* Further modified by Ben Leduc-Mills, standing on the shoulders of those mentioned below.
*
* Modified by Markus Lipp adding interleaved buffers, streaming, 32x32 & 24bit support
*
* Based on "_16x32_Matrix R3.0" by Creater Alex Medeiros, http://PenguinTech.tk
* Use code freely and distort its contents as much as you want, just remeber to thank the
* original creaters of the code by leaving their information in the header. :)
*/

//Define pins
const uint8_t

//PortC
APIN      = 23, BPIN      = 22, CPIN      = 21, DPIN = 20,
CLOCKPIN  = 11, LATCHPIN  = 10, OEPIN     = 9,

//PortD
R1PIN     = 2, R2PIN     = 5,
G1PIN     = 3, G2PIN     = 6,
B1PIN     = 4, B2PIN     = 7;

uint8_t pinTable[13] =
{
    R1PIN, R2PIN, G1PIN, G2PIN, B1PIN, B2PIN,
    APIN, BPIN, CPIN, DPIN, CLOCKPIN, LATCHPIN, OEPIN
};

//Addresses 1/8 rows Through a decoder
uint16_t const A = 1, B = 2, C = 4, D = 8;

//Acts like a 16 bit shift register
uint16_t const SCLK   = 16;
uint16_t const LATCH  = 32;
uint16_t const OE     = 64;

//Decoder counter var
uint16_t const abcVar[16] =
{
    0, A, B, A + B, C, C + A, C + B, A + B + C,
    0 + D, A + D, B + D, A + B + D, C + D, C + A + D, C + B + D, A + B + C + D
};

//Data Lines for row 1 red and row 9 red, ect.
uint16_t const RED1   = 1, RED2   = 8;
uint16_t const GREEN1 = 2, GREEN2 = 16;
uint16_t const BLUE1  = 4, BLUE2  = 32;

const uint8_t SIZEX = 64;
const uint8_t SIZEY = 32;
const uint16_t BUFSIZE = SIZEX * SIZEY * 4;

//Here is where the data is all read
uint8_t interleavedBuffer[BUFSIZE];

//BAM and interrupt variables
boolean actDisplay = false;
uint8_t rowN = 0;
uint16_t BAM;
uint8_t BAMMAX = 7; //now 24bit color! (0-7)


void setup()
{
    for(uint8_t i = 0; i < 13; i++)
    {
        pinMode(pinTable[i], OUTPUT);
    }
    timerInit();
    Serial.begin(115200);
    for (uint16_t clrPos = 0; clrPos < BUFSIZE; clrPos++) {
      interleavedBuffer[clrPos] = 0;
    }
    interleavedBuffer[0] = 128;
    interleavedBuffer[120] = 128;
    interleavedBuffer[240] = 128;
    interleavedBuffer[320] = 128;
    interleavedBuffer[640] = 128;
    interleavedBuffer[800] = 128;
}

uint8_t r, g, prevVal, val;
int dataPos = 0;

void loop()
{
    if (Serial.available())
    {
        prevVal = val;
        val = Serial.read();

        if ( (prevVal == 192 && val == 192) || dataPos >= BUFSIZE)
        {
            dataPos = 0;
        }
        else
        {
            interleavedBuffer[dataPos++] = val;
        }
    }
}

IntervalTimer timer1;

#define BAMDUR 2
void timerInit()
{
    BAM = 0;
    timer1.begin(attackMatrix, BAMDUR);
}



//The updating matrix stuff happens here
//Each pair of rows is taken through its BAM cycle,
//then the rowNumber is increased and id done again
void attackMatrix()
{
    uint16_t portData;

    //sets up which BAM the matrix is on
    if(BAM == 0)
    {
        timer1.begin(attackMatrix, BAMDUR);    //code takes max 41 microsec to complete
    }
    if(BAM == 1)
    {
        timer1.begin(attackMatrix, BAMDUR * 2);    //so 42 is a safe number
    }
    if(BAM == 2)
    {
        timer1.begin(attackMatrix, BAMDUR * 4);
    }
    if(BAM == 3)
    {
        timer1.begin(attackMatrix, BAMDUR * 8);
    }
    if(BAM == 4)
    {
        timer1.begin(attackMatrix, BAMDUR * 16);
    }
    if(BAM == 5)
    {
        timer1.begin(attackMatrix, BAMDUR * 32);
    }
    if(BAM == 6)
    {
        timer1.begin(attackMatrix, BAMDUR * 64);
    }
    if(BAM == 7)
    {
        timer1.begin(attackMatrix, BAMDUR * 128);
    }

    portData = 0; // Clear data to enter
    portData |= (abcVar[rowN]) | OE; // abc, OE
    portData &= ~ LATCH;       //LATCH LOW
    GPIOC_PDOR = portData;  // Write to Port

    uint8_t *start = &interleavedBuffer[rowN * SIZEX * 8 + ((7 - BAMMAX) + BAM) * 32];

    for(uint8_t _x = 0; _x < 32; _x++)
    {
        GPIOD_PDOR = start[_x]; // Transfer data
        GPIOC_PDOR |=  SCLK;// Clock HIGH
        GPIOC_PDOR &= ~ SCLK; // Clock LOW
    }

    GPIOC_PDOR |= LATCH;// Latch HIGH
    GPIOC_PDOR &= ~ OE; // OE LOW, Displays line


    if(BAM >= BAMMAX)   //Checks the BAM cycle for next time.
    {

        if(rowN == 15)
        {
            rowN = 0;
        }
        else
        {
            rowN ++;
        }
        BAM = 0;
        actDisplay = false;
    }
    else
    {
        BAM ++;
        actDisplay = true;
    }
}
