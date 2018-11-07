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


// ========== TAKSS ==========//
int flash_led_task () {
  static bool is_on = false;

  if (is_on) {
    is_on = false;
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    is_on = true;
    digitalWrite(LED_BUILTIN, HIGH);
  }

  return 100;
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

    // Block until the right time
    long current_time;
    while (next_time > (current_time = millis())) {}
    output = "Current_time: ";
    output += micros();
#if defined(DEBUG)
    Serial.println(output);
#endif


    task_list.last_run[next_task] = current_time;
    long delta_time = task_list.callback[next_task]();
    task_list.next_run[next_task] += delta_time;
  }

}
