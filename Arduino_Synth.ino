
/**********************************************************************************************************************************************
 * Change log:
 * 12/4/2020 - added a timer regulation on the loop to ensure that the program runs at a constant frame rate of 1 loop per millisecond
 * 12/7/2020 - made a faster timer regulation. 1 millisecond is really slow and I wouldn't be able to get anything close to an audible 8-bit 
 *             waveform. I will now try to clock speeds in microseconds using the micro() function.
 *             - After running tests measuring programs speed in microseconds, it was determined that at its slowest speeds, the program is
 *             only capable of an 8-bit waveform at 3.48 hZ. I will need to look at optimizing my code. The arduino when running nothing but
 *             an if statement and an addition equation could theoretically run at 306 Hz, which is above middle C.
 *             -Taking out the analogInput buffer: 11.28 Hz
 *             -Taking out the analogInput buffer and the division when calling play function: 11.018 Hz
 *             -Taking out the analogInput buffer and all notes except C (elimating a lot of addition):17.205 Hz
 *             -Taking out the analogInput buffer and elimating all addition and division on lines 188 and 189(ei play(sine[a]);:18.566 Hz
 *             -Completely removing the ReadIn DAC, which gives the arduino a serial update of the keys's conditions, which the arduino then 
 *              then has to decode (ReadIn() function, and lines 152 - 178): 44 Hz
 *             -Keeping all changes in the entry directly above, if I remove the play function altogether, I get 113.88 Hz, which is a good value.
 *              If I can change the switches out for something a little faster, we could have something useful.
 * 12/7/2020 - Modify play() function, removing the case switches: 47.58 Hz
 *             -Replaced play() function with faster alternative and replaced ReadIn DAC with 7 digitalRead pins: 27.1 Hz
 * 12/8/2020 - Thanks to my handy oscilloscope, I found that my function for converting 'output' to binary wasn't working correctly, so it was            
 *             putting out 4-bit as opposed to 8-bit. This latest revision has fixed the problem.
 * 12/16/2020- I realized that my program still runs at variable speeds. When I play more than 1 note, the program slows down. By 3 or 4 notes,
 *             the program is significantly slower, making it less likely to hear comprehensible chords. I added a clock back into the program.
 *             This causes the program to run at 300 micro-seconds per loop cycle. This way, the program will run at the same speed whether it
 *             be 1 note played, or 3. The clock will have no effect on more than 3 notes played simulataneously, being that I can only slow the
 *             program down, not speed it up.
 ************************************************************************************************************************************************/


int squ[257] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
int sine[257] = {0,128,131,134,137,140,143,146,149,152,156,159,162,165,168,171,174,176,179,182,185,188,191,193,196,199,201,204,206,209,211,213,216,218,220,222,224,226,228,230,232,
                234,236,237,239,240,242,243,245,246,247,248,249,250,251,252,252,253,254,254,255,255,255,255,255,255,255,255,255,255,255,254,254,253,252,252,251,250,249,248,247,246,
                245,243,242,240,239,237,236,234,232,230,228,226,224,222,220,218,216,213,211,209,206,204,201,199,196,193,191,188,185,182,179,176,174,171,168,165,162,159,156,152,149,
                146,143,140,137,134,131,128,124,121,118,115,112,109,106,103,99,96,93,90,87,84,81,79,76,73,70,67,64,62,59,56,54,51,49,46,44,42,39,37,35,33,31,29,27,25,23,21,19,18,16,
                15,13,12,10,9,8,7,6,5,4,3,3,2,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,2,3,3,4,5,6,7,8,9,10,12,13,15,16,18,19,21,23,25,27,29,31,33,35,37,39,42,44,46,49,51,54,56,59,62,64,67,70,
                73,76,78,81,84,87,90,93,96,99,103,106,109,112,115,118,121,124};
int saw[257] = {0,128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,192,194,196,198,200,202,204,206,208,
              210,212,214,216,218,220,222,224,226,228,230,232,234,236,238,240,242,244,246,248,250,252,254,255,254,252,250,248,246,244,242,240,238,236,234,232,230,228,226,224,222,220,
              218,216,214,212,210,208,206,204,202,200,198,196,194,192,190,188,186,184,182,180,178,176,174,172,170,168,166,164,162,160,158,156,154,152,150,148,146,144,142,140,138,136,
              134,132,130,128,126,124,122,120,118,116,114,112,110,108,106,104,102,100,98,96,94,92,90,88,86,84,82,80,78,76,74,72,70,68,66,64,62,60,58,56,54,52,50,48,46,44,42,40,38,36,
              34,32,30,28,26,24,22,20,18,16,14,12,10,8,6,4,2,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,
              84,86,88,90,92,94,96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126};

