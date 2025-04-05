#include <stdio.h>
#include <time.h>
#define MAX_USERS 168

typedef struct { //Intializing variables for the program
  int id;
  char first_name[50];
  char last_name[50];
  int isIn6A; // 1 for true and 0 for false
} Student;

typedef struct { //Intializing variables for the program relating to time
  int student_id;

  int month;
  int day;
  int year;

  int time_in_hour;
  int time_in_minute;

  int time_out_hour;
  int time_out_minute;
} RegisterItem;

int num_students = 0;
int num_register_items = 0;
Student students[MAX_USERS];
RegisterItem student_register[10000];

RegisterItem today() { //Function to get the current date and time
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  RegisterItem item;
  item.year = tm.tm_year + 1900;
  item.month = tm.tm_mon + 1;
  item.day = tm.tm_mday;
  item.time_in_hour = tm.tm_hour - 5;
  item.time_in_minute = tm.tm_min;
  return item;
}

void promptAndCreateStudent() { //Function to create a new student
  printf("Enter the student id: ");
  Student student;
  scanf("%d", &student.id);
  if (student.id > 9999 || student.id < 1000) {
    printf("Invalid student ID. Please enter a 4-digit number.\n");
    return;
  }
  printf("Enter the student first name: ");
  scanf("%s", student.first_name);

  printf("Enter the student last name: ");
  scanf("%s", student.last_name);

  printf("Is the student in 6A? (1 for yes, 0 for no): ");
  scanf("%d", &student.isIn6A);

  FILE *studentsFile = fopen("students.txt", "a");
  if (studentsFile == NULL) {
    printf("The file 'students.txt' could not be found\n"); //Error message if the file is not found
    exit(1);
  }
  fprintf(studentsFile, "%d %s %s %d\n", student.id, student.first_name,
          student.last_name, student.isIn6A);
  fclose(studentsFile);
}
Student *getStudent(int student_id) { //Function to get a student by ID
  for (int i = 0; i < num_students; i++) {
    if (student_id == students[i].id) {
      return &students[i];
    }
  }
  return NULL;
}
void addStudentToLeaveEarly() { //Function to add a student to the list of students who leave early
  int student_id;
  printf("Enter the student ID: ");
  // Set a student for early dismisal

  scanf("%d", &student_id);
  FILE *canLeaveEarly = fopen("canLeaveEarly.txt", "a");
  if (canLeaveEarly == NULL) {
    printf("The file 'canLeavedEarly' could not be found\n"); //Error message if the file is not found
    exit(1);
  }
  // This is what you should have:
  if (getStudent(student_id) == NULL) {
    // This student Id is not in the list of students
    printf("Student with ID# %d not found.\n", student_id);
  } else  {
    // This student is valid and will be added to the list of students who can leave early
    fprintf(canLeaveEarly, "%d\n", student_id);
    printf("Student with ID# %d added to the list of students who leave early\n", student_id);
  }

  fclose(canLeaveEarly);
}

void recreateRegisterFile() { //Function to recreate the register file
  FILE *fp = fopen("register.txt", "w");
  for (int i = 0; i < num_register_items; i++) {
    fprintf(fp, "%d %d %d %d %d %d %d %d\n", student_register[i].student_id,
            student_register[i].month, student_register[i].day,
            student_register[i].year, student_register[i].time_in_hour,
            student_register[i].time_in_minute,
            student_register[i].time_out_hour,
            student_register[i].time_out_minute);
  }
  fclose(fp);
}

void addStudentToRegister(int student_id) { //Function to add a student to the register
  Student *s = getStudent(student_id);
  if (s) {
    RegisterItem item = today();
    item.student_id = student_id;
    student_register[num_register_items] = item;
    num_register_items++;

    // Update the actual file
    recreateRegisterFile();
  }
}
void signInStudent(int student_id) { //Function to sign in a student
  printf("Attempting to sign in student #%d\n", student_id);

  Student *s = getStudent(student_id);
  if (!s) {
    printf("No such student");
    exit(0);
  }
  // Check if the student has been signed in today already
  int has_signed_in_today = 0; // Assume the student has not signed in
  for (int i = 0; i < num_register_items; i++) {
    if (student_id == student_register[i].student_id) {
      RegisterItem this_day = today();
      if (student_register[i].month == this_day.month &&
          student_register[i].year == this_day.year &&
          student_register[i].day == this_day.day) {
        has_signed_in_today = 1;
      }
    }
  }
  if (has_signed_in_today == 1) {
    printf("You have already signed in today. Get to class.");
  } else {
    addStudentToRegister(student_id);
    printf("You have been signed in!");
  }
}
int canLeaveEarly(int student_id) { //Function to check if a student can leave early
  FILE *fp = fopen("canLeaveEarly.txt", "r");
  int running_id;
  while (fscanf(fp, "%d", &running_id) == 1) {
    if (student_id == running_id) {
      return 1;
    }
  }
  return 0;
}

