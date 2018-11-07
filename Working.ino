// ========== TASK LIST ========== //
#define DEBUG

typedef int(*Function_t)();

const size_t NUM_T = 10;
struct {
  Function_t callback[NUM_T];
  byte end_of_task_list = 0;
  long last_run[NUM_T] = {0};
  long next_run[NUM_T] = {0};

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
  long next_time = -1;

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

  while (true) {
    // === Get the next task's information ===
    size_t next_task = get_next_task();
    String output = "Next_task: ";
    output += next_task;
#if defined(DEBUG)
    Serial.println(output);
#endif
    long next_time = task_list.next_run[next_task];
    output = "Next_time: ";
    output += next_time;
#if defined(DEBUG)
    Serial.println(output);
#endif

    // === Block until the right time ===
    long current_time;
    // Delay until t - (2 .. 0.9 milliseconds)
    while (next_time-2 > (current_time = millis())) {}
    long delay_micro = 2000 - (micros() % 2000);
    delayMicroseconds(delay_micro-42); // 44 is an experimentally defined correction factor.
    long begin_time = micros();

    // === Run the user code ===
    task_list.last_run[next_task] = task_list.next_run[next_task];
    long delta_time = task_list.callback[next_task]();
    task_list.next_run[next_task] += delta_time;
    long end_time = micros();

    // === Print debug timing ===
#if defined(DEBUG)
    Serial.println(begin_time);
    Serial.println(end_time);
#endif
  }

}
