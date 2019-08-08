
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

// These functions are provided by Xenomai
int rt_printf(const char *format, ...);
int rt_fprintf(FILE *stream, const char *format, ...);

// InterfaceTable contains pointers to functions in the host (scserver).
static InterfaceTable *ft;

// Holds UGen state variables
struct TrillData : public Unit {
  Trill sensor;
  AuxiliaryTask i2cReadTask;
  int readInterval;
  int readIntervalSamples;
};

static void TrillIn_Ctor(TrillData* unit); // constructor
static void TrillIn_next(TrillData* unit, int inNumSamples); // audio callback


void readSensor(TrillData* unit)
{
	if(unit->sensor.ready()) {
		unit->sensor.readI2C();
		for(unsigned int i=0; i < sizeof(unit->sensor.rawData)/sizeof(int); i++) {
			printf("%5d ", unit->sensor.rawData[i]);
		}
		printf("\n");
	}
}


void TrillIn_Ctor(TrillData* unit) {
  unit->readInterval = 500; // read every 500ms
  unit->readIntervalSamples = 0;

  unit->sensor.setup();
  unit->i2cReadTask = Bela_createAuxiliaryTask(readSensor, 50, "I2C-read", unit);
  unit->readIntervalSamples = SAMPLERATE * (unit->readInterval / 1000);

  SETCALC(TrillIn_next_k); // Use the same calc function no matter what the input rate is.
  // TrillIn outputs a kr signal.
  TrillIn_next(unit, 1); // calc 1 sample of output so that downstream Ugens don't access garbage
}

// the calculation function can have any name, but this is conventional. the first argument must be "unit."
// this function is called every control period (64 samples is typical)
// Don't change the names of the arguments, or the helper macros won't work.
void TrillIn_next(TrillData* unit, int inNumSamples) {
  static int readCount = 0;

  for(unsigned int n=0; n < inNumSamples; n++) {
    if(++readCount >= unit->readIntervalSamples) {
      readCount = 0;
      Bela_scheduleAuxiliaryTask(unit->i2cReadTask);
    }
  }

  // No inputs?
  //float *pollRate = IN(0); // in seconds

  // OUTPUT

  // TODO: 26 kr outputs ?
  float *out1 = OUT(0);

  // TODO: How to get data back from the auxiliary task?
  // unit->sensor.rawData

  for (int i = 0; i < inNumSamples; i++) {
      out[i] = 0.0;
  }
}

PluginLoad(Trill) {
    ft = inTable; // store pointer to InterfaceTable
    DefineSimpleUnit(Trill);
}
