#include <iostream>
#include <string>
#include <algorithm> // For reverse function

using namespace std;

// Recursive function to check if s1 can be transformed into s2
bool isTransformation(string s1, string s2) {
    // Base case: if the two strings are equal, return true
    if (s1 == s2) return true;

    // If the strings are of different lengths, return false
    if (s1.length() != s2.length()) return false;

    int n = s1.length();

    // If the length of the string is 1 and they are not equal, return false
    if (n == 1) return s1 == s2;

    // Try splitting the strings at every possible index
    for (int i = 1; i < n; i++) {
        // Case 1: Check if the split parts match in order (directly)
        bool withoutSwap = isTransformation(s1.substr(0, i), s2.substr(0, i)) &&
                           isTransformation(s1.substr(i, n - i), s2.substr(i, n - i));

        // Case 2: Check if the split parts match after swapping (crossed)
        bool withSwap = isTransformation(s1.substr(0, i), s2.substr(n - i, i)) &&
                        isTransformation(s1.substr(i, n - i), s2.substr(0, n - i));

        // If either case is true, return true
        if (withoutSwap || withSwap) return true;
    }

    // If no transformation is found, return false
    return false;
}

int main() {
    // Input two strings
    string s1, s2;
    cin >> s1 >> s2;

    cout<< isTransformation(s1, s2)<<endl;
    // // Check if s2 can be obtained from s1
    // if (isTransformation(s1, s2)) {
    //     cout << "Yes, s2 can be obtained from s1." << endl;
    // } else {
    //     cout << "No, s2 cannot be obtained from s1." << endl;
    // }

    return 0;
}