int isDismissalTime() { return today().time_in_hour >= 15; } //Function to check if it is time to dismiss
void signOutStudent(int student_id, int force) {
  // Ensure the student exist
  Student *s = getStudent(student_id);
  if (!s) {
    printf("No such student");
    exit(0);
  }
  // Check if the student has been signed out today already
  int has_signed_out_today = 1; // Assume the student has signed out
  RegisterItem *found;
  for (int i = 0; i < num_register_items; i++) {
    if (student_id == student_register[i].student_id) {
      RegisterItem this_day = today();
      if (student_register[i].month == this_day.month &&
          student_register[i].year == this_day.year &&
          student_register[i].day == this_day.day &&
          student_register[i].time_out_hour == 0) {

        has_signed_out_today = 0;
        found = &student_register[i];
      }
    }
  }
  if (has_signed_out_today) { // If yes, print "You were signed out early. Leave
                              // the campus please."
    printf("Please ensure you signed in earlier today. If you did sign in, you have already been signed out. Leave the campus please.\n");
  } else { // If no, set the check out time in today's register and print "You
           // have been signed out successfully!""

    /** Break this if statement into two parts
    The first part will check isDismaisalTime() || force
    That if statement inffcormation stayas the same

    The second part will check if the student can leave early
    If the cannot then print "You attempted to leave early without permission. The supervisior will be notified."
    Add the student ID to a file called "delinquents.txt"
    **/
    if (canLeaveEarly(student_id) || isDismissalTime() || force == 1) {
      found->time_out_hour = today().time_in_hour;
      found->time_out_minute = today().time_in_minute;
      recreateRegisterFile();
      printf("You have been signed out!\n");
    } else {
      printf("You attempted to leave early without permission. The supervisior will be notified\n");
      FILE *fp = fopen("register.txt", "r");
  if (fp == NULL) {
    printf("Register Database could not be found. There is an issue\n");

    }
  }
}

void loadRegisterDatabase() {
  // Load the register database from the file "register.txt"
  FILE *fp = fopen("register.txt", "r");
  if (fp == NULL) {
    printf("Register Database could not be found. There is an issue\n");
    exit(0); // Error
  }
  // Read the register database line by line

  while (fscanf(fp, "%d %d %d %d %d %d %d %d",
                &(&student_register[num_register_items])->student_id,
                &(&student_register[num_register_items])->month,
                &(&student_register[num_register_items])->day,
                &(&student_register[num_register_items])->year,
                &(&student_register[num_register_items])->time_in_hour,
                &(&student_register[num_register_items])->time_in_minute,
                &(&student_register[num_register_items])->time_out_hour,
                &(&student_register[num_register_items])->time_out_minute) ==
         8) {
    ;
    num_register_items++;
  }
  fclose(fp);
}

void loadStudentsFromDatabase() { //Function to load students from the database
  FILE *fp = fopen("students.txt", "r");
  if (fp == NULL) {
    printf("The student file could not be found"); //Error message if the file is not found
    exit(1);
  }
  while (fscanf(fp, "%d %s %s %d", &(&students[num_students])->id,
                (&students[num_students])->first_name,
                (&students[num_students])->last_name,
                &(&students[num_students])->isIn6A) == 4) {
    printf("Loaded student #%d: %s %s\n", (&students[num_students])->id,
           (&students[num_students])->first_name,
           (&students[num_students])->last_name);
    num_students++;
  }
}

int isStudentId(int id) { //Function to check if a student ID is valid
  if (id > 9999 || id < 1000) {
    printf("The ID was not valid. Please ensure you enter 4 digits and/or a ID "
           "starting with a number from 1 to 9.\n");
    exit(1);
  }
  // All student IDs start with a digit from 1-8
  // But a supervisor ID starts with a 9
  // Ex. Student ID would be 8001, Supervisor ID would be 9001
  int numAtStart = (id / 1000);
  if (numAtStart >= 1 && numAtStart <= 8) {
    return 1; // This is a student
  }

  return 0; // This is not a student
}

void manageSupervisorOptionSelected(int option) { //Function to manage the supervisor option selected

  int student_id;
  switch (option) {
  case 1:
    addStudentToLeaveEarly();
    break;
  case 2:
    printf("Enter the student id: ");
    // Sign Out a student
    scanf("%d", &student_id);
    signOutStudent(student_id, 1);
    //....
    break;
  case 3:
    // Create a student
    promptAndCreateStudent();
    loadStudentsFromDatabase();
    break;
  case 4:
    // View delinquents
            printf("The names of the students are, %s:\n", student.firstname);

            printf("The student ID of the delinquents are %s:\n", student_id);
  /** Case 4: Check Notifications
    Looks in the delinquents.txt file and prints out the student ID and name to let the supervisor know that these students tried to leave early without permission.
    **/
    break;
  }
}
int main(void) { //Main function
  loadStudentsFromDatabase();
  loadRegisterDatabase();
  int response;
  int id_number;
  printf("***********************************\n");
  printf("*        Welcome to Our System    *\n");
  printf("***********************************\n");
  printf("\n");
  printf("Please choose an option:\n");
  printf("1. Sign In\n");
  printf("2. Sign Out\n");
  // printf("3. Enter Supervisor password\n"");
  printf("\n");
  scanf("%d", &response);
  printf("You have chosen option %d\n", response);
  printf("Please scan your ID number: ");
  scanf("%d", &id_number);

  if (isStudentId(id_number) == 1) {
    if (response == 1) {
      // Wants to sign in
      signInStudent(id_number);
    } else if (response == 2) {
      // The person wants to sign out
      signOutStudent(id_number, 0);
    }
  } else { // supervisor teacher
    char password[50];
    printf("Please enter the supervisor password: ");
    scanf("%s", password);
    if (strcmp(password, "supervisor") != 0) {
      exit(0);
    }
    int option;
    do {
      // Prompt the supervisor to select an option
      printf("***********************************\n");
      printf("*      Please select an Option    *\n");
      printf("***********************************\n");
      printf("1. Set a student for early dismissal\n");
      printf("2. Sign out a student\n");
      printf("3. Create a student\n");
      printf("4. View Deliquents\n");

      printf("5. Exit\n");

      scanf("%d", &option);
      manageSupervisorOptionSelected(option);
    } while (option != 4);
  }
}
