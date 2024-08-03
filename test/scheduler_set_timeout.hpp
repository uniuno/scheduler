#include <chrono>
#include <functional>
#include <helpers.hpp>
#include <scheduler.h>
#include <unity.h>

void test_set_timeout_callback_is_executed_after_specified_time(void) {
  uniuno::Scheduler scheduler;
  scheduler.attach_to_loop();
  CallbackStub callback_stub;

  scheduler.set_timeout(&CallbackStub::timeout_callback_function,
                        &callback_stub, 50);

  TEST_ASSERT_EQUAL_INT32(0, callback_stub.calls_number);
  tick_for(51, scheduler);

  TEST_ASSERT_EQUAL_INT32(1, callback_stub.calls_number);
}

void test_set_timeout_callback_is_not_executed_after_specified_time_if_scheduler_is_detached(
    void) {
  uniuno::Scheduler scheduler;
  CallbackStub callback_stub;

  scheduler.set_timeout(&CallbackStub::timeout_callback_function,
                        &callback_stub, 50);
  tick_for(60, scheduler);

  TEST_ASSERT_EQUAL_INT32(0, callback_stub.calls_number);
}

void test_set_timeout_callback_and_related_callbacks_are_executed_after_specified_time_if_it_sets_additional_timeouts(
    void) {
  uniuno::Scheduler scheduler;
  scheduler.attach_to_loop();
  CallbackStub callback_stub2;
  CallbackStub callback_stub3;
  CallbackStub callback_stub4;
  CallbackStub callback_stub5;

  CallbackStub callback_stub1(0, [&]() {
    scheduler.set_timeout(&CallbackStub::timeout_callback_function,
                          &callback_stub2, 10);
    scheduler.set_timeout(&CallbackStub::timeout_callback_function,
                          &callback_stub3, 20);
    scheduler.set_timeout(&CallbackStub::timeout_callback_function,
                          &callback_stub4, 30);
    scheduler.set_timeout(&CallbackStub::timeout_callback_function,
                          &callback_stub5, 40);
  });

  scheduler.set_timeout(&CallbackStub::timeout_callback_function,
                        &callback_stub1, 10);
  tick_for(51, scheduler);

  TEST_ASSERT_EQUAL_INT32(1, callback_stub1.calls_number);
  TEST_ASSERT_EQUAL_INT32(1, callback_stub2.calls_number);
  TEST_ASSERT_EQUAL_INT32(1, callback_stub3.calls_number);
  TEST_ASSERT_EQUAL_INT32(1, callback_stub4.calls_number);
  TEST_ASSERT_EQUAL_INT32(1, callback_stub5.calls_number);
}