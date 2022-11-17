/*-------------------------------Arduino code for pulse oximetry-------------------------------

  [Objective]
  Develope a functional oscilloscope and signal generator on Arduino DUE with the following functions:
  1. Real-time signal sampling 
  2. Waveform display in serial ploter
  3. Allow amplitude, offset, frequency and pattern setting 
  4. Output pattern: DC, sinusoidal, sawtooth & square waves
  5. Output voltage range (Amplitude+offset) = 0-3.3V
  6. Signal frequency = 0-100 Hz

  ** Fill in all [......] to make it functional**


  [For Task 2 only]
  1. Add a parameter called "PhaseDelay" in addition to  DAC1 output channel.
  2. Set "PhaseDelay" to be half of the signal period.
  3. Set "amplitude" of DAC1 channel to be 0.5V.

---------------------------------------------------------------------------------------------*/

// Including libraries

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//==========================================================================================//
//                                1. Variable declaration                                   //
//==========================================================================================//

// Master clock info //
const int clock_rate = 115200;               // CLock frequency (Hz)
const int loop_period = 2500;               // Loop period (us)

// Signal generation //
bool output_signal = true;               // ON/OFF of signal generation
const float output_frequency = 20;                // Signal frequency (Hz)
const float output_period = 1/output_frequency;
const float pixel = 10;                // Pixel in a period (pix)
const int output_channels = 2;                // Number of output channel (analog ouput, starting from DAC0)
const float output_amplitude = 5;              // Signal amplitude (V)
const float output_offset = 0;                // Signal offset (V)
const float max_voltage = 3.3;                // Max. voltage output (V)
const int output_resolution = 10;               // Input resolution
const int input_resolution = 10;                // Input resolution
const bool antiphase = true;

int output_digital;                 // Output signal (digital)
float output;                 // Output signal (analog)

// Signal detection //
bool input_signal = false;                // ON/OFF of signal detection
bool input_channels = 1;                // Number of input channel (analog input, starting from A0)
float input_digital;          // Input reading (digital)
bool checker = false;
int value_change = 0;
float signal_difference = 50;

const int offset = 2;
#define Red 0
#define IR 1
const int n = 63;
const float numerators[n] = {-0.00022544666817018, -0.000938779806837583,  -0.00151530778104534, 0.000132888707232203, 0.00558654760000211,  0.0122417197575537, 0.0134280970300094, 0.00557846227859054,  -0.00485065058896525, -0.00642744880876713, 0.0025242609228204, 0.0091491956981348, 0.0023664495906769, -0.00947189965243818, -0.0079034886350454,  0.00744478661228586,  0.0138308760205761, -0.00189858017155633, -0.018417069989154, -0.00739853631529773, 0.0195829084725101, 0.0199477883168333, -0.0149384519295424,  -0.0344698756610589,  0.00163423751854071,  0.0490498522568523, 0.0250802864506343, -0.0615293290117183,  -0.080770651704926, 0.0699431739187571, 0.309479055311869,  0.427086828012949,  0.309479055311869,  0.0699431739187571, -0.080770651704926, -0.0615293290117183,  0.0250802864506343, 0.0490498522568523, 0.00163423751854071,  -0.0344698756610589,  -0.0149384519295424,  0.0199477883168333, 0.0195829084725101, -0.00739853631529773, -0.018417069989154, -0.00189858017155633, 0.0138308760205761, 0.00744478661228586,  -0.0079034886350454,  -0.00947189965243818, 0.0023664495906769, 0.0091491956981348, 0.0025242609228204, -0.00642744880876713, -0.00485065058896525, 0.00557846227859054,  0.0134280970300094, 0.0122417197575537, 0.00558654760000211,  0.000132888707232203, -0.00151530778104534, -0.000938779806837583,  -0.00022544666817018};
float raw_signal[2][n];
int current_position = 0;
const int filter_amount = n/(1/output_frequency/loop_period*1000000)*2;
float filter_temp[2] = {0,0};
float filter_prev[2] = {0,0};
int filter_counter[2] = {0,0};
float DAC_output = 2;
#define MAX 0
#define MIN 1
float signal_wrap[2][2] = {{-3300,3300},{-3300,3300}};
float signal_wrap_a[2][2] = {{1,1},{1,1}};
float alpha = 0.9;

