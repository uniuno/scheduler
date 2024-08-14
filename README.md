# Scheduler

Minimal C++ library to schedule function calls on an arduino framework platform.
It supports timeouts, intervals, and timeout intervals.

## Installation

Add following line to "lib_deps" section in `platformio.ini`:

```
uniuno/scheduler@^1.0.1
```

so it should look like:

```
; other config sections...

lib_deps =
; other deps...
	uniuno/scheduler@^1.0.0
```

## Usage

### Creating an instance and attaching to loop

1. Add following header to your main.cpp file:

```cpp
#include <scheduler.h>
```

2. Create a new instance of Scheduler class:

```cpp
uniuno::Scheduler scheduler;
```

3. Define parameters struct:

```cpp
struct ExampleParameters {
  int numParam;
  bool boolParam;
};
```

4. Schedule a function in the "setup" and attach the scheduler to the loop:

```cpp
// ...

void callback(void *pvParameters) {
  ExampleParameters *p = static_cast<ExampleParameters *>(pvParameters);

  Serial.println("5 seconds has passed");
  Serial.printf("My params: %d %d\n", p->numParam, p->boolParam);

  free(p);
}

void setup() {
  // ...

  // prepare optional params
  ExampleParameters *p = new ExampleParameters();
  p->numParam = 42;
  p->boolParam = true;

  // passed lambda will call in 5 seconds
  scheduler.set_timeout(&callback, static_cast<void *>(p), 5000);

  // attach scheduler to the loop
  scheduler.attach_to_loop();
}
// ...
```

4. Call tick method in the "loop":

```cpp
// ...
void loop() {
  // ...
  scheduler.tick();
  // ...
}
// ...
```

### API

#### set_timeout

Calls function after specified amount of time:

```cpp
  void fn(void *pvParameters) {
    Serial.println("5 seconds has passed");
  }

  unsigned long timeout = 5000;

  // ...
  scheduler.set_timeout(&fn, nullptr, timeout);
```

#### set_interval

Calls function at specified interval:

```cpp
  bool fn(void *pvParameters) {
    Serial.println("1 second has passed");
    return false; // return true to stop interval from the function
  }

  unsigned long interval = 1000;

  // ...
  scheduler.set_interval(&fn, nullptr interval);
```

#### set_interval_until

Calls function at specified interval for specified amount of time:

```cpp
  bool intervalFn(void *pvParameters) {
    Serial.println("1 second has passed");
    return false;
  };

  unsigned long interval = 1000;

  void onTimeout(void *pvParameters) {
    Serial.println("5 seconds has passed");
  };

  unsigned long timeout = 5000;

  // ...
  scheduler.set_interval_until(&intervalFn, nullptr, interval, &onTimeout, nullptr, timeout);
```

#### set_immediate

Calls function immediately:

```cpp
  void fn(void *pvParameters) { Serial.println("immediate call"); };

  // ...
  scheduler.set_immediate(&fn, nullptr);
```

#### set_on_loop

Calls function at every loop call:

```cpp
  bool fn(void *pvParameters) {
    Serial.println("loop call (tick)");
    return false;
  };

  // ...
  scheduler.set_on_loop(&fn, nullptr);
```

#### set_on_loop_until

Calls function at every loop call for the specified amount of time:

```cpp
  bool fn(void *pvParameters) {
    Serial.println("loop call (tick)");
    return false;
  };

  void onTimeout(void *pvParameters) { Serial.println("5 seconds has passed")};

  unsigned long timeout = 5000;

  // ...
  scheduler.set_on_loop_until(&fn, nullptr &onTimeout, nullptr, timeout);
```

#### clear_timeout

Removes scheduled timeout if scheduled function has not been called yet:

```cpp
  // function scheduled to be called in 5 seconds (it won't be called as we cancel it)
  void fn(void *pvParameters) { Serial.println("5 seconds has passed"); };

  unsigned long timeout = 5000;

  auto timeoutId = scheduler.set_timeout(&fn, nullptr, timeout);

  // function to clear the timeout before it is executed (in 4 seconds)
  void cancelFn(void *pvParameters) { scheduler.clear_timeout(timeoutId); };

  // ...
  scheduler.set_timeout(&cancelFn, 4000);

```

#### clear_interval

Removes scheduled interval - stops calling interval function:

```cpp
  // function scheduled to be called every 1 seconds (we'll stop calling it after 4 seconds)
  bool fn(void *pvParameters) {
    Serial.println("1 seconds has passed");
    return false;
  };

  unsigned long interval = 1000;

  auto intervalId = scheduler.set_interval(&fn, nullptr, timeout);

  // function to clear the interval after 4 seconds
  void cancelFn(void *pvParameters) {
    scheduler.clear_interval(intervalId);
  };

  // ...
  scheduler.set_timeout(&cancelFn, nullptr, 4000);

```

#### attach_to_loop

Attaches scheduler instance to the loop. When scheduler is attached the timeout and interval functions will be executed.

#### detach_from_loop

Detaches scheduler instance from the loop. When scheduler is detached no scheduled functions will be executed.

#### tick

Method responsible for calling scheduled functions on time.
It has to be executed at every single "loop" function call.

#### constructor

When creating a new instance of Timer you can specify "clock" function that returns current time in a unit of your choice (default is milliseconds):

For example for microseconds:

```cpp
// ...

unsigned long micros() {
  return std::chrono::system_clock::now().time_since_epoch() /
         std::chrono::microseconds(1);
};

// create microseconds scheduler
uniuno::Scheduler scheduler(micros);

// ...
```

### Parameters Lifetime

An example how to schedule parametrized function calls without the need of heap allocation:

```cpp

#include <scheduler.h>

uniuno::Scheduler scheduler;

class Example {
  public:
    static bool task(void *pvParameters) {
      Example *example = static_cast<Example *>(pvParameters);

      example->executeTask();

      return false;
    }

    void begin() {
      this->intervalId = scheduler.set_interval(&Example::task, this, 5000);
    }

    void end() {
      scheduler.clear_interval(this->intervalId);
    }

  private:
    void executeTask() {
      this->counter++;
    }

    uint32_t counter = 0;
    uint16_t intervalId;
};


Example example;

void setup() {
  example.begin();
}


```

## Licence

MIT.
