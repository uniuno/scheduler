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

    if (this->interval_ids_to_remove.size() > 0) {
      for (std::size_t i = 0; i < this->interval_ids_to_remove.size(); i++) {
        for (std::size_t j = 0; j < this->intervals.size(); j++) {
          if (this->intervals[j].id == this->interval_ids_to_remove[i]) {
            this->intervals.erase(this->intervals.begin() + j);
            break;
          } else {
            this->interval_ids_to_remove.erase(
                this->interval_ids_to_remove.begin() + i);
          }
        }
      }

      return;
    }

    if (this->timeout_ids_to_remove.size() > 0) {
      for (std::size_t i = 0; i < this->timeout_ids_to_remove.size(); i++) {
        for (std::size_t j = 0; j < this->timeouts.size(); j++) {
          if (this->timeouts[j].id == this->timeout_ids_to_remove[i]) {
            this->timeouts.erase(this->timeouts.begin() + j);
            break;
          } else {
            this->timeout_ids_to_remove.erase(
                this->timeout_ids_to_remove.begin() + i);
          }
        }
      }

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
        this->intervals.erase(this->intervals.begin() + i);
        printf("timeout\n");
        continue;
      }

      if (this->get_now() < this->intervals[i].next_call_time) {
        continue;
      }

      if (this->intervals[i].callback(this->intervals[i].pvParameters)) {
        this->intervals.erase(this->intervals.begin() + i);
      } else {
        this->intervals[i].next_call_time =
            this->get_now() + this->intervals[i].interval_time;
      }

      return;
    }
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