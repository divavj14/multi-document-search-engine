#include <iostream>
#include "inverted_index.h"
#include "bm25.h"
#include "tokenizer.h"

// Helper: indexes one "document" (just a string of text) under a given doc_id
void index_document(InvertedIndex& index, int doc_id, const std::string& text) {
    std::vector<std::string> words = tokenize(text);
    for (const std::string& word : words) {
        index.add_term(word, doc_id, 1);  // page 1 for all, simplification for this test
    }
}

int main() {
    InvertedIndex index;

    // Three short hand-crafted documents.
    // Doc 1: mentions "neural" once, otherwise unrelated
    // Doc 2: heavily about neural networks — should rank HIGHEST for "neural networks"
    // Doc 3: not about the topic at all — should rank LOWEST
    index_document(index, 1, "the cat sat on the mat near a neural device");
    index_document(index, 2, "neural networks and neural network architectures are powerful neural tools for learning");
    index_document(index, 3, "the weather today is sunny with a chance of rain in the evening");

    index.set_total_documents(3);

    BM25Scorer scorer(index);

    std::string query = "neural networks";
    std::cout << "Query: \"" << query << "\"" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    std::vector<std::pair<int, double>> results = scorer.rank_documents(query);

    for (const auto& result : results) {
        std::cout << "Doc " << result.first << " — score: " << result.second << std::endl;
    }

    return 0;
}