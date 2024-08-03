/*
 * scheduler_timeout_interval
 *
 * Blinks the built-in LED diode at specified interval for specified amount of
 * time.
 */

#include <scheduler.h>

uniuno::Scheduler scheduler;

void toggleLed(void *pvParameters) {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // toggles built-in LED
}

void onTimeout(void *pvParameters) {
  // last function to be called when timeout time is reached
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // set LED pin to OUTPUT

  scheduler.set_interval_until(
      toggleLed,
      nullptr, // you can pass parameters as void pointer here to be passed to
               // toggleLed
      1000, onTimeout,
      nullptr, // you can pass parameters as void pointer here to be passed to
               // onTimeout
      5000);   // schedules "toggleLed" function to be called every 1000ms for
               // 5000ms (non-blocking)
  scheduler.attach_to_loop(); // attaches scheduler to the loop ticker

  // you can run your code here
}

void loop() {
  scheduler.tick(); // tick the scheduler

  // you can run your code here
}