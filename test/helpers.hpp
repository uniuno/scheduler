#pragma once

#include <chrono>
#include <functional>
#include <scheduler.h>

class CallbackStub {
public:
  CallbackStub(
      int i = 0, std::function<void(void)> on_called = []() {})
      : on_called(on_called) {
    this->i = i;
  }

  void reset() { this->calls_number = 0; }

  static bool interval_callback_function(void *pvParameters) {
    CallbackStub *callback_stub = static_cast<CallbackStub *>(pvParameters);
    callback_stub->call();
    return false;
  }

  static void timeout_callback_function(void *pvParameters) {
    CallbackStub *callback_stub = static_cast<CallbackStub *>(pvParameters);
    callback_stub->call();
  }

  uint32_t calls_number = 0;
  int i;

private:
  void call() {
    this->on_called();
    this->calls_number++;
  }

  std::function<void(void)> on_called;
};

void tick_for(unsigned long time, uniuno::Scheduler &scheduler) {
  unsigned long current_time = millis();
  while (millis() < current_time + time) {
    scheduler.tick();
  }
}