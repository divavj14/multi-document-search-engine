#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "posting.h"

class InvertedIndex {
private:
    std::unordered_map<std::string, std::vector<Posting>> index_;
    int total_documents_ = 0;

    // NEW: tracks how many words are in each document.
    // Key: doc_id, Value: total word count for that document.
    // WHY: BM25 needs each document's length to apply length normalization.
    std::unordered_map<int, int> doc_lengths_;

public:
    void add_term(const std::string& word, int doc_id, int page_number) {
        std::vector<Posting>& postings = index_[word];

        for (Posting& p : postings) {
            if (p.doc_id == doc_id && p.page_number == page_number) {
                p.frequency++;
                doc_lengths_[doc_id]++;   // NEW: count this word toward doc length
                return;
            }
        }

        postings.emplace_back(doc_id, page_number, 1);
        doc_lengths_[doc_id]++;            // NEW
    }

    std::vector<Posting> get_postings(const std::string& word) const {
        auto it = index_.find(word);
        if (it == index_.end()) {
            return {};
        }
        return it->second;
    }

    int document_frequency(const std::string& word) const {
        auto it = index_.find(word);
        if (it == index_.end()) {
            return 0;
        }
        return static_cast<int>(it->second.size());
    }

    void set_total_documents(int count) {
        total_documents_ = count;
    }

    int get_total_documents() const {
        return total_documents_;
    }

    size_t vocabulary_size() const {
        return index_.size();
    }

    // NEW: returns the length (word count) of a specific document.
    int get_document_length(int doc_id) const {
        auto it = doc_lengths_.find(doc_id);
        if (it == doc_lengths_.end()) {
            return 0;
        }
        return it->second;
    }

    // NEW: computes the average document length across the whole collection.
    // WHY: BM25's length-normalization term compares each document's length
    // to this average — documents longer than average get penalized slightly,
    // shorter ones get boosted slightly, correcting for the fact that long
    // documents naturally rack up more word occurrences just by being long.
    double get_average_document_length() const {
        if (doc_lengths_.empty()) {
            return 0.0;
        }
        int total_length = 0;
        for (const auto& pair : doc_lengths_) {
            total_length += pair.second;
        }
        return static_cast<double>(total_length) / doc_lengths_.size();
    }
};

#endif