/*
 * scheduler_interval
 *
 * Blinks the built-in LED diode every second.
 */

#include <scheduler.h>

struct example_params {
  int param;
};

uniuno::Scheduler scheduler;

bool toggleLed(void *pvParameters) {
  example_params *params =
      (example_params *)pvParameters; // cast parameters from void pointer

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // toggles built-in LED

  return false; // returns false to keep the interval running
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // set LED pin to OUTPUT

  example_params params = {42}; // example parameters

  scheduler.set_interval(
      toggleLed,
      &params, // you can pass parameters as void pointer here
      1000);   // schedules "toggleLed" function to be called at 1000ms interval
  scheduler.attach_to_loop(); // attaches scheduler to the loop ticker (starts
                              // the interval calls)

  // you can run your code here
}

void loop() {
  scheduler.tick(); // tick the scheduler

  // you can run your code here
}