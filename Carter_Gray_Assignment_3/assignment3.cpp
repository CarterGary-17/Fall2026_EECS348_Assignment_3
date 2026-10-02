
/* EECS 348 Assignment 2-  CEO email prioritizer using a hand-built, array-based MaxHeap

Brief description:
 the program reads a list of emails then uses commands from and uses a an dynmaic array based max heap to act as aqueue for a ceo inbox 
 in the program emails are ordered first by the sender category (boss subordinate peer importantperson otherperson) and then by newest date first within the same category
 
 Inputs:
 lines inputs are either an EMAIL line with category subject and date separated by commas 
 or
 NEXT READ or COUNT command
 
 Outputs:
 for NEXT the next email to read or a message that there are none 
 for COUNT the number of unread emails 
 for READ it produces no output just removes

 Collaborators: Claude code (Sonnet 5) 
 Other sources: None 
 Author: Carter Gray
 Creation date: 8/9/2026
 Revision date: N/A
 Revisions: N/A

*/

// Usage: ./assignment3 [testfile]  (prompts for a filename if none is given)

#include <iostream> // these are for IO: cout, cin, cerr, endl 
#include <fstream> // these are for reading ifstream 
#include <sstream> // for support: string stream  
#include <string> // std::string, stoi, getline

using namespace std; // this lets me write str count

// ---------------------------------------------------------------------------
// Email: one message in the CEO's inbox
// ---------------------------------------------------------------------------

class Email { // EMAIL object that has one email and knows its own priority
private: // data hidden from outside code
    string sender; // sender category: Boss, Subordinate, Peer, ImportantPerson, OtherPerson
    string subject; // subject line text (may contain spaces, no commas)
    string date; // date exactly as given, format MM-DD-YYYY (used for display)
    int rank; // numeric priority of the sender category, higher = read sooner
    int dateKey; // make sorting date as YYYYMMDD integer so that means that larger = newer  
    long order; // arrival this will only be  as a final tie-breaker

    int rankFor(const string& s) const { // converts a sender category into a rank
        if (s == "Boss") return 5; // READ BOss
        if (s == "Subordinate") return 4; // then Subordinate 
        if (s == "Peer") return 3; // Peer is read next
        if (s == "ImportantPerson") return 2; // ImportantPerson then
        return 1; // anything other non person
    } // end 

    int keyFor(const string& d) const { // converts MM-DD-YYYY into the integer YYYYMMDD
        if (d.size() != 10 || d[2] != '-' || d[5] != '-') return 0; // too short to be a valid date, treat as oldest
        for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (!isdigit(static_cast<unsigned char>(d[i]))) return 0;
        }
        int mm = stoi(d.substr(0, 2)); // characters 0-1 are the month
        int dd = stoi(d.substr(3, 2)); // characters 3-4 are the day (index 2 is '-')
        int yyyy = stoi(d.substr(6, 4)); // characters 6-9 are the year (index 5 is '-')
        return yyyy * 10000 + mm * 100 + dd; // year first so integer comparison orders dates correctly
    } // end of keyFor

public: // functions other code may call
    Email() : sender(""), subject(""), date(""), rank(0), dateKey(0), order(0) {} // default constructor, needed to allocate the heap array

    Email(const string& s, const string& subj, const string& d, long ord) // main constructor
        : sender(s), subject(subj), date(d), order(ord) { // store the raw fields
        rank = rankFor(sender); // compute the numeric rank from the category
        dateKey = keyFor(date); // compute the sortable date number
    } // end of constructor

    string getSender() const { return sender; } // accessor for printing the sender
    string getSubject() const { return subject; } // accessor for printing the subject
    string getDate() const { return date; } // accessor for printing the date

    // True if this email should be read before 'other'
    bool hasHigherPriorityThan(const Email& other) const { // the comparison the heap uses
        if (rank != other.rank) return rank > other.rank; // different categories: higher rank wins
        if (dateKey != other.dateKey) return dateKey > other.dateKey; // same category: newer date wins
        return order < other.order; // same category and date: earlier arrival wins
    } // end of hasHigherPriorityThan
}; // end of Email class

