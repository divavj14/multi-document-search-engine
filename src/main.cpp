#include <iostream>
#include <string>
#include <vector>

// This is your first C++ program for the search engine project.
// It demonstrates the basic concepts you'll use throughout:
// - vectors (like Python lists)
// - strings
// - loops
// - functions

// A function that takes a sentence and splits it into words
// In Python this would be: sentence.split()
std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string current_word;

    for (char c : text) {
        if (c == ' ' || c == '\n' || c == '\t') {
            // Hit a space — if we were building a word, save it
            if (!current_word.empty()) {
                tokens.push_back(current_word);
                current_word.clear();
            }
        } else {
            // Regular character — add to current word
            current_word += c;
        }
    }

    // Don't forget the last word (no trailing space)
    if (!current_word.empty()) {
        tokens.push_back(current_word);
    }

    return tokens;
}

int main() {
    std::string sentence = "the quick brown fox jumps over the lazy dog";

    std::cout << "Original sentence: " << sentence << std::endl;

    std::vector<std::string> words = tokenize(sentence);

    std::cout << "Word count: " << words.size() << std::endl;
    std::cout << "Words found:" << std::endl;

    for (int i = 0; i < words.size(); i++) {
        std::cout << "  [" << i << "] " << words[i] << std::endl;
    }

    return 0;
}