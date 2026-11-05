// Compound Appoggiatura Transformation (C) 2025 <Roderick Harris>
// Enhanced version with percentage control for transformations and multiple choice variant selection
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <algorithm>
#include <iostream>
#include <iomanip> // For std::setw formatting
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <chrono>
#include <set>

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

// Helper functions from pasted.txt
void handleMeter(std::vector<std::pair<int, int>>& EmbRet, int pre1, int pre2, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        EmbRet.push_back({pre1, durPi / 4});
        EmbRet.push_back({pre2, durPi / 4});
        EmbRet.push_back({pi, durPi / 2});
    } else if (meter == TRIPLE) {
        EmbRet.push_back({pre1, durPi / 3});
        EmbRet.push_back({pre2, durPi / 3});
        EmbRet.push_back({pi, durPi / 3});
    }
}

void handleMeter4(std::vector<std::pair<int, int>>& EmbRet, int pre1, int pre2, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        EmbRet.push_back({pre1, durPi / 2}); // 1/4
        EmbRet.push_back({pre2, durPi / 4}); // 1/4
        EmbRet.push_back({pi, durPi / 4});   // 1/2
    } else if (meter == TRIPLE) {
        EmbRet.push_back({pre1, durPi / 2});
        EmbRet.push_back({pre2, durPi / 4});
        EmbRet.push_back({pi, durPi / 4});
    }
}

void handleMeter5(std::vector<std::pair<int, int>>& EmbRet, int pre1, int pre2, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        EmbRet.push_back({pre1, durPi / 4}); // 1/4
        EmbRet.push_back({pre2, durPi / 2}); // 1/4
        EmbRet.push_back({pi, durPi / 4});   // 1/2
    } else if (meter == TRIPLE) {
        EmbRet.push_back({pre1, durPi / 4}); // Splitting evenly
        EmbRet.push_back({pre2, durPi / 2});
        EmbRet.push_back({pi, durPi / 4});
    }
}

void handleMeter3(std::vector<std::pair<int, int>>& EmbRet, int pre1, int pre2, int pre3, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment = durPi / 4; // Clean division into 4 for 1/4 segments
        EmbRet.push_back({pre1, segment});
        EmbRet.push_back({pre2, segment});
        EmbRet.push_back({pre3, segment});
        EmbRet.push_back({pi, durPi / 4});
    } else if (meter == TRIPLE) {
        int segment = durPi / 3; // Proper thirds
        EmbRet.push_back({pre1, segment});
        EmbRet.push_back({pre2, segment});
        EmbRet.push_back({pre3, segment});
    }
}

void handleMeter6(std::vector<std::pair<int, int>>& EmbRet, int pre1, int pre2, int pre3, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        // Total duration is divided, pre1 gets 2x its share, half of durPi remains with pi
        int pre1Segment = (2 * durPi) / 4; // 2/4 of durPi
        int remainingSegment = durPi - pre1Segment - (durPi / 2); // Remaining after pre1 and pi

        // Split remaining duration between pre2 and pre3 equally
        int pre2Segment = remainingSegment / 2;
        int pre3Segment = remainingSegment / 2;

        // Add calculated splits to EmbRet
        EmbRet.push_back({pre1, pre1Segment});
        EmbRet.push_back({pre2, pre2Segment});
        EmbRet.push_back({pre3, pre3Segment});
        EmbRet.push_back({pi, durPi / 2}); // pi gets 1/2 durPi
    }
    else if (meter == TRIPLE) {
        // Total duration is split, pre1 gets double its share
        int pre1Segment = (2 * durPi) / 5; // For triple feel, pre1 2/5 of durPi
        int remainingSegment = durPi - pre1Segment - (durPi / 2); // DurPi minus pre1 and pi

        // Split remaining duration between pre2 and pre3 equally
        int pre2Segment = remainingSegment / 2;
        int pre3Segment = remainingSegment / 2;

        // Add calculated splits to EmbRet
        EmbRet.push_back({pre1, pre1Segment});
        EmbRet.push_back({pre2, pre2Segment});
        EmbRet.push_back({pre3, pre3Segment});
        EmbRet.push_back({pi, durPi / 2}); // pi gets 1/2 durPi
    }
}

void handleMeter7(std::vector<std::pair<int, int>>& EmbRet, int pre1, int pre2, int pre3, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        // Total duration is divided, pre2 gets 2x its share, half of durPi remains with pi
        int pre2Segment = (2 * durPi) / 4; // 2/4 of durPi
        int remainingSegment = durPi - pre2Segment - (durPi / 2); // Remaining after pre2 and pi

        // Split remaining duration between pre1 and pre3 equally
        int pre1Segment = remainingSegment / 2;
        int pre3Segment = remainingSegment / 2;

        // Add calculated splits to EmbRet
        EmbRet.push_back({pre1, pre1Segment});
        EmbRet.push_back({pre2, pre2Segment}); // Longer duration for pre2
        EmbRet.push_back({pre3, pre3Segment});
        EmbRet.push_back({pi, durPi / 2}); // pi gets 1/2 durPi
    }
    else if (meter == TRIPLE) {
        // Total duration is split, pre2 gets double its share
        int pre2Segment = (2 * durPi) / 5; // For triple feel, pre2 gets 2/5 of durPi
        int remainingSegment = durPi - pre2Segment - (durPi / 2); // Remaining after pre2 and pi

        // Split remaining duration between pre1 and pre3 equally
        int pre1Segment = remainingSegment / 2;
        int pre3Segment = remainingSegment / 2;

        // Add calculated splits to EmbRet
        EmbRet.push_back({pre1, pre1Segment});
        EmbRet.push_back({pre2, pre2Segment}); // Longer duration for pre2
        EmbRet.push_back({pre3, pre3Segment});
        EmbRet.push_back({pi, durPi / 2}); // pi gets 1/2 durPi
    }
}

