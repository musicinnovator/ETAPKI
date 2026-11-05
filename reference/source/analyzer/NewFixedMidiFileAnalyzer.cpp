//Midi file analyzer - CORRECTLY FIXED VERSION

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <map>
#include <sstream> // For user input parsing


// Function to generate a unique filename
std::string generateUniqueFilename(const std::string &baseFilename, const std::string &extension) {
    // Get the current timestamp
    std::time_t now = std::time(nullptr);
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", std::localtime(&now));

    // Combine base filename, timestamp, and extension
    return baseFilename + "_" + timestamp + extension;
}

    std::vector<std::string> selectedLabels;
// A helper structure for a MIDI note
struct Note {
    std::string name;
    double duration; // Duration in ticks
    int track;       // Track to which the note belongs
    std::string label; // To store the label (if any)
};

// Function to convert MIDI note number to note name
std::string getNoteName(int noteNumber) {
    static const std::string noteNames[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    int octave = noteNumber / 12 - 1;
    int noteIndex = noteNumber % 12;
    return noteNames[noteIndex] + std::to_string(octave);
}

// Function to read a variable-length quantity (VLQ) from the MIDI file
int readVariableLengthQuantity(const std::vector<unsigned char> &data, size_t &index) {
    int value = 0;
    unsigned char byte;
    do {
        byte = data[index++];
        value = (value << 7) | (byte & 0x7F);
    } while (byte & 0x80);
    return value;
}

// Function to prompt the user for label choices
std::vector<std::string> getUserLabelChoices() {
    std::vector<std::string> allLabels = {
        "RN", "U2R", "FN", "I8", "LP", "RLN", "QN", "CS", "I3", "BM", "MM",
        "EM", "SPU", "SPD", "CH", "CW", "CD", "HT", "FM", "SYP", "SLP",
        "RDN", "LAD", "DN", "DNW", "SN", "LNSN", "SAN", "RTR2", "TNTDN",
        "SMP", "SPNSA", "DHT", "DBC", "DDN", "LNR", "LNSAS", "LNSAL", "DI",
        "NLN", "TNTLN", "TNTTN", "Mmd7", "MmAug6", "MmAug4", "Mmm3", "NSALP",
        "BNIR", "IPASC", "IPASV", "RTDS", "BRTD", "SPCM", "SPDM", "SSN",
        "SVN", "ANS", "ANL", "FTB", "CDB", "DLP3", "IALD"};
    std::vector<std::string> selectedLabels;

    std::cout << "\nAvailable Labels:\n";
    for (size_t i = 0; i < allLabels.size(); ++i) {
        std::cout << i + 1 << ". " << allLabels[i] << "\n";
}
    std::cout << "\nEnter the numbers of the labels you want to analyze (separated by spaces): ";
    std::string input;
    std::getline(std::cin, input);

    std::stringstream ss(input);
    int choice;
    while (ss >> choice) {
        if (choice > 0 && choice <= static_cast<int>(allLabels.size())) {
            selectedLabels.push_back(allLabels[choice - 1]);
        } else {
            std::cout << "Invalid choice: " << choice << "\n";
        }
}

    return selectedLabels;
}

// Function to process labels on notes
class NoteUtils {
public:
    // Helper function to convert note name (e.g., "C4", "F#3") to MIDI note number
    static int getMidiNoteNumber(const std::string &noteName) {
        if (noteName.empty()) return -1;

        static const std::map<char, int> noteOffsets = {
            {'C', 0}, {'D', 2}, {'E', 4}, {'F', 5}, {'G', 7}, {'A', 9}, {'B', 11}
        };

        char note = noteName[0]; // Note letter (e.g., C, D, E, F...)
        int octave = noteName.back() - '0'; // Octave as integer
        int semitoneOffset = noteOffsets.at(note); // Get semitone offset for the note letter

        // Handle sharps (#) or flats (b)
        if (noteName.length() > 2) {
            if (noteName[1] == '#') semitoneOffset += 1; // Sharp
            else if (noteName[1] == 'b') semitoneOffset -= 1; // Flat
        }

        return (octave + 1) * 12 + semitoneOffset; // Return MIDI note number
    }
};

void processLabels(std::vector<Note> &notes, const std::vector<std::string> &selectedLabels) {
    // Process each note and assign a label if it matches a condition
    for (size_t i = 0; i < notes.size(); ++i) {
        // Example: Relatively Long Note (RLN)
        if (notes[i].duration >= 2048 && std::find(selectedLabels.begin(), selectedLabels.end(), "RLN") != selectedLabels.end()) {
            notes[i].label = "RLN";
        }

        // Label for the Final Note (FN)
        if (i == notes.size() - 1 && std::find(selectedLabels.begin(), selectedLabels.end(), "FN") != selectedLabels.end()) {
            notes[i].label = "FN";
        }


        // Example: Quick Note (QN)
        if (notes[i].duration <= 256 && std::find(selectedLabels.begin(), selectedLabels.end(), "QN") != selectedLabels.end()) {
            notes[i].label = "QN";
        }

        // Example: Repeated Note (RN)
        if (i > 0 && notes[i].name == notes[i - 1].name && std::find(selectedLabels.begin(), selectedLabels.end(), "RN") != selectedLabels.end()) {
            notes[i].label = "RN";
        }

        // Interval of an Octave (I8)
        if (i > 0 && std::find(selectedLabels.begin(), selectedLabels.end(), "I8") != selectedLabels.end()) {
            int currentNoteNumber = NoteUtils::getMidiNoteNumber(notes[i].name);
            int previousNoteNumber = NoteUtils::getMidiNoteNumber(notes[i - 1].name);
            int interval = abs(currentNoteNumber - previousNoteNumber);

            if (interval == 12) { // Exactly an octave (8th)
                notes[i].label = "I8";
            }
        }

        // Just Before a Leap (LP)
        if (i < notes.size() - 1 && std::find(selectedLabels.begin(), selectedLabels.end(), "LP") != selectedLabels.end()) {
            int currentNoteNumber = NoteUtils::getMidiNoteNumber(notes[i].name);
            int nextNoteNumber = NoteUtils::getMidiNoteNumber(notes[i + 1].name);
            int intervalToNext = abs(nextNoteNumber - currentNoteNumber);

            if (intervalToNext >= 5) { // Greater than or equal to a perfect 4th
                notes[i].label = "LP";
            }
        }

        // NEW: Rapidly Descending Note (RDN)
        if (i > 0 && notes[i].duration <= 512 && std::find(selectedLabels.begin(), selectedLabels.end(), "RDN") != selectedLabels.end()) {
            int currentNoteNumber = NoteUtils::getMidiNoteNumber(notes[i].name);
            int previousNoteNumber = NoteUtils::getMidiNoteNumber(notes[i - 1].name);

            if (currentNoteNumber < previousNoteNumber) { // Descending in pitch
                notes[i].label = "RDN";
            }
        }

        // NEW: Leap (ascending or descending) (LAD)
        if (i > 0 && std::find(selectedLabels.begin(), selectedLabels.end(), "LAD") != selectedLabels.end()) {
            int currentNoteNumber = NoteUtils::getMidiNoteNumber(notes[i].name);
            int previousNoteNumber = NoteUtils::getMidiNoteNumber(notes[i - 1].name);
            int interval = abs(currentNoteNumber - previousNoteNumber);

            if (interval >= 5) { // Leap is equal to or greater than a perfect 4th
                notes[i].label = "LAD";
            }
        }

        // NEW: Dotted Note (DN)
        if (std::find(selectedLabels.begin(), selectedLabels.end(), "DN") != selectedLabels.end()) {
            if (notes[i].duration == 768 || notes[i].duration == 1536 || notes[i].duration == 3072) {
                notes[i].label = "DN";
            }
        }

        // NEW: Interval of a 3rd (I3) with registration handling
        if (i > 0 && std::find(selectedLabels.begin(), selectedLabels.end(), "I3") != selectedLabels.end()) {
            int currentNoteNumber = NoteUtils::getMidiNoteNumber(notes[i].name);
            int previousNoteNumber = NoteUtils::getMidiNoteNumber(notes[i - 1].name);

            int interval = abs(currentNoteNumber - previousNoteNumber);

            if (interval == 3 || interval == 4) { // Minor third or major third
                notes[i].label = "I3";
            }
        }

        // NEW: Goes Up 2nd and Returns (U2R)
        if (i < notes.size() - 2 && std::find(selectedLabels.begin(), selectedLabels.end(), "U2R") != selectedLabels.end()) {
            int currentNoteNumber = NoteUtils::getMidiNoteNumber(notes[i].name);
            int nextNoteNumber = NoteUtils::getMidiNoteNumber(notes[i + 1].name);
            int nextNextNoteNumber = NoteUtils::getMidiNoteNumber(notes[i + 2].name);

            if (nextNoteNumber - currentNoteNumber == 2 && nextNextNoteNumber == currentNoteNumber) {
                notes[i].label = "U2R";
                notes[i + 1].label = "U2R";
                notes[i + 2].label = "U2R";
            }
        }

        // Long Note Followed by Short Note (LNSN)
        if (selectedLabels.end() != std::find(selectedLabels.begin(), selectedLabels.end(), "LNSN") &&
            i < notes.size() - 1 && notes[i].duration > notes[i + 1].duration) {
            notes[i].label = "LNSN";
            }

        if (i == notes.size() - 2 && std::find(selectedLabels.begin(), selectedLabels.end(), "NLN") != selectedLabels.end()) {
            notes[i].label = "NLN";
            }

        // Stepwise Ascending Note (SAN)
        if (selectedLabels.end() != std::find(selectedLabels.begin(), selectedLabels.end(), "SAN") &&
            i < notes.size() - 3) {
            if ((NoteUtils::getMidiNoteNumber(notes[i + 1].name) == NoteUtils::getMidiNoteNumber(notes[i].name) + 1) &&
                (NoteUtils::getMidiNoteNumber(notes[i + 2].name) == NoteUtils::getMidiNoteNumber(notes[i + 1].name) + 1) &&
                (NoteUtils::getMidiNoteNumber(notes[i + 3].name) == NoteUtils::getMidiNoteNumber(notes[i + 2].name) + 1)) {
                for (size_t j = i; j <= i + 3; ++j) {
                    notes[j].label = "SAN";
                }
                }
            }

        // Repeated Note Followed by an Ascending 2nd (RTR2)
        if (selectedLabels.end() != std::find(selectedLabels.begin(), selectedLabels.end(), "RTR2") &&
            i < notes.size() - 2) {
            if (notes[i].name == notes[i + 1].name &&
                NoteUtils::getMidiNoteNumber(notes[i + 2].name) == NoteUtils::getMidiNoteNumber(notes[i + 1].name) + 1) {
                notes[i + 2].label = "RTR2";
                }
            }

        // Dotted Note Separation (TNTDN)
        if (selectedLabels.end() != std::find(selectedLabels.begin(), selectedLabels.end(), "TNTDN") && i > 0) {
            if (notes[i].duration == 3072 || notes[i].duration == 1536 || notes[i].duration == 768) {
                notes[i - 1].label = "TNTDN";
            }
        }

        // Note Before Stepwise Passage (SMP)
        if (selectedLabels.end() != std::find(selectedLabels.begin(), selectedLabels.end(), "SMP") &&
            i < notes.size() - 2 && i > 0) {
            if (NoteUtils::getMidiNoteNumber(notes[i].name) == NoteUtils::getMidiNoteNumber(notes[i - 1].name) + 1 &&
                NoteUtils::getMidiNoteNumber(notes[i + 1].name) == NoteUtils::getMidiNoteNumber(notes[i].name) + 1) {
                notes[i - 1].label = "SMP";
                }
            }

        // Second Note in Slurred Pair in Stepwise Ascent (SPNSA)
        if (selectedLabels.end() != std::find(selectedLabels.begin(), selectedLabels.end(), "SPNSA") &&
            i < notes.size() - 1) {
            if (NoteUtils::getMidiNoteNumber(notes[i + 1].name) == NoteUtils::getMidiNoteNumber(notes[i].name) + 1) {
                notes[i + 1].label = "SPNSA";
            }
            }

        // Detached Dotted Note (DDN)
        if (selectedLabels.end() != std::find(selectedLabels.begin(), selectedLabels.end(), "DDN") && i > 0) {
            if ((notes[i].duration == 3072 || notes[i].duration == 1536 || notes[i].duration == 768) &&
                notes[i - 1].duration == 0) { // Gap before dotted note
                notes[i].label = "DDN";
                }
        }

        // Long Note Next to Rest (LNR)
        if (selectedLabels.end() != std::find(selectedLabels.begin(), selectedLabels.end(), "LNR") &&
            i < notes.size() - 1) {
            if (notes[i].duration > 1536 && notes[i + 1].duration == 0 &&
                notes[i + 1].duration >= notes[i].duration / 2) {
                notes[i].label = "LNR";
                }
            }

        // Longer Note Preceded by Shorter Notes in Stepwise Ascent (LNSAS)
        if (selectedLabels.end() != std::find(selectedLabels.begin(), selectedLabels.end(), "LNSAS") && i > 1) {
            if (notes[i].duration > notes[i - 1].duration &&
                notes[i - 1].duration > notes[i - 2].duration &&
                (NoteUtils::getMidiNoteNumber(notes[i - 1].name) == NoteUtils::getMidiNoteNumber(notes[i - 2].name) + 1)) {
                notes[i].label = "LNSAS";
                }
        }

        // Longer Note Preceded by Shorter Notes in Ascending Leap (LNSAL)
        if (selectedLabels.end() != std::find(selectedLabels.begin(), selectedLabels.end(), "LNSAL") && i > 1) {
            if (notes[i].duration > notes[i - 1].duration &&
                notes[i - 1].duration > notes[i - 2].duration &&
                (NoteUtils::getMidiNoteNumber(notes[i - 1].name) > NoteUtils::getMidiNoteNumber(notes[i - 2].name) + 1)) {
                notes[i].label = "LNSAL";
                }
        }

        // Dissonant Interval (DI)
        if (selectedLabels.end() != std::find(selectedLabels.begin(), selectedLabels.end(), "DI") &&
            i < notes.size() - 1) {
            std::vector<int> dissonantIntervals = {1, 6, 10, 11}; // Minor 2nd, Aug. 4th, etc.
            int interval = abs(NoteUtils::getMidiNoteNumber(notes[i].name) -
                               NoteUtils::getMidiNoteNumber(notes[i + 1].name)) %
                           12;
            if (std::find(dissonantIntervals.begin(), dissonantIntervals.end(), interval) !=
                dissonantIntervals.end()) {
                notes[i].label = "DI";
                }
            }


        // Example: Between Two Notes with RLN (TNTLN)
        if (i < notes.size() - 1 && std::find(selectedLabels.begin(), selectedLabels.end(), "TNTLN") != selectedLabels.end()) {
            if (notes[i + 1].duration >= 2048) { // Check if the next note is an RLN
                notes[i].label = "TNTLN";
            }
        }

        // Example: Between Two Notes with Tied Notes (TNTTN)
        if (i < notes.size() - 1 && std::find(selectedLabels.begin(), selectedLabels.end(), "TNTTN") != selectedLabels.end()) {
            if (notes[i + 1].duration == 3072 || notes[i + 1].duration == 1536 || notes[i + 1].duration == 768) {
                notes[i].label = "TNTTN";
            }
        }

        // Just Before Two Notes Separated by a Leap (NSALP)
        if (i < notes.size() - 2 && std::find(selectedLabels.begin(), selectedLabels.end(), "NSALP") != selectedLabels.end()) {
            int firstNoteNumber = NoteUtils::getMidiNoteNumber(notes[i].name);
            int secondNoteNumber = NoteUtils::getMidiNoteNumber(notes[i + 1].name);

            if (abs(secondNoteNumber - firstNoteNumber) >= 5) { // Interval >= Perfect Fourth
                notes[i].label = "NSALP"; // Label the note before the leap
            }
        }

        // Before a Note Isolated by a Rest (BNIR)
        if (i > 0 && i < notes.size() - 1 && std::find(selectedLabels.begin(), selectedLabels.end(), "BNIR") != selectedLabels.end()) {
            if (notes[i - 1].duration == 0 && notes[i + 1].duration == 0) { // Preceded and followed by rest
                notes[i].label = "BNIR";
            }
        }

        // Gap Between Two Notes of an Ascending Second (IPASC)
        if (i < notes.size() - 2 && std::find(selectedLabels.begin(), selectedLabels.end(), "IPASC") != selectedLabels.end()) {
            int firstNoteNumber = NoteUtils::getMidiNoteNumber(notes[i].name);
            int secondNoteNumber = NoteUtils::getMidiNoteNumber(notes[i + 1].name);

            if (secondNoteNumber == firstNoteNumber + 1) { // Ascending second
                notes[i].label = "IPASC"; // Label the gap (or preceding note)
            }
        }

        // Gap Between Two Notes of an Ascending Seventh (IPASV)
        if (i < notes.size() - 2 && std::find(selectedLabels.begin(), selectedLabels.end(), "IPASV") != selectedLabels.end()) {
            int firstNoteNumber = NoteUtils::getMidiNoteNumber(notes[i].name);
            int secondNoteNumber = NoteUtils::getMidiNoteNumber(notes[i + 1].name);

            if (secondNoteNumber == firstNoteNumber + 11) { // Ascending seventh
                notes[i].label = "IPASV"; // Label the gap (or preceding note)
            }
        }

        // Repetition of a Tone Followed by a Descending Second (RTDS)
        if (i < notes.size() - 2 && std::find(selectedLabels.begin(), selectedLabels.end(), "RTDS") != selectedLabels.end()) {
            int currentNoteNumber = NoteUtils::getMidiNoteNumber(notes[i].name);
            int nextNoteNumber = NoteUtils::getMidiNoteNumber(notes[i + 1].name);
            int descNextNoteNumber = NoteUtils::getMidiNoteNumber(notes[i + 2].name);

            if (currentNoteNumber == nextNoteNumber && descNextNoteNumber == currentNoteNumber - 1) {
                notes[i].label = "RTDS"; // Current note part of repetition before descending second
                notes[i + 1].label = "RTDS"; // Repeat note part of repetition
            }
        }

        // Before the Repetition of a Tone Followed by a Descent (BRTD)
        if (i < notes.size() - 2 && std::find(selectedLabels.begin(), selectedLabels.end(), "BRTD") != selectedLabels.end()) {
            int currentNoteNumber = NoteUtils::getMidiNoteNumber(notes[i].name);
            int nextNoteNumber = NoteUtils::getMidiNoteNumber(notes[i + 1].name);
            int descNextNoteNumber = NoteUtils::getMidiNoteNumber(notes[i + 2].name);

            if (currentNoteNumber == nextNoteNumber && descNextNoteNumber < currentNoteNumber) {
                notes[i].label = "BRTD"; // Label the note before the pair
            }
        }

        // Downward Leap of a 3rd (DLP3)
        if (i > 0 && std::find(selectedLabels.begin(), selectedLabels.end(), "DLP3") != selectedLabels.end()) {
            int currentNoteNumber = NoteUtils::getMidiNoteNumber(notes[i].name);
            int previousNoteNumber = NoteUtils::getMidiNoteNumber(notes[i - 1].name);

            int interval = previousNoteNumber - currentNoteNumber; // Descending interval
            if (interval == 3 || interval == 4) { // Minor or Major 3rd
                notes[i].label = "DLP3";
            }
        }

        // Inside and Ascending Leap Followed by a Descent (IALD)
        if (i > 0 && i < notes.size() - 1 && std::find(selectedLabels.begin(), selectedLabels.end(), "IALD") != selectedLabels.end()) {
            int previousNoteNumber = NoteUtils::getMidiNoteNumber(notes[i - 1].name);
            int currentNoteNumber = NoteUtils::getMidiNoteNumber(notes[i].name);
            int nextNoteNumber = NoteUtils::getMidiNoteNumber(notes[i + 1].name);

            int ascendingInterval = currentNoteNumber - previousNoteNumber; // Ascending
            int descendingInterval = currentNoteNumber - nextNoteNumber;   // Descending

            if (ascendingInterval > 4 && descendingInterval > 0) { // Ascending leap > P4, followed by descent
                notes[i].label = "IALD";
            }
        }

        // Existing labels logic...

        // Example: Relatively Long Note (RLN)
        if (notes[i].duration >= 2048 && std::find(selectedLabels.begin(), selectedLabels.end(), "RLN") != selectedLabels.end()) {
            notes[i].label = "RLN";
        }

        // Example: Quick Note (QN)
        if (notes[i].duration <= 256 && std::find(selectedLabels.begin(), selectedLabels.end(), "QN") != selectedLabels.end()) {
            notes[i].label = "QN";
        }

        // Example: Repeated Note (RN)
        if (i > 0 && notes[i].name == notes[i - 1].name && std::find(selectedLabels.begin(), selectedLabels.end(), "RN") != selectedLabels.end()) {
            notes[i].label = "RN";
        }
    }
    // Other existing labels...

        // NEW: Beginning of Movement (BM)
        if (!notes.empty() && std::find(selectedLabels.begin(), selectedLabels.end(), "BM") != selectedLabels.end()) {
            notes[0].label = "BM"; // Label the first note
        }

        // NEW: Middle Note of Movement (MM)
        if (!notes.empty() && std::find(selectedLabels.begin(), selectedLabels.end(), "MM") != selectedLabels.end()) {
            size_t middleIndex = notes.size() / 2;
            notes[middleIndex].label = "MM"; // Label the middle note
        }

        // NEW: Ending Notes of Movement (EM)
        if (!notes.empty() && std::find(selectedLabels.begin(), selectedLabels.end(), "EM") != selectedLabels.end()) {
            size_t startIndex = notes.size() >= 5 ? notes.size() - 5 : 0; // Consider the last 5 notes or fewer
            for (size_t i = startIndex; i < notes.size(); ++i) {
                notes[i].label = "EM";
            }
        }

        // NEW: Serial Pattern Upwards (SPU)
        if (!notes.empty() && std::find(selectedLabels.begin(), selectedLabels.end(), "SPU") != selectedLabels.end()) {
            for (size_t i = 0; i < notes.size() - 2; ++i) {
                int firstNoteNumber = NoteUtils::getMidiNoteNumber(notes[i].name);
                int secondNoteNumber = NoteUtils::getMidiNoteNumber(notes[i + 1].name);
                int thirdNoteNumber = NoteUtils::getMidiNoteNumber(notes[i + 2].name);

                if (secondNoteNumber > firstNoteNumber && thirdNoteNumber > secondNoteNumber) {
                    for (size_t j = i + 3; j < notes.size() - 2; ++j) {
                        int repeatFirst = NoteUtils::getMidiNoteNumber(notes[j].name);
                        int repeatSecond = NoteUtils::getMidiNoteNumber(notes[j + 1].name);
                        int repeatThird = NoteUtils::getMidiNoteNumber(notes[j + 2].name);

                        if ((secondNoteNumber - firstNoteNumber == repeatSecond - repeatFirst) &&
                            (thirdNoteNumber - secondNoteNumber == repeatThird - repeatSecond)) {
                            notes[i].label = "SPU";
                            notes[j].label = "SPU";
                            }
                    }
                }
            }
        }

        // NEW: Serial Pattern Downwards (SPD)
        if (std::find(selectedLabels.begin(), selectedLabels.end(), "SPD") != selectedLabels.end()) {
            for (size_t i = 0; i < notes.size() - 2; ++i) {
                int firstNoteNumber = NoteUtils::getMidiNoteNumber(notes[i].name);
                int secondNoteNumber = NoteUtils::getMidiNoteNumber(notes[i + 1].name);
                int thirdNoteNumber = NoteUtils::getMidiNoteNumber(notes[i + 2].name);

                if (firstNoteNumber > secondNoteNumber && secondNoteNumber > thirdNoteNumber) {
                    notes[i].label = "SPD";
                    notes[i + 1].label = "SPD";
                    notes[i + 2].label = "SPD";
                }
            }
        }
    }


int main(){

    // Get input file from user
    std::string filePath;
    std::cout << "=======================================================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "========= The Midi File Analyzer ======================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "==== This tool takes a MIDI file and reads =============" << std::endl;
    std::cout << "==== it for various characteristics and ===============" << std::endl;
    std::cout << "==== then labeled. The labeled notes can then be ======" << std::endl;
    std::cout << "==== used to transform the notes. =====================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "==== There are 7 Transformation Tools that are used: ==" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "  ===== 1. Appoggiatura  ==============================" << std::endl;
    std::cout << "  ===== 2. Compound Appoggiatura ======================" << std::endl;
    std::cout << "  ===== 3. Mordent ====================================" << std::endl;
    std::cout << "  ===== 4. Slides =====================================" << std::endl;
    std::cout << "  ===== 5. Snaps ======================================" << std::endl;
    std::cout << "  ===== 6. Trills =====================================" << std::endl;
    std::cout << "  ===== 7. Turns ======================================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "================Available Labels (62) =======================" << std::endl;
    std::cout << "1. RN, 2. U2R, 3. FN, 4. I8, 5. LP, 6. RLN, 7. QN, 8. CS" << std::endl;
    std::cout <<  std::endl;
    std::cout << "9. I3, 10. BM, 11. MM, 12. EM, 13. SPU, 14. SPD, 15. CH" << std::endl;
    std::cout <<  std::endl;
    std::cout << "16. CW, 17. CD, 18. HT, 19. FM, 20. SYP, 21. SLP, 22. RDN" << std::endl;
    std::cout <<  std::endl;
    std::cout << "23. LAD, 24. DN, 25. DNW, 26. SN, 27. LNSN, 28. SAN" << std::endl;
    std::cout <<  std::endl;
    std::cout << "29. RTR2, 30. TNTDN, 31. SMP, 32. SPNSA, 33. DHT" << std::endl;
    std::cout <<  std::endl;
    std::cout << "34. DBC, 35. DDN, 36. LNR, 37. LNSAS, 38. LNSAL" << std::endl;
    std::cout <<  std::endl;
    std::cout << "39. DI, 40. NLN, 41. TNTLN, 42. TNTTN, 43. Mmd7" << std::endl;
    std::cout <<  std::endl;
    std::cout << "44. MmAug6, 45. MmAug4, 46. Mmm3, 47. NSALP" << std::endl;
    std::cout <<  std::endl;
    std::cout << "48. BNIR, 49. IPASC, 50. IPASV, 51. RTDS" << std::endl;
    std::cout <<  std::endl;
    std::cout << "52. BRTD, 53. SPCM, 54. SPDM, 55. SSN" << std::endl;
    std::cout <<  std::endl;
    std::cout << "56. SVN, 57. ANS, 58. ANL" << std::endl;
    std::cout <<  std::endl;
    std::cout << "59. FTB, 60. CDB, 61. DLP3, 62. IALD" << std::endl;
    std::cout <<  std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "========================================================" << std::endl;


    std::cout << std::endl;
    std::cout << "Enter the path to the MIDI (.mid) file: ";
    std::getline(std::cin, filePath); // Reads the entire line input including spaces


    // Open the MIDI file
    std::ifstream midiFile(filePath, std::ios::binary);
    if (!midiFile.is_open()) {
        std::cerr << "Failed to open MIDI file: " << filePath << std::endl;
        return 1;
    }

    midiFile.seekg(0, std::ios::end);
    if (midiFile.tellg() == 0) {
        std::cerr << "The MIDI file is empty: " << filePath << std::endl;
        return 1;
    }
    midiFile.seekg(0, std::ios::beg);
    std::cout << "MIDI file loaded successfully!" << std::endl;

    std::vector<Note> notes;
    int trackCount = 0;

    // Read the header chunk
    char header[14];
    midiFile.read(header, 14);

    // Extract track count
    trackCount = (static_cast<unsigned char>(header[10]) << 8) | static_cast<unsigned char>(header[11]);
    std::cout << "Number of Tracks: " << trackCount << std::endl;

    // Prompt user for label choices
    std::vector<std::string> selectedLabels = getUserLabelChoices();

    // Process track chunks
    for (int track = 0; track < trackCount; ++track) {
        char chunkHeader[8];
        if (!midiFile.read(chunkHeader, 8)) {
            std::cerr << "Failed to read track chunk header!" << std::endl;
            break;
        }

        // Extract chunk size
        int chunkSize = (static_cast<unsigned char>(chunkHeader[4]) << 24) |
                        (static_cast<unsigned char>(chunkHeader[5]) << 16) |
                        (static_cast<unsigned char>(chunkHeader[6]) << 8) |
                        static_cast<unsigned char>(chunkHeader[7]);

        // Read track data
        std::vector<unsigned char> trackData(chunkSize);
        if (!midiFile.read(reinterpret_cast<char *>(trackData.data()), chunkSize)) {
            std::cerr << "Failed to read track data!" << std::endl;
            break;
        }

        // CORRECT FIX: Parse MIDI events with initial timing offset preservation
        size_t index = 0;
        int tickTime = 0;
        std::map<int, std::map<int, int>> activeNotes;
        int initialOffset = -1;  // Track the initial timing offset (if any)
        bool firstNoteOnFound = false;  // Flag to track if we've found the first note-on

        while (index < trackData.size()) {
            int deltaTime = readVariableLengthQuantity(trackData, index);
            tickTime += deltaTime;

            unsigned char statusByte = trackData[index++];

            if (statusByte == 0xFF) {
                // Meta event
                unsigned char type = trackData[index++];
                unsigned char length = trackData[index++];
                index += length;
            } else if ((statusByte & 0xF0) == 0x90) {
                // Note On event
                int channel = statusByte & 0x0F;
                unsigned char noteNumber = trackData[index++];
                unsigned char velocity = trackData[index++];

                if (velocity > 0) {
                    // CRITICAL FIX: Capture initial offset only for the very first note-on
                    if (!firstNoteOnFound) {
                        initialOffset = tickTime;  // Store the initial timing offset
                        firstNoteOnFound = true;
                    }

                    // Store note start time (keeping original relative timing)
                    activeNotes[channel][noteNumber] = tickTime;
                } else {
                    // Note off (velocity 0)
                    if (activeNotes[channel].count(noteNumber)) {
                        int startTime = activeNotes[channel][noteNumber];
                        activeNotes[channel].erase(noteNumber);

                        Note note;
                        note.name = getNoteName(noteNumber);
                        // KEEP ORIGINAL DURATION CALCULATION - no changes here
                        note.duration = tickTime - startTime;
                        note.track = track + 1;

                        notes.push_back(note);
                    }
                }
            } else if ((statusByte & 0xF0) == 0x80) {
                // Note Off event
                int channel = statusByte & 0x0F;
                unsigned char noteNumber = trackData[index++];
                index++; // Skip velocity
                if (activeNotes[channel].count(noteNumber)) {
                    int startTime = activeNotes[channel][noteNumber];
                    activeNotes[channel].erase(noteNumber);

                    Note note;
                    note.name = getNoteName(noteNumber);
                    // KEEP ORIGINAL DURATION CALCULATION - no changes here
                    note.duration = tickTime - startTime;
                    note.track = track + 1;

                    notes.push_back(note);
                }
            } else {
                // Other MIDI events - skip appropriately
                if (index < trackData.size()) {
                    index++;
                }
            }
        }

        // APPLY INITIAL OFFSET CORRECTION: Only if there was an initial offset
        if (initialOffset > 0) {
            std::cout << "Track " << (track + 1) << ": Applied initial timing offset of "
                      << initialOffset << " ticks" << std::endl;
        }
    }

    // Process labels
    processLabels(notes, selectedLabels);

    // Generate a unique filename
    std::string baseFilename = "output_file_correct_fix";
    std::string extension = ".txt";
    std::string uniqueFilename = generateUniqueFilename(baseFilename, extension);

    // Write output (keeping original format)
    std::ofstream outputFile(uniqueFilename);
    outputFile << "MIDI File Analyzed: " << filePath << "\n";
    outputFile << "INITIAL TIMING OFFSET FIX APPLIED: Preserves timing before first note-on\n\n";
    outputFile << std::setw(10) << "Track" << std::setw(10) << "Note"
               << std::setw(15) << "Duration" << std::setw(20) << "Label" << "\n";
    outputFile << "-----------------------------------------------------------\n";

    for (const auto &note : notes) {
        // Check conditions for including the note
        if (note.label.empty() || std::find(selectedLabels.begin(), selectedLabels.end(), note.label) != selectedLabels.end()) {
            outputFile << std::setw(10) << note.track << std::setw(10) << note.name
                       << std::setw(15) << note.duration << std::setw(20) << note.label << "\n";
        }
    }

    outputFile.close();
    midiFile.close();

    std::cout << "Analysis complete with initial timing offset fix applied. Output written to " << uniqueFilename << std::endl;
    std::cout << "FIX APPLIED: Only initial timing offset before first note-on is preserved." << std::endl;
    std::cout << "NOTE: All note durations remain unchanged - only initial track timing is corrected." << std::endl;
    return 0;
}