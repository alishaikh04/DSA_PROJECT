#include <iostream>
#include <fstream>
#include <sstream>
#include <string>


using namespace std;


const string PASSWORDS_FILE = "database.txt";
const string HISTORY_FILE = "history.txt";
const string ANALYTICS_FILE = "analytics.txt";


struct HashNode 
{
    string password;
    string status;
    HashNode* next;
};


struct StackNode 
{
    string operation;
    StackNode* next;
};


const int HASH_SIZE = 10;
HashNode* hashTable[HASH_SIZE] = {NULL};
StackNode* historyStack = NULL;
int totalChecks = 0;
int breachedCount = 0;
int safeCount = 0;

// ========== HASH FUNCTION ==========
int getHashIndex(string str) 
{
    int sum = 0;
    int size = str.length();
    
    if (size < 8)
    {
        return -1;
    }
 
         for (int i = 0; i < size; i++) 
    {
        sum = sum + (int)str[i];
    }
       return sum % HASH_SIZE;
}

// ========== VALIDATE PASSWORD ==========
bool isValidPassword(string password) 
{
    if (password.length() < 8) 
    {
        return false;
    }
    return true;
}

// ========== CHECK EXISTS ==========
bool exists(string password) 
{
    int index = getHashIndex(password);

    if (index == -1) 
    {
        return false;
    }

    HashNode* temp = hashTable[index];
    
    while (temp != NULL) 
    {
        if (temp->password == password) 
        {
            return true;
        }
        temp = temp->next;
    }
    return false;
}

// ========== INSERT INTO HASH TABLE ==========
void insertIntoHash(string password, string status) 
{
    if (exists(password)) 
    {
        return;
    }
    
    int index = getHashIndex(password);

    if (index == -1) 
    {
        return;
    }

    HashNode* newNode = new HashNode();
    newNode->password = password;
    newNode->status = status;
    newNode->next = hashTable[index];
    hashTable[index] = newNode;
}

// ========== STACK OPERATIONS ==========
void pushHistory(string operation) 
{
    StackNode* newNode = new StackNode();
    newNode->operation = operation;
    newNode->next = historyStack;
    historyStack = newNode;
}

// ========== DISPLAY HISTORY (CMD Version) ==========
void displayHistory() 
{
    if (historyStack == NULL) 
    {
        cout << endl;
        cout << "========== HISTORY ==========" << endl;
        cout << "No history found!" << endl;
        cout << "=============================" << endl;
        return;
    }
    
    cout << endl;
    cout << "========== HISTORY ==========" << endl;
    StackNode* temp = historyStack;
    int count = 1;
    
    while (temp != NULL) 
    {
        cout << count << ". " << temp->operation << endl;
        temp = temp->next;
        count++;
    }
    cout << "=============================" << endl;
}

// ========== CHECK PASSWORD ==========
string checkPassword(string password) 
{
    totalChecks = totalChecks + 1;

    if (!isValidPassword(password)) 
    {
        pushHistory("Checked: " + password + " (Invalid - Too Short)");
        return "Invalid: Password must be at least 8 characters long.";
    }
    
    int index = getHashIndex(password);

    if (index == -1) 
    {
        return "";
    }

    HashNode* temp = hashTable[index];
    
    while (temp != NULL) 
    {
        if (temp->password == password) 
        {
            if (temp->status == "Breached") 
            {
                breachedCount = breachedCount + 1;
                pushHistory("Checked: " + password + " (Breached)");
                return "Breached";
            } 
            else 
            {
                safeCount = safeCount + 1;
                pushHistory("Checked: " + password + " (Safe)");
                return "Safe";
            }
        }
        temp = temp->next;
    }
    
    safeCount = safeCount + 1;
    pushHistory("Checked: " + password + " (Safe - Not found)");
    return "Safe";
}

// ========== ADD PASSWORD (CMD Version) ==========
void addPassword(string password, string status) 
{

    if (status != "Breached" && status != "Safe") 
    {
        cout << "Error: Status must be 'Breached' or 'Safe'!" << endl;
        return;
    }

    if (!isValidPassword(password)) 
    {
        cout << "Error: Password must be at least 8 characters!" << endl;
        return;
    }
    
    if (exists(password)) 
    {
        cout << "Error: Password already exists in database!" << endl;
        return;
    }
    
    insertIntoHash(password, status);
    
    ofstream out(PASSWORDS_FILE, ios::app);
    out << password << "|" << status << endl;
    out.close();
    
    pushHistory("Added: " + password + " (" + status + ")");
    cout << "Password added successfully!" << endl;
}

// ========== SAVE ANALYTICS ==========
void saveAnalytics() 
{
    ofstream out(ANALYTICS_FILE);
    out << totalChecks << endl;
    out << breachedCount << endl;
    out << safeCount << endl;
    out.close();
}

void loadAnalytics() 
{
    ifstream in(ANALYTICS_FILE);
    if (in.is_open()) 
    {
        in >> totalChecks;
        in >> breachedCount;
        in >> safeCount;
        in.close();
    }
}