// Variables for controlling menu

#define menu_ 0
#define monitor_ 1
#define save_ 2
#define setting_ 3
#define easteregg_ 4

int mode = monitor_;

// Information for OLED displays
#define LOGO_HEIGHT 64
#define LOGO_WIDTH 128

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//==========================================================================================//
//                                          2. Functions                                    //
//==========================================================================================//

float exp_average(float prev, float now, float a){
  return (float)((1-a)*prev + a*now);
}

float lowpass_filter(int choice, int start, int end, int temp){
  if(start<n){
    if(end>n){end=n;}
    for(int i=start; i<end; i++){
      temp += numerators[i]*raw_signal[choice][(current_position+i)%n];
    }
  }
  return temp;
}

float * probe(float t){

  //================================== Signal generation ===================================//
  
  if (output_signal) {   // ON/OFF of signal generation 

    for (int i = 0; i < 2; i++) {    // Two signal generation channels by looping 
      // Shifting two signal in anti-phase
      if(i && antiphase){  
        t = t+1/output_frequency/2;
      }
      // Calculating y(t) of waveform
      if(remainder(t, 1/output_frequency)>0){
        output = HIGH;
      }
      else{
        output = LOW;  
      }
      // Output waveform //
      if (i) {
        digitalWrite(12, output);    // output at D12 (Red)
      }
      if (!i) {
        digitalWrite(11, output);    // output at D11 (IR)
      }


      if(DAC_output > max_voltage*5/6){DAC_output = max_voltage*5/6;} 
      if(DAC_output < max_voltage/6){DAC_output = max_voltage/6;}  
      analogWrite(DAC0, ((DAC_output-max_voltage/6)/(max_voltage*4/6))*1024);

      // Print output value to serial port to check waveform in serial plotter //
      //Serial.println(output);
    }
  }

  //================================== Signal detection ===================================//

 
  // Sampling at analog channel defined above //
  float period = remainder(t, 1/output_frequency)+(0.5/output_frequency);
  value_change = 0;
  if((period > output_period*0.3 && period < output_period*0.4 && !checker) || (period > output_period*0.8 && period < output_period*0.9 && checker)){
    float inputstore = (float)analogRead(A2);
    input_digital = 3300 - inputstore/1024*3.3*1000; // Read Signal from A2
    // Print the reading to serial port for visualization //  
    value_change = 0;
    if(!checker){
      //D12: IR
      raw_signal[IR][current_position] = input_digital;
      Serial.print("IR:");
      filter_temp[IR] -= exp_average(filter_prev[IR], filter_temp[IR], 0.8);
      filter_prev[IR] = filter_temp[IR];

      //Signal envelope decay
      signal_wrap_a[IR][MAX]*=(1/0.9);
      signal_wrap_a[IR][MIN]*=(1/0.9);
      if(filter_temp[IR]>signal_wrap[IR][MAX]-signal_wrap_a[IR][MAX]){
        signal_wrap[IR][MAX]=filter_temp[IR];
        signal_wrap_a[IR][MAX]=1;
      }
      if(filter_temp[IR]<signal_wrap[IR][MIN]+signal_wrap_a[IR][MIN]){
        signal_wrap[IR][MIN]=filter_temp[IR];
        signal_wrap_a[IR][MIN]=1;
      }
      
      Serial.print(filter_temp[IR]+signal_difference);
      //update_monitor("IR");
      value_change = 1;
    
      filter_temp[IR] = 0;
      filter_counter[IR] = 0;
    }
    else{
      //D11: RED
      raw_signal[Red][current_position] = input_digital;
      Serial.print(",RED:");
      filter_temp[Red] -= exp_average(filter_prev[Red], filter_temp[Red], 0.8);
      filter_prev[Red] = filter_temp[Red];

      //Signal envelope decay
      signal_wrap_a[Red][MAX]*=(1/0.9);
      signal_wrap_a[Red][MIN]*=(1/0.9);
      if(filter_temp[Red]>signal_wrap[Red][MAX]-signal_wrap_a[Red][MAX]){
        signal_wrap[Red][MAX]=filter_temp[Red];
        signal_wrap_a[Red][MAX]=1;
      }
      if(filter_temp[Red]<signal_wrap[Red][MIN]+signal_wrap_a[Red][MIN]){
        signal_wrap[Red][MIN]=filter_temp[Red];
        signal_wrap_a[Red][MIN]=1;
      }
      
      Serial.println(filter_temp[Red]-signal_difference);
      //update_monitor("RED");
      value_change = 2;

      filter_temp[Red] = 0;
      filter_counter[Red] = 0;
      
      current_position++;
      current_position %= n;
    }
    
    /*
    Serial.print(period);
    Serial.print(" ");
    Serial.print(output_period);
    Serial.print(" ");
    Serial.println(checker);
    */
    //Serial.println("");
    checker = !checker;
  }

  filter_temp[IR] = lowpass_filter(IR, filter_counter[IR]*filter_amount, (filter_counter[IR]+1)*filter_amount, filter_temp[IR]);
  filter_temp[Red] = lowpass_filter(Red, filter_counter[Red]*filter_amount, (filter_counter[Red]+1)*filter_amount, filter_temp[Red]);
  filter_counter[IR]++;
  filter_counter[Red]++;
}

