namespace atsamd
{
namespace safeboot
{

void init(int bootPin, bool logic, int ledPin = -1)
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

} // namespace safeboot
} // namespace atsamd

