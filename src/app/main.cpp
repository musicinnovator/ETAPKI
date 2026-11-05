#include "etapki/core/catalog.h"
#include "etapki/core/analysis.h"
#include "etapki/core/integrity.h"
#include "etapki/core/midi.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::vector<etapki::core::Byte> readFile(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) return {};
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

void printCatalog() {
    using namespace etapki::core;
    std::cout << "ETAPKI baseline catalog\n";
    for (const auto& type : embellishmentTypes()) {
        std::cout << "- " << type.displayName << ": " << type.declaredVariantCount
                  << " selectable/declared, " << type.directImplementationBranchCount
                  << " direct implementation branches\n";
    }
    std::cout << "- MIDI analyzer labels: " << analysisLabels().size() << "\n";
}

std::vector<std::string> splitLabels(const std::string& csv) {
    std::vector<std::string> labels;
    std::stringstream stream(csv);
    std::string label;
    while (std::getline(stream, label, ',')) {
        if (!label.empty()) labels.push_back(label);
    }
    return labels;
}

int inspect(const std::string& path) {
    using namespace etapki::core;
    const auto bytes = readFile(path);
    if (bytes.empty()) {
        std::cerr << "Unable to read a non-empty MIDI file: " << path << '\n';
        return 2;
    }
    MidiError error;
    const auto document = MidiDocument::parse(bytes, error);
    if (!document) {
        std::cerr << "MIDI validation failed at byte " << error.offset << ": " << error.message << '\n';
        return 3;
    }
    std::cout << "MIDI file: " << path << '\n'
              << "Format: " << document->header().format << '\n'
              << "Tracks: " << document->tracks().size() << '\n'
              << "Division: " << document->header().division << '\n'
              << "Events: " << document->eventCount() << '\n';

    const auto outputBytes = document->serialize();
    const auto report = validateMidiExport(*document, outputBytes);
    std::cout << "Integrity gate: " << (report.valid ? "PASS" : "FAIL") << '\n';
    return report.valid ? 0 : 4;
}

int analyze(const std::string& path, const std::string& labelsCsv) {
    using namespace etapki::core;
    const auto bytes = readFile(path);
    if (bytes.empty()) {
        std::cerr << "Unable to read a non-empty MIDI file: " << path << '\n';
        return 2;
    }
    MidiError error;
    const auto document = MidiDocument::parse(bytes, error);
    if (!document) {
        std::cerr << "MIDI validation failed at byte " << error.offset << ": " << error.message << '\n';
        return 3;
    }
    const auto labels = splitLabels(labelsCsv);
    if (labels.empty()) {
        std::cerr << "At least one comma-separated analyzer label is required.\n";
        return 1;
    }
    AnalysisOptions options;
    options.selectedLabels = labels;
    options.onProgress = [](const AnalysisProgress& progress) {
        if (progress.label == "complete" || progress.completed == progress.total) {
            std::cout << "Analysis progress: " << progress.label << " 100%\n";
        }
    };
    const auto report = analyzeMidi(*document, options);
    for (const auto& method : report.methods) {
        std::cout << method.label << ": " << toString(method.state) << " - " << method.message << '\n';
    }
    std::cout << "Notes: " << report.notes.size() << "\n"
              << "Findings: " << report.findings.size() << "\n"
              << "Completed: " << (report.completed ? "yes" : "no") << '\n';
    return report.completed ? 0 : 5;
}

void interactive() {
    std::cout << "ETAPKI Phase 2 analyzer core\n"
              << "Commands: catalog | inspect <file.mid> | analyze <file.mid> <labels> | help | quit\n";
    printCatalog();
    std::string command;
    while (true) {
        std::cout << "etapki> " << std::flush;
        if (!std::getline(std::cin, command)) break;
        if (command == "quit" || command == "exit") break;
        if (command == "help") {
            std::cout << "catalog          Show the seven-type and 62-label catalog\n"
                      << "inspect <file>   Validate and inspect a MIDI file\n"
                      << "analyze <file> <labels>  Analyze comma-separated labels\n"
                      << "quit             Close ETAPKI\n";
            continue;
        }
        if (command == "catalog") {
            printCatalog();
            continue;
        }
        const std::string prefix = "inspect ";
        if (command.rfind(prefix, 0) == 0 && command.size() > prefix.size()) {
            std::string path = command.substr(prefix.size());
            if (path.size() >= 2 && path.front() == '"' && path.back() == '"') {
                path = path.substr(1, path.size() - 2);
            }
            (void)inspect(path);
            continue;
        }
        const std::string analyzePrefix = "analyze ";
        if (command.rfind(analyzePrefix, 0) == 0 && command.size() > analyzePrefix.size()) {
            const auto separator = command.find(' ', analyzePrefix.size());
            if (separator != std::string::npos) {
                (void)analyze(command.substr(analyzePrefix.size(), separator - analyzePrefix.size()),
                              command.substr(separator + 1));
            } else {
                std::cout << "Usage: analyze <file.mid> <label1,label2>\n";
            }
            continue;
        }
        if (!command.empty()) std::cout << "Unknown command. Type help.\n";
    }
    std::cout << "ETAPKI closed.\n";
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        interactive();
        return 0;
    }
    if (std::string(argv[1]) == "catalog") {
        printCatalog();
        return 0;
    }
    if (std::string(argv[1]) == "inspect" && argc == 3) return inspect(argv[2]);
    if (std::string(argv[1]) == "analyze" && argc == 4) return analyze(argv[2], argv[3]);
    std::cerr << "Usage: etapki_cli catalog | inspect <file.mid> | analyze <file.mid> <labels>\n";
    return 1;
}