int draw_text(int x, int y, int size, int color, String text){
  display.setTextSize(size);      // Normal 1:1 pixel scale
  display.setTextColor(color); // Draw white text
  display.cp437(true);
  display.setCursor(x, y);     // Start at top-left corner;
  for(int i = 0; i<text.length(); i++){
    display.write(text[i]);  
  }
}

int display_monitor(){
  if((value_change-1)==1){
    //IR
    display.fillRect(10,0,54,8, BLACK);
    String text = String(filter_temp[IR]);
    draw_text(10,2,1,WHITE,text);
    int temp = (int)((filter_temp[IR]+signal_wrap[IR][MIN])/signal_wrap[IR][MAX]*24);
    display.drawPixel(126, 31-temp, WHITE);
  }
  else{
    //RED
    display.fillRect(74,0,54,8, BLACK);
    String text = String(filter_temp[Red]);
    draw_text(74,2,1,WHITE,text);
    int temp = (int)((filter_temp[IR]+signal_wrap[IR][MIN])/signal_wrap[IR][MAX]*24);
    display.drawPixel(126, 55-temp, WHITE);
    display.startscrollleft(1,6);
    display.stopscroll();
  }
  display.drawLine(0,0,0,127,BLACK);
  display.display();
  return 0;
}

//==========================================================================================//
//                            3. Main program (initialize environment)                      //
//==========================================================================================//

void setup() {
  digitalWrite(2, HIGH);
  digitalWrite(3, HIGH);
  delay(2000);
  for(int x = 0; x<10; x++){
    digitalWrite(2, HIGH);
    digitalWrite(3, LOW);
    delay(100);
    digitalWrite(2, LOW);
    digitalWrite(3, HIGH);
    delay(100);
  }
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  
  // Turn on serial port (baud, bit per second) //
  Serial.begin(clock_rate);
  // Set analog read & write resolution (bit) //
  analogWriteResolution(output_resolution);
  analogReadResolution(input_resolution);
  for(int i = 0; i<n; i++){
    raw_signal[0][i]=0;
    raw_signal[1][i]=0;
  }

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
}

//==========================================================================================//
//                                 4. Main program (looping)                                //
//==========================================================================================//
void loop() {

  // Set vaiable to record start time for loop time monitoring (i.e sampling period) //
  float start_time = micros();
    
  // Using counter to compute the phase of the waveform //
  float ct = start_time / 1000000;
  float t = ct;
  //Serial.println(t);

  switch(mode){
  case menu_:
    // Main Menu

  break;
  case monitor_:
    // Monitoring PPG signal
    probe(t);
    if(value_change){
      display_monitor();
    }
  break;
  case save_:
    // Saving data
  
  break;
  case setting_:
    // Setting
  
  break;
  case easteregg_:
    // Easter Egg
  
  break;
  }

  // Set vaiable to record end time for loop time monitoring (i.e sampling period) //
  float end_time = micros();
  int delay_t = loop_period - (end_time - start_time);

  /*
  Serial.print(" {");
  Serial.print(delay_t);
  Serial.println("} ");
  */
  
  if(delay_t < 0){delay_t = 0;}
  delayMicroseconds(delay_t);
  // Set delay equal to the difference between start and end times to control loop time //
}
