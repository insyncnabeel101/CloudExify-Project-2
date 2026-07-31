# CloudExify Internship 2026 — C++ Project 2
## Student Record System (Enhanced Edition + All Bonus Challenges)

**Name:** Muhammad Nabeel
**Registration Number:** CX-INT-2026-CPP-0093
**Internship:** CloudExify Internship 2026 — C++ Track, Month 1, Project 2

---

## Overview

A console-based student record management system built around a struct-based
array data model. A teacher can add students with marks in 3 subjects, view
records in a formatted table, edit or delete records, search by roll number
or name, sort by average or name, view class statistics and rankings, see
subject-wise averages and a grade distribution chart, and save/load
everything to/from a file — with an auto-loading, unsaved-changes-aware
workflow like a real data management tool.

---

## Core Features (Required)

- Add student with name, roll number, and 3 subject marks
- View all students in a clean, aligned formatted table
- Calculate and display each student's average
- Find and display the class topper
- Sort students by average grade (hand-written bubble sort)
- Save and load all records from a text file

---

## Bonus Features (All 5 Implemented)

| # | Bonus Challenge | Implementation |
|---|---|---|
| 1 | Letter grade (A/B/C/D/F) | Computed for every student, shown color-coded in every table and report |
| 2 | Sort by name alphabetically | Bubble sort, both A-Z and Z-A |
| 3 | Search student by roll number | Instant lookup with a full detail card |
| 4 | Delete student by ID | Confirmation prompt, safe array shifting, IDs never reused |
| 5 | Subject-wise class average | Plus the top scorer for each subject |

---

## Major Extras (Beyond Requirements)

| Feature | Description |
|---|---|
| Stable Student IDs | Auto-incrementing, never reused even after deletion — mirrors real database primary-key design |
| Edit Student | Update name, roll number, and/or marks for any existing student |
| Search by Name | Partial, case-insensitive matching across all records |
| Class Ranking (non-destructive) | A separate, read-only sorted view with rank numbers — doesn't touch the stored order, unlike the required Sort feature |
| Grade Distribution Report | ASCII bar chart visualizing how many students got each grade |
| "Needs Improvement" stat | Automatically highlights the lowest-performing student alongside the topper |
| Live Dashboard | Enrollment count, live class average, and an unsaved-changes indicator shown on every menu screen |
| Unsaved-changes tracking | Warns and offers to save before exiting if there's unsaved work |
| Auto-load on startup | Automatically restores your saved data when the program launches |
| Corruption-safe loading | Files are parsed into a temporary buffer first and every field is range-validated — a broken/missing/tampered file can never corrupt data already in memory |
| Export Formatted Report | Generates a clean, plain-text `report.txt` (table + stats + subject averages + grade breakdown) for printing/sharing |
| Full color-coded theming | Every section (Manage=Blue, Reports=Magenta, Sort=Yellow, File=Green) has a consistent identity color across its headers, menus, and prompts |
| Zebra-striped tables | Alternating row shading for readability, animated splash screen on startup |

---

## Files

| File               | Purpose                                          |
|--------------------|---------------------------------------------------|
| `student_system.cpp` | Complete source code                             |
| `students.txt`     | Auto-created — stores all student records          |
| `report.txt`       | Created on demand — human-readable exported report |

---

## How to Run

### Option 1 — VS Code "Run Code" button (recommended)
1. Open `student_system.cpp` in VS Code.
2. Click the ▶ **Run Code** button (top-right corner of the editor).
3. The program compiles and runs directly inside the VS Code terminal.

### Option 2 — Manual compile via terminal
```bash
g++ -std=c++17 -O2 -Wall -Wextra -o student_system student_system.cpp
./student_system          # Linux/macOS
student_system.exe        # Windows







## Project Report

### 1. Introduction
This project builds a console-based Student Record System using C++. It practices the Month 1 key concept — struct — alongside arrays of structs, file I/O, and hand-written bubble sort. The goal was to make a tool closer to a real data-management app: teachers can add, edit, search, delete, sort, analyze, and save/load student records.

### 2. Objectives Addressed
- Group related student data using a struct
- Manage multiple students using an array of structs
- Perform string operations (validation, searching, case handling)
- Read/write structured data with ifstream/ofstream
- Implement bubble sort manually as taught

### 3. Features Implemented
All 6 required core features are included. All 5 bonus challenges are implemented: letter grades (A-F), sort by name (A-Z / Z-A), search by roll number, delete by ID with confirmation, and subject-wise class averages with top scorers per subject. Extras include edit student, search by name, non-destructive class ranking, grade distribution chart, needs-improvement stat, live dashboard with unsaved-changes tracking, auto-load on startup, corruption-safe file loading, and export to report.txt.

### 4. Design & Architecture Decisions
- Single source of truth for derived data: all average/grade/status values are recalculated through one function (recomputeDerived()) so they can never go out of sync.
- Stable IDs: student IDs are assigned from an always-increasing counter and never reused, even after deletion. This keeps student identity stable regardless of array reordering.
- Non-destructive vs destructive: Class Ranking sorts a temporary copy just for viewing, while the required Sort Students menu permanently reorders the stored array.
- Defensive file loading: parseDataFile() reads into a temporary buffer first and validates every field range before committing, so a broken or edited file cannot corrupt data in memory.

### 5. Challenges Faced & Solutions
Challenge 1 — Reading names with spaces after reading numbers. Mixing cin >> and getline() causes leftover newlines. Solution: every input uses getline() and is validated as a string first.
Challenge 2 — Colored text breaking table alignment. Incorrect use of setw() with embedded ANSI codes can misalign columns. Solution: padded plain text first, then wrapped with color codes separately.
Challenge 3 — Stable IDs after deletion. If IDs were just array positions + 1, deleting a student would change every later ID. Solution: ever-increasing counter, never reassigned.
Challenge 4 — Numeric input crashes. stoi/stod can throw exceptions on extreme input. Solution: length guard + try/catch around every conversion.
Challenge 5 — Tampered save files. Students.txt is plain text and can be manually edited. Solution: validation of every value range (marks 0-100, positive IDs/rolls, non-empty names) rejects any corrupted file entirely.

### 6. Testing Summary
All checklist cases verified: valid/invalid data entry, table formatting, statistics, sorting in both directions, save/load round-trip, corrupted file handling, and invalid menu choices.

### 7. Conclusion & Key Learnings
This project reinforced how structs simplify grouped data compared to parallel arrays, how centralized input validation makes programs far more resilient, and why thinking about data integrity across restarts (stable IDs, safe loading, unsaved-changes tracking) matters for real-world tools.
