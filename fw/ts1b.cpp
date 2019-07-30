target::gpio_a::Peripheral *LED_PORT = &target::GPIOA;
const int LED_PIN = 2;

int led = 0;

class TempSensor : public genericTimer::Timer
{
public:
    void init()
    {
        start(10);
    }

    void onTimer()
    {
        led = !led;
        LED_PORT->ODR.setOD(LED_PIN, led);
        start(1);
    }
};

TempSensor tempSensor;

void initApplication()
{
    target::DBG.CR.setDBG_SLEEP(true);
    target::DBG.CR.setDBG_STANDBY(true);
    target::DBG.CR.setDBG_STOP(true);

    target::RCC.IOPENR.setIOPAEN(true);

    LED_PORT->MODER.setMODE(LED_PIN, 1);
    LED_PORT->ODR.setOD(LED_PIN, 1);

    tempSensor.init();
    
}
