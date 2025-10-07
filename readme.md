Files included:

main.cpp : C++ source implementing the student system (no STL containers used).

Makefile : Build instructions.

README.txt : This file.

Design summary:

Uses object-oriented features: abstraction (Student base class), inheritance (BTechStudent, MTechStudent, PhDStudent),
polymorphism (virtual methods), data hiding (private/protected members + getters/setters).

Operator overloading:

Course::operator+=(Student*) to add a student to the course (Course takes ownership).

Course::operator()(const char* roll) to lookup and obtain a modifiable reference to a student by roll.

Exception handling:

Custom exception classes derived from std::exception:
BufferOverflowException, NoSecondNameException, InvalidSecondNameException, InvalidRollException, RollNotFoundException.

Input validators throw these exceptions which are caught in demo().

Storage:

A simple singly linked list stores Student objects (no STL containers).

Sorting:

Quicksort implementations over a Student* array for roll and marks components (no STL sort).

Name-sorting implemented using a Trie data structure: names inserted into trie, traversed lexicographically to produce sorted order.

Input validation:

Name validation enforces at least two words and disallows digits in second name.

Roll validation allows alphanumeric, '/', and '-' characters only.

Marks:

Marks struct with components: assignment, midterm, lab, final. total() returns the aggregate.

How to build:

Ensure g++ is installed.

Run: make

Run: ./assignment