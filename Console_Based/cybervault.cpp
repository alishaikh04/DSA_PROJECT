#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <string>

using namespace std;

// File paths
const string PASSWORDS_FILE = "database.txt";
const string HISTORY_FILE = "history.txt";
const string ANALYTICS_FILE = "analytics.txt";

// Hash Node Structure
struct HashNode {
    string password;
    string status;
    HashNode* next;
};

// Stack Node for History
struct StackNode {
    string operation;
    string timestamp;
    StackNode* next;
};

// Global Variables
const int HASH_SIZE = 10;
HashNode* hashTable[HASH_SIZE] = {NULL};
StackNode* historyStack = NULL;
int totalChecks = 0;
int breachedCount = 0;
int safeCount = 0;

// ========== HASH FUNCTION ==========
int getHashIndex(string str) {
    int sum = 0;
    for (int i = 0; i < str.length(); i++) {
        sum = sum + (int)str[i];
    }
    return sum % HASH_SIZE;
}

// ========== VALIDATE PASSWORD ==========
bool isValidPassword(string pass) {
    if (pass.length() < 8) {
        return false;
    }
    return true;
}

// ========== CHECK EXISTS ==========
bool exists(string pass) {
    int index = getHashIndex(pass);
    HashNode* temp = hashTable[index];

    while (temp != NULL) {
        if (temp->password == pass) {
            return true;
        }
        temp = temp->next;
    }
    return false;
}

// ========== INSERT INTO HASH TABLE ==========
void insertIntoHash(string pass, string stat) {
    if (exists(pass)) {
        return;
    }

    int index = getHashIndex(pass);
    HashNode* newNode = new HashNode();
    newNode->password = pass;
    newNode->status = stat;
    newNode->next = hashTable[index];
    hashTable[index] = newNode;
}

// ========== GET CURRENT TIME ==========
string getCurrentTime() {
    time_t now = time(0);
    char* dt = ctime(&now);
    string timeStr(dt);
    // Remove newline character
    timeStr = timeStr.substr(0, timeStr.length() - 1);
    return timeStr;
}

// ========== STACK OPERATIONS ==========
void pushHistory(string op) {
    StackNode* newNode = new StackNode();
    newNode->operation = op;
    newNode->timestamp = getCurrentTime();
    newNode->next = historyStack;
    historyStack = newNode;
}

void displayHistory() {
    if (historyStack == NULL) {
        cout << "{\"success\":true,\"history\":[]}" << endl;
        return;
    }

    cout << "{\"success\":true,\"history\":[";
    StackNode* temp = historyStack;
    bool first = true;

    while (temp != NULL) {
        if (!first) {
            cout << ",";
        }
        first = false;
        cout << "{\"operation\":\"" << temp->operation
             << "\",\"time\":\"" << temp->timestamp << "\"}";
        temp = temp->next;
    }
    cout << "]}" << endl;
}

// ========== CHECK PASSWORD ==========
string checkPassword(string pass) {
    totalChecks = totalChecks + 1;

    int index = getHashIndex(pass);
    HashNode* temp = hashTable[index];

    while (temp != NULL) {
        if (temp->password == pass) {
            if (temp->status == "Breached") {
                breachedCount = breachedCount + 1;
                pushHistory("Checked: " + pass + " (Breached)");
                return "Breached";
            } else {
                safeCount = safeCount + 1;
                pushHistory("Checked: " + pass + " (Safe)");
                return "Safe";
            }
        }
        temp = temp->next;
    }

    safeCount = safeCount + 1;
    pushHistory("Checked: " + pass + " (Safe - Not found)");
    return "Safe";
}

// ========== ADD PASSWORD ==========
void addPassword(string pass, string stat) {
    // Validate password length
    if (!isValidPassword(pass)) {
        cout << "{\"success\":false,\"error\":\"Password must be at least 8 characters\"}" << endl;
        return;
    }

    if (exists(pass)) {
        cout << "{\"success\":false,\"error\":\"Password already exists\"}" << endl;
        return;
    }

    insertIntoHash(pass, stat);

    ofstream out(PASSWORDS_FILE, ios::app);
    out << pass << "|" << stat << endl;
    out.close();

    pushHistory("Added: " + pass + " (" + stat + ")");
    cout << "{\"success\":true}" << endl;
}

// ========== SAVE ANALYTICS ==========
void saveAnalytics() {
    ofstream out(ANALYTICS_FILE);
    out << totalChecks << endl;
    out << breachedCount << endl;
    out << safeCount << endl;
    out.close();
}

void loadAnalytics() {
    ifstream in(ANALYTICS_FILE);
    if (in.is_open()) {
        in >> totalChecks;
        in >> breachedCount;
        in >> safeCount;
        in.close();
    }
}