// ---------------------------------------------------------------------------
// MaxHeap: list-based (dynamic array) binary max-heap of Emails
// ---------------------------------------------------------------------------
class MaxHeap { // priority queue implemented from scratch
private: // internal heap machinery
    Email* data; // pointer to the dynamically allocated array holding the heap
    int count; // number of emails currently stored
    int capacity; // number of slots allocated in the array

    int parent(int i) const { return (i - 1) / 2; } // index of the parent of node i
    int left(int i) const { return 2 * i + 1; } // index of the left child of node i
    int right(int i) const { return 2 * i + 2; } // index of the right child of node i

    void swapAt(int a, int b) { // swaps the emails at two indexes
        Email tmp = data[a]; // save the first email
        data[a] = data[b]; // overwrite first slot with the second email
        data[b] = tmp; // put the saved email in the second slot
    } // end of swapAt

    void resize(int newCap) { // changes how many Email slots are allocated, used to grow or shrink the array
    Email* resized = new Email[newCap]; // allocate a new array to the newCap slots
    for (int i = 0; i < count; i++) resized[i] = move(data[i]); // move each stored email into the new array
    delete[] data; // free the old array to avoid memory leak
    data = resized; // point the heap at the new array
    capacity = newCap; // remember the new number of slots
} // end of resize

    void grow() { //add to the new roles when it is full
    resize(capacity * 2);
}

    void siftUp(int i) { // moves a newly inserted email up to its correct spot
        while (i > 0 && data[i].hasHigherPriorityThan(data[parent(i)])) { // keep going while higher priority than parent
            swapAt(i, parent(i)); // swap with the parent
            i = parent(i); // continue checking from the parent's position
        } // end of loop
    } // end of siftUp

    void siftDown(int i) { // moves an email down to restore the heap after a removal
        while (true) { // loop until the email is in the right place
            int l = left(i), r = right(i), best = i; // candidates: node itself and its two children
            if (l < count && data[l].hasHigherPriorityThan(data[best])) best = l; // left child beats current best
            if (r < count && data[r].hasHigherPriorityThan(data[best])) best = r; // right child beats current best
            if (best == i) break; // node already has higher priority than both children, done
            swapAt(i, best); // swap with the higher-priority child
            i = best; // continue from the child's position
        } // end of loop
    } // end of siftDown

public: // operations the rest of the program uses
    MaxHeap() : count(0), capacity(16) { data = new Email[capacity]; } // start empty with room for 16 emails
    ~MaxHeap() { delete[] data; } // destructor frees the array

    // Prevent accidental shallow copies of the internal array
    MaxHeap(const MaxHeap&) = delete; // disallow copy construction
    MaxHeap& operator=(const MaxHeap&) = delete; // disallow copy assignment

    bool isEmpty() const { return count == 0; } // true when no emails are stored
    int size() const { return count; } // number of unread emails

    void insert(const Email& e) { // adds an email to the heap
        if (count == capacity) grow(); // make room if the array is full
        data[count] = e; // place the new email in the next free slot (bottom of the heap)
        siftUp(count); // move it up until the heap property holds
        count++; // one more email stored
    } // end of insert

    // Returns the highest-priority email without removing it (caller checks isEmpty)
    const Email& peek() const { return data[0]; } // the root is always the highest priority

    // Removes the highest-priority email (caller checks isEmpty)
    void removeMax() { // deletes the root of the heap
        data[0] = data[count - 1]; // move the last email into the root
        count--; // shrink the heap by one
        if (count > 0) siftDown(0); // push the moved email down to its correct place
        if (capacity > 16 && count <= capacity / 4) resize(capacity / 2);
    } // end of removeMax



}; // end of MaxHeap class

