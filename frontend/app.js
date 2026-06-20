const API_BASE = "http://127.0.0.1:5000";

const searchInput = document.getElementById("search-input");
const autocompleteDropdown = document.getElementById("autocomplete-dropdown");
const resultsContainer = document.getElementById("results-container");
const resultsMeta = document.getElementById("results-meta");
const statsLine = document.getElementById("stats-line");

// ---------------------------------------------------------------
// WHY debouncing is needed:
// Without it, EVERY keystroke would immediately fire a network
// request to /autocomplete. Typing "neural" (6 letters) would fire
// 6 separate requests in under a second, most of which become
// instantly outdated as soon as the next letter is typed. This
// wastes server resources and can even cause results to arrive
// out of order (a slow response for "n" arriving AFTER a fast
// response for "neu", overwriting the more relevant result).
//
// Debouncing works by waiting a short pause (here, 250ms) after
// the LAST keystroke before actually firing the request. If the
// user keeps typing, each new keystroke cancels and restarts the
// timer — so a request only actually fires once the user pauses,
// even briefly. This is one of the most common patterns in any
// real search-as-you-type UI (Google, GitHub search, etc. all do
// some version of this).
// ---------------------------------------------------------------
function debounce(func, delayMs) {
    let timeoutId;
    return function (...args) {
        clearTimeout(timeoutId);
        timeoutId = setTimeout(() => func.apply(this, args), delayMs);
    };
}

async function fetchAutocomplete(prefix) {
    if (prefix.length === 0) {
        hideDropdown();
        return;
    }

    try {
        const response = await fetch(`${API_BASE}/autocomplete?prefix=${encodeURIComponent(prefix)}`);
        const data = await response.json();
        renderSuggestions(data.suggestions);
    } catch (error) {
        console.error("Autocomplete request failed:", error);
        hideDropdown();
    }
}

function renderSuggestions(suggestions) {
    if (!suggestions || suggestions.length === 0) {
        hideDropdown();
        return;
    }

    autocompleteDropdown.innerHTML = "";

    suggestions.forEach((word) => {
        const item = document.createElement("div");
        item.className = "suggestion-item";
        item.textContent = word;

        // Clicking a suggestion fills the search box and triggers a real search
        item.addEventListener("click", () => {
            searchInput.value = word;
            hideDropdown();
            performSearch(word);
        });

        autocompleteDropdown.appendChild(item);
    });

    autocompleteDropdown.style.display = "block";
}

function hideDropdown() {
    autocompleteDropdown.style.display = "none";
    autocompleteDropdown.innerHTML = "";
}

async function performSearch(query) {
    if (!query.trim()) {
        resultsContainer.innerHTML = "";
        resultsMeta.textContent = "";
        return;
    }

    resultsMeta.textContent = "Searching...";

    try {
        const response = await fetch(`${API_BASE}/search?q=${encodeURIComponent(query)}`);
        const data = await response.json();
        renderResults(data);
    } catch (error) {
        console.error("Search request failed:", error);
        resultsMeta.textContent = "Search failed — is the server running?";
    }
}

function renderResults(data) {
    resultsContainer.innerHTML = "";

    if (data.results.length === 0) {
        resultsMeta.textContent = `No results for "${data.query}"`;
        return;
    }

    resultsMeta.textContent = `${data.results.length} result(s) for "${data.query}"`;

    data.results.forEach((result) => {
        const card = document.createElement("div");
        card.className = "result-card";
        card.innerHTML = `
            <span class="result-filename">${result.filename}</span>
            <span class="result-score">score ${result.score}</span>
        `;
        resultsContainer.appendChild(card);
    });
}

async function loadStats() {
    try {
        const response = await fetch(`${API_BASE}/stats`);
        const data = await response.json();
        statsLine.textContent = `${data.total_documents} documents indexed · ${data.vocabulary_size} unique words`;
    } catch (error) {
        statsLine.textContent = "Could not load index stats — is the server running?";
    }
}

// ---------------------------------------------------------------
// EVENT WIRING
// ---------------------------------------------------------------

// Autocomplete: debounced, fires 250ms after the user stops typing
const debouncedAutocomplete = debounce((value) => {
    fetchAutocomplete(value.toLowerCase());
}, 250);

searchInput.addEventListener("input", (e) => {
    debouncedAutocomplete(e.target.value);
});

// Pressing Enter triggers an actual search immediately (not debounced —
// the user explicitly signaled "I'm done typing, search now")
searchInput.addEventListener("keydown", (e) => {
    if (e.key === "Enter") {
        hideDropdown();
        performSearch(searchInput.value);
    }
});

// Clicking outside the search box / dropdown should close the dropdown
document.addEventListener("click", (e) => {
    if (!e.target.closest(".search-wrapper")) {
        hideDropdown();
    }
});

// Load index stats once when the page first loads
loadStats();