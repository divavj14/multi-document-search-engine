#ifndef BM25_H
#define BM25_H

#include <cmath>     // for std::log
#include <string>
#include <vector>
#include <unordered_map>
#include "inverted_index.h"
#include "tokenizer.h"

// BM25 tuning constants — these are the standard, widely-used defaults.
// k1 controls how quickly term frequency saturates (higher = slower saturation,
// meaning repeated occurrences keep contributing more before flattening out).
// b controls how strongly document length is penalized (0 = no length
// normalization at all, 1 = full normalization).
const double BM25_K1 = 1.5;
const double BM25_B = 0.75;

class BM25Scorer {
private:
    const InvertedIndex& index_;

public:
    // Takes a reference to an already-built InvertedIndex to score against.
    // WHY a reference, not a copy: the index could be large, and we never
    // need to modify it here — just read from it.
    BM25Scorer(const InvertedIndex& index) : index_(index) {}

    // Computes the IDF (Inverse Document Frequency) component for one word.
    // WHY this formula: words that appear in almost every document
    // (e.g. "the") get an IDF close to log(1) = ~0, contributing almost
    // nothing to the score. Rare words get a much higher IDF, since N/df
    // becomes large when df (document frequency) is small relative to N
    // (total documents).
    // The "+1" terms (Robertson-Sparck Jones smoothing) prevent division
    // by zero and avoid negative scores for very common words.
    double compute_idf(const std::string& word) const {
        int N = index_.get_total_documents();
        int df = index_.document_frequency(word);

        double numerator = N - df + 0.5;
        double denominator = df + 0.5;

        return std::log((numerator / denominator) + 1.0);
    }

    // Computes the full BM25 score for ONE document, given a tokenized query.
    double score_document(int doc_id, const std::vector<std::string>& query_terms) const {
        double score = 0.0;
        double doc_length = index_.get_document_length(doc_id);
        double avg_doc_length = index_.get_average_document_length();

        // Avoid division by zero if avg_doc_length is somehow 0
        if (avg_doc_length == 0.0) {
            return 0.0;
        }

        for (const std::string& term : query_terms) {
            // How many times does this term appear in THIS document?
            // We need to sum across all postings for this term that
            // belong to this doc_id (a term could appear on multiple pages).
            int f = 0;
            std::vector<Posting> postings = index_.get_postings(term);
            for (const Posting& p : postings) {
                if (p.doc_id == doc_id) {
                    f += p.frequency;
                }
            }

            if (f == 0) {
                continue;  // term doesn't appear in this document at all — contributes 0
            }

            double idf = compute_idf(term);

            double numerator = f * (BM25_K1 + 1.0);
            double denominator = f + BM25_K1 * (1.0 - BM25_B + BM25_B * (doc_length / avg_doc_length));

            score += idf * (numerator / denominator);
        }

        return score;
    }

    // Scores and ranks ALL documents that contain at least one query term,
    // returning a sorted list of (doc_id, score) pairs, highest score first.
    // WHY this approach: rather than scoring every document (even ones with
    // zero overlap with the query), we only consider documents that actually
    // contain at least one query word — found by checking the postings of
    // each query term. This avoids wasted computation on obviously irrelevant
    // documents.
    std::vector<std::pair<int, double>> rank_documents(const std::string& query) const {
        std::vector<std::string> query_terms = tokenize(query);

        // Collect every distinct doc_id that contains at least one query term
        std::unordered_map<int, bool> candidate_docs;
        for (const std::string& term : query_terms) {
            std::vector<Posting> postings = index_.get_postings(term);
            for (const Posting& p : postings) {
                candidate_docs[p.doc_id] = true;
            }
        }

        // Score each candidate document
        std::vector<std::pair<int, double>> results;
        for (const auto& pair : candidate_docs) {
            int doc_id = pair.first;
            double score = score_document(doc_id, query_terms);
            results.emplace_back(doc_id, score);
        }

        // Sort by score, descending (best matches first)
        std::sort(results.begin(), results.end(),
            [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
                return a.second > b.second;
            });

        return results;
    }
};

#endif