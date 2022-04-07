#include <twr.h>

#define ADC_MEASUREMENT_INTERVAL_MS (10 * 60 * 1000)
#define BATTERY_MEASUREMENT_INTERVAL_MS (60 * 60 * 1000)
#define TEMPERATURE_MEASUREMENT_INTERVAL_MS (15 * 60 * 1000)

#define VDD_POWER_ON_DELAY_MS (500)
#define ADC_MEASUREMENT_DELAY_MS (100)

// LED instance
twr_led_t led;

// Button instance
twr_button_t button;

// Thermometer instance
twr_tmp112_t tmp112;

typedef enum
{
    STATE_POWER_ON,
    STATE_MEASURE_CHANNEL_A,
    STATE_MEASURE_CHANNEL_B,
    STATE_MEASURE_CHANNEL_C,
    STATE_POWER_OFF
} app_state_t;

app_state_t app_state = STATE_POWER_ON;

#define ADC_CHANNEL_A 0
#define ADC_CHANNEL_B 1
#define ADC_CHANNEL_C 2

float adc_values[3];

// This function dispatches thermometer events
void tmp112_event_handler(twr_tmp112_t *self, twr_tmp112_event_t event, void *event_param)
{
    // Update event?
    if (event == TWR_TMP112_EVENT_UPDATE)
    {
        float temperature;

        // Successfully read temperature?
        if (twr_tmp112_get_temperature_celsius(self, &temperature))
        {
            twr_log_info("APP: Temperature = %0.1f C", temperature);

            // Publish temperature message on radio
            twr_radio_pub_temperature(TWR_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_ALTERNATE, &temperature);
        }
    }
    // Error event?
    else if (event == TWR_TMP112_EVENT_ERROR)
    {
        twr_log_error("APP: Thermometer error");
    }
}

// This function dispatches button events
void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    if (event == TWR_BUTTON_EVENT_CLICK)
    {
        if (app_state == STATE_POWER_ON)
        {
            // Plan application_task now
            twr_scheduler_plan_now(0);
        }
    }
}

void battery_event_handler(twr_module_battery_event_t event, void *event_param)
{
    // Update event?
    if (event == TWR_MODULE_BATTERY_EVENT_UPDATE)
    {
        float voltage;

        // Read battery voltage
        if (twr_module_battery_get_voltage(&voltage))
        {
            twr_log_info("APP: Battery voltage = %.2f", voltage);

            // Publish battery voltage
            twr_radio_pub_battery(&voltage);
        }
    }
}

void adc_event_handler(twr_adc_channel_t channel, twr_adc_event_t event, void *param)
{
    (void)param;

    if (event == TWR_ADC_EVENT_DONE)
    {
        if(channel == TWR_ADC_CHANNEL_A4)
        {
            twr_adc_async_get_voltage(TWR_ADC_CHANNEL_A4, &adc_values[ADC_CHANNEL_A]);
            twr_log_debug("A(A4): %0.2f", adc_values[ADC_CHANNEL_A]);
        }
        if(channel == TWR_ADC_CHANNEL_A5)
        {
            twr_adc_async_get_voltage(TWR_ADC_CHANNEL_A5, &adc_values[ADC_CHANNEL_B]);
            twr_log_debug("B(A5): %0.2f", adc_values[ADC_CHANNEL_B]);
        }
        if(channel == TWR_ADC_CHANNEL_A6)
        {
            twr_adc_async_get_voltage(TWR_ADC_CHANNEL_A6, &adc_values[ADC_CHANNEL_C]);
            twr_log_debug("C(A6): %0.2f", adc_values[ADC_CHANNEL_C]);
        }
    }
}

void application_init(void)
{
    // Initialize logging
    twr_log_init(TWR_LOG_LEVEL_DUMP, TWR_LOG_TIMESTAMP_ABS);

    // Initialize LED
    twr_led_init(&led, TWR_GPIO_LED, false, false);
    twr_led_pulse(&led, 500);

    // Initialize button
    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, false);
    twr_button_set_event_handler(&button, button_event_handler, NULL);

    // Initialize battery
    twr_module_battery_init();
    twr_module_battery_set_event_handler(battery_event_handler, NULL);
    twr_module_battery_set_update_interval(BATTERY_MEASUREMENT_INTERVAL_MS);

    // Initialize thermometer
    twr_tmp112_init(&tmp112, TWR_I2C_I2C0, 0x49);
    twr_tmp112_set_event_handler(&tmp112, tmp112_event_handler, NULL);
    twr_tmp112_set_update_interval(&tmp112, TEMPERATURE_MEASUREMENT_INTERVAL_MS);

    // Initialize Sensor Module
    twr_module_sensor_init();

    // Initialize ADC channel
    twr_adc_init();
    // Channel A
    twr_adc_set_event_handler(TWR_ADC_CHANNEL_A4, adc_event_handler, NULL);
    twr_adc_resolution_set(TWR_ADC_CHANNEL_A4, TWR_ADC_RESOLUTION_12_BIT);
    // Channel B
    twr_adc_set_event_handler(TWR_ADC_CHANNEL_A5, adc_event_handler, NULL);
    twr_adc_resolution_set(TWR_ADC_CHANNEL_A5, TWR_ADC_RESOLUTION_12_BIT);
    // Channel C
    twr_adc_set_event_handler(TWR_ADC_CHANNEL_A6, adc_event_handler, NULL);
    twr_adc_resolution_set(TWR_ADC_CHANNEL_A6, TWR_ADC_RESOLUTION_12_BIT);

    // Initialize radio
    twr_radio_init(TWR_RADIO_MODE_NODE_SLEEPING);

    // Send radio pairing request
    twr_radio_pairing_request("sensor-module", VERSION);

    // Pulse LED
    twr_led_pulse(&led, 2000);
}

void application_task(void)
{
    switch(app_state)
    {
        case STATE_POWER_ON:
        {
            twr_led_pulse(&led, 100);

            twr_module_sensor_set_vdd(true);
            app_state = STATE_MEASURE_CHANNEL_A;
            twr_scheduler_plan_current_relative(VDD_POWER_ON_DELAY_MS);
            break;
        }

        case STATE_MEASURE_CHANNEL_A:
        {
            twr_adc_async_measure(TWR_ADC_CHANNEL_A4);

            app_state = STATE_MEASURE_CHANNEL_B;
            twr_scheduler_plan_current_relative(ADC_MEASUREMENT_DELAY_MS);
            break;
        }

        case STATE_MEASURE_CHANNEL_B:
        {
            twr_adc_async_measure(TWR_ADC_CHANNEL_A5);

            app_state = STATE_MEASURE_CHANNEL_C;
            twr_scheduler_plan_current_relative(ADC_MEASUREMENT_DELAY_MS);
            break;
        }

        case STATE_MEASURE_CHANNEL_C:
        {
            twr_adc_async_measure(TWR_ADC_CHANNEL_A6);

            app_state = STATE_POWER_OFF;
            twr_scheduler_plan_current_relative(ADC_MEASUREMENT_DELAY_MS);
            break;
        }

        case STATE_POWER_OFF:
        {
            twr_module_sensor_set_vdd(false);

            twr_radio_pub_float("sensormodule/a/voltage", &adc_values[ADC_CHANNEL_A]);
            twr_radio_pub_float("sensormodule/b/voltage", &adc_values[ADC_CHANNEL_B]);
            twr_radio_pub_float("sensormodule/c/voltage", &adc_values[ADC_CHANNEL_C]);

            app_state = STATE_POWER_ON;
            twr_scheduler_plan_current_relative(ADC_MEASUREMENT_INTERVAL_MS);
            break;
        }
    }
}
