import os
import sys

# WHY this needs to run before importing search_engine_cpp:
# our .pyd was compiled with MinGW-w64, so it depends on MinGW's runtime
# DLLs at import time — same issue we hit on Day 4. Since this server
# might be started fresh (a new Python process) at any point, we must
# register the DLL directory every time, not just once in a leftover
# interactive shell.
if sys.platform == "win32":
    os.add_dll_directory(r"C:\msys64\mingw64\bin")

import search_engine_cpp
from flask import Flask, request, jsonify
from flask_cors import CORS
import glob
from flask import send_from_directory

app = Flask(__name__)
CORS(app)  # allows requests from a frontend running on a different port

# ---------------------------------------------------------------------
# GLOBAL STATE: built ONCE when the server starts, reused across requests
# WHY globals here, despite generally being discouraged in larger apps:
# the index and trie represent the "loaded state" of our search engine,
# conceptually similar to a database connection — expensive to set up,
# meant to be reused for the server's entire lifetime. For a small
# single-process learning project, this is a reasonable, common pattern.
# (In a production system with multiple worker processes, you'd need a
# different strategy — e.g. a shared external index store — since each
# worker process would otherwise build its own separate copy in memory.)
# ---------------------------------------------------------------------
index = search_engine_cpp.InvertedIndex()
trie = search_engine_cpp.Trie()
doc_id_to_filename = {}  # maps doc_id -> original PDF filename, for display


def extract_text_from_pdf(pdf_path: str) -> str:
    """Calls pdftotext as a subprocess, same approach as the C++ version,
    but done here in Python since this is just I/O glue code, not
    performance-critical logic — no need to involve C++ for this part."""
    import subprocess
    result = subprocess.run(
        ["pdftotext", "-layout", pdf_path, "-"],
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="ignore",
    )
    return result.stdout


def ingest_pdf(pdf_path: str, doc_id: int):
    """Extracts text from a PDF and feeds every word into both the
    index and the trie. This mirrors what build_index.cpp did, but
    orchestrated from Python since it's a one-time setup step, not
    something that needs to run at C++ speed repeatedly."""
    text = extract_text_from_pdf(pdf_path)
    words = search_engine_cpp.tokenize(text)

    for word in words:
        index.add_term(word, doc_id, 1)  # page tracking simplified for now
        trie.insert(word)

    doc_id_to_filename[doc_id] = os.path.basename(pdf_path)


def build_index_from_data_folder():
    """Runs once at server startup: finds every PDF in data/, and
    ingests each one with a unique doc_id."""
    pdf_files = glob.glob("../data/*.pdf")

    for i, pdf_path in enumerate(pdf_files):
        doc_id = i + 1
        print(f"Ingesting doc {doc_id}: {pdf_path}")
        ingest_pdf(pdf_path, doc_id)

    index.set_total_documents(len(pdf_files))
    print(f"Indexed {len(pdf_files)} documents. Vocabulary size: {index.vocabulary_size()}")


# ---------------------------------------------------------------------
# ENDPOINTS
# ---------------------------------------------------------------------
@app.route("/")
def serve_index():
    return send_from_directory("../frontend", "index.html")


@app.route("/app.js")
def serve_js():
    return send_from_directory("../frontend", "app.js")

@app.route("/search", methods=["GET"])
def search():
    """Expects a query string parameter, e.g. /search?q=neural+networks
    Returns ranked documents as JSON."""
    query = request.args.get("q", "")

    if not query:
        return jsonify({"error": "Missing query parameter 'q'"}), 400

    scorer = search_engine_cpp.BM25Scorer(index)
    results = scorer.rank_documents(query)

    # Convert C++ (doc_id, score) pairs into JSON-friendly dictionaries,
    # and attach the filename for display purposes
    response = []
    for doc_id, score in results:
        response.append({
            "doc_id": doc_id,
            "filename": doc_id_to_filename.get(doc_id, "unknown"),
            "score": round(score, 4),
        })

    return jsonify({"query": query, "results": response})


@app.route("/autocomplete", methods=["GET"])
def autocomplete():
    """Expects a prefix string parameter, e.g. /autocomplete?prefix=neur
    Returns a list of suggested completions."""
    prefix = request.args.get("prefix", "").lower()

    if not prefix:
        return jsonify({"error": "Missing query parameter 'prefix'"}), 400

    suggestions = trie.autocomplete(prefix, 5)
    return jsonify({"prefix": prefix, "suggestions": suggestions})


@app.route("/stats", methods=["GET"])
def stats():
    """Returns basic information about the currently loaded index —
    useful for sanity-checking the server state, and for the frontend
    to display something like "X documents indexed"."""
    return jsonify({
        "total_documents": index.get_total_documents(),
        "vocabulary_size": index.vocabulary_size(),
        "average_document_length": round(index.get_average_document_length(), 2),
        "documents": doc_id_to_filename,
    })


if __name__ == "__main__":
    build_index_from_data_folder()
    app.run(debug=True, port=5000)