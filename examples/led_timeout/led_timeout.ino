/*
 * scheduler_timeout
 *
 * Switches on the built-in LED diode after specified amount of time.
 */

#include <scheduler.h>

struct example_params {
  int param;
};

uniuno::Scheduler scheduler;

void toggleLed(void *pvParameters) {
  example_params *params =
      (example_params *)pvParameters; // cast parameters from void pointer

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // toggles built-in LED
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // set LED pin to OUTPUT

  example_params params = {42}; // example parameters

  scheduler.set_timeout(toggleLed,
                        &params, // you can pass parameters as void pointer here
                        5000);   // schedules "toggleLed" function to be called
                                 // after 5000ms of wait (non-blocking)
  scheduler.attach_to_loop();    // attaches scheduler to the loop ticker

  // you can run your code here
}

void loop() {
  scheduler.tick(); // tick the scheduler

  // you can run your code here
}