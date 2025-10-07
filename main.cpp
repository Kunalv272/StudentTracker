/*
main.cpp
OOPD Assignment: Student tracking system

* No use of STL containers (vector/map/string). Uses C-style strings and custom data structures.
* Demonstrates encapsulation, data hiding, polymorphism, operator overloading, and exception handling.
* Compile: g++ -std=c++17 -O2 -Wall main.cpp -o assignment

Supported operations (interactive demo in main):

* Add students using operator+=
* Modify student record by roll using operator()
* Compute total marks per student
* Sort by roll, by marks of a component, or by name (using a Trie)
* Demonstrates custom exceptions for invalid input
  */

#include <iostream>
#include <cstring>
#include <cctype>
#include <stdexcept>

using namespace std;

/* -------------------------
Exception classes
------------------------- */
class StudentException : public std::exception {
protected:
const char* msg;
public:
StudentException(const char* m="Student exception") : msg(m) {}
const char* what() const noexcept override { return msg; }
};

class BufferOverflowException : public StudentException {
public:
BufferOverflowException() : StudentException("Buffer overflow in input") {}
};

class NoSecondNameException : public StudentException {
public:
NoSecondNameException() : StudentException("No second name provided") {}
};

class InvalidSecondNameException : public StudentException {
public:
InvalidSecondNameException() : StudentException("Second name contains digits or special characters") {}
};

class InvalidRollException : public StudentException {
public:
InvalidRollException() : StudentException("Unrecognized character in roll number") {}
};

class RollNotFoundException : public StudentException {
public:
RollNotFoundException() : StudentException("Roll number not found") {}
};

/* -------------------------
Helper functions (C-style)
------------------------- */
inline bool isValidNameChar(char c) {
return isalpha((unsigned char)c) || c==' ' || c=='-';
}

inline bool isValidRollChar(char c) {
// allow alnum and '/'
return isalnum((unsigned char)c) || c=='/' || c=='-';
}

/* safe copy into fixed buffer, throws BufferOverflowException */
void safeStrCpy(char* dst, const char* src, int maxlen) {
int len = strlen(src);
if (len >= maxlen) throw BufferOverflowException();
strcpy(dst, src);
}

/* validate name: require at least two words (first + second) and no digits in second name */
void validateName(const char* name) {
if (!name || strlen(name) == 0) throw NoSecondNameException();
int n = strlen(name);
int words = 0;
bool inword = false;
int lastWordStart = -1;
for (int i=0;i<n;i++) {
char c = name[i];
if (!isValidNameChar(c)) throw InvalidSecondNameException();
if (!isspace((unsigned char)c) && !inword) {
inword = true;
lastWordStart = i;
words++;
} else if (isspace((unsigned char)c)) {
inword = false;
}
}
if (words < 2) throw NoSecondNameException();

// check second word doesn't contain digits or special chars
// find second word start
int wordCount = 0;
inword = false;
int wordStart = -1;
for (int i=0;i<n;i++) {
    char c = name[i];
    if (!isspace((unsigned char)c) && !inword) {
        inword = true;
        wordCount++;
        wordStart = i;
        if (wordCount==2) {
            // validate until end of this word
            int j = i;
            while (j < n && !isspace((unsigned char)name[j])) {
                if (!isalpha((unsigned char)name[j]) && name[j] != '-' ) throw InvalidSecondNameException();
                j++;
            }
            break;
        }
    } else if (isspace((unsigned char)c)) {
        inword = false;
    }
}
}

/* validate roll */
void validateRoll(const char* roll) {
if (!roll || strlen(roll)==0) throw InvalidRollException();
for (int i=0;i<(int)strlen(roll);i++) {
if (!isValidRollChar(roll[i])) throw InvalidRollException();
}
}

/* -------------------------
Marks and student classes
------------------------- */

enum Branch { BR_CSE=0, BR_ECE=1 };
const char* branchToStr(Branch b) { return b==BR_CSE ? "CSE" : "ECE"; }
const int NAME_MAX = 64;
const int ROLL_MAX = 32;

struct Marks {
// components: assignment, midterm, lab, final
double assignment;
double midterm;
double lab;
double finalexam;

Marks(): assignment(0), midterm(0), lab(0), finalexam(0) {}
double total() const {
    return assignment + midterm + lab + finalexam;
}

};

