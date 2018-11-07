// ========== TASK LIST ========== //
//#define DEBUG

typedef int(*Function_t)();
typedef long time_t; // time type should really be unsigned, but then you get lots of problems....

const size_t NUM_T = 10;
struct {
  Function_t callback[NUM_T];
  char name[NUM_T][12] = {""};
  byte end_of_task_list = 0;
  time_t last_run[NUM_T] = {0};
  time_t next_run[NUM_T] = {0};
} task_list;

void add_task (Function_t function, const char* const task_name) {
  task_list.callback[task_list.end_of_task_list] = function;
  strncpy(task_list.name[task_list.end_of_task_list], task_name, 12);
  task_list.name[task_list.end_of_task_list][11] = '\0';
  task_list.end_of_task_list++;
}

size_t get_next_task() {
  size_t next_task = -1;
  time_t next_time = 0x7FFFFFFF;

  for (int i = 0; i < task_list.end_of_task_list; ++i) {
    if (task_list.next_run[i] < next_time) {
      next_time = task_list.next_run[i];
      next_task = i;
    }
  }

  if (next_task == -1) {
#if defined(DEBUG)
    Serial.println(F("ERROR: Could not find a task to run!!!!"));
#endif
    delay(100000);
  }
  return next_task;
}


// ========== TASKS ==========//
int flash_led_task () {
  static bool is_on = false;

  if (is_on) {
    is_on = false;
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    is_on = true;
    digitalWrite(LED_BUILTIN, HIGH);
  }

  //return 123;
  return 987;
  //return 14805; // Used to be an issue, would lose 15.500 milliseconds
}

int double_flash_led_task() {
  enum State_t : byte {
    OFF,
    ON_1,
    OFF_1,
    ON_2
  };
  static State_t state = OFF;

  switch (state) {
    case OFF:
      state = ON_1;
      digitalWrite(LED_BUILTIN, LOW);
      return 3500;
    case ON_1:
      state = OFF_1;
      digitalWrite(LED_BUILTIN, HIGH);
      return 100;
    case OFF_1:
      state = ON_2;
      digitalWrite(LED_BUILTIN, LOW);
      return 100;
    case ON_2:
      state = OFF;
      digitalWrite(LED_BUILTIN, HIGH);
      return 100;
  }
  return 0; // Should never reach
}

int error_list[4] = {0};
int error_index = 0;

void add_error(long error_time) {
  if (error_index++ % 10 == 0) {
    String es = "Error results: ";
    for (int i = 0; i < 4; ++i) {
      es += error_list[i];
      es += " ";
      error_list[i] = 0;
    }
    Serial.println(es);
  }
  switch (error_time) {
    case 0:
      error_list[0]++;
      break;
    case 4:
      error_list[1]++;
      break;
    case -4:
      error_list[2]++;
      break;
    default:
      error_list[3]++;
      break;
  }
}


// ========== MAIN ========== //
void setup() {
  Serial.begin(9600);
#if defined(DEBUG)
  Serial.println("Hello world");
#endif

  pinMode(LED_BUILTIN, OUTPUT);

  // Add all the tasks (in order of priority)
  add_task(&double_flash_led_task, "Double");
  add_task(&flash_led_task, "Single");
}

void loop() {

  // === Get the next task's information ===
  size_t next_task = get_next_task();
  String output = "Next_task: ";
  output += task_list.name[next_task];
#if defined(DEBUG)
  Serial.println(output);
#endif
  time_t next_time = task_list.next_run[next_task];
  output = "Next_time: ";
  output += next_time;
#if defined(DEBUG)
  Serial.println(output);
#endif

  // === Block until the right time ===
  // Delay until t - (2 .. 0.9 milliseconds)
  long next_time_last_digit = next_time %10;
  while (next_time-2 > long(millis())) {}
  while (next_time_last_digit != ((micros() + 100) / 1000) % 10) {
    // Delay till the top of the millisecond
    time_t delay_micro = 1000 - (micros() % 1000) - 124;
    delay_micro = max(delay_micro, 0);
    delayMicroseconds(delay_micro);
  }
  time_t begin_time = micros();
  time_t error = begin_time - (next_time*1000);
  add_error(error);

  // === Run the user code ===
  task_list.last_run[next_task] = task_list.next_run[next_task];
  time_t delta_time = task_list.callback[next_task]();
  task_list.next_run[next_task] += delta_time;
  time_t end_time = micros();

  // === Print debug timing ===
#if defined(DEBUG)
  Serial.println(error);
  Serial.println(begin_time);
  Serial.println(end_time);
#endif

}
