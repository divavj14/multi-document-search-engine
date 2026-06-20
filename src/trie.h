#ifndef TRIE_H
#define TRIE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

// One node in the trie represents ONE character position.
// WHY unordered_map<char, TrieNode*> for children, not an array of 26:
// Using a fixed array of 26 (for a-z) wastes memory if most nodes only
// have 1-2 actual children (which is typical — most prefixes don't
// branch in 26 directions). A map only stores the children that
// actually exist. This is a real engineering tradeoff: array = faster
// lookup but wasteful memory; map = slightly slower lookup but
// proportional memory use. For a search engine vocabulary (not just
// lowercase English letters — could include numbers, etc.), map is
// the safer, more general choice.
struct TrieNode {
    std::unordered_map<char, TrieNode*> children;

    // true if a complete word ends exactly at this node
    // (needed because "near" being a prefix of "neard" — if that
    // existed — shouldn't mean "near" itself isn't also a valid word)
    bool is_end_of_word = false;

    // Stores frequency — how many times this exact word was indexed.
    // WHY: lets autocomplete suggestions be ranked by popularity later,
    // e.g. suggesting the most common completions first.
    int frequency = 0;
};

class Trie {
private:
    TrieNode* root_;

    // Recursively collects every complete word reachable below a given
    // node, building up the full word string as it descends.
    // WHY recursion fits naturally here: a trie IS a recursive structure
    // (each child is itself the root of a smaller trie) — so "collect
    // all words below this node" naturally decomposes into "collect
    // all words below each child, prefixed appropriately."
    void collect_words(TrieNode* node, std::string current_word,
                        std::vector<std::pair<std::string, int>>& results) const {
        if (node->is_end_of_word) {
            results.emplace_back(current_word, node->frequency);
        }

        for (const auto& pair : node->children) {
            char ch = pair.first;
            TrieNode* child = pair.second;
            collect_words(child, current_word + ch, results);
        }
    }

public:
    Trie() {
        root_ = new TrieNode();
    }

    // Inserts one word into the trie, character by character.
    void insert(const std::string& word) {
        TrieNode* current = root_;

        for (char ch : word) {
            // If this character doesn't exist as a child yet, create it
            if (current->children.find(ch) == current->children.end()) {
                current->children[ch] = new TrieNode();
            }
            current = current->children[ch];
        }

        current->is_end_of_word = true;
        current->frequency++;
    }

    // Returns up to `limit` suggestions for a given prefix, sorted by
    // frequency (most common completions first).
    std::vector<std::string> autocomplete(const std::string& prefix, int limit = 5) const {
        TrieNode* current = root_;

        // Walk down the trie following the prefix, character by character
        for (char ch : prefix) {
            auto it = current->children.find(ch);
            if (it == current->children.end()) {
                return {};  // prefix doesn't exist in the trie at all
            }
            current = it->second;
        }

        // Collect every complete word below this point
        std::vector<std::pair<std::string, int>> matches;
        collect_words(current, prefix, matches);

        // Sort by frequency, descending (most common words first)
        std::sort(matches.begin(), matches.end(),
            [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
                return a.second > b.second;
            });

        // Extract just the words (drop frequency) and trim to `limit`
        std::vector<std::string> results;
        for (size_t i = 0; i < matches.size() && static_cast<int>(i) < limit; i++) {
            results.push_back(matches[i].first);
        }

        return results;
    }

    // Checks if an exact word exists in the trie
    bool contains(const std::string& word) const {
        TrieNode* current = root_;
        for (char ch : word) {
            auto it = current->children.find(ch);
            if (it == current->children.end()) {
                return false;
            }
            current = it->second;
        }
        return current->is_end_of_word;
    }
};

#endif