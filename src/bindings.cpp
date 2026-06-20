#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "inverted_index.h"
#include "posting.h"
#include "tokenizer.h"
#include "bm25.h"
#include "trie.h"

namespace py = pybind11;

PYBIND11_MODULE(search_engine_cpp, m) {
    m.doc() = "C++ search engine core, exposed to Python via pybind11";

    py::class_<Posting>(m, "Posting")
        .def(py::init<int, int, int>())
        .def_readonly("doc_id", &Posting::doc_id)
        .def_readonly("page_number", &Posting::page_number)
        .def_readonly("frequency", &Posting::frequency);

    py::class_<InvertedIndex>(m, "InvertedIndex")
        .def(py::init<>())
        .def("add_term", &InvertedIndex::add_term)
        .def("get_postings", &InvertedIndex::get_postings)
        .def("document_frequency", &InvertedIndex::document_frequency)
        .def("set_total_documents", &InvertedIndex::set_total_documents)
        .def("get_total_documents", &InvertedIndex::get_total_documents)
        .def("vocabulary_size", &InvertedIndex::vocabulary_size)
        .def("get_document_length", &InvertedIndex::get_document_length)
        .def("get_average_document_length", &InvertedIndex::get_average_document_length);

    py::class_<BM25Scorer>(m, "BM25Scorer")
        .def(py::init<const InvertedIndex&>())
        .def("compute_idf", &BM25Scorer::compute_idf)
        .def("score_document", &BM25Scorer::score_document)
        .def("rank_documents", &BM25Scorer::rank_documents);

    // NEW: expose Trie to Python
    py::class_<Trie>(m, "Trie")
        .def(py::init<>())
        .def("insert", &Trie::insert)
        .def("autocomplete", &Trie::autocomplete, py::arg("prefix"), py::arg("limit") = 5)
        .def("contains", &Trie::contains);

    m.def("tokenize", &tokenize, "Splits text into lowercase alphanumeric tokens");
}