/* Abstract base class Student */
class Student {
protected:
char name[NAME_MAX];
char roll[ROLL_MAX];
Branch branch;
Marks marks;
int level; // 0 BTech, 1 MTech, 2 PhD
public:
Student() {
name[0]='\0';
roll[0]='\0';
branch = BR_CSE;
level = 0;
}
virtual ~Student() {}
// Data hiding: provide setters/getters
void setName(const char* nm) {
validateName(nm);
safeStrCpy(name, nm, NAME_MAX);
}
const char* getName() const { return name; }

void setRoll(const char* r) {
    validateRoll(r);
    safeStrCpy(roll, r, ROLL_MAX);
}
const char* getRoll() const { return roll; }

void setBranch(Branch b) { branch = b; }
Branch getBranch() const { return branch; }

void setLevel(int lv) { level = lv; }
int getLevel() const { return level; }

void setMarks(const Marks& m) { marks = m; }
Marks getMarks() const { return marks; }

double totalMarks() const { return marks.total(); }

virtual const char* type() const = 0; // pure virtual for polymorphism

// print
virtual void print() const {
    cout << "Roll: " << roll << " | Name: " << name << " | Level: " << type()
         << " | Branch: " << branchToStr(branch)
         << " | Marks: A=" << marks.assignment << " M=" << marks.midterm
         << " L=" << marks.lab << " F=" << marks.finalexam
         << " | Total=" << totalMarks() << "\n";
}

};

/* Derived classes for levels */
class BTechStudent : public Student {
public:
BTechStudent() { level = 0; }
const char* type() const override { return "BTech"; }
};

class MTechStudent : public Student {
public:
MTechStudent() { level = 1; }
const char* type() const override { return "MTech"; }
};

class PhDStudent : public Student {
public:
PhDStudent() { level = 2; }
const char* type() const override { return "PhD"; }
};

/* -------------------------
Storage: singly linked list of students
operator overloading:
Course += Student*  (adds a student - Course takes ownership)
Course(roll) -> returns pointer to Student for modification (throws if not found)
------------------------- */

