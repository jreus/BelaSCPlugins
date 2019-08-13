
#include "Bela.h"
#include "Trill.h"

/* TODO: Needs a deconstructor?
void cleanup(BelaContext *context, void *userData)
{
	unit->sensor.cleanup();
}
*/

// See the Server Plugin API for more info
// http://doc.sccode.org/Reference/ServerPluginAPI.html
#include "SC_PlugIn.h"

// number of trill sensors in raw data
#define NUM_SENSORS 26

// These functions are provided by Xenomai
int rt_printf(const char *format, ...);
int rt_fprintf(FILE *stream, const char *format, ...);

// InterfaceTable contains pointers to functions in the host (scserver).
static InterfaceTable *ft;

// Holds UGen state variables
struct TrillIn : public Unit {
  Trill sensor;
  AuxiliaryTask i2cReadTask;
  unsigned int readInterval;
  unsigned int readIntervalSamples;
  unsigned int readCount;
};

static void TrillIn_Ctor(TrillIn* unit); // constructor
static void TrillIn_next_k(TrillIn* unit, int inNumSamples); // audio callback




void readSensor(void* data)
{
  TrillIn *unit = (TrillIn*)data;
	if(unit->sensor.ready()) {
		unit->sensor.readI2C();
	}
}


void TrillIn_Ctor(TrillIn* unit) {
  unit->readInterval = 500; // read every 500ms
  unit->readCount = 0;

  unit->sensor.setup();
  unit->i2cReadTask = Bela_createAuxiliaryTask(readSensor, 50, "I2C-read", (void*)unit);
  unit->readIntervalSamples = SAMPLERATE * (unit->readInterval / 1000);

  unit->sensor.readI2C();

  SETCALC(TrillIn_next_k); // Use the same calc function no matter what the input rate is.
  // TrillIn outputs a kr signal.
  TrillIn_next_k(unit, 1); // calc 1 sample of output so that downstream Ugens don't access garbage
}


// NOTE:: Which ugens to make available to users?
//  >>> For each: a 2D and 1D version
//  > One that provides 30 raw data points
//  > 1D Position that provides 5 position points

// the calculation function can have any name, but this is conventional. the first argument must be "unit."
// this function is called every control period (64 samples is typical)
// Don't change the names of the arguments, or the helper macros won't work.
void TrillIn_next_k(TrillIn* unit, int inNumSamples) {
  // ***DEBUGGING***
  static unsigned int debugCounter = 0;
  static unsigned char debugPrintRate = 4; // 4 times per second
  bool DEBUG = false;
  debugCounter += inNumSamples;
  if(debugCounter >= (SAMPLERATE / debugPrintRate)) {
    debugCounter = 0;
    DEBUG = true;
  }
  // ***DEBUGGING***


  // Any inputs needed? Maybe I2C address or something similar...
  // TODO: make sure multiple UGens access the same I2C data.
  //      This follows the idiom of Input UGens accessing global signal busses.
  //      ?? < does this mean I2C data should be global?
  //float *pollRate = IN(0); // in seconds
  //static int readCount = 0; // NOTE: probably not a good idea to use static variables here, might be shared between plugin instances!
  // 26 kr outputs, one for each trill sensor raw value
  float outs[NUM_SENSORS];
  for(unsigned char i = 0; i < NUM_SENSORS; i++) {
    outs[i] = OUT0(i);
  }

  // check if another read is necessary
  for(unsigned char n=0; n < inNumSamples; n++) {
    // This kind of sample-precision is not possible
    //   in the callback with Aux tasks, BUT this is realibly
    //   counting samples so the AUX task is called at a regular rate.
    // Running Aux tasks is more of a "request" than a demand..
    //   if an Aux task is called a second time the first call will be
    //   ignored... 
    if(++unit->readCount >= unit->readIntervalSamples) {
      unit->readCount = 0;
      Bela_scheduleAuxiliaryTask(unit->i2cReadTask);
    }
  }

  if(DEBUG) {
    rt_printf("[ %f", unit->sensor.rawData[0]);
    for(unsigned char i=1; i < NUM_SENSORS; i++) {
      rt_printf(", %f", unit->sensor.rawData[i]);
    }
    rt_printf(" ]\n");
  }

  for (unsigned char i = 0; i < NUM_SENSORS; i++) {
      outs[i] = unit->sensor.rawData[i];
  }
}

PluginLoad(Trill) {
    ft = inTable; // store pointer to InterfaceTable
    DefineSimpleUnit(TrillIn);
}
