#include "MKL25Z4.h"
#include "encoder.h"

#define ENCODER_PORT_CLOCK  SIM_SCGC5_PORTD_MASK
#define ENCODER_PORT        PORTD
#define ENCODER_GPIO        PTD
#define ENCODER_PIN_A       0u
#define ENCODER_PIN_B       2u
#define ENCODER_PIN_MASK    ((1u << ENCODER_PIN_A) | (1u << ENCODER_PIN_B))

static volatile int16_t encoder_count;
static volatile uint8_t encoder_state;

static uint8_t encoder_read_state(void)
{
    uint32_t pdir = ENCODER_GPIO->PDIR;
    uint8_t state = 0u;

    if ((pdir & (1u << ENCODER_PIN_A)) != 0u) {
        state |= 0x01u;
    }
    if ((pdir & (1u << ENCODER_PIN_B)) != 0u) {
        state |= 0x02u;
    }

    return state;
}

static void encoder_update(void)
{
    static const int8_t step_table[16] = {
         0, -1,  1,  0,
         1,  0,  0, -1,
        -1,  0,  0,  1,
         0,  1, -1,  0
    };
    uint8_t new_state = encoder_read_state();
    uint8_t index = (uint8_t)((encoder_state << 2) | new_state);

    encoder_count += step_table[index];
    encoder_state = new_state;
}

void Encoder_Init(void)
{
    SIM->SCGC5 |= ENCODER_PORT_CLOCK;

    ENCODER_PORT->PCR[ENCODER_PIN_A] = PORT_PCR_MUX(1)
                                     | PORT_PCR_PE_MASK
                                     | PORT_PCR_PS_MASK
                                     | PORT_PCR_IRQC(0x0Bu);
    ENCODER_PORT->PCR[ENCODER_PIN_B] = PORT_PCR_MUX(1)
                                     | PORT_PCR_PE_MASK
                                     | PORT_PCR_PS_MASK
                                     | PORT_PCR_IRQC(0x0Bu);

    ENCODER_GPIO->PDDR &= ~ENCODER_PIN_MASK;
    ENCODER_PORT->ISFR = ENCODER_PIN_MASK;
    encoder_state = encoder_read_state();
    encoder_count = 0;

    NVIC_ClearPendingIRQ(PORTD_IRQn);
    NVIC_EnableIRQ(PORTD_IRQn);
}

int16_t Encoder_Get(void)
{
    int16_t count;

    NVIC_DisableIRQ(PORTD_IRQn);
    count = encoder_count;
    encoder_count = 0;
    NVIC_EnableIRQ(PORTD_IRQn);

    return count;
}

void PORTD_IRQHandler(void)
{
    uint32_t flags = ENCODER_PORT->ISFR & ENCODER_PIN_MASK;

    if (flags != 0u) {
        encoder_update();
        ENCODER_PORT->ISFR = flags;
    }
}