void handleMeter8(std::vector<std::pair<int, int>>& EmbRet, int pre1, int pre2, int pre3, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        // Total duration is divided, pre3 gets 2x its share, half of durPi remains with pi
        int pre3Segment = (2 * durPi) / 4; // 2/4 of durPi
        int remainingSegment = durPi - pre3Segment - (durPi / 2); // Remaining after pre3 and pi

        // Split remaining duration between pre1 and pre2 equally
        int pre1Segment = remainingSegment / 2;
        int pre2Segment = remainingSegment / 2;

        // Add calculated splits to EmbRet
        EmbRet.push_back({pre1, pre1Segment});
        EmbRet.push_back({pre2, pre2Segment});
        EmbRet.push_back({pre3, pre3Segment}); // Longer duration for pre3
        EmbRet.push_back({pi, durPi / 2}); // pi gets 1/2 durPi
    }
    else if (meter == TRIPLE) {
        // Total duration is split, pre3 gets double its share
        int pre3Segment = (2 * durPi) / 5; // 2/5 of durPi
        int remainingSegment = durPi - pre3Segment - (durPi / 2); // Remaining after pre3 and pi

        // Split remaining duration between pre1 and pre2 equally
        int pre1Segment = remainingSegment / 2;
        int pre2Segment = remainingSegment / 2;

        // Add calculated splits to EmbRet
        EmbRet.push_back({pre1, pre1Segment});
        EmbRet.push_back({pre2, pre2Segment});
        EmbRet.push_back({pre3, pre3Segment}); // Longer duration for pre3
        EmbRet.push_back({pi, durPi / 2}); // pi gets 1/2 durPi
    }
}

// Add this forward declaration before applyCompoundTransformation function
std::vector<std::pair<int, int>> applyCompAppReg(int pi, int durPi, TimeMeter meter, const std::string& variant);

// Structure to represent a compound transformation variant
struct CompoundTransformation {
    std::string name;
    std::string description;
    std::vector<std::pair<int, int>> notes; // Will be populated when applied
};

