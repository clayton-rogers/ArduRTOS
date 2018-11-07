// ========== TASK LIST ========== //
#define DEBUG

typedef int(*Function_t)();
typedef long time_t; // time type should really be unsigned, but then you get lots of problems....

const size_t NUM_T = 10;
struct {
  Function_t callback[NUM_T];
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

void add_task (Function_t function) {
  task_list.callback[task_list.end_of_task_list++] = function;
}

size_t get_next_task() {
  size_t next_task = -1;
  time_t next_time = -1;

  for (int i = task_list.end_of_task_list - 1; i >= 0; --i) {
    if (task_list.next_run[i] > next_time) {
      next_time = task_list.next_run[i];
      next_task = i;
    }
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

  return 987;
}


// ========== MAIN ========== //
void setup() {
  Serial.begin(9600);

  Serial.println("Hello world");


  pinMode(LED_BUILTIN, OUTPUT);

  // Add all the tasks
  add_task(&flash_led_task);
  task_list.print();
}

void loop() {

  // === Get the next task's information ===
  size_t next_task = get_next_task();
  String output = "Next_task: ";
  output += next_task;
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
  time_t current_time;
  // Delay until t - (2 .. 0.9 milliseconds)
  while (next_time - 2 > (current_time = millis())) {}
  time_t delay_micro = 2000 - (micros() % 2000);
  delayMicroseconds(delay_micro - 42); // 44 is an experimentally defined correction factor.
  time_t begin_time = micros();

  // === Run the user code ===
  task_list.last_run[next_task] = task_list.next_run[next_task];
  time_t delta_time = task_list.callback[next_task]();
  task_list.next_run[next_task] += delta_time;
  time_t end_time = micros();

  // === Print debug timing ===
#if defined(DEBUG)
  Serial.println(begin_time);
  Serial.println(end_time);
#endif

}
