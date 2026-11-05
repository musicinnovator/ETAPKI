//  Snap Transformation (C) 2025 <Roderick Harris>
// Enhanced version with percentage control for transformations
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <iostream>
#include <iomanip> // For std::setw formatting
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <chrono>
#include <random>

// Helper to get note name (from MIDI number)
std::string getNoteName(int noteNumber) {
    static const std::string noteNames[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    int octave = noteNumber / 12 - 1;
    int noteIndex = noteNumber % 12;
    return noteNames[noteIndex] + std::to_string(octave);
}

// Helper to get MIDI number from note name
int getNoteNumber(const std::string& noteName) {
    static const std::string noteNames[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

    // Extract note and octave
    std::string baseNote = noteName.substr(0, noteName.size() - 1);
    int octave = std::stoi(noteName.substr(noteName.size() - 1));

    // Find base note index
    auto it = std::find(std::begin(noteNames), std::end(noteNames), baseNote);
    if (it == std::end(noteNames)) {
        throw std::invalid_argument("Invalid note name: " + noteName);
    }
    int noteIndex = std::distance(std::begin(noteNames), it);

    return (octave + 1) * 12 + noteIndex;
}

// Enum for TimeMeter
enum TimeMeter {
    DUPLE,
    TRIPLE
};

// Helper function for SP1 - principal and upper auxiliary (1 whole step above) played very rapidly
void handleSnap1(std::vector<std::pair<int, int>>& EmbRet, int principal, int upper, int durPi, TimeMeter meter) {
    // Ensure durPi is positive and greater than zero to avoid mathematical issues
    if (durPi <= 0) {
        return; // Invalid duration; exit early to avoid incorrect logic
    }

    if (meter == DUPLE) {
        // If DUPLE meter is specified - rapid alternation between principal and upper auxiliary
        int durPiece = durPi / 16; // Very rapid execution
        int remainingDuration = durPi - (2 * durPiece);
        EmbRet.push_back({principal, durPiece}); // Principal note (very brief)
        EmbRet.push_back({upper, durPiece});     // Upper auxiliary (1 whole step above, very brief)
        EmbRet.push_back({principal, remainingDuration}); // Resolve to principal for remaining duration

    } else if (meter == TRIPLE) {
        // If TRIPLE meter is specified
        int durPiece = durPi / 12; // Very rapid for triple meter
        int remainingDuration = durPi - (2 * durPiece);
        EmbRet.push_back({principal, durPiece}); // Principal note
        EmbRet.push_back({upper, durPiece});     // Upper auxiliary (1 whole step above)
        EmbRet.push_back({principal, remainingDuration}); // Resolve to principal

    } else {
        // Handle unsupported or invalid meters as a fallback
        EmbRet.push_back({principal, durPi}); // Assign the entire duration to the principal note
    }
}

// Helper function for SP5 - principal and upper auxiliary (1/2 step above) played very rapidly
void handleSnap5(std::vector<std::pair<int, int>>& EmbRet, int principal, int upper, int durPi, TimeMeter meter) {
    if (durPi <= 0) {
        return;
    }

    if (meter == DUPLE) {
        // Rapid alternation between principal and upper auxiliary (1/2 step above)
        int durPiece = durPi / 16; // Very rapid execution
        int remainingDuration = durPi - (2 * durPiece);
        EmbRet.push_back({principal, durPiece}); // Principal note (very brief)
        EmbRet.push_back({upper, durPiece});     // Upper auxiliary (1/2 step above, very brief)
        EmbRet.push_back({principal, remainingDuration}); // Resolve to principal

    } else if (meter == TRIPLE) {
        int durPiece = durPi / 12; // Very rapid for triple meter
        int remainingDuration = durPi - (2 * durPiece);
        EmbRet.push_back({principal, durPiece}); // Principal note
        EmbRet.push_back({upper, durPiece});     // Upper auxiliary (1/2 step above)
        EmbRet.push_back({principal, remainingDuration}); // Resolve to principal

    } else {
        EmbRet.push_back({principal, durPi});
    }
}

// Helper function for SPvm3 - principal and upper auxiliary (m3 above) played very rapidly
void handleSnapMinor3(std::vector<std::pair<int, int>>& EmbRet, int principal, int upper, int durPi, TimeMeter meter) {
    if (durPi <= 0) {
        return;
    }

    if (meter == DUPLE) {
        // Rapid alternation between principal and upper auxiliary (minor 3rd above)
        int durPiece = durPi / 16; // Very rapid execution
        int remainingDuration = durPi - (2 * durPiece);
        EmbRet.push_back({principal, durPiece}); // Principal note (very brief)
        EmbRet.push_back({upper, durPiece});     // Upper auxiliary (m3 above, very brief)
        EmbRet.push_back({principal, remainingDuration}); // Resolve to principal

    } else if (meter == TRIPLE) {
        int durPiece = durPi / 12; // Very rapid for triple meter
        int remainingDuration = durPi - (2 * durPiece);
        EmbRet.push_back({principal, durPiece}); // Principal note
        EmbRet.push_back({upper, durPiece});     // Upper auxiliary (m3 above)
        EmbRet.push_back({principal, remainingDuration}); // Resolve to principal

    } else {
        EmbRet.push_back({principal, durPi});
    }
}

// Helper function for SPvM3 - principal and upper auxiliary (M3 above) played very rapidly
void handleSnapMajor3(std::vector<std::pair<int, int>>& EmbRet, int principal, int upper, int durPi, TimeMeter meter) {
    if (durPi <= 0) {
        return;
    }

    if (meter == DUPLE) {
        // Rapid alternation between principal and upper auxiliary (major 3rd above)
        int durPiece = durPi / 16; // Very rapid execution
        int remainingDuration = durPi - (2 * durPiece);
        EmbRet.push_back({principal, durPiece}); // Principal note (very brief)
        EmbRet.push_back({upper, durPiece});     // Upper auxiliary (M3 above, very brief)
        EmbRet.push_back({principal, remainingDuration}); // Resolve to principal

    } else if (meter == TRIPLE) {
        int durPiece = durPi / 12; // Very rapid for triple meter
        int remainingDuration = durPi - (2 * durPiece);
        EmbRet.push_back({principal, durPiece}); // Principal note
        EmbRet.push_back({upper, durPiece});     // Upper auxiliary (M3 above)
        EmbRet.push_back({principal, remainingDuration}); // Resolve to principal

    } else {
        EmbRet.push_back({principal, durPi});
    }
}

// Helper function for SPp4 - principal and upper auxiliary (P4 above) played very rapidly
void handleSnapPerfect4(std::vector<std::pair<int, int>>& EmbRet, int principal, int upper, int durPi, TimeMeter meter) {
    if (durPi <= 0) {
        return;
    }

    if (meter == DUPLE) {
        // Rapid alternation between principal and upper auxiliary (perfect 4th above)
        int durPiece = durPi / 16; // Very rapid execution
        int remainingDuration = durPi - (2 * durPiece);
        EmbRet.push_back({principal, durPiece}); // Principal note (very brief)
        EmbRet.push_back({upper, durPiece});     // Upper auxiliary (P4 above, very brief)
        EmbRet.push_back({principal, remainingDuration}); // Resolve to principal

    } else if (meter == TRIPLE) {
        int durPiece = durPi / 12; // Very rapid for triple meter
        int remainingDuration = durPi - (2 * durPiece);
        EmbRet.push_back({principal, durPiece}); // Principal note
        EmbRet.push_back({upper, durPiece});     // Upper auxiliary (P4 above)
        EmbRet.push_back({principal, remainingDuration}); // Resolve to principal

    } else {
        EmbRet.push_back({principal, durPi});
    }
}

// Main function to apply snap variants - COMPREHENSIVE IMPLEMENTATION
std::vector<std::pair<int, int>> applySnapVariants(int pi, int durPi, TimeMeter meter, const std::string& variant) {
    if (durPi <= 0) {
        throw std::invalid_argument("Duration (durPi) must be greater than 0");
    }
    if (meter != DUPLE && meter != TRIPLE) {
        throw std::invalid_argument("Invalid TimeMeter");
    }

    std::vector<std::pair<int, int>> EmbRet;

    // Snap variants based on the original function descriptions
    if (variant == "SP1") {  // principal and upper auxiliary (1 whole step above) are played very rapidly
        int upper = pi + 2;  // 1 whole step above
        handleSnap1(EmbRet, pi, upper, durPi, meter);

    } else if (variant == "SP5") {  // principal and upper auxiliary (1/2 step above) are played very rapidly
        int upper = pi + 1;  // 1/2 step above
        handleSnap5(EmbRet, pi, upper, durPi, meter);

    } else if (variant == "SPvm3") {  // principal and upper auxiliary (m3 above) are played very rapidly
        int upper = pi + 3;  // minor 3rd above
        handleSnapMinor3(EmbRet, pi, upper, durPi, meter);

    } else if (variant == "SPvM3") {  // principal and upper auxiliary (M3 step above) are played very rapidly
        int upper = pi + 4;  // major 3rd above
        handleSnapMajor3(EmbRet, pi, upper, durPi, meter);

    } else if (variant == "SPp4") {  // principal and upper auxiliary (P4 above) are played very rapidly
        int upper = pi + 5;  // perfect 4th above
        handleSnapPerfect4(EmbRet, pi, upper, durPi, meter);

    } else {
        // Handle unknown variant
        std::cerr << "Unknown snap variant: " << variant << '\n';
        EmbRet.push_back({pi, durPi}); // Return original note as fallback
    }

    return EmbRet;
}

// Simple Snap Variant Pool - ORIGINAL FUNCTION NAME PRESERVED AND ENHANCED
std::vector<std::string> applySnapVariant(int poolSize = 5) {
    // Define the pool of snap variants - all options from the original implementation
    const std::vector<std::string> allVariants = {
        "SP1",    // principal and upper auxiliary (1 whole step above) are played very rapidly
        "SP5",    // principal and upper auxiliary (1/2 step above) are played very rapidly
        "SPvm3",  // principal and upper auxiliary (m3 above) are played very rapidly
        "SPvM3",  // principal and upper auxiliary (M3 step above) are played very rapidly
        "SPp4"    // principal and upper auxiliary (P4 above) are played very rapidly
    };

    // Create a copy of all variants and shuffle it
    std::vector<std::string> shuffledVariants = allVariants;
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(shuffledVariants.begin(), shuffledVariants.end(), g);

    // Return the first poolSize variants
    std::vector<std::string> pool;
    for (int i = 0; i < std::min(poolSize, static_cast<int>(shuffledVariants.size())); ++i) {
        pool.push_back(shuffledVariants[i]);
    }

    return pool;
}

// ORIGINAL FUNCTION ENHANCED - generateRandomSnap equivalent
std::string generateRandomSnap() {
    std::vector<std::string> options = {
        "SP1 - the principal and upper auxiliary (1 whole step above) are played very rapidly",
        "SP5 - the principal and upper auxiliary (1/2 step above) are played very rapidly",
        "SPvm3 - the principal and upper auxiliary (m3 above) are played very rapidly",
        "SPvM3 - the principal and upper auxiliary (M3 step above) are played very rapidly",
        "SPp4 - the principal and upper auxiliary (P4 above) are played very rapidly"
    };

    // Use proper random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, options.size() - 1);

    return options[dis(gen)];
}

std::vector<std::string> getUserVariantChoice(){
    std::cout << "\n=== Snap Variant Selection ===" << std::endl;
    std::cout << "Choose your variant selection method:" << std::endl;
    std::cout << "1. Random selection (original behavior)" << std::endl;
    std::cout << "2. Choose from a random pool of variants" << std::endl;
    std::cout << "3. Select multiple variants from displayed choices" << std::endl;
    std::cout << "Enter your choice (1, 2, or 3): ";

    int choice;
    while (!(std::cin >> choice) ||(choice < 1 || choice > 3)) {
        std::cout << "Invalid input. Please enter 1, 2, or 3: ";
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }

    if (choice == 1){
        std::cout << "Using random variant selection." << std::endl;
        std::cout << "======================================\n" << std::endl;
        return {"RANDOM"};
    }

    // Generate random pool of variants
    std::vector<std::string> variantPool = applySnapVariant(5);

    std::cout << "\nHere are 5 randomly selected variants to choose from:" << std::endl;
    std::cout << "======================================================" << std::endl;

    for (size_t i = 0; i < variantPool.size(); ++i) {
        std::cout << (i + 1) << ". " << variantPool[i] << std::endl;
    }

    if (choice == 2) {
        std::cout << "\nEnter the number of your preferred variant (1-" << variantPool.size() << "): ";

        int variantChoice;
        while (!(std::cin >> variantChoice) || variantChoice < 1 || variantChoice > static_cast<int>(variantPool.size())) {
            std::cout << "Invalid choice. Please enter a number between 1 and " << variantPool.size() << ": ";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }

        std::string selectedVariant = variantPool[variantChoice - 1];
        std::cout << "You selected: " << selectedVariant << std::endl;
        std::cout << "======================================\n" << std::endl;

        return {selectedVariant};
    }

    // Choice 3: Multiple selection
    std::cout << "\nSelect any number (separated by spaces) from 1 to " << variantPool.size()
              << " of the variants you would like to randomly apply: ";

    std::cin.ignore(); // Clear the input buffer
    std::string input;
    std::getline(std::cin, input);

    std::vector<std::string> selectedVariants;
    std::istringstream iss(input);
    std::string token;

    while (iss >> token) {
        try {
            int num = std::stoi(token);
            if (num >= 1 && num <= static_cast<int>(variantPool.size())) {
                // Check if already selected to avoid duplicates
                std::string variant = variantPool[num - 1];
                if (std::find(selectedVariants.begin(), selectedVariants.end(), variant) == selectedVariants.end()) {
                    selectedVariants.push_back(variant);
                }
            } else {
                std::cout << "Warning: " << num << " is out of range (1-" << variantPool.size() << "), skipping." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Warning: '" << token << "' is not a valid number, skipping." << std::endl;
        }
    }

    if (selectedVariants.empty()) {
        std::cout << "No valid selections made. Using random selection as fallback." << std::endl;
        return {"RANDOM"};
    }

    std::cout << "You selected " << selectedVariants.size() << " variants: ";
    for (size_t i = 0; i < selectedVariants.size(); ++i) {
        std::cout << selectedVariants[i];
        if (i < selectedVariants.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
    std::cout << "======================================\n" << std::endl;

    return selectedVariants;
}

// ENHANCED applyVariant function with multiple variant support
std::vector<std::pair<int, int>> applyVariant(int pi, int durPi, std::string& selectedVariant, const std::vector<std::string>& userChoices = {}) {
    if (userChoices.empty() || (userChoices.size() == 1 && userChoices[0] == "RANDOM")) {
        // Original random selection behavior
        const std::vector<std::string> variants = {
            "SP1", "SP5", "SPvm3", "SPvM3", "SPp4"
        };
        // Randomly select a variant
        selectedVariant = variants[rand() % variants.size()];
    } else {
        // Randomly select from user's chosen variants
        selectedVariant = userChoices[rand() % userChoices.size()];
    }

    // Use the comprehensive implementation with DUPLE meter by default
    return applySnapVariants(pi, durPi, DUPLE, selectedVariant);
}

// NEW FUNCTION: Check if a label should be transformed based on percentage
bool shouldTransformLabel(double transformationPercentage) {
    // Generate random number between 0 and 100
    double randomValue = (static_cast<double>(rand()) / RAND_MAX) * 100.0;
    return randomValue < transformationPercentage;
}

// NEW FUNCTION: Get user input for transformation percentage
double getUserTransformationPercentage() {
    double percentage;
    std::cout << "\n=== Snap Transformation Settings ===" << std::endl;
    std::cout << "Enter the percentage of labeled notes to transform (0-100): ";

    while (!(std::cin >> percentage) || percentage < 0 || percentage > 100) {
        std::cout << "Invalid input. Please enter a number between 0 and 100: ";
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }

    std::cout << "Will transform approximately " << percentage << "% of labeled notes." << std::endl;
    std::cout << "======================================\n" << std::endl;

    return percentage;
}

// ENHANCED processFile function with percentage control and multiple variant choice support
void processFile(const std::string& inputFile, const std::string& outputFile, const std::vector<std::string>& userVariantChoices) {
    std::ifstream input(inputFile);
    std::ofstream output(outputFile);

    if (!input.is_open() || !output.is_open()) {
        std::cerr << "Error opening files.\n";
        return;
    }

    // Get transformation percentage from user
    double transformationPercentage = getUserTransformationPercentage();

    // Write header to the output file
    output << std::left << std::setw(11) << "Track"
           << std::setw(11) << "Note"
           << std::setw(20) << "Duration"
           << std::setw(20) << "Label"
           << std::setw(10) << "Variant"
           << "\n";
    output << "---------------------------------------------------------------\n";

    // Statistics tracking
    int totalEligibleNotes = 0;
    int transformedNotes = 0;

    std::string line;
    while (std::getline(input, line)) {
        std::istringstream ss(line);

        int track, duration;
        std::string noteName, label;

        // Parse line with Note in string format (e.g., "C4")
        if (!(ss >> track >> noteName >> duration)) {
            output << line << "\n";  // Handle malformed lines
            continue;
        }

        std::getline(ss, label);
        label.erase(0, label.find_first_not_of(" \t"));  // Trim leading whitespace
        // Remove trailing carriage return and whitespace (Windows line endings)
        label.erase(label.find_last_not_of(" \t\r\n") + 1);

        // Check if this label is eligible for transformation - ORIGINAL LABELS PRESERVED
        if (label == "QN" || label == "CS" || label == "U2R" || label == "RDN" ||
            label == "SN" || label == "DNW" || label == "SPNSA" || label == "LNR") {

            totalEligibleNotes++;

            // NEW: Check if this note should be transformed based on percentage
            if (shouldTransformLabel(transformationPercentage)) {
                transformedNotes++;

                try {
                    // Convert note name to MIDI number
                    int noteIndex = getNoteNumber(noteName);

                    // Use applyVariant to transform noteIndex and duration with user choices
                    std::string selectedVariant;
                    auto transformed = applyVariant(noteIndex, duration, selectedVariant, userVariantChoices);

                    // Output the transformed notes
                    for (const auto& [transformedNote, transformedDuration] : transformed) {
                        std::string transNote = getNoteName(transformedNote); // Convert MIDI to readable name
                        output << std::left
                               << std::setw(11) << track
                               << std::setw(11) << transNote
                               << std::setw(20) << transformedDuration
                               << std::setw(20) << label
                               << std::setw(10) << selectedVariant
                               << "\n";
                    }
                } catch (const std::exception& e) {
                    // Handle cases where getNoteNumber produces an error
                    std::cerr << "Error processing note '" << noteName << "': " << e.what() << "\n";
                }
            } else {
                // Output original data for notes not selected for transformation
                output << std::left
                       << std::setw(11) << track
                       << std::setw(11) << noteName
                       << std::setw(20) << duration
                       << std::setw(20) << label
                       << std::setw(10) << "ORIGINAL" // Mark as original
                       << "\n";
            }
        } else {
            // Output original data for non-eligible labels
            output << std::left
                   << std::setw(11) << track
                   << std::setw(11) << noteName
                   << std::setw(20) << duration
                   << std::setw(20) << label
                   << std::setw(10) << "" // Empty variant column
                   << "\n";
        }
    }

    input.close();
    output.close();

    // Display transformation statistics
    double actualPercentage = totalEligibleNotes > 0 ?
        (static_cast<double>(transformedNotes) / totalEligibleNotes) * 100.0 : 0.0;

    std::cout << "\n=== Transformation Statistics ===" << std::endl;
    std::cout << "Total eligible notes found: " << totalEligibleNotes << std::endl;
    std::cout << "Notes transformed: " << transformedNotes << std::endl;
    std::cout << "Actual transformation percentage: " << std::fixed << std::setprecision(1)
              << actualPercentage << "%" << std::endl;
    std::cout << "Processing complete. Transformed results written to " << outputFile << std::endl;
    std::cout << "================================\n" << std::endl;
}

// Structure to represent a MIDI note event
struct MidiEvent {
    int track;
    int noteNumber;
    int startTime;
    int duration;
    bool isNoteOn;
};

// New function to convert processed data to MIDI file with MIDI sync fix
void convertToMidi(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream input(inputFile);
    if (!input.is_open()) {
        std::cerr << "Error opening input file: " << inputFile << std::endl;
        return;
    }

    // Skip header lines
    std::string line;
    std::getline(input, line); // Skip column headers
    std::getline(input, line); // Skip separator line

    // Parse the file and collect note events
    std::map<int, std::vector<MidiEvent>> trackEvents;
    std::map<int, int> trackPositions; // FIXED: Track positions for sequential notes within each track

    while (std::getline(input, line)) {
        std::istringstream ss(line);
        int track;
        std::string noteName;
        int duration;
        std::string label, variant;

        // Skip lines that don't contain note data
        if (line.empty() || line[0] == '-' || line.find("MIDI File Analyzed") != std::string::npos) {
            continue;
        }

        // Parse the line
        if (!(ss >> track >> noteName >> duration)) {
            continue; // Skip malformed lines
        }

        // Skip header or non-note lines
        if (noteName == "Note" || noteName == "Track") {
            continue;
        }

        try {
            int noteNumber = getNoteNumber(noteName);

            // FIXED: Use track-specific positioning for sequential notes within each track
            int& trackPosition = trackPositions[track];

            // Create note-on event at the track's current position
            MidiEvent noteOn{track, noteNumber, trackPosition, duration, true};
            trackEvents[track].push_back(noteOn);

            // Create note-off event
            MidiEvent noteOff{track, noteNumber, trackPosition + duration, 0, false};
            trackEvents[track].push_back(noteOff);

            // Update the position for this track (notes within a track are sequential)
            trackPosition += duration;

        } catch (const std::exception& e) {
            std::cerr << "Error processing note '" << noteName << "': " << e.what() << std::endl;
        }
    }

    input.close();

    // Write MIDI file
    std::ofstream midiFile(outputFile, std::ios::binary);
    if (!midiFile.is_open()) {
        std::cerr << "Error opening output MIDI file: " << outputFile << std::endl;
        return;
    }

    // Write MIDI header
    // Format: MThd + <length> + <format> + <tracks> + <division>
    midiFile.write("MThd", 4); // Chunk type

    // Header length (always 6 bytes)
    char headerLength[4] = {0, 0, 0, 6};
    midiFile.write(headerLength, 4);

    // Format (0 = single track, 1 = multiple tracks, same timebase)
    char format[2] = {0, 1};
    midiFile.write(format, 2);

    // Number of tracks
    int numTracks = trackEvents.size();
    char tracksCount[2] = {static_cast<char>((numTracks >> 8) & 0xFF),
                          static_cast<char>(numTracks & 0xFF)};
    midiFile.write(tracksCount, 2);

    // Division (ticks per quarter note = 1024)
    char division[2] = {0x04, 0x00}; // 1024 in big-endian
    midiFile.write(division, 2);

    // Write each track
    for (const auto& [trackNum, events] : trackEvents) {
        // Sort events by time
        std::vector<MidiEvent> sortedEvents = events;
        std::sort(sortedEvents.begin(), sortedEvents.end(),
                 [](const MidiEvent& a, const MidiEvent& b) {
                     return a.startTime < b.startTime ||
                            (a.startTime == b.startTime && !a.isNoteOn && b.isNoteOn);
                 });

        // Write track header
        midiFile.write("MTrk", 4);

        // Placeholder for track length (will be filled in later)
        long trackLengthPos = midiFile.tellp();
        midiFile.write("\0\0\0\0", 4);

        // Track start position
        long trackStartPos = midiFile.tellp();

        // Write track events
        int lastTime = 0;

        // Set instrument (program change) - using piano (0) as default
        char programChange[4] = {0x00, static_cast<char>(0xC0), 0x00, 0x00}; // Delta time, command, program number, end of track
        midiFile.write(programChange, 3);

        for (const auto& event : sortedEvents) {
            // Write delta time (variable length)
            int deltaTime = event.startTime - lastTime;
            lastTime = event.startTime;

            // Convert delta time to variable length quantity
            std::vector<char> vlq;
            if (deltaTime == 0) {
                vlq.push_back(0);
            } else {
                while (deltaTime > 0) {
                    char byte = deltaTime & 0x7F;
                    deltaTime >>= 7;
                    if (!vlq.empty()) {
                        byte |= 0x80;
                    }
                    vlq.push_back(byte);
                }
                std::reverse(vlq.begin(), vlq.end());
            }

            for (char byte : vlq) {
                midiFile.put(byte);
            }

            // Write note event
            if (event.isNoteOn) {
                // Note on: 0x90 | channel, note, velocity
                midiFile.put(0x90);
                midiFile.put(static_cast<char>(event.noteNumber));
                midiFile.put(0x64); // Velocity (100)
            } else {
                // Note off: 0x80 | channel, note, velocity
                midiFile.put(0x80);
                midiFile.put(static_cast<char>(event.noteNumber));
                midiFile.put(0x00); // Velocity (0)
            }
        }

        // Write end of track
        midiFile.put(0x00); // Delta time
        midiFile.put(0xFF); // Meta event
        midiFile.put(0x2F); // End of track
        midiFile.put(0x00); // Length

        // Calculate and write track length
        long trackEndPos = midiFile.tellp();
        long trackLength = trackEndPos - trackStartPos;

        midiFile.seekp(trackLengthPos);
        char trackLengthBytes[4] = {
            static_cast<char>((trackLength >> 24) & 0xFF),
            static_cast<char>((trackLength >> 16) & 0xFF),
            static_cast<char>((trackLength >> 8) & 0xFF),
            static_cast<char>(trackLength & 0xFF)
        };
        midiFile.write(trackLengthBytes, 4);
        midiFile.seekp(trackEndPos);
    }

    midiFile.close();
    std::cout << "MIDI file created successfully: " << outputFile << std::endl;
}

int main() {
    // Seed the random number generator
    srand(static_cast<unsigned>(time(0)));

    // Get current time for unique filename generation
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;

#ifdef _WIN32
    localtime_s(&tm_now, &time_t_now);
#else
    tm_now = *localtime(&time_t_now);
#endif

    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &tm_now);

    // Get input file from user
    std::string inputFile;
    std::cout << "=======================================================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "========= The Snap Transformation Tool ================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "====This transformation tool takes a note==============" << std::endl;
    std::cout << "====that is appropriately labeled and applies==========" << std::endl;
    std::cout << "==========a snap to that note.=========================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "=== Use with labels: QN, CS, U2R, RDN, SN, =============" << std::endl;
    std::cout << "============ DNW, SPNSA, and LNR =======================" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "======== with % Transformation Selection ===============" << std::endl;
    std::cout << "======== Midi File Output and Text Output ==============" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Enter the path to the input file: ";
    std::cout << std::endl;
    std::getline(std::cin, inputFile);

    // Remove quotes if the user included them
    if (inputFile.size() >= 2 &&
        ((inputFile.front() == '"' && inputFile.back() == '"') ||
         (inputFile.front() == '\'' && inputFile.back() == '\''))) {
        inputFile = inputFile.substr(1, inputFile.size() - 2);
         }

    // FIXED: Call getUserVariantChoice() to get user's preferences (now returns vector)
    std::vector<std::string> userVariantChoices = getUserVariantChoice();

    // Generate unique output filenames with timestamp
    std::string outputFile = "Snap_transformation_Multiple_User_Choice_" + std::string(timestamp) + ".txt";
    std::cout << std::endl;
    std::string midiOutputFile = "Snap_transformation_Multiple_User_Choice_" + std::string(timestamp) + ".mid";
    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << "Processing file: " << inputFile << std::endl;
    std::cout << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "Output will be written to: " << outputFile << std::endl;
    std::cout << std::endl;
    std::cout << "MIDI output will be written to: " << midiOutputFile << std::endl;
    std::cout << "========================================================" << std::endl;
    // FIXED: Pass user variant choices to processFile
    processFile(inputFile, outputFile, userVariantChoices);

    // Convert the processed file to MIDI
    convertToMidi(outputFile, midiOutputFile);

    std::cout << "Processing complete!" << std::endl;
    std::cout << std::endl;
    std::cout << "Press Enter to exit...";
    std::cin.get();

    return 0;
}