unsigned long lastTime = 0;

int WR = 3;
int CS = 4;
int AB = 5;

int b0 = 6;
int b1 = 7;
int b2 = 8;
int b3 = 9;
int b4 = 10;
int b5 = 11;
int b6 = 12;
int b7 = 13;

bool reading[8] = {0,0,0,0,0,0,0,0};
bool keys[7] = {0,0,0,0,0,0,0};
float index[7] = {1,1,1,1,1,1,1};
int steps[8] = {3,7,15,31,63,127,255};
int output = 0;

void setup() {
  // put your setup code here, to run once:

  pinMode(b0, OUTPUT);
  pinMode(b1, OUTPUT);
  pinMode(b2, OUTPUT);
  pinMode(b3, OUTPUT);
  pinMode(b4, OUTPUT);
  pinMode(b5, OUTPUT);
  pinMode(b6, OUTPUT);
  pinMode(b7, OUTPUT);
  pinMode(WR, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(AB, OUTPUT);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(2, INPUT);
  
  digitalWrite(AB, LOW);
  digitalWrite(CS, HIGH);
  digitalWrite(WR, HIGH);
}

//converts whole integers to binary values
int Analyze(int position)
{
  if (output >= steps[position])
  {
    output = output - steps[position] - 1;
    
    return 1;
  }
  else
  {
    return 0;
  }
}

//Configures the DAC for a new reading and outputs it
void SendOut()
{
    //Turn On CS
  digitalWrite(CS, LOW);
  
  //Turn on WR
  digitalWrite(WR, LOW);
  
  //Turn Off WR
  digitalWrite(WR, HIGH);
    
  //Turn Off CS
  digitalWrite(11, HIGH);
  
}

void loop() {
  
  //Reads the current state of piano keys
  keys[0] = digitalRead(2);
  keys[1] = digitalRead(A5);
  keys[2] = digitalRead(A4);
  keys[3] = digitalRead(A3);
  keys[4] = digitalRead(A2);
  keys[5] = digitalRead(A1);
  keys[6] = digitalRead(A0);
  
  //Reset waveform iterations once they reach 256
  if (index[0] >= 256) {index[0] = 1;}
  else if (index[1] > 256) {index[1] = 1;}
  else if (index[2] > 256) {index[2] = 1;}
  else if (index[3] > 256) {index[3] = 1;} 
  else if (index[4] > 256) {index[4] = 1;}
  else if (index[5] > 256) {index[5] = 1;}
  else if (index[6] > 256) {index[6] = 1;}
  else {}

  // Play the notes being pressed
  output = (sine[int(index[0])] + sine[int(index[1])] + sine[int(index[2])] + sine[int(index[3])] + sine[int(index[4])] + sine[int(index[5])] + sine[int(index[6])])/
           (keys[0] + keys[1] + keys[2] + keys[3] + keys[4] + keys[5] + keys[6]);
  play();
  SendOut();
  
  //If key state is high, iterate waveform index, if low, set waveform index to 0 
  index[0] = (index[0] + 5.5)*keys[0];
  index[1] = (index[1] + 6.2)*keys[1];
  index[2] = (index[2] + 6.9)*keys[2];
  index[3] = (index[3] + 7.3)*keys[3];
  index[4] = (index[4] + 8.3)*keys[4];
  index[5] = (index[5] + 9.4)*keys[5];
  index[6] = (index[6] + 10.5)*keys[6];

  //Clock that controls the time elapse of the main loop (300 micro-seconds)
  while ((micros() - lastTime) < 300)
  {}
  lastTime = micros();

}

/*****************************************************************************************************
 * This function converts the output waveform to binary, and performs a digital write to the 8-bit DAC
 ****************************************************************************************************/
void play()
{
  //Converts output into an binary array
  for (int position = 7; position >= 0; position--)
  {
    reading[position] = Analyze(position);
  }

  //Write binary array out to DAC
  digitalWrite(b0, reading[0]);
  digitalWrite(b1, reading[1]);
  digitalWrite(b2, reading[2]);
  digitalWrite(b3, reading[3]);
  digitalWrite(b4, reading[4]);
  digitalWrite(b5, reading[5]);
  digitalWrite(b6, reading[6]);
  digitalWrite(b7, reading[7]);
}
