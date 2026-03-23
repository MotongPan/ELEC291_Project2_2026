int main(void)
{
    unsigned int left_val, right_val, inter_val;

    ADC_Pin_Init();
    initADC();

    delayms(500); // give PuTTY time to open

    printf("\r\n");
    printf("Field detector ADC test\r\n");
    printf("Left detector  -> PA0 (ADC_IN0)\r\n");
    printf("Right detector -> PA1 (ADC_IN1)\r\n");
    printf("Inter detector -> PA4 (ADC_IN4)\r\n");
    printf("PuTTY baud rate: 115200\r\n");
    printf("\r\n");

    while (1)
    {
        left_val  = adc_read_left();
        right_val = adc_read_right();
        inter_val = adc_read_intersection();

        printf("L=%4u   R=%4u   I=%4u\r\n", left_val, right_val, inter_val);

        delayms(200);
    }
}
