target::gpio_a::Peripheral* LED_PORT = &target::GPIOA;
const int LED_PIN = 4;

int led = 0;

class TempSensor: public genericTimer::Timer {
public:
    void init() {
        start(10);
    }

    void onTimer() {
        led = !led;
        LED_PORT->BSRR = ((1 << 16) | led) << LED_PIN;
    }
};

TempSensor tempSensor;

void initApplication()
{
	target::RCC.IOPENR.setIOPAEN(true);
	LED_PORT->MODER.setMODE(1 << (LED_PIN << 1));

    tempSensor.init();
}
