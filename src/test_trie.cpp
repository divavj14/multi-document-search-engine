#include <iostream>
#include "trie.h"

int main() {
    Trie trie;

    // Insert a small vocabulary, some words multiple times to simulate
    // "popularity" via frequency
    trie.insert("near");
    trie.insert("neural");
    trie.insert("neural");
    trie.insert("neural");
    trie.insert("network");
    trie.insert("network");
    trie.insert("networking");
    trie.insert("net");

    std::cout << "Testing prefix: \"ne\"" << std::endl;
    std::vector<std::string> results = trie.autocomplete("ne");
    for (const std::string& word : results) {
        std::cout << "  " << word << std::endl;
    }

    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Testing prefix: \"net\"" << std::endl;
    results = trie.autocomplete("net");
    for (const std::string& word : results) {
        std::cout << "  " << word << std::endl;
    }

    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Testing prefix: \"xyz\" (should not exist)" << std::endl;
    results = trie.autocomplete("xyz");
    std::cout << "  Results found: " << results.size() << std::endl;

    std::cout << "----------------------------------------" << std::endl;
    std::cout << "contains(\"neural\"): " << trie.contains("neural") << std::endl;
    std::cout << "contains(\"neur\"): " << trie.contains("neur") << std::endl;

    return 0;
}