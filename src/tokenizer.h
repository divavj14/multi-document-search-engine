#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
#include <cctype>

// Splits text into lowercase words, stripping punctuation.
// WHY lowercase: so that "Neural" and "neural" are treated as the SAME
// word in the index. Without this, a search for "neural" would miss
// any document that happened to capitalize it differently.
//
// WHY strip punctuation: so "networks," and "networks" are the same word.
// Otherwise the comma becomes part of the token, and they'd be indexed
// as two completely different words.
inline std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string current_word;

    for (char c : text) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            // isalnum = "is alphanumeric" (a letter or digit)
            current_word += std::tolower(static_cast<unsigned char>(c));
        } else {
            // Any non-alphanumeric character (space, comma, period, etc.)
            // ends the current word
            if (!current_word.empty()) {
                tokens.push_back(current_word);
                current_word.clear();
            }
        }
    }

    if (!current_word.empty()) {
        tokens.push_back(current_word);
    }

    return tokens;
}

#endif