// ========== LOAD DATABASE ==========
void loadDatabase() 
{
    ifstream in(PASSWORDS_FILE);
    
    if (!in.is_open()) 
    {
        ofstream create(PASSWORDS_FILE);
        create << "password123|Breached" << endl;
        create << "admin1234|Breached" << endl;
        create << "qwerty1234|Breached" << endl;
        create << "ali999000|Safe" << endl;
        create << "newpass2024|Safe" << endl;
        create.close();
        in.open(PASSWORDS_FILE);
    }
    
    string line;
    while (getline(in, line)) 
    {
        if (line.empty()) 
        {
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
void loadHistory() 
{
    ifstream in(HISTORY_FILE);
    if (!in.is_open()) 
    {
        return;
    }
    
    string line;
    while (getline(in, line)) 
    
    {
        if (line.empty()) 
        {
            continue;
        }
        pushHistory(line);
    }
    in.close();
}

void saveHistory() 
{
    ofstream out(HISTORY_FILE, ios::trunc);
    
    // Create a copy of stack to preserve original
    StackNode* temp = historyStack;
    
    // Count items
    int count = 0;
    StackNode* counter = historyStack;
    while (counter != NULL) 
    {
        count = count + 1;
        counter = counter->next;
    }
    
    // Create array to store operations in order
    string* operations = new string[count];
    int index = 0;
    temp = historyStack;
    while (temp != NULL) 
    {
        operations[index] = temp->operation;
        index = index + 1;
        temp = temp->next;
    }
    
    // Write in reverse order (oldest first)
    for (int i = count - 1; i >= 0; i--) 
    {
        out << operations[i] << endl;
    }
    
    delete[] operations;
    out.close();
}

void displayStats() 
{
    cout << endl;
    cout << "========== ANALYTICS ==========" << endl;
    cout << "Total Password Checks: " << totalChecks << endl;
    cout << "Breached Passwords: " << breachedCount << endl;
    cout << "Safe Passwords: " << safeCount << endl;
    cout << "History Entries: ";
    
    int count = 0;
    StackNode* temp = historyStack;
    while (temp != NULL) 
    {
        count++;
        temp = temp->next;
    }
    cout << count << endl;
    cout << "================================" << endl;
}

// ========== CLEAR HISTORY ==========
void clearHistory() 
{
    while (historyStack != NULL) 
    {
        StackNode* temp = historyStack;
        historyStack = historyStack->next;
        delete temp;
    }
    saveHistory();
    cout << "History cleared successfully!" << endl;
}

// ========== DISPLAY HASH TABLE (CMD Version) ==========
void displayHashTable() 
{
    cout << endl;
    cout << "========== HASH TABLE ==========" << endl;
    
    for (int i = 0; i < HASH_SIZE; i = i + 1) 
    {
        cout << "Bucket " << i << ": ";
        
        HashNode* temp = hashTable[i];
        if (temp == NULL) 
        {
            cout << "Empty" << endl;
        } 
        else 
        {
            bool first = true;
            while (temp != NULL) 
            {
                if (!first) 
                {
                    cout << " -> ";
                }
                first = false;
                cout << "[" << temp->password << ":" << temp->status << "]";
                temp = temp->next;
            }
            cout << endl;
        }
    }
    cout << "=================================" << endl;
}


int main() {

    // Load all data
    cout << "Loading CyberVault..." << endl;

    loadDatabase();
    loadAnalytics();
    loadHistory();

    cout << "CyberVault Ready!" << endl;
    cout << "Default passwords loaded: password123, admin1234, qwerty1234, ali9990000, newpass2024" << endl;
    
    int choice;
    string password, status;
    
    while (true) {
        
    cout << endl;
    cout << "===== CYBERVAULT PASSWORD MANAGER =====" << endl;
    cout << "1. Check Password Security" << endl;
    cout << "2. Add New Password" << endl;
    cout << "3. Display Hash Table" << endl;
    cout << "4. View History" << endl;
    cout << "5. View Analytics" << endl;
    cout << "6. Clear History" << endl;
    cout << "7. Save All Data" << endl;
    cout << "8. Exit" << endl;
    cout << "=======================================" << endl;
    cout << "Enter your choice: ";

        cin >> choice;
        cin.ignore(); // Clear input buffer
        
        if  (choice == 1) 
            {
                cout << "Enter password to check: ";
                getline(cin, password);
                string result = checkPassword(password);
                saveAnalytics();
                saveHistory();
                cout << "Result: " << result << "!" << endl;
            }
            
        else if (choice == 2) 
            { 
                // Add New Password
                cout << "Enter password to add: ";
                getline(cin, password);
                cout << "Enter status (Breached/Safe): ";
                getline(cin, status);
                addPassword(password, status);
                saveHistory();
            }
            
        else if (choice == 3) 
            {
                displayHashTable();
            }
                
        else if (choice == 4) 
            {
                displayHistory();
            }    
                
        else if (choice == 5) 
            {  
                displayStats();
            }

        else if (choice == 6) 
            {  
                clearHistory();
            } 
                
        else if (choice == 7) 
            {
                saveAnalytics();
                saveHistory();
                cout << "All data saved successfully!" << endl;
            }
                  
        else if (choice == 8)  
            {   
                cout << "Saving and exiting..." << endl;
                saveAnalytics();
                saveHistory();
                cout << "Goodbye!" << endl;
                break;
            }
                
            else 
            {
                cout << "Invalid choice! Please try again." << endl;
            }
        }
    
    return 0;
}
