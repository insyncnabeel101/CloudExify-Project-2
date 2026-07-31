/*****************************************************************
 *  CloudExify Internship 2026 — C++ Month 1, Project 2
 *  ---------------------------------------------------------
 *  STUDENT RECORD SYSTEM  (Enhanced Edition + All Bonus Challenges
 *                          + Full Color Theming + Extra Robustness)
 *  ---------------------------------------------------------
 *  File   : game.cpp
 *
 *  DESCRIPTION
 *    A console-based student record management system built around
 *    a struct-based array data model. A teacher can add students
 *    with marks in 3 subjects, view records in a formatted table,
 *    edit/delete records, search by roll number or name, sort by
 *    average or name (bubble sort), view class statistics and a
 *    non-destructive ranking, see subject-wise averages and a grade
 *    distribution chart, and save/load everything to/from a file.
 *
 *  CORE FEATURES (Required)
 *    - Add student: name, roll number, 3 subject marks
 *    - View all students in a formatted, aligned table
 *    - Calculate and display each student's average
 *    - Find and display the class topper
 *    - Sort students by average (bubble sort)
 *    - Save and load records from a text file
 *
 *  BONUS FEATURES (All 5 Implemented)
 *    1. Letter grade (A/B/C/D/F) computed and shown for every student
 *    2. Sort by name alphabetically (A-Z and Z-A, bubble sort)
 *    3. Search student by roll number
 *    4. Delete student by ID (with confirmation)
 *    5. Subject-wise class average (plus top scorer per subject)
 *
 *  MAJOR EXTRAS (beyond requirements)
 *    - Stable, never-reused Student IDs
 *    - Edit Student (name / roll number / marks)
 *    - Search by name (partial match, case-insensitive)
 *    - Non-destructive "Class Ranking" report
 *    - Grade Distribution report with an ASCII bar chart
 *    - "Needs Improvement" (lowest performer) statistic
 *    - Live dashboard with color-coded status indicators
 *    - Unsaved-changes tracking, with a save prompt on exit
 *    - Auto-loads saved data on startup
 *    - Corruption-safe file loading with full field-range validation
 *    - "Export Formatted Report" to report.txt
 *    - Animated splash screen
 *    - Full color-coded theming: every menu section has its own
 *      identity color (Manage=Blue, Reports=Magenta, Sort=Yellow,
 *      File=Green), used consistently in headers, dividers, menu
 *      numbers, and prompts throughout that section
 *    - Zebra-striped table rows for readability
 *
 *  CURRICULUM SCOPE NOTE
 *    This project deliberately avoids features not yet taught
 *    (no classes/OOP, no switch-statements, no std::sort — bubble
 *    sort is hand-written exactly as required). It fully embraces
 *    this month's NEW concepts: struct, arrays of structs, file
 *    I/O, and bubble sort, while reinforcing everything from
 *    Project 1.
 *
 *  A NOTE ON TABLE ALIGNMENT WITH COLOR
 *    cout << color << setw(n) << value << RESET works correctly
 *    ONLY when color/RESET are separate stream insertions from the
 *    setw()-controlled value (setw applies to exactly one following
 *    insertion). The one place this can silently break is if a
 *    color code is concatenated INTO the same string that's then
 *    passed to setw() as a whole — because setw() would then count
 *    the invisible escape codes as visible width. This program
 *    avoids that specific pattern everywhere; colored cells either
 *    use separate insertions, or (for extra clarity) pre-pad plain
 *    text with padRight() before wrapping it in color.
 *
 *  ROBUSTNESS
 *    - All input is read with getline() and manually validated
 *      character-by-character before use — never mixed with cin >>.
 *    - EOF-safe: the program exits cleanly instead of looping
 *      forever if the input stream closes unexpectedly.
 *    - Numeric conversions (stoi/stod) are guarded by both a
 *      maximum input length AND a try/catch block, so no possible
 *      user input can ever throw an unhandled exception and crash
 *      the program.
 *    - File loading validates every field's value range (not just
 *      that it parsed as a number) before accepting the file,
 *      protecting against a manually corrupted data file.
 *
 *  COMPILE
 *      g++ -std=c++17 -O2 -Wall -Wextra -o student_system student_system.cpp
 *  RUN
 *      ./student_system      (Linux/Mac)
 *      student_system.exe    (Windows)
 *      or just click "Run Code" in VS Code
 *****************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cctype>
#include <ctime>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
#endif

using namespace std;

// ============================================================
//  CONSTANTS
// ============================================================

const string RESET   = "\033[0m";
const string BOLD    = "\033[1m";
const string DIM     = "\033[2m";
const string RED     = "\033[31m";
const string GREEN   = "\033[32m";
const string YELLOW  = "\033[33m";
const string BLUE    = "\033[34m";
const string MAGENTA = "\033[35m";
const string CYAN    = "\033[36m";
const string WHITE   = "\033[37m";

const string BRIGHT_RED     = "\033[91m";
const string BRIGHT_GREEN   = "\033[92m";
const string BRIGHT_YELLOW  = "\033[93m";
const string BRIGHT_BLUE    = "\033[94m";
const string BRIGHT_MAGENTA = "\033[95m";
const string BRIGHT_CYAN    = "\033[96m";

const int    MAX_STUDENTS    = 50;
const int    NUM_SUBJECTS    = 3;
const string SUBJECT_NAMES[NUM_SUBJECTS] = {"Math", "Physics", "English"};
const double PASS_MARK       = 50.0;
const int    MAX_NUMBER_LEN  = 12;   // Guards against absurdly long numeric input

const string DATA_FILE    = "students.txt";
const string REPORT_FILE  = "report.txt";
const int    BANNER_WIDTH = 64;

// Theme colors — one per major section, used consistently throughout
const string THEME_MAIN    = CYAN;
const string THEME_MANAGE  = BLUE;
const string THEME_REPORTS = MAGENTA;
const string THEME_SORT    = YELLOW;
const string THEME_FILE    = GREEN;

// ============================================================
//  STRUCT — groups all data for one student together
// ============================================================

struct Student {
    int    id;                    // Permanent unique ID, never reused
    string name;
    int    rollNumber;            // User-assigned, must be unique
    double marks[NUM_SUBJECTS];   // Math, Physics, English
    double average;               // Derived: recomputed via recomputeDerived()
    char   grade;                 // Derived: A/B/C/D/F
    bool   passed;                // Derived: average >= PASS_MARK
};

// ============================================================
//  GLOBAL STATE
// ============================================================

Student students[MAX_STUDENTS];
int  studentCount   = 0;
int  nextId         = 1;
bool unsavedChanges = false;

// ============================================================
//  FUNCTION PROTOTYPES
// ============================================================

// -- Display / UI helpers --
void   enableAnsiSupport();
void   clearScreen();
void   pauseScreen();
string centerText(const string& text, int width);
void   printBanner();
void   printDivider(int width, const string& color = CYAN);
void   printMenuHeader(const string& title, const string& color);
void   printMenuOption(const string& number, const string& label, const string& color);
void   printError(const string& msg);
void   printSuccess(const string& msg);
void   printInfo(const string& msg);
void   printWarning(const string& msg);
void   printDashboard();
string choicePrompt(const string& text);
string labelPrompt(const string& text);

// -- Animation --
void sleepMillis(int ms);
void showStartupSequence();

// -- String utilities --
string trim(const string& s);
string toUpperStr(const string& s);
string toLowerStr(const string& s);
bool   isAllDigits(const string& s);
bool   isValidDecimalNumber(const string& s);
bool   isValidNameChars(const string& s);
bool   hasAtLeastOneLetter(const string& s);
string padRight(const string& text, int width);
string formatFixed(double value, int precision);

// -- Input validation --
string readLine(const string& prompt);
int    getValidatedInt(const string& prompt, int minVal, int maxVal, bool allowQuit, bool& quitFlag);
double getValidatedDouble(const string& prompt, double minVal, double maxVal);
string getValidatedName(const string& prompt);
bool   askYesNo(const string& prompt);

// -- Struct / data helpers --
char   assignGrade(double average);
string gradeColor(char grade);
void   recomputeDerived(Student& s);
int    findIndexById(int id);
int    findIndexByRoll(int rollNumber);
bool   isRollNumberTaken(int rollNumber, int excludeIndex);
double computeClassAverage();
string buildRatioBar(int part, int total, int barWidth, const string& color);
void   printStudentCard(const Student& s);

// -- Core features --
void addStudent();
void editStudent();
void deleteStudent();
void searchByRoll();
void searchByName();
void viewAllStudents();
void showClassStatistics();
void showClassRanking();
void showSubjectWiseAverages();
void showGradeDistribution();
void sortByAverage(bool ascending);
void sortByName(bool ascending);

// -- File I/O --
bool parseDataFile(Student temp[], int& tempCount, int& tempNextId);
void autoLoadAtStartup();
void saveToFile();
void loadFromFile();
void exportReport();

// -- Menu flows --
void manageStudentsMenu();
void reportsMenu();
void sortMenu();
void fileMenu();

// ============================================================
//  MAIN
// ============================================================

int main() {
    enableAnsiSupport();
    showStartupSequence();
    autoLoadAtStartup();

    bool running = true;

    while (running) {
        clearScreen();
        printBanner();
        printDashboard();

        printMenuHeader("MAIN MENU", THEME_MAIN);
        printMenuOption("1", "Manage Students   (Add / Edit / Delete / Search)", THEME_MANAGE);
        printMenuOption("2", "View & Reports    (Table / Stats / Ranking / Analytics)", THEME_REPORTS);
        printMenuOption("3", "Sort Students", THEME_SORT);
        printMenuOption("4", "File Operations   (Save / Load / Export Report)", THEME_FILE);
        printMenuOption("5", "Exit", BRIGHT_RED);
        printDivider(BANNER_WIDTH, THEME_MAIN);

        bool dummy = false;
        int choice = getValidatedInt(choicePrompt("Enter your choice (1-5): "), 1, 5, false, dummy);

        if (choice == 1) {
            manageStudentsMenu();
        } else if (choice == 2) {
            reportsMenu();
        } else if (choice == 3) {
            sortMenu();
        } else if (choice == 4) {
            fileMenu();
        } else {
            if (unsavedChanges) {
                bool save = askYesNo("\nYou have unsaved changes. Save before exiting? (Y/N): ");
                if (save) saveToFile();
            }
            running = false;
        }
    }

    clearScreen();
    printBanner();
    cout << "\n" << BOLD << BRIGHT_GREEN << "Thank you for using the Student Record System!" << RESET << "\n";
    cout << "Total students on record: " << BOLD << studentCount << RESET << "\n\n";
    return 0;
}

// ============================================================
//  DISPLAY / UI HELPERS
// ============================================================

void enableAnsiSupport() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) return;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, mode);
#endif
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pauseScreen() {
    cout << BOLD << BRIGHT_YELLOW << "\nPress ENTER to continue..." << RESET;
    string dummy;
    if (!getline(cin, dummy)) {
        exit(0);
    }
}

string centerText(const string& text, int width) {
    if ((int)text.size() >= width) return text.substr(0, width);
    int totalPad = width - (int)text.size();
    int left = totalPad / 2;
    int right = totalPad - left;
    return string(left, ' ') + text + string(right, ' ');
}

void printBanner() {
    string border = "+" + string(BANNER_WIDTH, '-') + "+";
    string blank  = "|" + string(BANNER_WIDTH, ' ') + "|";

    cout << BOLD << CYAN << border << RESET << "\n";
    cout << BOLD << CYAN << blank  << RESET << "\n";

    cout << BOLD << CYAN << "|" << RESET
         << BOLD << BRIGHT_YELLOW << centerText("CLOUDEXIFY INTERNSHIP 2026", BANNER_WIDTH) << RESET
         << BOLD << CYAN << "|" << RESET << "\n";

    cout << BOLD << CYAN << "|" << RESET
         << BOLD << BRIGHT_CYAN << centerText("STUDENT RECORD SYSTEM", BANNER_WIDTH) << RESET
         << BOLD << CYAN << "|" << RESET << "\n";

    cout << BOLD << CYAN << blank  << RESET << "\n";
    cout << BOLD << CYAN << border << RESET << "\n";
}

void printDivider(int width, const string& color) {
    cout << color << string(width, '-') << RESET << "\n";
}

void printMenuHeader(const string& title, const string& color) {
    cout << "\n" << BOLD << color << title << RESET << "\n";
    printDivider(BANNER_WIDTH, color);
}

void printMenuOption(const string& number, const string& label, const string& color) {
    cout << "  " << BOLD << color << number << "." << RESET << " " << label << "\n";
}

void printError(const string& msg) {
    cout << BOLD << BRIGHT_RED << "  [!] " << RESET << RED << msg << RESET << "\n";
}

void printSuccess(const string& msg) {
    cout << BOLD << BRIGHT_GREEN << "  [OK] " << RESET << GREEN << msg << RESET << "\n";
}

void printInfo(const string& msg) {
    cout << BOLD << BRIGHT_CYAN << "  [i] " << RESET << CYAN << msg << RESET << "\n";
}

void printWarning(const string& msg) {
    cout << BOLD << BRIGHT_YELLOW << "  [!] " << RESET << YELLOW << msg << RESET << "\n";
}

void printDashboard() {
    cout << "\n" << BOLD << BRIGHT_CYAN << "STATUS" << RESET << BOLD << "  ::  " << RESET;
    cout << BOLD << "Students: " << RESET << BRIGHT_YELLOW << studentCount << RESET
         << " / " << MAX_STUDENTS;

    if (studentCount > 0) {
        cout << "   " << BOLD << "Class Avg: " << RESET << BRIGHT_GREEN
             << formatFixed(computeClassAverage(), 1) << RESET;
    }

    cout << "   " << BOLD << "Unsaved: " << RESET
         << (unsavedChanges ? (BOLD + BRIGHT_RED + "YES" + RESET) : (BOLD + BRIGHT_GREEN + "NO" + RESET))
         << "\n";

    if (studentCount >= MAX_STUDENTS) {
        printWarning("Maximum capacity reached! Cannot add more students.");
    } else if (studentCount >= MAX_STUDENTS - 5) {
        printWarning("Approaching maximum capacity (" + to_string(studentCount) + "/" + to_string(MAX_STUDENTS) + ")");
    }
}

// Wraps a menu-choice prompt in a bold, bright, attention-grabbing color.
string choicePrompt(const string& text) {
    return BOLD + BRIGHT_YELLOW + text + RESET;
}

// Wraps a field-input label (e.g. "Full Name: ") in a consistent bold color.
string labelPrompt(const string& text) {
    return BOLD + BRIGHT_CYAN + text + RESET;
}

// ============================================================
//  ANIMATION
// ============================================================

void sleepMillis(int ms) {
    clock_t start = clock();
    while (((double)(clock() - start) / CLOCKS_PER_SEC) * 1000.0 < (double)ms) {
        // Busy-wait for a short, precise delay (keeps compilation dependency-free).
    }
}

void showStartupSequence() {
    clearScreen();
    cout << BOLD << BRIGHT_CYAN;
    cout << "\n";
    cout << "              ______________\n";
    cout << "             /             /|\n";
    cout << "            /____________ / |\n";
    cout << "           |  " << BRIGHT_YELLOW << "STUDENT" << BRIGHT_CYAN << "    |  |\n";
    cout << "           |  " << BRIGHT_YELLOW << "RECORDS" << BRIGHT_CYAN << "    | /\n";
    cout << "           |_____________|/\n";
    cout << RESET << "\n";

    cout << BOLD << BRIGHT_MAGENTA << centerText("CLOUDEXIFY STUDENT RECORD SYSTEM", BANNER_WIDTH) << RESET << "\n\n";

    cout << BOLD << "  Initializing database engine" << RESET << flush;
    for (int i = 0; i < 3; i++) {
        sleepMillis(200);
        cout << BOLD << "." << RESET << flush;
    }
    cout << "\n\n";

    const int barWidth = 24;
    for (int i = 0; i <= barWidth; i++) {
        int percent = (i * 100) / barWidth;
        cout << "\r  [" << BOLD << BRIGHT_GREEN << string(i, '#') << RESET
             << string(barWidth - i, '-') << "] " << BOLD << percent << "%  " << RESET << flush;
        sleepMillis(15);
    }
    cout << "\n\n";
    sleepMillis(250);
}

// ============================================================
//  STRING UTILITIES
// ============================================================

string trim(const string& s) {
    size_t start = 0;
    while (start < s.size() && isspace((unsigned char)s[start])) start++;
    if (start == s.size()) return "";
    size_t end = s.size() - 1;
    while (end > start && isspace((unsigned char)s[end])) end--;
    return s.substr(start, end - start + 1);
}

string toUpperStr(const string& s) {
    string result = s;
    for (char& c : result) c = (char)toupper((unsigned char)c);
    return result;
}

string toLowerStr(const string& s) {
    string result = s;
    for (char& c : result) c = (char)tolower((unsigned char)c);
    return result;
}

bool isAllDigits(const string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!isdigit((unsigned char)c)) return false;
    }
    return true;
}

// Allows an optional single decimal point, e.g. "85", "85.5", "85."
bool isValidDecimalNumber(const string& s) {
    if (s.empty()) return false;
    int dotCount = 0;
    int digitCount = 0;
    for (char c : s) {
        if (c == '.') {
            dotCount++;
            if (dotCount > 1) return false;
        } else if (isdigit((unsigned char)c)) {
            digitCount++;
        } else {
            return false;
        }
    }
    return digitCount > 0;
}

bool isValidNameChars(const string& s) {
    for (char c : s) {
        if (!isalpha((unsigned char)c) && c != ' ' && c != '\'' && c != '-' && c != '.') {
            return false;
        }
    }
    return true;
}

bool hasAtLeastOneLetter(const string& s) {
    for (char c : s) {
        if (isalpha((unsigned char)c)) return true;
    }
    return false;
}

// Pads plain text to an exact width BEFORE any color codes are added.
string padRight(const string& text, int width) {
    if ((int)text.size() >= width) return text.substr(0, width);
    return text + string(width - text.size(), ' ');
}

// Formats a double to a fixed number of decimal places as a clean string.
string formatFixed(double value, int precision) {
    ostringstream oss;
    oss << fixed << setprecision(precision) << value;
    return oss.str();
}

// ============================================================
//  INPUT VALIDATION
// ============================================================

string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) {
        cout << "\n" << BOLD << BRIGHT_RED << "Input stream closed unexpectedly. Exiting." << RESET << "\n";
        exit(0);
    }
    return line;
}

// Converts a validated digit string to int. Guarded by BOTH a length
// check and a try/catch, so no possible input can ever throw an
// unhandled exception, even if the digit-length guard were ever
// changed or bypassed in the future.
int getValidatedInt(const string& prompt, int minVal, int maxVal, bool allowQuit, bool& quitFlag) {
    quitFlag = false;
    while (true) {
        string trimmed = trim(readLine(prompt));

        if (allowQuit && trimmed.size() == 1 && (trimmed[0] == 'q' || trimmed[0] == 'Q')) {
            quitFlag = true;
            return 0;
        }

        if (!isAllDigits(trimmed) || (int)trimmed.size() > MAX_NUMBER_LEN) {
            printError("Please enter a valid whole number" + string(allowQuit ? " (or 'Q' to cancel)." : "."));
            continue;
        }

        int value;
        try {
            value = stoi(trimmed);
        } catch (...) {
            printError("That number is invalid or too large. Please try again.");
            continue;
        }

        if (value < minVal || value > maxVal) {
            printError("Please enter a number between " + to_string(minVal) + " and " + to_string(maxVal) + ".");
            continue;
        }

        return value;
    }
}

// Converts a validated decimal string to double. Same defense-in-depth
// approach as getValidatedInt: length guard AND try/catch together.
double getValidatedDouble(const string& prompt, double minVal, double maxVal) {
    while (true) {
        string trimmed = trim(readLine(prompt));

        if ((int)trimmed.size() > MAX_NUMBER_LEN || !isValidDecimalNumber(trimmed)) {
            printError("Please enter a valid number (e.g., 85 or 85.5).");
            continue;
        }

        double value;
        try {
            value = stod(trimmed);
        } catch (...) {
            printError("That number is invalid or too large. Please try again.");
            continue;
        }

        if (value < minVal || value > maxVal) {
            printError("Please enter a value between " + to_string((int)minVal) +
                        " and " + to_string((int)maxVal) + ".");
            continue;
        }

        return value;
    }
}

string getValidatedName(const string& prompt) {
    while (true) {
        string trimmed = trim(readLine(prompt));

        if (trimmed.empty() || trimmed.size() > 40) {
            printError("Name must be 1-40 characters long.");
            continue;
        }
        if (!isValidNameChars(trimmed)) {
            printError("Name can only contain letters, spaces, apostrophes, hyphens, or periods.");
            continue;
        }
        if (!hasAtLeastOneLetter(trimmed)) {
            printError("Name must contain at least one letter.");
            continue;
        }
        return trimmed;
    }
}

bool askYesNo(const string& prompt) {
    while (true) {
        string trimmed = toUpperStr(trim(readLine(prompt)));

        if (trimmed == "Y" || trimmed == "YES") return true;
        if (trimmed == "N" || trimmed == "NO")  return false;

        printError("Please answer with Y or N.");
    }
}

// ============================================================
//  STRUCT / DATA HELPERS
// ============================================================

char assignGrade(double average) {
    if (average >= 90) return 'A';
    else if (average >= 75) return 'B';
    else if (average >= 60) return 'C';
    else if (average >= 50) return 'D';
    else return 'F';
}

string gradeColor(char grade) {
    if (grade == 'A') return BOLD + BRIGHT_GREEN;
    else if (grade == 'B') return BOLD + GREEN;
    else if (grade == 'C') return BOLD + YELLOW;
    else if (grade == 'D') return BOLD + BRIGHT_YELLOW;
    else return BOLD + BRIGHT_RED;
}

// Recalculates average, grade, and pass/fail status from raw marks.
void recomputeDerived(Student& s) {
    double total = 0;
    for (int j = 0; j < NUM_SUBJECTS; j++) {
        total += s.marks[j];
    }
    s.average = total / NUM_SUBJECTS;
    s.grade = assignGrade(s.average);
    s.passed = (s.average >= PASS_MARK);
}

int findIndexById(int id) {
    for (int i = 0; i < studentCount; i++) {
        if (students[i].id == id) return i;
    }
    return -1;
}

int findIndexByRoll(int rollNumber) {
    for (int i = 0; i < studentCount; i++) {
        if (students[i].rollNumber == rollNumber) return i;
    }
    return -1;
}

bool isRollNumberTaken(int rollNumber, int excludeIndex) {
    for (int i = 0; i < studentCount; i++) {
        if (i != excludeIndex && students[i].rollNumber == rollNumber) return true;
    }
    return false;
}

double computeClassAverage() {
    if (studentCount == 0) return 0.0;
    double total = 0;
    for (int i = 0; i < studentCount; i++) {
        total += students[i].average;
    }
    return total / studentCount;
}

string buildRatioBar(int part, int total, int barWidth, const string& color) {
    if (total <= 0) return "[" + string(barWidth, '-') + "]";
    int filled = (int)(((double)part / (double)total) * barWidth + 0.5);
    if (filled > barWidth) filled = barWidth;
    if (filled < 0) filled = 0;
    return "[" + color + string(filled, '#') + RESET + string(barWidth - filled, '-') + "]";
}

void printStudentCard(const Student& s) {
    cout << "\n";
    printDivider(50, THEME_MANAGE);
    cout << BOLD << THEME_MANAGE << "STUDENT RECORD" << RESET << "\n";
    printDivider(50, THEME_MANAGE);
    cout << BOLD << "ID          : " << RESET << s.id << "\n";
    cout << BOLD << "Name        : " << RESET << s.name << "\n";
    cout << BOLD << "Roll Number : " << RESET << s.rollNumber << "\n";
    for (int j = 0; j < NUM_SUBJECTS; j++) {
        cout << BOLD << SUBJECT_NAMES[j] << " Marks : " << RESET << formatFixed(s.marks[j], 1) << "\n";
    }
    cout << BOLD << "Average     : " << RESET << formatFixed(s.average, 1) << "\n";
    cout << BOLD << "Grade       : " << RESET << gradeColor(s.grade) << s.grade << RESET << "\n";
    cout << BOLD << "Status      : " << RESET
         << (s.passed ? (BOLD + BRIGHT_GREEN + "PASS" + RESET) : (BOLD + BRIGHT_RED + "FAIL" + RESET)) << "\n";
    printDivider(50, THEME_MANAGE);
}

// ============================================================
//  CORE FEATURES
// ============================================================

void addStudent() {
    if (studentCount >= MAX_STUDENTS) {
        printError("Maximum capacity reached! Cannot add more students.");
        return;
    }

    cout << "\n" << BOLD << THEME_MANAGE << "ADD NEW STUDENT" << RESET << "\n";
    printDivider(40, THEME_MANAGE);

    string name = getValidatedName(labelPrompt("Full Name: "));

    int rollNumber;
    bool dummy = false;
    while (true) {
        rollNumber = getValidatedInt(labelPrompt("Roll Number (positive integer): "), 1, 999999, false, dummy);
        if (isRollNumberTaken(rollNumber, -1)) {
            printError("That roll number is already in use. Please choose a different one.");
            continue;
        }
        break;
    }

    Student s;
    s.id = nextId;
    s.name = name;
    s.rollNumber = rollNumber;

    for (int j = 0; j < NUM_SUBJECTS; j++) {
        s.marks[j] = getValidatedDouble(labelPrompt(SUBJECT_NAMES[j] + " marks (0-100): "), 0.0, 100.0);
    }

    recomputeDerived(s);

    students[studentCount] = s;
    studentCount++;
    nextId++;
    unsavedChanges = true;

    cout << "\n";
    printSuccess("Student added! ID: " + to_string(s.id) +
                  ", Average: " + formatFixed(s.average, 1) +
                  ", Grade: " + string(1, s.grade));
}

void editStudent() {
    if (studentCount == 0) {
        printInfo("No students to edit.");
        return;
    }

    bool dummy = false;
    int id = getValidatedInt(labelPrompt("Enter Student ID to edit (or 0 to cancel): "), 0, 999999999, false, dummy);
    if (id == 0) {
        printInfo("Edit cancelled.");
        return;
    }

    int idx = findIndexById(id);
    if (idx == -1) {
        printError("No student found with ID " + to_string(id) + ".");
        return;
    }

    cout << "\n" << BOLD << "Current details:" << RESET << "\n";
    cout << "  Name: " << students[idx].name << "\n  Roll: " << students[idx].rollNumber << "\n";
    for (int j = 0; j < NUM_SUBJECTS; j++) {
        cout << "  " << SUBJECT_NAMES[j] << ": " << formatFixed(students[idx].marks[j], 1) << "\n";
    }

    cout << "\n" << BOLD << THEME_MANAGE << "What would you like to edit?" << RESET << "\n";
    printMenuOption("1", "Name", THEME_MANAGE);
    printMenuOption("2", "Roll Number", THEME_MANAGE);
    printMenuOption("3", "Marks", THEME_MANAGE);
    printMenuOption("4", "Everything", THEME_MANAGE);
    printMenuOption("5", "Cancel", CYAN);
    int choice = getValidatedInt(choicePrompt("Enter choice (1-5): "), 1, 5, false, dummy);

    if (choice == 5) {
        printInfo("Edit cancelled.");
        return;
    }

    if (choice == 1 || choice == 4) {
        students[idx].name = getValidatedName(labelPrompt("New Name: "));
    }

    if (choice == 2 || choice == 4) {
        int newRoll;
        while (true) {
            newRoll = getValidatedInt(labelPrompt("New Roll Number: "), 1, 999999, false, dummy);
            if (isRollNumberTaken(newRoll, idx)) {
                printError("That roll number is already used by another student.");
                continue;
            }
            break;
        }
        students[idx].rollNumber = newRoll;
    }

    if (choice == 3 || choice == 4) {
        for (int j = 0; j < NUM_SUBJECTS; j++) {
            students[idx].marks[j] = getValidatedDouble(labelPrompt("New " + SUBJECT_NAMES[j] + " marks (0-100): "), 0.0, 100.0);
        }
        recomputeDerived(students[idx]);
    }

    unsavedChanges = true;
    printSuccess("Student updated successfully.");
}

void deleteStudent() {
    if (studentCount == 0) {
        printInfo("No students to delete.");
        return;
    }

    bool dummy = false;
    int id = getValidatedInt(labelPrompt("Enter Student ID to delete (or 0 to cancel): "), 0, 999999999, false, dummy);
    if (id == 0) {
        printInfo("Delete cancelled.");
        return;
    }

    int idx = findIndexById(id);
    if (idx == -1) {
        printError("No student found with ID " + to_string(id) + ".");
        return;
    }

    cout << "\n" << BOLD << "Student to delete:" << RESET << "\n";
    cout << "  ID: " << students[idx].id << "\n";
    cout << "  Name: " << students[idx].name << "\n";
    cout << "  Roll: " << students[idx].rollNumber << "\n";
    cout << "  Average: " << formatFixed(students[idx].average, 1)
         << "  Grade: " << gradeColor(students[idx].grade) << students[idx].grade << RESET << "\n";

    bool confirm = askYesNo(BOLD + BRIGHT_YELLOW + "\nAre you sure you want to delete this student? (Y/N): " + RESET);
    if (!confirm) {
        printInfo("Delete cancelled.");
        return;
    }

    for (int i = idx; i < studentCount - 1; i++) {
        students[i] = students[i + 1];
    }
    studentCount--;
    unsavedChanges = true;

    printSuccess("Student deleted successfully.");
}

void searchByRoll() {
    if (studentCount == 0) {
        printInfo("No students yet.");
        return;
    }

    bool dummy = false;
    int roll = getValidatedInt(labelPrompt("Enter Roll Number to search: "), 1, 999999, false, dummy);
    int idx = findIndexByRoll(roll);

    if (idx == -1) {
        printError("No student found with roll number " + to_string(roll) + ".");
        return;
    }

    printStudentCard(students[idx]);
}

void searchByName() {
    if (studentCount == 0) {
        printInfo("No students yet.");
        return;
    }

    string query = trim(readLine(labelPrompt("Enter name (or part of name) to search: ")));
    if (query.empty()) {
        printError("Search query cannot be empty.");
        return;
    }

    string queryLower = toLowerStr(query);
    int matches = 0;

    cout << "\n" << BOLD << THEME_MANAGE << "Search Results:" << RESET << "\n";

    for (int i = 0; i < studentCount; i++) {
        if (toLowerStr(students[i].name).find(queryLower) != string::npos) {
            matches++;
            printStudentCard(students[i]);
        }
    }

    if (matches == 0) {
        printInfo("No students matched \"" + query + "\".");
    } else {
        cout << "\n" << BOLD << matches << RESET << " match(es) found.\n";
    }
}

void viewAllStudents() {
    if (studentCount == 0) {
        printInfo("No students yet! Add some first.");
        return;
    }

    int topIdx = 0;
    for (int i = 0; i < studentCount; i++) {
        if (students[i].average > students[topIdx].average) topIdx = i;
    }

    const int tableWidth = 90;

    cout << "\n";
    printDivider(tableWidth, THEME_REPORTS);

    cout << BOLD << BRIGHT_CYAN;
    cout << left << setw(5) << "ID" << setw(20) << "Name" << setw(8) << "Roll";
    for (int j = 0; j < NUM_SUBJECTS; j++) {
        cout << setw(9) << SUBJECT_NAMES[j];
    }
    cout << setw(8) << "Avg" << setw(7) << "Grade" << setw(8) << "Status" << RESET << "\n";
    printDivider(tableWidth, THEME_REPORTS);

    for (int i = 0; i < studentCount; i++) {
        const Student& s = students[i];

        // Subtle zebra striping: dim every other row for readability.
        // Safe for alignment because DIM/RESET are separate insertions
        // from the setw()-controlled fields, not concatenated into them.
        string rowStyle = (i % 2 == 1) ? DIM : "";

        cout << rowStyle;
        cout << left << setw(5) << s.id << setw(20) << s.name << setw(8) << s.rollNumber;
        for (int j = 0; j < NUM_SUBJECTS; j++) {
            cout << fixed << setprecision(1) << left << setw(9) << s.marks[j];
        }
        cout << fixed << setprecision(1) << left << setw(8) << s.average << RESET;

        string gradeStr(1, s.grade);
        cout << gradeColor(s.grade) << padRight(gradeStr, 7) << RESET;

        string statusStr = s.passed ? "Pass" : "Fail";
        string statusColor = s.passed ? (BOLD + BRIGHT_GREEN) : (BOLD + BRIGHT_RED);
        cout << statusColor << padRight(statusStr, 8) << RESET;

        if (i == topIdx) {
            cout << "  " << BOLD << BRIGHT_YELLOW << "<< TOPPER" << RESET;
        }
        cout << "\n";
    }

    printDivider(tableWidth, THEME_REPORTS);
    cout << BOLD << "Total Students: " << RESET << studentCount << " / " << MAX_STUDENTS << "\n";
}

void showClassStatistics() {
    if (studentCount == 0) {
        printInfo("No students yet! Add some first.");
        return;
    }

    double classTotal = 0;
    int topIdx = 0, bottomIdx = 0;

    for (int i = 0; i < studentCount; i++) {
        classTotal += students[i].average;
        if (students[i].average > students[topIdx].average) topIdx = i;
        if (students[i].average < students[bottomIdx].average) bottomIdx = i;
    }

    double classAvg = classTotal / studentCount;

    int passedCount = 0;
    for (int i = 0; i < studentCount; i++) {
        if (students[i].passed) passedCount++;
    }
    int failedCount = studentCount - passedCount;

    cout << "\n" << BOLD << THEME_REPORTS << "CLASS STATISTICS" << RESET << "\n";
    printDivider(50, THEME_REPORTS);
    cout << BOLD << "Total Students   : " << RESET << studentCount << "\n";
    cout << BOLD << "Class Average    : " << RESET << formatFixed(classAvg, 1) << "\n";
    cout << BOLD << "Passed           : " << RESET << BOLD << BRIGHT_GREEN << passedCount << RESET
         << "   " << buildRatioBar(passedCount, studentCount, 20, BRIGHT_GREEN) << "\n";
    cout << BOLD << "Failed           : " << RESET << BOLD << BRIGHT_RED << failedCount << RESET
         << "   " << buildRatioBar(failedCount, studentCount, 20, BRIGHT_RED) << "\n";
    printDivider(50, THEME_REPORTS);

    cout << BOLD << BRIGHT_YELLOW << "CLASS TOPPER: " << RESET
         << BOLD << students[topIdx].name << RESET << " (Roll " << students[topIdx].rollNumber
         << ") - Average: " << formatFixed(students[topIdx].average, 1)
         << ", Grade: " << gradeColor(students[topIdx].grade) << students[topIdx].grade << RESET << "\n";

    if (studentCount > 1) {
        cout << BOLD << BRIGHT_CYAN << "Needs Improvement: " << RESET
             << students[bottomIdx].name << " (Roll " << students[bottomIdx].rollNumber
             << ") - Average: " << formatFixed(students[bottomIdx].average, 1) << "\n";
    }
    printDivider(50, THEME_REPORTS);
}

// Non-destructive: sorts a TEMPORARY COPY for display only.
void showClassRanking() {
    if (studentCount == 0) {
        printInfo("No students yet! Add some first.");
        return;
    }

    Student ranked[MAX_STUDENTS];
    int n = studentCount;
    for (int i = 0; i < n; i++) ranked[i] = students[i];

    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (ranked[j].average < ranked[j + 1].average) {
                Student temp = ranked[j];
                ranked[j] = ranked[j + 1];
                ranked[j + 1] = temp;
            }
        }
    }

    cout << "\n" << BOLD << THEME_REPORTS << "CLASS RANKING (by Average, Highest First)" << RESET << "\n";
    printDivider(60, THEME_REPORTS);

    cout << BOLD << BRIGHT_CYAN;
    cout << left << setw(6) << "Rank" << setw(20) << "Name" << setw(8) << "Roll" << setw(8) << "Avg" << "Grade" << RESET << "\n";
    printDivider(60, THEME_REPORTS);

    for (int i = 0; i < n; i++) {
        string rowStyle = (i % 2 == 1) ? DIM : "";
        cout << rowStyle;
        cout << left << setw(6) << (i + 1) << setw(20) << ranked[i].name << setw(8) << ranked[i].rollNumber
             << fixed << setprecision(1) << setw(8) << ranked[i].average << RESET;
        cout << gradeColor(ranked[i].grade) << padRight(string(1, ranked[i].grade), 6) << RESET;
        if (i == 0) cout << "  " << BOLD << BRIGHT_YELLOW << "#1" << RESET;
        cout << "\n";
    }

    printDivider(60, THEME_REPORTS);
    printInfo("This is a read-only ranking view — storage order is unchanged.");
    cout << "      Use the 'Sort Students' menu to permanently reorder records.\n";
}

void showSubjectWiseAverages() {
    if (studentCount == 0) {
        printInfo("No students yet! Add some first.");
        return;
    }

    cout << "\n" << BOLD << THEME_REPORTS << "SUBJECT-WISE CLASS AVERAGES" << RESET << "\n";
    printDivider(60, THEME_REPORTS);

    cout << BOLD << BRIGHT_CYAN;
    cout << left << setw(12) << "Subject" << setw(15) << "Class Avg" << setw(20) << "Top Scorer" << "Score" << RESET << "\n";
    printDivider(60, THEME_REPORTS);

    for (int j = 0; j < NUM_SUBJECTS; j++) {
        double total = 0;
        int topIdx = 0;

        for (int i = 0; i < studentCount; i++) {
            total += students[i].marks[j];
            if (students[i].marks[j] > students[topIdx].marks[j]) topIdx = i;
        }

        double subjAvg = total / studentCount;

        cout << left << setw(12) << SUBJECT_NAMES[j]
             << setw(15) << formatFixed(subjAvg, 1)
             << setw(20) << students[topIdx].name
             << BOLD << BRIGHT_GREEN << formatFixed(students[topIdx].marks[j], 1) << RESET << "\n";
    }
    printDivider(60, THEME_REPORTS);
}

void showGradeDistribution() {
    if (studentCount == 0) {
        printInfo("No students yet! Add some first.");
        return;
    }

    int counts[5] = {0, 0, 0, 0, 0}; // A, B, C, D, F

    for (int i = 0; i < studentCount; i++) {
        if (students[i].grade == 'A') counts[0]++;
        else if (students[i].grade == 'B') counts[1]++;
        else if (students[i].grade == 'C') counts[2]++;
        else if (students[i].grade == 'D') counts[3]++;
        else counts[4]++;
    }

    string letters[5] = {"A", "B", "C", "D", "F"};
    string colors[5]  = {BRIGHT_GREEN, GREEN, YELLOW, BRIGHT_YELLOW, BRIGHT_RED};

    cout << "\n" << BOLD << THEME_REPORTS << "GRADE DISTRIBUTION" << RESET << "\n";
    printDivider(60, THEME_REPORTS);

    for (int g = 0; g < 5; g++) {
        cout << BOLD << colors[g] << "Grade " << letters[g] << RESET << " : "
             << buildRatioBar(counts[g], studentCount, 25, colors[g])
             << "  " << BOLD << counts[g] << RESET << " student(s)\n";
    }
    printDivider(60, THEME_REPORTS);
}

void sortByAverage(bool ascending) {
    if (studentCount < 2) {
        printInfo("Need at least 2 students to sort.");
        return;
    }

    for (int i = 0; i < studentCount - 1; i++) {
        for (int j = 0; j < studentCount - i - 1; j++) {
            bool shouldSwap = ascending ? (students[j].average > students[j + 1].average)
                                        : (students[j].average < students[j + 1].average);
            if (shouldSwap) {
                Student temp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = temp;
            }
        }
    }

    unsavedChanges = true;
    printSuccess(string("Students sorted by average (") + (ascending ? "ascending" : "descending") + ").");
}

void sortByName(bool ascending) {
    if (studentCount < 2) {
        printInfo("Need at least 2 students to sort.");
        return;
    }

    for (int i = 0; i < studentCount - 1; i++) {
        for (int j = 0; j < studentCount - i - 1; j++) {
            string nameA = toLowerStr(students[j].name);
            string nameB = toLowerStr(students[j + 1].name);
            bool shouldSwap = ascending ? (nameA > nameB) : (nameA < nameB);
            if (shouldSwap) {
                Student temp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = temp;
            }
        }
    }

    unsavedChanges = true;
    printSuccess(string("Students sorted by name (") + (ascending ? "A-Z" : "Z-A") + ").");
}

// ============================================================
//  FILE I/O
//
//  FILE FORMAT (students.txt):
//    Line 1: total student count
//    Line 2: next available ID
//    Then, repeated per student:
//      <id>
//      <full name>
//      <roll number>
//      <mark0> <mark1> <mark2>
// ============================================================

// Parses the data file into a temporary buffer. Returns false if the
// file is missing, malformed, OR contains any value outside a sane
// range (negative IDs/rolls, marks outside 0-100, empty names) —
// this means a broken/tampered file can NEVER corrupt data already
// safely in memory, and can never load obviously-invalid records.
bool parseDataFile(Student temp[], int& tempCount, int& tempNextId) {
    ifstream file(DATA_FILE);
    if (!file.is_open()) return false;

    if (!(file >> tempCount)) return false;
    file.ignore();

    if (!(file >> tempNextId)) return false;
    file.ignore();

    if (tempCount < 0 || tempCount > MAX_STUDENTS) return false;
    if (tempNextId < 1) return false;

    for (int i = 0; i < tempCount; i++) {
        if (!(file >> temp[i].id)) return false;
        file.ignore();
        if (temp[i].id <= 0) return false;

        if (!getline(file, temp[i].name)) return false;
        temp[i].name = trim(temp[i].name);
        if (temp[i].name.empty()) return false;

        if (!(file >> temp[i].rollNumber)) return false;
        if (temp[i].rollNumber <= 0) return false;

        for (int j = 0; j < NUM_SUBJECTS; j++) {
            if (!(file >> temp[i].marks[j])) return false;
            if (temp[i].marks[j] < 0.0 || temp[i].marks[j] > 100.0) return false;
        }
        file.ignore();

        recomputeDerived(temp[i]);
    }

    file.close();
    return true;
}

void autoLoadAtStartup() {
    Student temp[MAX_STUDENTS];
    int tempCount = 0, tempNextId = 1;

    if (parseDataFile(temp, tempCount, tempNextId)) {
        for (int i = 0; i < tempCount; i++) students[i] = temp[i];
        studentCount = tempCount;
        nextId = tempNextId;
        unsavedChanges = false;
    }
    // If parsing fails (no file yet, or first run), silently start empty.
}

void saveToFile() {
    ofstream file(DATA_FILE);
    if (!file.is_open()) {
        printError("Could not open file for saving!");
        return;
    }

    file << studentCount << "\n" << nextId << "\n";

    for (int i = 0; i < studentCount; i++) {
        const Student& s = students[i];
        file << s.id << "\n" << s.name << "\n" << s.rollNumber << "\n";
        for (int j = 0; j < NUM_SUBJECTS; j++) {
            file << s.marks[j] << (j < NUM_SUBJECTS - 1 ? ' ' : '\n');
        }
    }

    file.close();
    unsavedChanges = false;
    printSuccess("Saved " + to_string(studentCount) + " student(s) to " + DATA_FILE + ".");
}

void loadFromFile() {
    if (unsavedChanges) {
        bool proceed = askYesNo(BOLD + BRIGHT_YELLOW +
            "You have unsaved changes that will be lost. Continue loading? (Y/N): " + RESET);
        if (!proceed) {
            printInfo("Load cancelled.");
            return;
        }
    }

    Student temp[MAX_STUDENTS];
    int tempCount = 0, tempNextId = 1;

    if (!parseDataFile(temp, tempCount, tempNextId)) {
        printError("Could not load data (file missing or corrupted). Your current data is unchanged.");
        return;
    }

    for (int i = 0; i < tempCount; i++) students[i] = temp[i];
    studentCount = tempCount;
    nextId = tempNextId;
    unsavedChanges = false;

    printSuccess("Loaded " + to_string(studentCount) + " student(s) from file.");
}

void exportReport() {
    if (studentCount == 0) {
        printInfo("No data to export yet.");
        return;
    }

    ofstream file(REPORT_FILE);
    if (!file.is_open()) {
        printError("Could not create report file.");
        return;
    }

    time_t now = time(nullptr);

    file << "==================================================\n";
    file << "        STUDENT RECORD SYSTEM - CLASS REPORT       \n";
    file << "==================================================\n";
    file << "Generated: " << ctime(&now);
    file << "Total Students: " << studentCount << "\n\n";

    file << left << setw(5) << "ID" << setw(20) << "Name" << setw(8) << "Roll";
    for (int j = 0; j < NUM_SUBJECTS; j++) file << setw(10) << SUBJECT_NAMES[j];
    file << setw(8) << "Avg" << setw(6) << "Grade" << setw(8) << "Status" << "\n";
    file << string(80, '-') << "\n";

    double classTotal = 0;
    int topIdx = 0;
    int passedCount = 0;

    for (int i = 0; i < studentCount; i++) {
        const Student& s = students[i];
        file << left << setw(5) << s.id << setw(20) << s.name << setw(8) << s.rollNumber;
        for (int j = 0; j < NUM_SUBJECTS; j++) {
            file << fixed << setprecision(1) << setw(10) << s.marks[j];
        }
        file << fixed << setprecision(1) << setw(8) << s.average
             << setw(6) << string(1, s.grade)
             << setw(8) << (s.passed ? "Pass" : "Fail") << "\n";

        classTotal += s.average;
        if (s.average > students[topIdx].average) topIdx = i;
        if (s.passed) passedCount++;
    }

    file << string(80, '-') << "\n\n";

    double classAvg = classTotal / studentCount;
    file << "Class Average: " << fixed << setprecision(1) << classAvg << "\n";
    file << "Passed: " << passedCount << " | Failed: " << (studentCount - passedCount) << "\n";
    file << "Class Topper: " << students[topIdx].name << " (Average: "
         << fixed << setprecision(1) << students[topIdx].average
         << ", Grade: " << students[topIdx].grade << ")\n\n";

    file << "Subject-wise Averages:\n";
    for (int j = 0; j < NUM_SUBJECTS; j++) {
        double total = 0;
        for (int i = 0; i < studentCount; i++) total += students[i].marks[j];
        file << "  " << SUBJECT_NAMES[j] << ": " << fixed << setprecision(1)
             << (total / studentCount) << "\n";
    }

    int gradeCounts[5] = {0, 0, 0, 0, 0};
    for (int i = 0; i < studentCount; i++) {
        if (students[i].grade == 'A') gradeCounts[0]++;
        else if (students[i].grade == 'B') gradeCounts[1]++;
        else if (students[i].grade == 'C') gradeCounts[2]++;
        else if (students[i].grade == 'D') gradeCounts[3]++;
        else gradeCounts[4]++;
    }

    string letters[5] = {"A", "B", "C", "D", "F"};
    file << "\nGrade Distribution:\n";
    for (int g = 0; g < 5; g++) {
        file << "  " << letters[g] << ": " << gradeCounts[g] << " student(s)\n";
    }

    file.close();
    printSuccess("Report exported to " + REPORT_FILE + ".");
}

// ============================================================
//  MENU FLOWS
// ============================================================

void manageStudentsMenu() {
    bool back = false;
    while (!back) {
        clearScreen();
        printBanner();
        printMenuHeader("MANAGE STUDENTS", THEME_MANAGE);
        printMenuOption("1", "Add Student", THEME_MANAGE);
        printMenuOption("2", "Edit Student", THEME_MANAGE);
        printMenuOption("3", "Delete Student", THEME_MANAGE);
        printMenuOption("4", "Search by Roll Number", THEME_MANAGE);
        printMenuOption("5", "Search by Name", THEME_MANAGE);
        printMenuOption("6", "Back to Main Menu", CYAN);
        printDivider(BANNER_WIDTH, THEME_MANAGE);

        bool dummy = false;
        int choice = getValidatedInt(choicePrompt("Enter your choice (1-6): "), 1, 6, false, dummy);

        if (choice == 1)      { addStudent();    pauseScreen(); }
        else if (choice == 2) { editStudent();   pauseScreen(); }
        else if (choice == 3) { deleteStudent(); pauseScreen(); }
        else if (choice == 4) { searchByRoll();  pauseScreen(); }
        else if (choice == 5) { searchByName();  pauseScreen(); }
        else back = true;
    }
}

void reportsMenu() {
    bool back = false;
    while (!back) {
        clearScreen();
        printBanner();
        printMenuHeader("VIEW & REPORTS", THEME_REPORTS);
        printMenuOption("1", "View All Students (Table)", THEME_REPORTS);
        printMenuOption("2", "Class Statistics & Topper", THEME_REPORTS);
        printMenuOption("3", "Class Ranking", THEME_REPORTS);
        printMenuOption("4", "Subject-wise Averages", THEME_REPORTS);
        printMenuOption("5", "Grade Distribution", THEME_REPORTS);
        printMenuOption("6", "Back to Main Menu", CYAN);
        printDivider(BANNER_WIDTH, THEME_REPORTS);

        bool dummy = false;
        int choice = getValidatedInt(choicePrompt("Enter your choice (1-6): "), 1, 6, false, dummy);

        if (choice == 1)      { viewAllStudents();         pauseScreen(); }
        else if (choice == 2) { showClassStatistics();     pauseScreen(); }
        else if (choice == 3) { showClassRanking();        pauseScreen(); }
        else if (choice == 4) { showSubjectWiseAverages(); pauseScreen(); }
        else if (choice == 5) { showGradeDistribution();   pauseScreen(); }
        else back = true;
    }
}

void sortMenu() {
    bool back = false;
    while (!back) {
        clearScreen();
        printBanner();
        printMenuHeader("SORT STUDENTS", THEME_SORT);
        printMenuOption("1", "Sort by Average (Ascending)", THEME_SORT);
        printMenuOption("2", "Sort by Average (Descending)", THEME_SORT);
        printMenuOption("3", "Sort by Name (A-Z)", THEME_SORT);
        printMenuOption("4", "Sort by Name (Z-A)", THEME_SORT);
        printMenuOption("5", "Back to Main Menu", CYAN);
        printDivider(BANNER_WIDTH, THEME_SORT);

        bool dummy = false;
        int choice = getValidatedInt(choicePrompt("Enter your choice (1-5): "), 1, 5, false, dummy);

        if (choice == 1)      { sortByAverage(true);  pauseScreen(); }
        else if (choice == 2) { sortByAverage(false); pauseScreen(); }
        else if (choice == 3) { sortByName(true);     pauseScreen(); }
        else if (choice == 4) { sortByName(false);    pauseScreen(); }
        else back = true;
    }
}

void fileMenu() {
    bool back = false;
    while (!back) {
        clearScreen();
        printBanner();
        printMenuHeader("FILE OPERATIONS", THEME_FILE);
        printMenuOption("1", "Save to File", THEME_FILE);
        printMenuOption("2", "Load from File", THEME_FILE);
        printMenuOption("3", "Export Formatted Report", THEME_FILE);
        printMenuOption("4", "Back to Main Menu", CYAN);
        printDivider(BANNER_WIDTH, THEME_FILE);

        bool dummy = false;
        int choice = getValidatedInt(choicePrompt("Enter your choice (1-4): "), 1, 4, false, dummy);

        if (choice == 1)      { saveToFile();    pauseScreen(); }
        else if (choice == 2) { loadFromFile();  pauseScreen(); }
        else if (choice == 3) { exportReport();  pauseScreen(); }
        else back = true;
    }
}