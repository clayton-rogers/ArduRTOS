// ========== TASK LIST ========== //
#define DEBUG

typedef int(*Function_t)();
typedef long time_t; // time type should really be unsigned, but then you get lots of problems....

const size_t NUM_T = 10;
struct {
  Function_t callback[NUM_T];
  char name[NUM_T][12] = {""};
  byte end_of_task_list = 0;
  time_t last_run[NUM_T] = {0};
  time_t next_run[NUM_T] = {0};

  void print() {
    Serial.println("==BEGIN==");
    Serial.println((long) callback[0]);
    Serial.println(end_of_task_list);
    Serial.println(last_run[0]);
    Serial.println(next_run[0]);
    Serial.println("== END ==");
  }
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
    Serial.println(F("ERROR: Could not find a task to run!!!!"));
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

  return 900;
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


// ========== MAIN ========== //
void setup() {
  Serial.begin(9600);

  Serial.println("Hello world");

  pinMode(LED_BUILTIN, OUTPUT);

  // Add all the tasks (in order of priority)
  add_task(&double_flash_led_task, "Double");
  add_task(&flash_led_task, "Single");

  task_list.print();
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
  while (next_time-2 > long(millis())) {}
  // Delay till the top of the millisecond
  time_t delay_micro_1 = 1000 - (micros() % 1000);
  delayMicroseconds(delay_micro_1);
  time_t delay_micro_2 = 0;
  if (next_time > millis()) {
    // Delay till the top of the millisecond (again)
    delay_micro_2 = 1000 - (micros() % 1000);
    delayMicroseconds(delay_micro_2-40);
  }
  time_t begin_time = micros();

  // === Run the user code ===
  task_list.last_run[next_task] = task_list.next_run[next_task];
  time_t delta_time = task_list.callback[next_task]();
  task_list.next_run[next_task] += delta_time;
  time_t end_time = micros();

  // === Print debug timing ===
#if defined(DEBUG)
  Serial.println(delay_micro_1);
  Serial.println(delay_micro_2);
  Serial.println(begin_time);
  Serial.println(end_time);
#endif

}