// Generate all available compound variants - FIXED: Complete list
std::vector<CompoundTransformation> generateCompoundVariants() {
    return {
        // Two-note compound appoggiaturas
        {"A4M", "Augmented 4th below, Major 2nd up, original (3 notes)"},
        {"P5M", "Perfect 5th below, Major 2nd up, original (3 notes)"},
        {"P4M", "Perfect 4th below, Major 2nd up, original (3 notes)"},
        {"m7M", "Minor 7th below, Major 2nd up, original (3 notes)"},
        {"m6M", "Minor 6th below, Major 2nd up, original (3 notes)"},
        {"m3M", "Minor 3rd below, Major 2nd up, original (3 notes)"},
        {"M6M", "Major 6th below, Major 2nd up, original (3 notes)"},
        {"M3M", "Major 3rd below, Major 2nd up, original (3 notes)"},
        {"M7M", "Major 7th below, Major 2nd up, original (3 notes)"},
        {"P8M", "Perfect 8th below, Major 2nd up, original (3 notes)"},

        // 1CA variants
        {"1CAP5", "Perfect 4th below, Perfect 5th up, original (3 notes)"},
        {"1CAP4", "Minor 3rd below, Perfect 4th up, original (3 notes)"},
        {"1CAm7", "Minor 6th below, Minor 7th up, original (3 notes)"},
        {"1CAm6", "Augmented 4th below, Minor 6th up, original (3 notes)"},
        {"1CAm3", "Diminished 2nd below, Minor 3rd up, original (3 notes)"},
        {"1CAA4", "Major 3rd below, Augmented 4th up, original (3 notes)"},
        {"1CAM7", "Major 6th below, Major 7th up, original (3 notes)"},
        {"1CAM6", "Perfect 4th below, Major 6th up, original (3 notes)"},
        {"1CAM3", "Major 2nd below, Perfect 5th up, original (3 notes)"},

        // 1d dotted variants
        {"1dCAP5", "1st pos dotted: Perfect 4th below, Perfect 5th up, original"},
        {"1dCAP4", "1st pos dotted: Minor 3rd below, Perfect 4th up, original"},
        {"1dCAm7", "1st pos dotted: Minor 6th below, Minor 7th up, original"},
        {"1dCAm6", "1st pos dotted: Augmented 4th below, Minor 6th up, original"},
        {"1dCAm3", "1st pos dotted: Diminished 2nd below, Minor 3rd up, original"},
        {"1dCAA4", "1st pos dotted: Major 3rd below, Augmented 4th up, original"},
        {"1dCAM7", "1st pos dotted: Major 6th below, Major 7th up, original"},
        {"1dCAM6", "1st pos dotted: Perfect 4th below, Major 6th up, original"},
        {"1dCAM3", "1st pos dotted: Major 2nd below, Perfect 5th up, original"},

        // 2CA variants
        {"2CAP5", "Augmented 4th below, Perfect 5th up, original (3 notes)"},
        {"2CAP4", "Major 3rd below, Perfect 4th up, original (3 notes)"},
        {"2CAm7", "Major 6th below, Minor 7th up, original (3 notes)"},
        {"2CAm6", "Perfect 5th below, Minor 6th up, original (3 notes)"},
        {"2CAm3", "Augmented 4th below, Minor 3rd up, original (3 notes)"},
        {"2CAA4", "Perfect 4th below, Augmented 4th up, original (3 notes)"},
        {"2CAM7", "Minor 7th below, Major 7th up, original (3 notes)"},
        {"2CAM6", "Minor 6th below, Major 6th up, original (3 notes)"},
        {"2CAM3", "Minor 3rd below, Major 3rd up, original (3 notes)"},

        // RCA variants
        {"RCAP5m", "Minor 2nd above, Augmented 4th down, original (3 notes)"},
        {"RCAP5M", "Major 2nd above, Perfect 5th down, original (3 notes)"},
        {"RCAP4M", "Major 2nd above, Perfect 4th down, original (3 notes)"},
        {"RCAP4m", "Minor 2nd above, Major 3rd down, original (3 notes)"},
        {"RCAm7M", "Major 2nd above, Perfect 8th down, original (3 notes)"},
        {"RCAm7m", "Minor 2nd above, Major 7th down, original (3 notes)"},
        {"RCAm6m", "Minor 2nd above, Major 6th down, original (3 notes)"},
        {"RCAm6M", "Major 2nd above, Minor 7th down, original (3 notes)"},
        {"RCAm3m", "Minor 2nd above, Major 3rd down, original (3 notes)"},
        {"RCAM6m", "Minor 2nd above, Minor 7th down, original (3 notes)"},
        {"RCAM6M", "Major 2nd above, Major 7th down, original (3 notes)"},
        {"RCAM3M", "Major 2nd above, Augmented 4th down, original (3 notes)"},
        {"RCAM3m", "Minor 2nd above, Perfect 4th down, original (3 notes)"},

        // SAN variants
        {"SANA4M", "Major 2nd above, Augmented 4th down, original (3 notes)"},
        {"SANM3M", "Major 2nd above, Major 3rd down, original (3 notes)"},
        {"SANM6P", "Major 2nd above, Major 6th down, original (3 notes)"},
        {"SANM7M", "Major 2nd above, Major 7th down, original (3 notes)"},
        {"SANm3m", "Major 2nd above, Minor 3rd down, original (3 notes)"},
        {"SANm6d", "Major 2nd above, Minor 6th down, original (3 notes)"},
        {"SANm7m", "Major 2nd above, Minor 7th down, original (3 notes)"},
        {"SANP4m", "Major 2nd above, Minor 4th down, original (3 notes)"},
        {"SANP5P", "Major 2nd above, Perfect 5th down, original (3 notes)"},

        // Three-note compound appoggiaturas
        {"TA4Md1", "Minor 3rd below, Aug 4th up, Major 2nd down, original (4 notes)"},
        {"TP5Md1", "Major 3rd below, Perfect 5th up, Major 2nd down, original (4 notes)"},
        {"TP4Md1", "Major 2nd below, Perfect 4th up, Major 2nd down, original (4 notes)"},
        {"Tm7Md1", "Perfect 5th below, Minor 7th up, Major 2nd down, original (4 notes)"},
        {"Tm6Md1", "Perfect 4th below, Minor 6th up, Major 2nd down, original (4 notes)"},
        {"Tm3Md1", "On principal, Minor 3rd up, Major 2nd down, original (4 notes)"},
        {"TM6Md1", "Augmented 4th below, Major 6th up, Major 2nd down, original (4 notes)"},
        {"TM3Md1", "Minor 2nd below, Major 3rd up, Major 2nd down, original (4 notes)"},
        {"TM7Md1", "Minor 6th below, Major 7th up, Major 2nd down, original (4 notes)"},
        {"TP8Md1", "Major 6th below, Perfect 8th up, Major 2nd down, original (4 notes)"},

        // CAT variants
        {"CATm3M", "On principal, Minor 3rd up, Major 2nd down, resolve (4 notes)"},
        {"CATm6m", "On principal, Minor 6th up, Perfect 5th down, resolve (4 notes)"},
        {"CATm7M", "On principal, Minor 7th up, Minor 6th down, resolve (4 notes)"},
        {"CATP4M", "On principal, Perfect 4th up, Minor 3rd down, resolve (4 notes)"},
        {"CATP5M", "On principal, Perfect 5th up, Perfect 4th down, resolve (4 notes)"},
        {"CATA4M", "On principal, Augmented 4th up, Major 3rd down, resolve (4 notes)"},
        {"CATM3M", "On principal, Major 3rd up, Major 2nd down, resolve (4 notes)"},
        {"CATM6M", "On principal, Major 6th up, Perfect 5th down, resolve (4 notes)"},
        {"CATM7M", "On principal, Major 7th up, Major 6th down, resolve (4 notes)"},

        // RCAT variants
        {"RCATm3M", "On principal, Major 2nd up, Minor 2nd up, resolve (4 notes)"},
        {"RCATm6m", "On principal, Minor 2nd up, Perfect 5th up, resolve (4 notes)"},
        {"RCATm7M", "On principal, Major 2nd up, Minor 6th up, resolve (4 notes)"},
        {"RCATP4M", "On principal, Major 2nd up, Minor 3rd up, resolve (4 notes)"},
        {"RCATP5M", "On principal, Major 2nd up, Perfect 4th up, resolve (4 notes)"},
        {"RCATA4M", "On principal, Major 2nd up, Major 3rd up, resolve (4 notes)"},
        {"RCATM3M", "On principal, Major 2nd up, Major 2nd up, resolve (4 notes)"},
        {"RCATM6M", "On principal, Major 2nd up, Perfect 5th up, resolve (4 notes)"},
        {"RCATM7M", "On principal, Major 2nd up, Major 6th up, resolve (4 notes)"},

        // RPN variants
        {"RPNP5M", "On principal, Perfect 5th down, Perfect 4th up, resolve (4 notes)"},
        {"RPNP4M", "On principal, Perfect 4th down, Minor 3rd up, resolve (4 notes)"},
        {"RPNm7M", "On principal, Major 7th down, Major 6th up, resolve (4 notes)"},
        {"RPNM6M", "On principal, Major 6th down, Perfect 5th up, resolve (4 notes)"},
        {"RPNM3M", "On principal, Major 3rd down, Major 2nd up, resolve (4 notes)"},
        {"RPNA6M", "On principal, Minor 7th down, Minor 6th up, resolve (4 notes)"},
        {"RPNA5M", "On principal, Augmented 5th down, Augmented 4th up, resolve (4 notes)"},
        {"RPNA4M", "On principal, Augmented 4th down, Major 3rd up, resolve (4 notes)"},
        {"RPNm3M", "On principal, Major 3rd down, Minor 2nd up, resolve (4 notes)"},

        // A4md1 variants (minor 2nd down variants)
        {"A4md1", "Minor 3rd below, Augmented 4th up, Minor 2nd down, original (4 notes)"},
        {"P5md1", "Major 3rd below, Perfect 5th up, Minor 2nd down, original (4 notes)"},
        {"P4md1", "Major 2nd below, Perfect 4th up, Minor 2nd down, original (4 notes)"},
        {"m7md1", "Perfect 5th below, Minor 7th up, Minor 2nd down, original (4 notes)"},
        {"m6md1", "Perfect 4th below, Minor 6th up, Minor 2nd down, original (4 notes)"},
        {"m3md1", "On principal, Minor 3rd up, Minor 2nd down, original (4 notes)"},
        {"M6md1", "Augmented 4th below, Major 6th up, Minor 2nd down, original (4 notes)"},
        {"M3md1", "Minor 2nd below, Major 3rd up, Minor 2nd down, original (4 notes)"},
        {"M7md1", "Minor 6th below, Major 7th up, Minor 2nd down, original (4 notes)"},
        {"P8md1", "Major 6th below, Perfect 8th up, Minor 2nd down, original (4 notes)"}
    };
}

