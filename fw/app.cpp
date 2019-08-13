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

	for (volatile int w = 0; w < 20000; w++)
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
			outer->encoder.send((unsigned char *)"Hello", 5);
			start(10);
		}
	} timer;

	void init()
	{
		safeBootInit(25, false, LED_PIN);

		for (;;)
			;

		timer.bind(this);

		target::PM.APBBMASK.setPORT(1);
		target::PORT.DIRSET.setDIRSET(1 << LED_PIN);

		target::PORT.DIRSET.setDIRSET(1 << RF_DATA_PIN);

		target::PM.APBCMASK.setTC(1, 1);
		// target::GCLK.GENCTRL = 1 << 16 | 1 << 19 | 1;
		// target::GCLK.CLKCTRL = 2 << 14 | 0x12; // TC1 & TC2 clock from generator 1

		//target::GCLK.CLKCTRL = 1 << 14 | 0x12; // TC1 & TC2 clock from generator 0

		target::TC1.COUNT16.CC[0].setCC(1000);
		target::TC1.COUNT16.CTRLA.setWAVEGEN(1); // top = CC0
		target::TC1.COUNT16.CTRLA.setENABLE(1);
		target::TC1.COUNT16.INTENSET.setOVF(1);

		encoder.init(1);
		timer.start(10);
	}

} app;

void interruptHandlerTC1()
{
	target::TC1.COUNT16.INTFLAG.setOVF(1);
	app.encoder.handleTimerInterrupt();
}

void initApplication()
{
	app.init();
}
