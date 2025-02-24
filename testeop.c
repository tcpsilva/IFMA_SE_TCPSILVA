#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

// Pinos dos servos
#define SERVO_LEFT_PIN 2
#define SERVO_RIGHT_PIN 3

// Pinos dos botões
#define BTN_UP_LEFT 4
#define BTN_DOWN_LEFT 5
#define BTN_UP_RIGHT 6
#define BTN_DOWN_RIGHT 7

// Pinos do joystick (ADC)
#define JOY_VERT_PIN 26
#define JOY_HORZ_PIN 27

// Função para configurar PWM para o servo
void setup_servo(uint servo_pin) {
    gpio_set_function(servo_pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(servo_pin);
    pwm_set_wrap(slice, 20000);      // Ciclo de 20 ms
    pwm_set_clkdiv(slice, 64.f);     // Divisor de clock
    pwm_set_enabled(slice, true);
}

// Função para mover o servo para o ângulo desejado
void move_servo(uint servo_pin, int angle) {
    uint slice = pwm_gpio_to_slice_num(servo_pin);
    uint16_t pulse_width = 500 + (angle * 2000) / 180;  // Conversão do ângulo para largura de pulso (500-2500 µs)
    pwm_set_gpio_level(servo_pin, pulse_width);
}

// Função para configurar botões
void setup_button(uint pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);  // Pull-up interno para leitura estável
}

// Função para configurar o ADC (joystick)
void setup_adc() {
    adc_init();
    adc_gpio_init(JOY_VERT_PIN);
    adc_gpio_init(JOY_HORZ_PIN);
}

// Função para ler a direção do joystick
const char* read_joystick() {
    adc_select_input(0);  // Canal para o pino vertical (JOY_VERT_PIN)
    int vert = adc_read();

    adc_select_input(1);  // Canal para o pino horizontal (JOY_HORZ_PIN)
    int horz = adc_read();

    if (vert > 3000) return "CIMA";
    if (vert < 1000) return "BAIXO";
    if (horz > 3000) return "DIREITA";
    if (horz < 1000) return "ESQUERDA";

    return "CENTRO";
}

int main() {
    stdio_init_all();

    // Configuração dos ADCs e periféricos
    setup_adc();
    
    // Configuração dos servos (posicionamento das lentes)
    setup_servo(SERVO_LEFT_PIN);
    setup_servo(SERVO_RIGHT_PIN);

    // Configuração dos botões para controle dos servos
    setup_button(BTN_UP_LEFT);
    setup_button(BTN_DOWN_LEFT);
    setup_button(BTN_UP_RIGHT);
    setup_button(BTN_DOWN_RIGHT);

    int left_angle = 90;   // Posição inicial das lentes esquerdas
    int right_angle = 90;  // Posição inicial das lentes direitas

    while (true) {
        // Controle do servo esquerdo
        if (!gpio_get(BTN_UP_LEFT) && left_angle < 180) left_angle += 5;
        if (!gpio_get(BTN_DOWN_LEFT) && left_angle > 0) left_angle -= 5;

        // Controle do servo direito
        if (!gpio_get(BTN_UP_RIGHT) && right_angle < 180) right_angle += 5;
        if (!gpio_get(BTN_DOWN_RIGHT) && right_angle > 0) right_angle -= 5;

        // Movimenta os servos com base nos ângulos definidos
        move_servo(SERVO_LEFT_PIN, left_angle);
        move_servo(SERVO_RIGHT_PIN, right_angle);

        // Lê e exibe a direção do joystick no terminal
        const char* direction = read_joystick();
        printf("Posição Lente Esquerda: %d°, Direita: %d° | Leitura Joystick: %s\n", left_angle, right_angle, direction);

        sleep_ms(200);  // Aguarda para evitar leituras rápidas
    }

    return 0;
}
