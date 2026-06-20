#include <iostream>
#include <string>
#include <cstdio>      // for popen, pclose
#include <array>
#include <stdexcept>

// Runs a shell command and captures everything it prints to stdout.
// This is how we'll call pdftotext from inside C++.
//
// WHY this approach: pdftotext is a battle-tested, mature tool.
// Re-implementing PDF parsing in C++ ourselves would take weeks and
// be far more bug-prone than calling a tool that already does it well.
std::string run_command(const std::string& command) {
    std::array<char, 256> buffer;
    std::string result;

    // _popen runs a command and gives us a stream we can read its output from.
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

// Extracts plain text from a PDF file using pdftotext.
std::string extract_text_from_pdf(const std::string& pdf_path) {
    std::string command = "pdftotext -layout \"" + pdf_path + "\" -";
    return run_command(command);
}

int main() {
    std::string pdf_path = "data/test1.pdf";

    std::cout << "Extracting text from: " << pdf_path << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    try {
        std::string extracted_text = extract_text_from_pdf(pdf_path);
        std::cout << extracted_text << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "Extraction successful. Total characters: "
                   << extracted_text.size() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}