// NEW FUNCTION: Generate random pool of compound variants for user selection
std::vector<CompoundTransformation> generateRandomCompoundVariantPool(int poolSize) {
    std::vector<CompoundTransformation> allVariants = generateCompoundVariants();
    std::vector<CompoundTransformation> pool;

    std::random_device rd;
    std::mt19937 gen(rd());

    // Ensure we don't request more variants than available
    int actualPoolSize = std::min(poolSize, static_cast<int>(allVariants.size()));

    // Randomly select variants without replacement
    std::sample(allVariants.begin(), allVariants.end(),
                std::back_inserter(pool), actualPoolSize, gen);

    return pool;
}

// NEW FUNCTION: Parse user input for multiple selections
std::vector<int> parseMultipleChoices(const std::string& input, int maxChoice) {
    std::vector<int> choices;
    std::istringstream iss(input);
    std::string token;

    while (iss >> token) {
        try {
            int choice = std::stoi(token);
            if (choice >= 1 && choice <= maxChoice) {
                choices.push_back(choice);
            } else {
                std::cout << "Warning: Choice " << choice << " is out of range (1-" << maxChoice << "). Ignoring." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Warning: Invalid input '" << token << "'. Ignoring." << std::endl;
        }
    }

    // Remove duplicates
    std::sort(choices.begin(), choices.end());
    choices.erase(std::unique(choices.begin(), choices.end()), choices.end());

    return choices;
}

// ENHANCED FUNCTION: Get user's compound variant choice with multiple selection support
std::vector<CompoundTransformation> getUserCompoundVariantChoices() {
    std::cout << "\n=== Compound Appoggiatura Variant Selection ===" << std::endl;
    std::cout << "Choose your variant selection method:" << std::endl;
    std::cout << "1. Random selection (original behavior)" << std::endl;
    std::cout << "2. Choose from a random pool of variants (single or multiple)" << std::endl;
    std::cout << "Enter your choice (1 or 2): ";

    int choice;
    while (!(std::cin >> choice) || (choice != 1 && choice != 2)) {
        std::cout << "Invalid input. Please enter 1 or 2: ";
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }

    if (choice == 1) {
        std::cout << "Using random compound variant selection." << std::endl;
        std::cout << "======================================\n" << std::endl;

        // Return a single random variant
        std::vector<CompoundTransformation> allVariants = generateCompoundVariants();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, allVariants.size() - 1);

        CompoundTransformation randomVariant = allVariants[dis(gen)];
        randomVariant.name = "RANDOM_" + randomVariant.name;
        return {randomVariant};
    }

    // Generate random pool of variants
    std::vector<CompoundTransformation> variantPool = generateRandomCompoundVariantPool(15);

    std::cout << "\nHere are 15 randomly selected compound variants to choose from:" << std::endl;
    std::cout << "=============================================================" << std::endl;

    for (size_t i = 0; i < variantPool.size(); ++i) {
        std::cout << (i + 1) << ". " << variantPool[i].name
                  << " - " << variantPool[i].description << std::endl;
    }

    std::cout << "\nYou can select multiple variants!" << std::endl;
    std::cout << "Enter the numbers of your preferred variants separated by spaces" << std::endl;
    std::cout << "(e.g., '1 3 5 7' to select variants 1, 3, 5, and 7): ";

    std::cin.ignore(); // Clear the input buffer
    std::string userInput;
    std::getline(std::cin, userInput);

    std::vector<int> selectedIndices = parseMultipleChoices(userInput, variantPool.size());

    if (selectedIndices.empty()) {
        std::cout << "No valid selections made. Using the first variant as default." << std::endl;
        selectedIndices.push_back(1);
    }

    std::vector<CompoundTransformation> selectedVariants;
    std::cout << "\nYou selected:" << std::endl;
    for (int index : selectedIndices) {
        CompoundTransformation selectedVariant = variantPool[index - 1];
        selectedVariants.push_back(selectedVariant);
        std::cout << "- " << selectedVariant.name << " - " << selectedVariant.description << std::endl;
    }
    std::cout << "======================================\n" << std::endl;

    return selectedVariants;
}

//Apply compound transformation to a note
std::vector<std::pair<int, int>> applyCompoundTransformation(int originalNote, int originalDuration,
                                                          const CompoundTransformation& transformation) {
   // Use your existing comprehensive implementation
   return applyCompAppReg(originalNote, originalDuration, DUPLE, transformation.name);
}

