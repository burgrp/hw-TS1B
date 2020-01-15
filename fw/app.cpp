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

class App
{
public:
	class : public ookey::tx::Encoder
	{
	public:
		void setTimerInterrupt(bool enabled)
		{
			target::TC1.COUNT16.COUNT = 0;
			target::TC1.COUNT16.CTRLA.setENABLE(enabled);				
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
			start(100);
		}
	} timer;

	void init()
	{
		atsamd::safeboot::init(25, false, LED_PIN);

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

		target::TC1.COUNT16.CC[0].setCC(26000 / 2); // 2000 symbols per second
		target::TC1.COUNT16.CTRLA.setWAVEGEN(1); // top = CC0
		target::TC1.COUNT16.INTENSET.setOVF(1);

		target::NVIC.ISER.setSETENA(1 << target::interrupts::External::TC1);

		


		encoder.init(1);
		timer.start(100);		
	}

} app;

void interruptHandlerTC1()
{
	app.encoder.handleTimerInterrupt();
	target::TC1.COUNT16.INTFLAG.setOVF(1);
}

// TODO:
// NVIC 1 ne 11
// sdilena Inner a safeBoot

void initApplication()
{
	genericTimer::clkHz = 26E6;
	app.init();
}

