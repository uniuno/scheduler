#include <scheduler_set_interval.hpp>
#include <scheduler_set_timeout.hpp>
#include <unity.h>

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_set_timeout_callback_is_executed_after_specified_time);
  RUN_TEST(
      test_set_timeout_callback_is_not_executed_after_specified_time_if_scheduler_is_detached);
  RUN_TEST(
      test_set_timeout_callback_and_related_callbacks_are_executed_after_specified_time_if_it_sets_additional_timeouts);
  RUN_TEST(test_set_interval_callback_is_executed_in_specified_interval);
  RUN_TEST(test_set_interval_callback_is_not_executed_if_scheduler_is_detached);
  RUN_TEST(
      test_set_interval_callback_is_not_executed_if_interval_has_been_cleared_before_first_execution);
  RUN_TEST(
      test_set_interval_callback_and_related_callbacks_are_executed_in_specified_interval_if_callback_sets_additional_intervals);
  RUN_TEST(
      test_set_interval_executes_interconnected_intervals_with_the_use_of_set_interval_until);
  return UNITY_END();
}