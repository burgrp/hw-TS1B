int LED_PIN = 24;
int RF_DATA_PIN = 14;

template <class T>
class Inner
{
public:
	T *outer;
	void bind(T *outer)
	{
		this->outer = outer;
	}
};

void safeBootInit(int bootPin, bool logic, int ledPin = -1)
{
	target::PM.APBBMASK.setPORT(1);

	target::PORT.DIRCLR.setDIRCLR(1 << bootPin);
	if (logic)
	{
		target::PORT.OUTCLR.setOUTCLR(1 << bootPin);
	}
	else
	{
		target::PORT.OUTSET.setOUTSET(1 << bootPin);
	}
	target::PORT.PINCFG[bootPin].setPULLEN(1);

	if (ledPin != -1)
	{
		target::PORT.DIRSET.setDIRSET(1 << ledPin);
		target::PORT.OUTSET.setOUTSET(1 << ledPin);
	}

	for (volatile int w = 0; w < 10000; w++)
	{
		if (((target::PORT.IN.getIN() >> bootPin) & 1) == logic)
		{
			for (;;)
				;
		}
	}

	if (ledPin != -1)
	{
		target::PORT.OUTCLR.setOUTCLR(1 << ledPin);
	}
}

/////////////////////////////////////////////////////////////////////

class App
{
public:
	class : public ookey::tx::Encoder
	{
	public:
		void setTimerInterrupt(bool enabled)
		{
			if (enabled)
			{
				target::NVIC.ISER.setSETENA(11 << target::interrupts::External::TC1);
			}
			else
			{
				target::NVIC.ICER.setCLRENA(11 << target::interrupts::External::TC1);
			}
		}

		void setRfPin(bool state)
		{
			if (state)
			{
				target::PORT.OUTSET.setOUTSET(1 << RF_DATA_PIN);
			}
			else
			{
				target::PORT.OUTCLR.setOUTCLR(1 << RF_DATA_PIN);
			}
		}

	} encoder;

	class : public genericTimer::Timer, public Inner<App>
	{
		void onTimer()
		{
			target::PORT.OUTTGL.setOUTTGL(1 << LED_PIN);
			if (target::PORT.IN.getIN() >> LED_PIN & 1) {
				outer->encoder.send((unsigned char *)"Hello", 5);
			}			
			start(50);
		}
	} timer;

	void init()
	{
		safeBootInit(25, false, LED_PIN);

		timer.bind(this);

		target::PM.APBBMASK.setPORT(1);
		target::PORT.DIRSET.setDIRSET(1 << LED_PIN);

		target::PORT.DIRSET.setDIRSET(1 << RF_DATA_PIN);

		target::PM.APBCMASK.setTC(1, 1);
		
		// GCLK_IO[0] from PA8
		target::PORT.PINCFG[8].setPMUXEN(1);
		target::PORT.PINCFG[8].setINEN(1);
		target::PORT.PMUX[4].setPMUXE(7);

		target::GCLK.GENCTRL = 1 << 16 | 1 << 8 | 0; // generator 0 sourced from GCLK_IO[0]
		target::GCLK.CLKCTRL = 1 << 14 | 0 << 8 | 0x12; // TC1 & TC2 from generator 0

		target::TC1.COUNT16.CC[0].setCC(13000); // ?????? 26000
		target::TC1.COUNT16.CTRLA.setWAVEGEN(1); // top = CC0
		target::TC1.COUNT16.CTRLA.setENABLE(1);
		target::TC1.COUNT16.INTENSET.setOVF(1);

		encoder.init(1);
		timer.start(50);
	}

} app;

void interruptHandlerTC1()
{
	target::TC1.COUNT16.INTFLAG.setOVF(1);
	target::PORT.OUTTGL.setOUTTGL(1 << RF_DATA_PIN);
	app.encoder.handleTimerInterrupt();
}

void initApplication()
{
	genericTimer::clkHz = 26E6;
	app.init();
}