class Course {
private:
struct Node {
Student* student;
Node* next;
Node(Student* s=nullptr): student(s), next(nullptr) {}
};
Node* head;
int count;

// disallow copying to respect data hiding ownership
Course(const Course&) = delete;
Course& operator=(const Course&) = delete;

public:
Course(): head(nullptr), count(0) {}
~Course() {
Node* cur = head;
while (cur) {
Node* nxt = cur->next;
delete cur->student;
delete cur;
cur = nxt;
}
}

// add student (Course takes ownership). Use operator+=
Course& operator+=(Student* s) {
    Node* n = new Node(s);
    // insert at head for simplicity
    n->next = head;
    head = n;
    count++;
    return *this;
}

// find student by roll. returns Student*, or nullptr
Student* findByRoll(const char* roll) {
    Node* cur = head;
    while (cur) {
        if (strcmp(cur->student->getRoll(), roll) == 0) return cur->student;
        cur = cur->next;
    }
    return nullptr;
}

// operator() to access/modify by roll number. Throws RollNotFoundException if not present.
Student& operator()(const char* roll) {
    Student* s = findByRoll(roll);
    if (!s) throw RollNotFoundException();
    return *s;
}

int size() const { return count; }

// export to array (array of Student*) for sorting
Student** exportArray() {
    if (count == 0) return nullptr;
    Student** arr = new Student*[count];
    int idx = 0;
    Node* cur = head;
    while (cur) {
        arr[idx++] = cur->student;
        cur = cur->next;
    }
    return arr;
}

// print all
void printAll() const {
    Node* cur = head;
    while (cur) {
        cur->student->print();
        cur = cur->next;
    }
}

// remove student by roll (optional helper)
bool removeByRoll(const char* roll) {
    Node* cur = head;
    Node* prev = nullptr;
    while (cur) {
        if (strcmp(cur->student->getRoll(), roll) == 0) {
            if (prev) prev->next = cur->next; else head = cur->next;
            delete cur->student;
            delete cur;
            count--;
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}

};

/* -------------------------
Sorting utilities

* Sort by roll (lexicographic)
* Sort by marks component (assignment|midterm|lab|final)
* Sort by name using Trie
  No STL: implement quicksort on array of Student*
  ------------------------- */

int cmpRoll(const char* a, const char* b) {
return strcmp(a,b);
}

enum MarkComponent { MC_ASSIGN=0, MC_MID=1, MC_LAB=2, MC_FINAL=3 };

double getComponent(const Student* s, MarkComponent mc) {
switch(mc) {
case MC_ASSIGN: return s->getMarks().assignment;
case MC_MID: return s->getMarks().midterm;
case MC_LAB: return s->getMarks().lab;
case MC_FINAL: return s->getMarks().finalexam;
}
return 0;
}

/* quicksort for Student* array using comparator function */
typedef int (*CmpFn)(Student*, Student*);

int cmpByRollPtr(Student* a, Student* b) {
return strcmp(a->getRoll(), b->getRoll());
}

int cmpByMarksPtrFactory(Student* a, Student* b, MarkComponent mc) {
double da = getComponent(a, mc);
double db = getComponent(b, mc);
if (da < db) return -1;
if (da > db) return 1;
return strcmp(a->getRoll(), b->getRoll()); // tie-breaker
}

void quickSwap(Student** arr, int i, int j) {
Student* t = arr[i]; arr[i]=arr[j]; arr[j]=t;
}

/* quicksort by roll */
void quickSortRoll(Student** arr, int lo, int hi) {
if (lo >= hi) return;
Student* pivot = arr[(lo+hi)/2];
int i = lo, j = hi;
while (i <= j) {
while (strcmp(arr[i]->getRoll(), pivot->getRoll()) < 0) i++;
while (strcmp(arr[j]->getRoll(), pivot->getRoll()) > 0) j--;
if (i <= j) {
quickSwap(arr, i, j);
i++; j--;
}
}
if (lo < j) quickSortRoll(arr, lo, j);
if (i < hi) quickSortRoll(arr, i, hi);
}

/* quicksort by marks component */
void quickSortMarks(Student** arr, int lo, int hi, MarkComponent mc) {
if (lo >= hi) return;
double pivotVal = getComponent(arr[(lo+hi)/2], mc);
int i = lo, j = hi;
while (i <= j) {
while (getComponent(arr[i], mc) < pivotVal) i++;
while (getComponent(arr[j], mc) > pivotVal) j--;
if (i <= j) {
quickSwap(arr, i, j);
i++; j--;
}
}
if (lo < j) quickSortMarks(arr, lo, j, mc);
if (i < hi) quickSortMarks(arr, i, hi, mc);
}

/* -------------------------
Trie for name sorting

* store pointers to Student at terminal nodes
* traverse trie in lexicographic order
  ------------------------- */

const int TRIE_ALPHABET = 27; // 'a'..'z' + space as 26
inline int chIndex(char c) {
if (c == ' ') return 26;
c = tolower((unsigned char)c);
if (c >= 'a' && c <= 'z') return c - 'a';
return 26; // map anything else to space (safe)
}

struct TrieNode {
TrieNode* children[TRIE_ALPHABET];
Student** students; // dynamic array of pointers (for multiple students with same name)
int studCount;
int studCap;
TrieNode() {
for (int i=0;i<TRIE_ALPHABET;i++) children[i]=nullptr;
students = nullptr;
studCount = 0;
studCap = 0;
}
~TrieNode() {
for (int i=0;i<TRIE_ALPHABET;i++) if (children[i]) delete children[i];
if (students) delete [] students;
}
void addStudent(Student* s) {
if (studCount == studCap) {
int newcap = (studCap==0)?4:studCap*2;
Student** tmp = new Student*[newcap];
for (int i=0;i<studCount;i++) tmp[i]=students[i];
if (students) delete [] students;
students = tmp;
studCap = newcap;
}
students[studCount++] = s;
}
};

class NameTrie {
private:
TrieNode* root;
public:
NameTrie() { root = new TrieNode(); }
~NameTrie() { delete root; }

void insert(Student* s) {
    const char* name = s->getName();
    TrieNode* cur = root;
    int n = strlen(name);
    for (int i=0;i<n;i++) {
        int idx = chIndex(name[i]);
        if (!cur->children[idx]) cur->children[idx] = new TrieNode();
        cur = cur->children[idx];
    }
    cur->addStudent(s);
}

// traverse and append to output array
void traverseCollect(TrieNode* node, Student** out, int &idx) {
    if (!node) return;
    // if students at node, append them
    if (node->studCount > 0) {
        // append all students in insertion order
        for (int k=0;k<node->studCount;k++) out[idx++] = node->students[k];
    }
    for (int i=0;i<TRIE_ALPHABET;i++) {
        if (node->children[i]) traverseCollect(node->children[i], out, idx);
    }
}

// produce sorted array of size 'count'. Caller must ensure out has capacity count.
void collectSorted(Student** out, int count) {
    int idx = 0;
    traverseCollect(root, out, idx);
    // idx should be == count
}

};

/* -------------------------
Utility to sort by name using trie
------------------------- */
Student** sortByNameUsingTrie(Course& c) {
int n = c.size();
if (n==0) return nullptr;
Student** arr = c.exportArray();
NameTrie trie;
for (int i=0;i<n;i++) trie.insert(arr[i]);
Student** out = new Student*[n];
for (int i=0;i<n;i++) out[i]=nullptr;
trie.collectSorted(out, n);
delete [] arr;
return out;
}

/* -------------------------
Demo / simple interactive CLI in main()
------------------------- */

void demo() {
Course course;

try {
    // Create some students
    BTechStudent* s1 = new BTechStudent();
    s1->setName("Amit Kumar");
    s1->setRoll("20CS1001");
    s1->setBranch(BR_CSE);
    Marks m1; m1.assignment=15; m1.midterm=24; m1.lab=10; m1.finalexam=45;
    s1->setMarks(m1);
    course += s1; // operator+=

    MTechStudent* s2 = new MTechStudent();
    s2->setName("Sunita Sharma");
    s2->setRoll("21EC2001");
    s2->setBranch(BR_ECE);
    Marks m2; m2.assignment=18; m2.midterm=28; m2.lab=12; m2.finalexam=40;
    s2->setMarks(m2);
    course += s2;

    PhDStudent* s3 = new PhDStudent();
    s3->setName("Rahul Verma");
    s3->setRoll("19CS0999");
    s3->setBranch(BR_CSE);
    Marks m3; m3.assignment=20; m3.midterm=30; m3.lab=15; m3.finalexam=50;
    s3->setMarks(m3);
    course += s3;

    cout << "All students (unordered/insertion order):\n";
    course.printAll();

    // Modify a student using operator()
    cout << "\nModifying marks for roll 21EC2001\n";
    Student& ref = course("21EC2001");
    Marks nm = ref.getMarks();
    nm.finalexam = 42.5;
    ref.setMarks(nm);
    cout << "After modification:\n";
    ref.print();

    // compute totals
    cout << "\nTotals:\n";
    Student** arr = course.exportArray();
    int n = course.size();
    for (int i=0;i<n;i++) {
        cout << arr[i]->getRoll() << " -> Total = " << arr[i]->totalMarks() << "\n";
    }

    // sort by roll
    cout << "\nSorted by roll:\n";
    quickSortRoll(arr, 0, n-1);
    for (int i=0;i<n;i++) arr[i]->print();

    // sort by midterm marks
    cout << "\nSorted by midterm marks:\n";
    Student** arr2 = course.exportArray();
    quickSortMarks(arr2, 0, n-1, MC_MID);
    for (int i=0;i<n;i++) arr2[i]->print();

    // sort by name using trie
    cout << "\nSorted by name (Trie):\n";
    Student** nameSorted = sortByNameUsingTrie(course);
    for (int i=0;i<n;i++) nameSorted[i]->print();

    delete [] arr;
    delete [] arr2;
    delete [] nameSorted;

    // demonstrate exception handling
    try {
        BTechStudent* s4 = new BTechStudent();
        s4->setName("SingleName"); // should throw NoSecondNameException
        s4->setRoll("20CS1002");
        course += s4;
    } catch (StudentException &e) {
        cout << "Caught exception while adding student: " << e.what() << "\n";
    }

    // invalid roll
    try {
        BTechStudent* s5 = new BTechStudent();
        s5->setName("Maya Rao");
        s5->setRoll("20CS#1003"); // invalid char
        course += s5;
    } catch (StudentException &e) {
        cout << "Caught exception while adding student: " << e.what() << "\n";
    }

    // accessing non-existent roll
    try {
        course("0000/NOTFOUND");
    } catch (StudentException &e) {
        cout << "Caught exception while accessing student: " << e.what() << "\n";
    }

} catch (std::exception &e) {
    cout << "Unhandled exception: " << e.what() << "\n";
}

}

int main() {
cout << "OOPD Assignment demo\n";
demo();
cout << "\nDemo finished.\n";
return 0;
}
