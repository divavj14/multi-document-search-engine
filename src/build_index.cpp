#include <iostream>
#include <string>
#include <cstdio>
#include <array>
#include <stdexcept>
#include "tokenizer.h"
#include "inverted_index.h"
#include "trie.h"

std::string run_command(const std::string& command) {
    std::array<char, 256> buffer;
    std::string result;
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Failed to run command: " + command);
    }
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    _pclose(pipe);
    return result;
}

std::string extract_text_from_pdf(const std::string& pdf_path) {
    std::string command = "pdftotext -layout \"" + pdf_path + "\" -";
    return run_command(command);
}

int main() {
    InvertedIndex index;
    Trie trie;  // NEW

    std::string pdf_path = "data/test1.pdf";
    int doc_id = 1;
    int page_number = 1;

    std::cout << "Extracting and indexing: " << pdf_path << std::endl;

    std::string text = extract_text_from_pdf(pdf_path);
    std::vector<std::string> words = tokenize(text);

    std::cout << "Total words extracted: " << words.size() << std::endl;

    for (const std::string& word : words) {
        index.add_term(word, doc_id, page_number);
        trie.insert(word);  // NEW — feed every word into the trie too
    }

    index.set_total_documents(1);

    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Vocabulary size (unique words): " << index.vocabulary_size() << std::endl;

    // NEW — test autocomplete on real document content.
    // Pick a prefix you know exists in YOUR PDF based on earlier days
    // (e.g. if your document mentions "hackerearth", try "hack")
    std::string test_prefix = "hack";
    std::cout << "Autocomplete for \"" << test_prefix << "\":" << std::endl;
    std::vector<std::string> suggestions = trie.autocomplete(test_prefix);
    for (const std::string& s : suggestions) {
        std::cout << "  " << s << std::endl;
    }

    return 0;
}