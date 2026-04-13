/*
 115Attiny85FanPWM
 用於 FFB1212EH、PFC1212DE 控制藍腳 PWM 來讓風扇轉速改變
 可變電阻 PB4 10K 輸入
 控制輸出腳 PB1 頻率約為 25KHz，空佔比 0~100 (切10階)
 作者: 羽山秋人(https://3wa.tw)
 版本: V0.02
 ATTiny85 Clock: Internal 16MHz
 手冊 Spec: https://www.delta-fan.com/Download/Spec/PFC1212DE-F00.pdf
*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define PWM_TOP 39

static void adc_init(void)
{
    // Vref = Vcc, ADC2 = PB4
    ADMUX = (1 << MUX1);

    // ADC enable, prescaler = 64
    // 16MHz / 64 = 250kHz
    ADCSRA =
        (1 << ADEN)  |
        (1 << ADPS2) |
        (1 << ADPS1);
}

static uint16_t adc_read_once(void)
{
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;   // 0~1023
}

static void pwm1_init_25khz_pb1(void)
{
    DDRB |= (1 << PB1);   // PB1 = OC1A output

    TCCR1 = 0;
    GTCCR = 0;

    OCR1C = PWM_TOP;
    OCR1A = PWM_TOP;   // 先關

    TCCR1 |= (1 << PWM1A);
    TCCR1 |= (1 << COM1A1);

    // CK/16 -> 約 25kHz
    TCCR1 |= (1 << CS12) | (1 << CS10);
}

static void pwm_set_level_10(uint8_t level)
{
    uint8_t fanDuty;
    uint8_t outDuty;

    if (level > 10) level = 10;

    // 0~10 直接映射到 0~39
    fanDuty = (uint32_t)level * PWM_TOP / 10;

    // 經 2N2222 反相
    outDuty = PWM_TOP - fanDuty;

    OCR1A = outDuty;
}

int main(void)
{
    uint16_t adcValue;
    uint8_t level10;

    adc_init();
    pwm1_init_25khz_pb1();

    while (1)
    {
        adcValue = adc_read_once();              // 0~1023
        level10 = (uint32_t)adcValue * 10 / 1023; // 0~10

        // 如果你的可變電阻轉向和你要的相反，
        // 就把下面這行取消註解：
        // level10 = 10 - level10;

        pwm_set_level_10(level10);

        _delay_ms(10);
    }
}
