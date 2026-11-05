#include "etapki/core/catalog.h"
#include "etapki/core/integrity.h"
#include "etapki/core/midi.h"

#include <fstream>
#include <iostream>
#include <iterator>
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

void interactive() {
    std::cout << "ETAPKI Phase 1 interactive core\n"
              << "Commands: catalog | inspect <file.mid> | help | quit\n";
    printCatalog();
    std::string command;
    while (true) {
        std::cout << "etapki> " << std::flush;
        if (!std::getline(std::cin, command)) break;
        if (command == "quit" || command == "exit") break;
        if (command == "help") {
            std::cout << "catalog          Show the seven-type and 62-label catalog\n"
                      << "inspect <file>   Validate and inspect a MIDI file\n"
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
    std::cerr << "Usage: etapki_cli catalog | inspect <file.mid>\n";
    return 1;
}
