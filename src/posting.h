#ifndef POSTING_H
#define POSTING_H

#include <string>

// A "Posting" represents ONE occurrence-record of a word in a document.
// WHY a struct instead of just storing numbers separately:
// Without this, you'd need 3 separate parallel arrays (doc_ids[], pages[],
// freqs[]) and have to manually keep them in sync — very bug-prone.
// Bundling related data into one struct is a core C++/engineering habit.
struct Posting {
    int doc_id;       // which document (we'll assign each document a number)
    int page_number;  // which page within that document
    int frequency;     // how many times the word appeared on that page

    // Constructor: lets us write Posting(1, 2, 3) instead of manually
    // setting each field one by one
    Posting(int doc, int page, int freq)
        : doc_id(doc), page_number(page), frequency(freq) {}
};

#endif