// ---------------------------------------------------------------------------
// CEOInbox: wraps the heap and handles the four commands
// ---------------------------------------------------------------------------
class CEOInbox { // object representing the CEO's inbox
private: // internal state and helpers
    MaxHeap heap; // the priority queue holding all unread emails
    long arrivals; // counter that gives each email an arrival number

    static string trim(const string& s) { // removes leading/trailing whitespace
        size_t a = s.find_first_not_of(" \t\r\n"); // position of first non-whitespace character
        if (a == string::npos) return ""; // string is all whitespace, return empty
        size_t b = s.find_last_not_of(" \t\r\n"); // position of last non-whitespace character
        return s.substr(a, b - a + 1); // return the trimmed middle portion
    } // end of trim

public: // commands the program uses
    CEOInbox() : arrivals(0) {} // start with no arrivals counted

    // args = "<sender category>,<subject line>,<date>"
    void addEmail(const string& args) { // parses an EMAIL command's fields and queues the email
        size_t c1 = args.find(','); // position of the first comma
        size_t c2 = (c1 == string::npos) ? string::npos : args.find(',', c1 + 1); // position of the second comma
        if (c1 == string::npos || c2 == string::npos) return; // fewer than two commas, malformed line, ignore it

        string sender = trim(args.substr(0, c1)); // text before the first comma is the sender
        string subject = trim(args.substr(c1 + 1, c2 - c1 - 1)); // text between the commas is the subject
        string date = trim(args.substr(c2 + 1)); // text after the second comma is the date
        heap.insert(Email(sender, subject, date, arrivals++)); // build the Email and add it to the heap
    } // end of addEmail

    void next() const { // NEXT command: show the email to read next
        if (heap.isEmpty()) { // nothing to show
            cout << "No emails to read." << endl; // report an empty inbox
            return; // leave without printing an email
        } // end of empty check
        const Email& e = heap.peek(); // look at the top email without removing it
        cout << "Next email:" << endl; // header line
        cout << "Sender: " << e.getSender() << endl; // print the sender category
        cout << "Subject: " << e.getSubject() << endl; // print the subject line
        cout << "Date: " << e.getDate() << endl; // print the date
    } // end of next

    void read() { // READ command: CEO has dealt with the top email
        if (!heap.isEmpty()) heap.removeMax(); // remove it silently if one exists
    } // end of read

    void count() const { // COUNT command: show unread total
        cout << "There are " << heap.size() << " emails to read." << endl; // print the number of unread emails
    } // end of count

    // Parse and execute one line from the test file
    void processLine(const string& rawLine) { // decides which command a line is and runs it
        string line = trim(rawLine); // strip whitespace and any '\r'
        if (line.empty()) return; // skip blank lines

        if (line.compare(0, 6, "EMAIL ") == 0) { // line starts with "EMAIL "
            addEmail(line.substr(6)); // pass everything after "EMAIL " to the parser
        } else if (line == "NEXT") { // NEXT command
            next(); // display the next email
        } else if (line == "READ") { // READ command
            read(); // remove the top email
        } else if (line == "COUNT") { // COUNT command
            count(); // display the unread count
        } // unknown lines are ignored
    } // end of processLine
}; // end of CEOInbox class

// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) { // program entry point
    ifstream fileIn; // file stream, only opened if a filename is given
    istream* in = &cin; // pointer to the input source, defaults to standard input

    if (argc > 1) { // a filename was passed on the command line
        fileIn.open(argv[1]); // open that file for reading
        if (!fileIn) { // opening failed
            cerr << "Error: could not open file '" << argv[1] << "'" << endl; // report the problem on the error stream
            return 1; // exit with a non-zero status
        } // end of open check
        in = &fileIn; // switch the input source from cin to the file
    } // end of filename check

    CEOInbox inbox; // create the CEO's inbox object
    string line; // holds one line of input at a time
    while (getline(*in, line)) { // read from whichever source is selected until the end
        inbox.processLine(line); // run the command on that line
    } // end of read loop

    return 0; // success
} // end of main
