# Multi-Document Full-Text Search Engine

A search engine built from scratch — inverted index, BM25 ranking, and trie-based autocomplete implemented in C++, exposed to a Python Flask API via pybind11, with a live browser frontend for search and autocomplete.

Paste in a set of PDFs, get back ranked, relevant results — the same core algorithm family that powered Elasticsearch's default ranking for years, implemented from the underlying formula rather than a library call.

## Features

- **PDF ingestion** — extracts text from real PDF documents using Poppler (`pdftotext`)
- **Inverted index** — maps every word to the documents and pages it appears in, built from scratch in C++
- **BM25 ranking** — scores and ranks documents by relevance using inverse document frequency, saturating term frequency, and document-length normalization
- **Trie-based autocomplete** — prefix-based search-as-you-type suggestions, ranked by word frequency
- **REST API** — Flask endpoints (`/search`, `/autocomplete`, `/stats`) backed entirely by the C++ engine
- **Browser frontend** — live search with debounced autocomplete, built in vanilla HTML/CSS/JS

## Architecture

```
PDF files
   |
   v
pdftotext (Poppler)  -->  plain text
   |
   v
C++ engine (compiled to a Python extension via pybind11)
   |--- Inverted index   (word -> doc, page, frequency)
   |--- BM25 ranker      (relevance scoring)
   |--- Trie             (prefix -> autocomplete suggestions)
   |
   v
Flask REST API (Python)
   |
   v
Browser frontend (HTML / CSS / vanilla JS)
```

The performance-critical logic — indexing, ranking, prefix search — runs as compiled C++. Python is used only for the thin orchestration layer (the API and PDF-ingestion glue), the same architectural pattern used by libraries like NumPy and PyTorch.

## Tech stack

| Layer | Technology |
|---|---|
| Core engine | C++17 |
| Python bridge | pybind11 |
| Build system | CMake + MinGW-w64 (g++) |
| PDF parsing | Poppler (`pdftotext`) |
| API | Flask + flask-cors |
| Frontend | HTML, CSS, vanilla JavaScript |

## How it works

**Indexing.** Every PDF in `data/` is converted to plain text, tokenized into lowercase alphanumeric words, and fed into two structures: an inverted index (`word -> [(doc_id, page, frequency), ...]`) and a trie (for prefix lookups). This happens once, when the Flask server starts — not on every request.

**Ranking.** A search query is scored against every candidate document using BM25:

```
score(D, Q) = Σ over each word in Q of:
    IDF(word) × [ f × (k1 + 1) ] / [ f + k1 × (1 - b + b × (docLen / avgDocLen)) ]
```

Rare words contribute more to the score than common ones (IDF), repeated word occurrences help but with diminishing returns (saturating term frequency), and longer documents are normalized against the collection's average length so they don't win purely by being long.

**Autocomplete.** The trie answers "what words start with this prefix?" in time proportional to the prefix length, independent of vocabulary size — the same prefix-tree approach used by real autocomplete systems.

## Running it locally

### Prerequisites
- Windows with MinGW-w64 (g++) and CMake
- Python 3.10+
- [Poppler for Windows](https://github.com/oschwartz10612/poppler-windows/releases) (`pdftotext` on PATH)

### Build the C++ engine

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

This produces `search_engine_cpp.<tag>.pyd` — copy it into `api/`.

### Install Python dependencies

```bash
pip install flask flask-cors pybind11
```

### Add documents

Drop one or more PDFs into `data/`.

### Run the server

```bash
cd api
python app.py
```

Open `http://127.0.0.1:5000/` in a browser.

## API reference

**`GET /search?q=<query>`**
Returns documents ranked by BM25 relevance.
```json
{
  "query": "neural networks",
  "results": [
    {"doc_id": 2, "filename": "doc2.pdf", "score": 1.1366}
  ]
}
```

**`GET /autocomplete?prefix=<prefix>`**
Returns up to 5 suggested completions, ranked by frequency.
```json
{ "prefix": "hack", "suggestions": ["hackerearth", "hackers"] }
```

**`GET /stats`**
Returns the current index's loaded state.
```json
{ "total_documents": 3, "vocabulary_size": 4982, "average_document_length": 26528.0 }
```

## Known limitations

This is a learning project, and a few simplifications were made deliberately:

- The trie does not free its allocated nodes (no destructor) — acceptable for short-lived runs, would need `std::unique_ptr`-based children for production use
- BM25 scoring re-scans postings per call rather than caching per-document term frequencies
- No incremental indexing — adding a document requires restarting the server
- No automated test suite — correctness was verified with hand-crafted test cases designed so the expected output could be predicted in advance

## Project structure

```
.
├── src/          C++ source (inverted index, BM25, trie, PDF extraction, pybind11 bindings)
├── api/          Flask application
├── frontend/     Browser UI
├── data/         PDF documents to index
└── CMakeLists.txt
```