// Main function from pasted.txt, renamed to applyCompAppReg
std::vector<std::pair<int, int>> applyCompAppReg(int pi, int durPi, TimeMeter meter, const std::string& variant) {
    // Full implementation from pasted.txt
    // ...
    // (Include the entire applyCompAppReg function from the pasted file)
    if (durPi <= 0) {
        throw std::invalid_argument("Duration (durPi) must be greater than 0");
    }
    if (meter != DUPLE && meter != TRIPLE) {
        throw std::invalid_argument("Invalid TimeMeter");
    }

    std::vector<std::pair<int, int>> EmbRet;
// Two -note compound appoggiaturas--------------------------------------------------
    if (variant == "A4M") {
        handleMeter(EmbRet, pi - 4, pi + 2, pi, durPi, meter);
    } else if (variant == "P5M") {
        handleMeter(EmbRet, pi - 5, pi + 2, pi, durPi, meter);
    } else if (variant == "P4M") {
        handleMeter(EmbRet, pi - 3, pi + 2, pi, durPi, meter);
    } else if (variant == "m7M") {
        handleMeter(EmbRet, pi - 8, pi + 2, pi, durPi, meter);
    } else if (variant == "m6M") {
        handleMeter(EmbRet, pi - 6, pi + 2, pi, durPi, meter);
    } else if (variant == "m3M") {
        handleMeter(EmbRet, pi - 1, pi + 2, pi, durPi, meter);
    } else if (variant == "M6M") {
        handleMeter(EmbRet, pi - 7, pi + 2, pi, durPi, meter);
    } else if (variant == "M3M") {
        handleMeter(EmbRet, pi - 2, pi + 2, pi, durPi, meter);
    } else if (variant == "M7M") {
        handleMeter(EmbRet, pi - 9, pi + 2, pi, durPi, meter);
    } else if (variant == "P8M") {
        handleMeter(EmbRet, pi - 10, pi + 2, pi, durPi, meter);
    }
    // Process 1CAP5... Process new "up from last location" formulas for each variant
    if (variant == "1CAP5") {  // P4 below, then up P5, back to principal
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 7;  // P5 up from the first note
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "1CAP4") {  // m3 below, then up P4, back to principal
        int pre1 = pi - 3;  // m3 below principal
        int pre2 = pre1 + 5;  // P4 up from the first note
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "1CAm7") {  // m6 below, then up m7, back to principal
        int pre1 = pi - 8;  // m6 below principal
        int pre2 = pre1 + 10;  // m7 up from the first note
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "1CAm6") {  // A4 below, then up m6, back to principal
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 9;  // m6 up from the first note
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "1CAm3") {  // d2 below, then up m3, back to principal
        int pre1 = pi - 1;  // d2 below principal
        int pre2 = pre1 + 3;  // m3 up from the first note
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "1CAA4") {  // M3 below, then up A4, back to principal
        int pre1 = pi - 4;  // M3 below principal
        int pre2 = pre1 + 8;  // A4 up from the first note
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "1CAM7") {  // M6 below, then up M7, back to principal
        int pre1 = pi - 9;  // M6 below principal
        int pre2 = pre1 + 11;  // M7 up from the first note
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "1CAM6") {  // P4 below, then up M6, back to principal
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 9;  // M6 up from the first note
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "1CAM3") {  // M2 below, then up P5, back to principal
        int pre1 = pi - 2;  // M2 below principal
        int pre2 = pre1 + 7;  // P5 up from the first note
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    }

    //  1dCAP5... dotted variants of 1CAP5 // 2nd position variants need to be made but start with handleMeter_? that will accomodate
    if (variant == "1dCAP5") {  // P4 below, then up P5, back to principal
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 7;  // P5 up from the first note
        handleMeter4(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "1dCAP4") {  // m3 below, then up P4, back to principal
        int pre1 = pi - 3;  // m3 below principal
        int pre2 = pre1 + 5;  // P4 up from the first note
        handleMeter4(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "1dCAm7") {  // m6 below, then up m7, back to principal
        int pre1 = pi - 8;  // m6 below principal
        int pre2 = pre1 + 10;  // m7 up from the first note
        handleMeter4(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "1dCAm6") {  // A4 below, then up m6, back to principal
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 9;  // m6 up from the first note
        handleMeter4(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "1dCAm3") {  // d2 below, then up m3, back to principal
        int pre1 = pi - 1;  // d2 below principal
        int pre2 = pre1 + 3;  // m3 up from the first note
        handleMeter4(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "1dCAA4") {  // M3 below, then up A4, back to principal
        int pre1 = pi - 4;  // M3 below principal
        int pre2 = pre1 + 8;  // A4 up from the first note
        handleMeter4(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "1dCAM7") {  // M6 below, then up M7, back to principal
        int pre1 = pi - 9;  // M6 below principal
        int pre2 = pre1 + 11;  // M7 up from the first note
        handleMeter4(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "1dCAM6") {  // P4 below, then up M6, back to principal
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 9;  // M6 up from the first note
        handleMeter4(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "1dCAM3") {  // M2 below, then up P5, back to principal
        int pre1 = pi - 2;  // M2 below principal
        int pre2 = pre1 + 7;  // P5 up from the first note
        handleMeter4(EmbRet, pre1, pre2, pi, durPi, meter);
    }

   // Process 2CAP formulas
    if (variant == "2CAP5") {  // A4 below, then up P5, back to principal
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 7;  // P5 up from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "2CAP4") {  // M3 below, then up P4, back to principal
        int pre1 = pi - 4;  // M3 below principal
        int pre2 = pre1 + 5;  // P4 up from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "2CAm7") {  // M6 below, then up m7, back to principal
        int pre1 = pi - 9;  // M6 below principal
        int pre2 = pre1 + 10;  // m7 up from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "2CAm6") {  // P5 below, then up m6, back to principal
        int pre1 = pi - 7;  // P5 below principal
        int pre2 = pre1 + 9;  // m6 up from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "2CAm3") {  // A4 below, then up m3, back to principal
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 3;  // m3 up from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "2CAA4") {  // P4 below, then up A4, back to principal
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 8;  // A4 up from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "2CAM7") {  // m7 below, then up M7, back to principal
        int pre1 = pi - 10;  // m7 below principal
        int pre2 = pre1 + 11;  // M7 up from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "2CAM6") {  // m6 below, then up M6, back to principal
        int pre1 = pi - 8;  // m6 below principal
        int pre2 = pre1 + 9;  // M6 up from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "2CAM3") {  // m3 below, then up M3, back to principal
        int pre1 = pi - 3;  // m3 below principal
        int pre2 = pre1 + 4;  // M3 up from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    }

    // Process RCAP formulas
    if (variant == "RCAP5m") {  // m2 above, then down A4, back to principal
        int pre1 = pi + 1;  // m2 above principal
        int pre2 = pre1 - 6;  // A4 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "RCAP5M") {  // M2 above, then down P5, back to principal
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 7;  // P5 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "RCAP4M") {  // M2 above, then down P4, back to principal
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 5;  // P4 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "RCAP4m") {  // m2 above, then down M3, back to principal
        int pre1 = pi + 1;  // m2 above principal
        int pre2 = pre1 - 4;  // M3 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "RCAm7M") {  // M2 above, then down P8 (octave), back to principal
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 12;  // P8 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "RCAm7m") {  // m2 above, then down M7, back to principal
        int pre1 = pi + 1;  // m2 above principal
        int pre2 = pre1 - 11;  // M7 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "RCAm6m") {  // m2 above, then down M6, back to principal
        int pre1 = pi + 1;  // m2 above principal
        int pre2 = pre1 - 9;  // M6 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "RCAm6M") {  // M2 above, then down m7, back to principal
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 10;  // m7 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "RCAm3m") {  // m2 above, then down M3, back to principal
        int pre1 = pi + 1;  // m2 above principal
        int pre2 = pre1 - 4;  // M3 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "RCAM6m") {  // m2 above, then down m7, back to principal
        int pre1 = pi + 1;  // m2 above principal
        int pre2 = pre1 - 10;  // m7 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "RCAM6M") {  // M2 above, then down M7, back to principal
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 11;  // M7 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "RCAM3M") {  // M2 above, then down A4, back to principal
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 8;  // A4 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "RCAM3m") {  // m2 above, then down P4, back to principal
        int pre1 = pi + 1;  // m2 above principal
        int pre2 = pre1 - 5;  // P4 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    }

   //  SAN formulas
    if (variant == "SANA4M") {  // M2 above, then down A4, back to principal
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 8;  // A4 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "SANM3M") {  // M2 above, then down M3, back to principal
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 4;  // M3 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "SANM6P") {  // M2 above, then down M6, back to principal
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 9;  // M6 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "SANM7M") {  // M2 above, then down M7, back to principal
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 11;  // M7 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "SANm3m") {  // M2 above, then down m3, back to principal
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 3;  // m3 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "SANm6d") {  // M2 above, then down m6, back to principal
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 8;  // m6 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "SANm7m") {  // M2 above, then down m7, back to principal
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 10;  // m7 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "SANP4m") {  // M2 above, then down m4, back to principal
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 6;  // m4 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "SANP5P") {  // M2 above, then down P5, back to principal
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 7;  // P5 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    }

   // ------------------------------------Three -note Compound Appoggiatura formulas-----------------------------------------------------------------------------------
   // Process TA4Md1-like formulas
    if (variant == "TA4Md1") {  // m3 below, up Aug 4, down M2, down to principal
        int pre1 = pi - 3;  // m3 below principal
        int pre2 = pre1 + 6;  // A4 up from pre1
        int pre3 = pre2 - 2;  // M2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TP5Md1") {  // M3 below, up P5, down M2, down to principal
        int pre1 = pi - 4;  // M3 below principal
        int pre2 = pre1 + 7;  // P5 up from pre1
        int pre3 = pre2 - 2;  // M2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TP4Md1") {  // M2 below, up P4, down M2, down to principal
        int pre1 = pi - 2;  // M2 below principal
        int pre2 = pre1 + 5;  // P4 up from pre1
        int pre3 = pre2 - 2;  // M2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "Tm7Md1") {  // P5 below, up m7, down M2, down to principal
        int pre1 = pi - 7;  // P5 below principal
        int pre2 = pre1 + 10;  // m7 up from pre1
        int pre3 = pre2 - 2;  // M2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "Tm6Md1") {  // P4 below, up m6, down M2, down to principal
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 8;  // m6 up from pre1
        int pre3 = pre2 - 2;  // M2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "Tm3Md1") {  // On principal, up m3, down M2, down to principal
        int pre1 = pi;  // On principal
        int pre2 = pre1 + 3;  // m3 up from pre1
        int pre3 = pre2 - 2;  // M2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TM6Md1") {  // A4 below, up M6, down M2, then to principal
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 9;  // M6 up from pre1
        int pre3 = pre2 - 2;  // M2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TM3Md1") {  // m2 below, up M3, down M2, down to principal
        int pre1 = pi - 1;  // m2 below principal
        int pre2 = pre1 + 4;  // M3 up from pre1
        int pre3 = pre2 - 2;  // M2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TM7Md1") {  // m6 below, up M7, down M2, down to principal
        int pre1 = pi - 8;  // m6 below principal
        int pre2 = pre1 + 11;  // M7 up from pre1
        int pre3 = pre2 - 2;  // M2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TP8Md1") {  // M6 below, up P8, down M2, down to principal
        int pre1 = pi - 10;  // M6 below principal
        int pre2 = pre1 + 13;  // P8 up from pre1
        int pre3 = pre2 - 2;  // M2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    }

// CATm3M 3-note compound appoggitura type
    if (variant == "CATm3M") {  // On principal, up m3, down M2, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 3;  // m3 up from pre1
    int pre3 = pre2 - 2;  // M2 down from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "CATm6m") {  // On principal, up m6, down P5, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 8;  // m6 up from pre1
    int pre3 = pre2 - 7;  // P5 down from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "CATm7M") {  // On principal, up m7, down m6, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 10;  // m7 up from pre1
    int pre3 = pre2 - 9;  // m6 down from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "CATP4M") {  // On principal, up P4, down m3, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 5;  // P4 up from pre1
    int pre3 = pre2 - 3;  // m3 down from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "CATP4m") {  // On principal, up P4, down M3, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 5;  // P4 up from pre1
    int pre3 = pre2 - 4;  // M3 down from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "CATP5M") {  // On principal, up P5, down P4, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 7;  // P5 up from pre1
    int pre3 = pre2 - 5;  // P4 down from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "CATP5m") {  // On principal, up P5, down A4, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 7;  // P5 up from pre1
    int pre3 = pre2 - 6;  // A4 down from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "CATA4M") {  // On principal, up A4, down M3, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 6;  // A4 up from pre1
    int pre3 = pre2 - 4;  // M3 down from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "CATA4m") {  // On principal, up A4, down P4, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 6;  // A4 up from pre1
    int pre3 = pre2 - 5;  // P4 down from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "CATM3M") {  // On principal, up M3, down M2, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 4;  // M3 up from pre1
    int pre3 = pre2 - 2;  // M2 down from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "CATM3m") {  // On principal, up M3, down m3, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 4;  // M3 up from pre1
    int pre3 = pre2 - 3;  // m3 down from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "CATM6M") {  // On principal, up M6, down P5, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 9;  // M6 up from pre1
    int pre3 = pre2 - 7;  // P5 down from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "CATM6m") {  // On principal, up M6, down m6, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 9;  // M6 up from pre1
    int pre3 = pre2 - 8;  // m6 down from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "CATM7M") {  // On principal, up M7, down M6, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 11;  // M7 up from pre1
    int pre3 = pre2 - 9;  // M6 down from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "CATm3m") {  // On principal, up m3, down m2, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 3;  // m3 up from pre1
    int pre3 = pre2 - 1;  // m2 down from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "CATm6M") {  // On principal, up m6, down A4, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 8;  // m6 up from pre1
    int pre3 = pre2 - 6;  // A4 down from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
}

if (variant == "RCATm3M") {  // Reverse Triple Comp. App starts on principal, up a M2, up a m2, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 2;  // M2 up from principal
    int pre3 = pre2 + 1;  // m2 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RCATm6m") {  // Reverse Triple Comp. App starts on principal, up a m2, up a P5, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 1;  // m2 up from principal
    int pre3 = pre2 + 7;  // P5 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RCATm7M") {  // Reverse Triple Comp. App starts on principal, up a M2, up m6, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 2;  // M2 up from principal
    int pre3 = pre2 + 8;  // m6 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RCATP4M") {  // Reverse Triple Comp. App starts on principal, up a M2, up a m3, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 2;  // M2 up from principal
    int pre3 = pre2 + 3;  // m3 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RCATP4m") {  // Reverse Triple Comp. App starts on principal, up a m2, up a M3, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 1;  // m2 up from principal
    int pre3 = pre2 + 4;  // M3 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RCATP5M") {  // Reverse Triple Comp. App starts on principal, up a M2, up P4, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 2;  // M2 up from principal
    int pre3 = pre2 + 5;  // P4 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RCATP5m") {  // Reverse Triple Comp. App starts on principal, up a m2, up a d5, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 1;  // m2 up from principal
    int pre3 = pre2 + 6;  // d5 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RCATA4M") {  // Reverse Triple Comp. App starts on principal, up a M2, up a M3, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 2;  // M2 up from principal
    int pre3 = pre2 + 4;  // M3 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RCATA4m") {  // Reverse Triple Comp. App starts on principal, up a m2, up a P4, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 1;  // m2 up from principal
    int pre3 = pre2 + 5;  // P4 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RCATM3M") {  // Reverse Triple Comp. App starts on principal, up a M2, up a M2, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 2;  // M2 up from principal
    int pre3 = pre2 + 2;  // M2 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RCATM3m") {  // Reverse Triple Comp. App starts on principal, up a m2, up a m3, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 1;  // m2 up from principal
    int pre3 = pre2 + 3;  // m3 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RCATM6M") {  // Reverse Triple Comp. App starts on principal, up a M2, up a P5, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 2;  // M2 up from principal
    int pre3 = pre2 + 7;  // P5 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RCATM6m") {  // Reverse Triple Comp. App starts on principal, up a m2, up a m6, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 1;  // m2 up from principal
    int pre3 = pre2 + 8;  // m6 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RCATM7M") {  // Reverse Triple Comp. App starts on principal, up a M2, up a M6, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 2;  // M2 up from principal
    int pre3 = pre2 + 9;  // M6 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RCATm3m") {  // Reverse Triple Comp. App starts on principal, up a m2, up a M2, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 1;  // m2 up from principal
    int pre3 = pre2 + 2;  // M2 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RCATm6M") {  // Reverse Triple Comp. App starts on principal, up a M2, up a d5, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 + 2;  // M2 up from principal
    int pre3 = pre2 + 6;  // d5 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
}

// 3 note compound appoggitura type RPNP5
   if (variant == "RPNP5M") {  // Reverse Triple Comp. App starts on principal, down a P5, up a P4, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 - 7;  // P5 down from principal
    int pre3 = pre2 + 5;  // P4 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RPNP4M") {  // Reverse Triple Comp. App starts on principal, down a P4, up a m3, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 - 5;  // P4 down from principal
    int pre3 = pre2 + 3;  // m3 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RPNm7M") {  // Reverse Triple Comp. App starts on principal, down a M7, up a M6, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 - 11;  // M7 down from principal
    int pre3 = pre2 + 9;  // M6 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RPNM6M") {  // Reverse Triple Comp. App starts on principal, down a M6, up a P5, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 - 9;  // M6 down from principal
    int pre3 = pre2 + 7;  // P5 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RPNM3M") {  // Reverse Triple Comp. App starts on principal, down a M3, up a M2, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 - 4;  // M3 down from principal
    int pre3 = pre2 + 2;  // M2 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RPNA6M") {  // Reverse Triple Comp. App starts on principal, down a m7, up a m6, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 - 10;  // m7 down from principal
    int pre3 = pre2 + 8;  // m6 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RPNA5M") {  // Reverse Triple Comp. App starts on principal, down an A5, up an A4, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 - 8;  // A5 down from principal
    int pre3 = pre2 + 6;  // A4 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RPNA4M") {  // Reverse Triple Comp. App starts on principal, down an A4, up a M3, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 - 6;  // A4 down from principal
    int pre3 = pre2 + 4;  // M3 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
} else if (variant == "RPNm3M") {  // Reverse Triple Comp. App starts on principal, down a M3, up a m2, then resolve
    int pre1 = pi;  // On principal
    int pre2 = pre1 - 4;  // M3 down from principal
    int pre3 = pre2 + 1;  // m2 up from pre2
    handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
}

    	// 3 note appoggitura type A4md1
    if (variant == "A4md1") {  // m3 below, up Aug 4, down m2, then down to principal
        int pre1 = pi - 3;  // m3 below principal
        int pre2 = pre1 + 6;  // A4 up from pre1
        int pre3 = pre2 - 1;  // m2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "P5md1") {  // M3 below, up P5, down m2, then down to principal
        int pre1 = pi - 4;  // M3 below principal
        int pre2 = pre1 + 7;  // P5 up from pre1
        int pre3 = pre2 - 1;  // m2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "P4md1") {  // M2 below, up P4, down m2, then down to principal
        int pre1 = pi - 2;  // M2 below principal
        int pre2 = pre1 + 5;  // P4 up from pre1
        int pre3 = pre2 - 1;  // m2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "m7md1") {  // P5 below, up m7, down m2, then down to principal
        int pre1 = pi - 7;  // P5 below principal
        int pre2 = pre1 + 10;  // m7 up from pre1
        int pre3 = pre2 - 1;  // m2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "m6md1") {  // P4 below, up m6, down m2, then down to principal
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 8;  // m6 up from pre1
        int pre3 = pre2 - 1;  // m2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "m3md1") {  // On principal, up m3, down m2, then down to principal
        int pre1 = pi;  // On principal
        int pre2 = pre1 + 3;  // m3 up from pre1
        int pre3 = pre2 - 1;  // m2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "M6md1") {  // A4 below, up M6, down m2, then down to principal
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 9;  // M6 up from pre1
        int pre3 = pre2 - 1;  // m2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "M3md1") {  // m2 below, up M3, down m2, then down to principal
        int pre1 = pi - 1;  // m2 below principal
        int pre2 = pre1 + 4;  // M3 up from pre1
        int pre3 = pre2 - 1;  // m2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "M7md1") {  // m6 below, up M7, down m2, then down to principal
        int pre1 = pi - 8;  // m6 below principal
        int pre2 = pre1 + 11;  // M7 up from pre1
        int pre3 = pre2 - 1;  // m2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "P8md1") {  // M6 below, up P8, down m2, then down to principal
        int pre1 = pi - 9;  // M6 below principal
        int pre2 = pre1 + 12;  // P8 up from pre1
        int pre3 = pre2 - 1;  // m2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    }

return EmbRet;

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
    std::cout << "\n=== Compound Appoggiatura Transformation Settings ===" << std::endl;
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

// ENHANCED FUNCTION: Process file with multiple variant support
void processFile(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream input(inputFile);
    std::ofstream output(outputFile);

    if (!input.is_open() || !output.is_open()) {
        std::cerr << "Error opening files.\n";
        return;
    }

    // Get transformation percentage from user
    double transformationPercentage = getUserTransformationPercentage();

    // Get user's compound variant choices (now supports multiple)
    std::vector<CompoundTransformation> userVariantChoices = getUserCompoundVariantChoices();

    // Write header to the output file
    output << std::left << std::setw(11) << "Track"
           << std::setw(11) << "Note"
           << std::setw(20) << "Duration"
           << std::setw(20) << "Label"
           << std::setw(25) << "Compound_Variant"
           << "\n";
    output << "---------------------------------------------------------------------------------\n";

    // Statistics tracking
    int totalEligibleNotes = 0;
    int transformedNotes = 0;
    std::map<std::string, int> variantUsageCount;

    // Random number generator for variant selection
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> variantDis(0, userVariantChoices.size() - 1);

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

        // Check if this label is eligible for compound transformation
        if (label == "I8" || label == "RN" || label == "LAD" || label == "RTR2" || label == "DI") {

            totalEligibleNotes++;

            // Check if this note should be transformed based on percentage
            if (shouldTransformLabel(transformationPercentage)) {
                transformedNotes++;

                try {
                    // Convert note name to MIDI number
                    int noteIndex = getNoteNumber(noteName);

                    // ENHANCED: Randomly select from user's chosen variants
                    CompoundTransformation selectedVariant = userVariantChoices[variantDis(gen)];

                    // Apply compound transformation
                    auto transformed = applyCompoundTransformation(noteIndex, duration, selectedVariant);

                    // Track variant usage
                    variantUsageCount[selectedVariant.name]++;

                    // Output the transformed notes
                    for (const auto& [transformedNote, transformedDuration] : transformed) {
                        std::string transNote = getNoteName(transformedNote);
                        output << std::left
                               << std::setw(11) << track
                               << std::setw(11) << transNote
                               << std::setw(20) << transformedDuration
                               << std::setw(20) << label
                               << std::setw(25) << selectedVariant.name
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
                       << std::setw(25) << "ORIGINAL"
                       << "\n";
            }
        } else {
            // Output original data for non-eligible labels
            output << std::left
                   << std::setw(11) << track
                   << std::setw(11) << noteName
                   << std::setw(20) << duration
                   << std::setw(20) << label
                   << std::setw(25) << ""
                   << "\n";
        }
    }

    input.close();
    output.close();

    // Display transformation statistics
    double actualPercentage = totalEligibleNotes > 0 ?
        (static_cast<double>(transformedNotes) / totalEligibleNotes) * 100.0 : 0.0;

    std::cout << "\n=== Compound Transformation Statistics ===" << std::endl;
    std::cout << "Total eligible notes found: " << totalEligibleNotes << std::endl;
    std::cout << "Notes transformed: " << transformedNotes << std::endl;
    std::cout << "Actual transformation percentage: " << std::fixed << std::setprecision(1)
              << actualPercentage << "%" << std::endl;

    std::cout << "Selected variants (" << userVariantChoices.size() << " total):" << std::endl;
    for (const auto& variant : userVariantChoices) {
        std::cout << "- " << variant.name << ": " << variant.description << std::endl;
    }

    if (!variantUsageCount.empty()) {
        std::cout << "\nVariant usage breakdown:" << std::endl;
        for (const auto& [variant, count] : variantUsageCount) {
            std::cout << "  " << variant << ": " << count << " times" << std::endl;
        }
    }

    std::cout << "Processing complete. Transformed results written to " << outputFile << std::endl;
    std::cout << "==========================================\n" << std::endl;
}

// Structure to represent a MIDI note event
struct MidiEvent {
    int track;
    int noteNumber;
    int startTime;
    int duration;
    bool isNoteOn;
};

// New function to convert processed data to MIDI file
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
    std::cout << "=== The Compound Appoggiatura Transformation Tool ======" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "====This transformation tool takes a note==============" << std::endl;
    std::cout << "====that is appropriately labeled and applies==========" << std::endl;
    std::cout << "==========a compound appoggiatura to that note.========" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "=== Use with labels:  I8, RN, LAD, RTR2, DI ======" << std::endl;
    std::cout << "=========================================== ===========" << std::endl;
    std::cout << "======== with % Transformation Selection ==============" << std::endl;
    std::cout << "======== Midi File Output and Text Output =============" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "=======================================================" << std::endl;
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
    std::cout << "=======================================================" << std::endl;
    // Generate unique output filenames with timestamp
    std::string outputFile = "Enhanced-Compound-Appoggiatura_Transformation_Multiple_User_Choice_" + std::string(timestamp) + ".txt";

    std::cout << "=======================================================" << std::endl;
    std::string midiOutputFile = "Enhanced-Compound-Appoggiatura_Transformation_Multiple_User_Choice_" + std::string(timestamp) + ".mid";

    std::cout << "Processing file: " << inputFile << std::endl;
    std::cout << "Output will be written to: " << outputFile << std::endl;
    std::cout << "MIDI output will be written to: " << midiOutputFile << std::endl;

    // Process the file (now with multiple variant support and percentage control)
    processFile(inputFile, outputFile);

    // Convert the processed file to MIDI
    convertToMidi(outputFile, midiOutputFile);

    std::cout << "Processing complete!" << std::endl;
    std::cout << "Press Enter to exit...";
    std::cin.get();

    return 0;
}