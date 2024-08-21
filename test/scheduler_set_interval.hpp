#include <array>
#include <chrono>
#include <functional>
#include <helpers.hpp>
#include <scheduler.h>
#include <stdio.h>
#include <unity.h>

void test_set_interval_callback_is_executed_in_specified_interval(void) {
  uniuno::Scheduler scheduler;
  scheduler.attach_to_loop();
  CallbackStub callback_stub;

  scheduler.set_interval(&CallbackStub::interval_callback_function,
                         &callback_stub, 10);
  tick_for(51, scheduler);

  TEST_ASSERT_EQUAL_INT32(5, callback_stub.calls_number);
}

void test_set_interval_callback_is_not_executed_if_scheduler_is_detached(void) {
  uniuno::Scheduler scheduler;
  CallbackStub callback_stub;

  scheduler.set_interval(&CallbackStub::interval_callback_function,
                         &callback_stub, 50);
  tick_for(11, scheduler);

  TEST_ASSERT_EQUAL_INT32(0, callback_stub.calls_number);
}

void test_set_interval_callback_is_not_executed_if_interval_has_been_cleared_before_first_execution(
    void) {
  uniuno::Scheduler scheduler;
  scheduler.attach_to_loop();
  CallbackStub callback_stub;

  auto id = scheduler.set_interval(&CallbackStub::interval_callback_function,
                                   &callback_stub, 20);
  tick_for(19, scheduler);
  scheduler.clear_interval(id);
  tick_for(12, scheduler);

  TEST_ASSERT_EQUAL_INT32(0, callback_stub.calls_number);
}

void test_set_interval_callback_and_related_callbacks_are_executed_in_specified_interval_if_callback_sets_additional_intervals(
    void) {
  uniuno::Scheduler scheduler;
  scheduler.attach_to_loop();
  CallbackStub callback_stub2(2);
  CallbackStub callback_stub3(3);
  CallbackStub callback_stub4(4);
  CallbackStub callback_stub5(5);

  CallbackStub callback_stub1(1, [&]() {
    scheduler.set_interval(&CallbackStub::interval_callback_function,
                           &callback_stub2, 10);
    scheduler.set_interval(&CallbackStub::interval_callback_function,
                           &callback_stub3, 20);
    scheduler.set_interval(&CallbackStub::interval_callback_function,
                           &callback_stub4, 30);
    scheduler.set_interval(&CallbackStub::interval_callback_function,
                           &callback_stub5, 40);
  });

  scheduler.set_interval(&CallbackStub::interval_callback_function,
                         &callback_stub1, 10);
  tick_for(101, scheduler);

  TEST_ASSERT_GREATER_OR_EQUAL(8, callback_stub1.calls_number);
  TEST_ASSERT_GREATER_OR_EQUAL(45, callback_stub2.calls_number);
  TEST_ASSERT_GREATER_OR_EQUAL(20, callback_stub3.calls_number);
  TEST_ASSERT_GREATER_OR_EQUAL(12, callback_stub4.calls_number);
  TEST_ASSERT_GREATER_OR_EQUAL(2, callback_stub5.calls_number);
}

uint8_t counter = 0;

struct interval_until_params {
  uniuno::Scheduler *scheduler;
  std::array<CallbackStub, 16> *callback_stubs;
};

bool interval_until_callback_function(void *pvParameters) {
  interval_until_params *params = (interval_until_params *)pvParameters;

  if (counter++ == params->callback_stubs->size()) {
    for (uint8_t i = 0; i < params->callback_stubs->size(); i++) {
      params->scheduler->set_interval(&CallbackStub::interval_callback_function,
                                      &params->callback_stubs->at(i), 10 + i);
    }
    return true;
  }
  return false;
}

void test_set_interval_executes_interconnected_intervals_with_the_use_of_set_interval_until(
    void) {
  uniuno::Scheduler scheduler;
  scheduler.attach_to_loop();
  std::array<CallbackStub, 16> callback_stubs;
  for (uint8_t i = 0; i < callback_stubs.size(); i++) {
    callback_stubs[i] = CallbackStub(i);
  }

  counter = 0;
  interval_until_params params = {&scheduler, &callback_stubs};
  scheduler.set_interval_until(&interval_until_callback_function, &params, 11);

  tick_for(100 * callback_stubs.size(), scheduler);

  TEST_ASSERT_EQUAL(callback_stubs.size() + 1, counter);
  for (uint8_t i = 0; i < callback_stubs.size(); i++) {
    printf("testing callback %u\n", i);
    TEST_ASSERT_GREATER_OR_EQUAL(1, callback_stubs[i].calls_number);
  }
}