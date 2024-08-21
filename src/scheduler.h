#pragma once

#include <algorithm>
#include <chrono>
#include <functional>
#include <queue>
#include <vector>

typedef void (*TaskTimeoutFunction_t)(void *);
typedef unsigned long (*GetTimeFunction_t)();
typedef bool (*TaskIntervalFunction_t)(void *);

#ifndef ARDUINO
static unsigned long millis() {
  return std::chrono::system_clock::now().time_since_epoch() /
         std::chrono::milliseconds(1);
};
#endif

namespace uniuno {

class Scheduler {
public:
  Scheduler(GetTimeFunction_t get_time = millis, size_t size = 64,
            size_t queue_size = 16) {
    this->now = get_time;

    this->timeouts.reserve(size);
    this->intervals.reserve(size);

    this->timeout_ids_to_remove.reserve(queue_size);
    this->interval_ids_to_remove.reserve(queue_size);
  }

  uint16_t set_timeout(TaskTimeoutFunction_t callback, void *pvParameters,
                       uint32_t timeout_ms) {
    this->timeouts.push_back(Timeout{this->index, this->get_now() + timeout_ms,
                                     callback, pvParameters});

    std::sort(this->timeouts.begin(), this->timeouts.end(),
              &Scheduler::sort_timeouts);

    return this->index++;
  }

  uint16_t set_immediate(TaskTimeoutFunction_t callback, void *pvParameters) {
    return this->set_timeout(callback, pvParameters, 0);
  }

  uint16_t set_interval(TaskIntervalFunction_t callback, void *pvParameters,
                        uint32_t interval_time) {
    this->intervals.push_back(Interval{this->index,
                                       this->get_now() + interval_time,
                                       interval_time, callback, pvParameters});

    return this->index++;
  }

  uint16_t set_on_loop(TaskIntervalFunction_t callback, void *pvParameters) {
    return this->set_interval(callback, pvParameters, 0);
  }

  void clear_timeout(uint16_t timeout_id) {
    this->timeout_ids_to_remove.push_back(timeout_id);
  }

  void clear_interval(uint16_t interval_id) {
    this->interval_ids_to_remove.push_back(interval_id);
  }

  uint16_t set_interval_until(TaskIntervalFunction_t callback,
                              void *pvParameters, uint32_t interval_time,
                              TaskTimeoutFunction_t on_timeout,
                              void *pvTimeoutParameters,
                              uint32_t timeout_ticks) {
    uint32_t timeout_time = this->get_now() + timeout_ticks;

    this->intervals.push_back(Interval{
        this->index, this->get_now() + interval_time, interval_time, callback,
        pvParameters, on_timeout, pvTimeoutParameters, timeout_time});

    return this->index++;
  }

  uint16_t set_interval_until(TaskIntervalFunction_t callback,
                              void *pvParameters, uint32_t interval_time) {
    this->intervals.push_back(Interval{this->index,
                                       this->get_now() + interval_time,
                                       interval_time, callback, pvParameters});

    return this->index++;
  }

  uint16_t set_on_loop_until(TaskIntervalFunction_t callback,
                             void *pvParameters,
                             TaskTimeoutFunction_t on_timeout,
                             void *pvTimeoutParameters, uint32_t timeout_time) {
    return this->set_interval_until(callback, pvParameters, 0, on_timeout,
                                    pvTimeoutParameters, timeout_time);
  }

  void attach_to_loop() { this->attached = true; }
  void detach_from_loop() { this->attached = false; }

  void tick() {
    if (!this->attached) {
      return;
    }

    if (!this->interval_ids_to_remove.empty()) {
      for (const auto &id_to_remove : this->interval_ids_to_remove) {
        auto it = std::remove_if(this->intervals.begin(), this->intervals.end(),
                                 [&id_to_remove](const Interval &interval) {
                                   return interval.id == id_to_remove;
                                 });
        if (it != this->intervals.end()) {
          this->intervals.erase(it, this->intervals.end());
        }
      }

      this->interval_ids_to_remove.clear();
      return;
    }

    if (!this->timeout_ids_to_remove.empty()) {
      for (const auto &id_to_remove : this->timeout_ids_to_remove) {
        auto it = std::remove_if(this->timeouts.begin(), this->timeouts.end(),
                                 [&id_to_remove](const Timeout &timeout) {
                                   return timeout.id == id_to_remove;
                                 });
        if (it != this->timeouts.end()) {
          this->timeouts.erase(it, this->timeouts.end());
        }
      }

      this->timeout_ids_to_remove.clear();
      return;
    }

    for (std::size_t i = 0; i < this->timeouts.size(); i++) {
      if (this->get_now() < this->timeouts[i].next_call_time) {
        break;
      }

      this->timeouts[i].callback(this->timeouts[i].pvParameters);
      this->timeouts.erase(this->timeouts.begin() + i);
    }

    for (std::size_t i = 0; i < this->intervals.size(); i++) {
      if (this->get_now() >= this->intervals[i].timeout_time &&
          this->intervals[i].on_timeout_callback != nullptr) {
        this->intervals[i].on_timeout_callback(
            this->intervals[i].pvTimeoutParameters);

        this->timeout_ids_to_remove.push_back(this->intervals[i].id);
        continue;
      }

      if (this->get_now() < this->intervals[i].next_call_time) {
        continue;
      }

      if (this->intervals[i].callback(this->intervals[i].pvParameters)) {
        this->interval_ids_to_remove.push_back(this->intervals[i].id);
      } else {
        this->intervals[i].next_call_time =
            this->get_now() + this->intervals[i].interval_time;
      }

      return;
    }
  }

  bool has_tasks() {
    return this->timeouts.size() > 0 && this->intervals.size() > 0;
  }

  uint32_t get_time_until_next_call() {
    if (this->timeouts.size() == 0 && this->intervals.size() == 0) {
      return 0;
    }

    uint32_t min_time = 0;

    for (std::size_t i = 0; i < this->timeouts.size(); i++) {
      if (this->timeouts[i].next_call_time < min_time) {
        min_time = this->timeouts[i].next_call_time;
      }
    }

    for (std::size_t i = 0; i < this->intervals.size(); i++) {
      if (this->intervals[i].next_call_time < min_time) {
        min_time = this->intervals[i].next_call_time;
      }
    }

    if (min_time <= 0) {
      return 0;
    }

    return min_time - this->get_now();
  }

private:
  GetTimeFunction_t now;

  uint16_t index = 0;

  struct Timeout {
    uint16_t id;
    uint32_t next_call_time;
    TaskTimeoutFunction_t callback;
    void *pvParameters;
  };

  std::vector<Timeout> timeouts;

  struct Interval {
    uint16_t id;
    uint32_t next_call_time;
    uint32_t interval_time;
    TaskIntervalFunction_t callback;
    void *pvParameters;
    TaskTimeoutFunction_t on_timeout_callback;
    void *pvTimeoutParameters;
    uint32_t timeout_time;
  };

  std::vector<Interval> intervals;
  std::vector<uint16_t> interval_ids_to_remove;
  std::vector<uint16_t> timeout_ids_to_remove;

  bool attached = false;

  uint32_t get_now() {
    auto now = this->now();
    return static_cast<uint32_t>(now);
  }

  static bool sort_timeouts(Scheduler::Timeout t1, Scheduler::Timeout t2) {
    return t1.next_call_time < t2.next_call_time;
  }
};

} // namespace uniuno