// ========== LOAD DATABASE ==========
void loadDatabase() {
    ifstream in(PASSWORDS_FILE);

    if (!in.is_open()) {
        ofstream create(PASSWORDS_FILE);
        create << "password123|Breached" << endl;
        create << "admin123|Breached" << endl;
        create << "qwerty|Breached" << endl;
        create << "ali999|Safe" << endl;
        create << "newpass2024|Safe" << endl;
        create.close();
        in.open(PASSWORDS_FILE);
    }

    string line;
    while (getline(in, line)) {
        if (line.empty()) {
            continue;
        }

        stringstream ss(line);
        string pass, stat;
        getline(ss, pass, '|');
        getline(ss, stat, '|');

        insertIntoHash(pass, stat);
    }
    in.close();
}

// ========== LOAD HISTORY ==========
void loadHistory() {
    ifstream in(HISTORY_FILE);
    if (!in.is_open()) {
        return;
    }

    string line;
    while (getline(in, line)) {
        if (line.empty()) {
            continue;
        }
        pushHistory(line);
    }
    in.close();
}

void saveHistory() {
    ofstream out(HISTORY_FILE, ios::trunc);

    // Create a copy of stack to preserve original
    StackNode* temp = historyStack;

    // Count items
    int count = 0;
    StackNode* counter = historyStack;
    while (counter != NULL) {
        count = count + 1;
        counter = counter->next;
    }

    // Create array to store operations in order
    string* operations = new string[count];
    int index = 0;
    temp = historyStack;
    while (temp != NULL) {
        operations[index] = temp->operation;
        index = index + 1;
        temp = temp->next;
    }

    // Write in reverse order (oldest first)
    for (int i = count - 1; i >= 0; i--) {
        out << operations[i] << endl;
    }

    delete[] operations;
    out.close();
}

// ========== GET STATS ==========
void getStats() {
    cout << "{\"success\":true,\"stats\":{"
         << "\"totalChecks\":" << totalChecks << ","
         << "\"breachedCount\":" << breachedCount << ","
         << "\"safeCount\":" << safeCount
         << "}}" << endl;
}

// ========== CLEAR HISTORY ==========
void clearHistory() {
    while (historyStack != NULL) {
        StackNode* temp = historyStack;
        historyStack = historyStack->next;
        delete temp;
    }
    saveHistory();
    cout << "{\"success\":true}" << endl;
}

// ========== GET HASH TABLE ==========
void getHashTable() {
    cout << "{\"success\":true,\"hashTable\":[";

    for (int i = 0; i < HASH_SIZE; i = i + 1) {
        if (i > 0) {
            cout << ",";
        }
        cout << "[";

        HashNode* temp = hashTable[i];
        bool first = true;
        while (temp != NULL) {
            if (!first) {
                cout << ",";
            }
            first = false;
            cout << "{\"password\":\"" << temp->password
                 << "\",\"status\":\"" << temp->status << "\"}";
            temp = temp->next;
        }
        cout << "]";
    }
    cout << "]}" << endl;
}

void showCommandMenu() {
    cerr << "==========================================" << endl;
    cerr << " CyberVault Backend Server Ready" << endl;
    cerr << " Files in use: database.txt, history.txt, analytics.txt" << endl;
    cerr << "==========================================" << endl;
    cerr << "Available Commands (type and press Enter):" << endl;
    cerr << "  CHECK_PASSWORD <password>" << endl;
    cerr << "  ADD_PASSWORD <password> <Safe|Breached>" << endl;
    cerr << "  GET_HISTORY" << endl;
    cerr << "  GET_STATS" << endl;
    cerr << "  GET_HASHTABLE" << endl;
    cerr << "  CLEAR_HISTORY" << endl;
    cerr << "  EXIT" << endl;
    cerr << "------------------------------------------" << endl;
    cerr << "Example: CHECK_PASSWORD ali999" << endl;
    cerr << "==========================================" << endl;
}

// ========== MAIN ==========
int main() {
    loadDatabase();
    loadAnalytics();
    loadHistory();

    showCommandMenu();

    string line;
    while (getline(cin, line)) {
        if (line.empty()) {
            continue;
        }

        stringstream ss(line);
        string cmd;
        ss >> cmd;

        if (cmd == "CHECK_PASSWORD") {
            string pass;
            ss >> pass;
            string result = checkPassword(pass);
            saveAnalytics();
            saveHistory();
            cout << "{\"success\":true,\"result\":\"" << result << "\"}" << endl;
        }
        else if (cmd == "ADD_PASSWORD") {
            string pass, stat;
            ss >> pass >> stat;
            addPassword(pass, stat);
            saveHistory();
        }
        else if (cmd == "GET_HISTORY") {
            displayHistory();
        }
        else if (cmd == "GET_STATS") {
            getStats();
        }
        else if (cmd == "GET_HASHTABLE") {
            getHashTable();
        }
        else if (cmd == "CLEAR_HISTORY") {
            clearHistory();
        }
        else if (cmd == "EXIT") {
            break;
        }
        else {
            cout << "{\"error\":\"Unknown command\"}" << endl;
        }

        cout.flush();
    }

    return 0;
}