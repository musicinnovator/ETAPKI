// Enhanced Appoggiatura Transformation (C) 2025
// Version with percentage control, multiple choice variant selection, and bug fixes
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <ctime>
#include <iomanip>
#include <map>
#include <chrono>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <functional>

// Helper to get note name (from MIDI number)
std::string getNoteName(int noteNumber) {
    static const std::string noteNames[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
    int octave = noteNumber / 12 - 1;
    int noteIndex = noteNumber % 12;
    return noteNames[noteIndex] + std::to_string(octave);
}

// Helper to get MIDI number from note name
int getNoteNumber(const std::string& noteName) {
    static const std::string noteNames[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };

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

// Enum for time meter
enum TimeMeter {
    DUPLE,
    TRIPLE
};

// Helper function for basic two-note patterns
void handleMeter(std::vector<std::pair<int, int>>& EmbRet, int note1, int note2, int principal, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        // In duple meter, divide duration in half
        int halfDur = durPi / 2;
        EmbRet.push_back(std::make_pair(note1, halfDur));
        EmbRet.push_back(std::make_pair(note2, halfDur));
    } else if (meter == TRIPLE) {
        // In triple meter, divide duration into thirds
        int thirdDur = durPi / 3;
        EmbRet.push_back(std::make_pair(note1, thirdDur));
        EmbRet.push_back(std::make_pair(note2, thirdDur * 2));
    }
}

// Helper function for three-note patterns
void handleMeter3(std::vector<std::pair<int, int>>& EmbRet, int note1, int note2, int note3, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        // In duple meter, distribute as: 1/4, 1/4, 1/2
        int quarterDur = durPi / 4;
        int halfDur = durPi / 2;
        EmbRet.push_back(std::make_pair(note1, quarterDur));
        EmbRet.push_back(std::make_pair(note2, quarterDur));
        EmbRet.push_back(std::make_pair(note3, halfDur));
    } else if (meter == TRIPLE) {
        // In triple meter, divide duration into thirds
        int thirdDur = durPi / 3;
        EmbRet.push_back(std::make_pair(note1, thirdDur));
        EmbRet.push_back(std::make_pair(note2, thirdDur));
        EmbRet.push_back(std::make_pair(note3, thirdDur));
    }
}

// Main appoggiatura function - Complete implementation
std::vector<std::pair<int, int>> applyAppoggiatura(int pi, int durPi, TimeMeter meter, const std::string& variant) {
    if (durPi <= 0) {
        throw std::invalid_argument("Duration (durPi) must be greater than 0");
    }
    if (meter != DUPLE && meter != TRIPLE) {
        throw std::invalid_argument("Invalid TimeMeter");
    }

    std::vector<std::pair<int, int>> EmbRet;

    // RPNB - Repeat Previous Note Below Principal Note variants
    if (variant == "RPNB") {
        // Basic: Appoggiatura repeats the previous note from below the Principal Note
        int app = pi - 1; // Assume previous note is 1 semitone below (can be adjusted)
        handleMeter(EmbRet, app, pi, pi, durPi, meter);
    }
    else if (variant == "RPNB.PREb1") {
        // Repeats previous note below Principal with terminal 1 whole step below appoggiatura, followed by Principal
        int app = pi - 1;
        int terminal = app - 2; // 1 whole step below appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "RPNB.PREb5") {
        // Repeats previous note below Principal with terminal 1/2 step below appoggiatura, followed by Principal
        int app = pi - 1;
        int terminal = app - 1; // 1/2 step below appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "RPNB.PREa1") {
        // Repeats previous note below Principal with terminal 1 whole step above appoggiatura, followed by Principal
        int app = pi - 1;
        int terminal = app + 2; // 1 whole step above appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "RPNB.PREa5") {
        // Repeats previous note below Principal with terminal 1/2 step above appoggiatura, followed by Principal
        int app = pi - 1;
        int terminal = app + 1; // 1/2 step above appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "RPNB.POSTb1") {
        // Repeats previous note below Principal followed by Principal and ends with terminal 1 whole step below appoggiatura
        int app = pi - 1;
        int terminal = app - 2; // 1 whole step below appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "RPNB.POSTb5") {
        // Repeats previous note below Principal followed by Principal and ends with terminal 1/2 step below appoggiatura
        int app = pi - 1;
        int terminal = app - 1; // 1/2 step below appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "RPNB.POSTa1") {
        // Repeats previous note below Principal followed by Principal and ends with terminal 1 whole step above appoggiatura
        int app = pi - 1;
        int terminal = app + 2; // 1 whole step above appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "RPNB.POSTa5") {
        // Repeats previous note below Principal followed by Principal and ends with terminal 1/2 step above appoggiatura
        int app = pi - 1;
        int terminal = app + 1; // 1/2 step above appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }

  // RPNA - Repeat Previous Note Above Principal Note variants
    else if (variant == "RPNA") {
        // Basic: Appoggiatura repeats the previous note from above the Principal Note
        int app = pi + 1; // Assume previous note is 1 semitone above (can be adjusted)
        handleMeter(EmbRet, app, pi, pi, durPi, meter);
    }
    else if (variant == "RPNA.PREb1") {
        // Repeats previous note above Principal with terminal 1 whole step below appoggiatura, followed by Principal
        int app = pi + 1;
        int terminal = app - 2; // 1 whole step below appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "RPNA.PREb5") {
        // Repeats previous note above Principal with terminal 1/2 step below appoggiatura, followed by Principal
        int app = pi + 1;
        int terminal = app - 1; // 1/2 step below appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "RPNA.PREa5") {
        // Repeats previous note above Principal with terminal 1/2 step above appoggiatura, followed by Principal
        int app = pi + 1;
        int terminal = app + 1; // 1/2 step above appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "RPNA.PREa1") {
        // Repeats previous note above Principal with terminal 1 whole step above appoggiatura, followed by Principal
        int app = pi + 1;
        int terminal = app + 2; // 1 whole step above appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "RPNA.POSTb1") {
        // Repeats previous note above Principal followed by Principal and ends with terminal 1 whole step below appoggiatura
        int app = pi + 1;
        int terminal = app - 2; // 1 whole step below appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "RPNA.POSTb5") {
        // Repeats previous note above Principal followed by Principal and ends with terminal 1/2 step below appoggiatura
        int app = pi + 1;
        int terminal = app - 1; // 1/2 step below appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "RPNA.POSTa5") {
        // Repeats previous note above Principal followed by Principal and ends with terminal 1/2 step above appoggiatura
        int app = pi + 1;
        int terminal = app + 1; // 1/2 step above appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "RPNA.POSTa1") {
        // Repeats previous note above Principal followed by Principal and ends with terminal 1 whole step above appoggiatura
        int app = pi + 1;
        int terminal = app + 2; // 1 whole step above appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }

    // SAN - Step Above Note variants (approaches principle note from above)
    else if (variant == "SAN1") {
        // Appoggiatura approaches principle note from 1 whole step above
        int app = pi + 2; // 1 whole step above
        handleMeter(EmbRet, app, pi, pi, durPi, meter);
    }
    else if (variant == "SAN5") {
        // Appoggiatura approaches principle note from 1/2 step above
        int app = pi + 1; // 1/2 step above
        handleMeter(EmbRet, app, pi, pi, durPi, meter);
    }
    else if (variant == "SAN1.PREb1") {
        // Approaches from 1 whole step above with terminal 1 whole step below appoggiatura
        int app = pi + 2;
        int terminal = app - 2; // 1 whole step below appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "SAN5.PREb5") {
        // Approaches from 1/2 step above with terminal 1/2 step below appoggiatura
        int app = pi + 1;
        int terminal = app - 1; // 1/2 step below appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "SAN1.PREb5") {
        // Approaches from 1 whole step above with terminal 1/2 step below appoggiatura
        int app = pi + 2;
        int terminal = app - 1; // 1/2 step below appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "SAN5.PREb1") {
        // Approaches from 1/2 step above with terminal 1 whole step below appoggiatura
        int app = pi + 1;
        int terminal = app - 2; // 1 whole step below appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "SAN1.PREa1") {
        // Approaches from 1 whole step above with terminal 1 whole step above appoggiatura
        int app = pi + 2;
        int terminal = app + 2; // 1 whole step above appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "SAN5.PREa5") {
        // Approaches from 1/2 step above with terminal 1/2 step above appoggiatura
        int app = pi + 1;
        int terminal = app + 1; // 1/2 step above appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "SAN1.PREa5") {
        // Approaches from 1 whole step above with terminal 1/2 step above appoggiatura
        int app = pi + 2;
        int terminal = app + 1; // 1/2 step above appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "SAN5.PREa1") {
        // Approaches from 1/2 step above with terminal 1 whole step above appoggiatura
        int app = pi + 1;
        int terminal = app + 2; // 1 whole step above appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }

    // SAN POST variants
    else if (variant == "SAN1.POSTa1") {
        // Approaches from 1 whole step above followed by principal with terminal 1 whole step above appoggiatura
        int app = pi + 2;
        int terminal = app + 2; // 1 whole step above appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "SAN1.POSTa5") {
        // Approaches from 1 whole step above followed by principal with terminal 1/2 step above appoggiatura
        int app = pi + 2;
        int terminal = app + 1; // 1/2 step above appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "SAN5.POSTa5") {
        // Approaches from 1/2 step above followed by principal with terminal 1/2 step above appoggiatura
        int app = pi + 1;
        int terminal = app + 1; // 1/2 step above appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "SAN5.POSTa1") {
        // Approaches from 1/2 step above followed by principal with terminal 1 whole step above appoggiatura
        int app = pi + 1;
        int terminal = app + 2; // 1 whole step above appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "SAN1.POSTb1") {
        // Approaches from 1 whole step above followed by principal with terminal 1 whole step below appoggiatura
        int app = pi + 2;
        int terminal = app - 2; // 1 whole step below appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "SAN1.POSTb5") {
        // Approaches from 1 whole step above followed by principal with terminal 1/2 step below appoggiatura
        int app = pi + 2;
        int terminal = app - 1; // 1/2 step below appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "SAN5.POSTb5") {
        // Approaches from 1/2 step above followed by principal with terminal 1/2 step below appoggiatura
        int app = pi + 1;
        int terminal = app - 1; // 1/2 step below appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "SAN5.POSTb1") {
        // Approaches from 1/2 step above followed by principal with terminal 1 whole step below appoggiatura
        int app = pi + 1;
        int terminal = app - 2; // 1 whole step below appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }

    // SBN - Step Below Note variants (approaches principle note from below)
    else if (variant == "SBN1") {
        // Appoggiatura approaches principle note from 1 whole step below
        int app = pi - 2; // 1 whole step below
        handleMeter(EmbRet, app, pi, pi, durPi, meter);
    }
    else if (variant == "SBN5") {
        // Appoggiatura approaches principle note from 1/2 step below
        int app = pi - 1; // 1/2 step below
        handleMeter(EmbRet, app, pi, pi, durPi, meter);
    }
    else if (variant == "SBN1.PREb1") {
        // Approaches from 1 whole step below with terminal 1 whole step below appoggiatura
        int app = pi - 2;
        int terminal = app - 2; // 1 whole step below appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "SBN1.PREb5") {
        // Approaches from 1 whole step below with terminal 1/2 step below appoggiatura
        int app = pi - 2;
        int terminal = app - 1; // 1/2 step below appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "SBN5.PREb5") {
        // Approaches from 1/2 step below with terminal 1/2 step below appoggiatura
        int app = pi - 1;
        int terminal = app - 1; // 1/2 step below appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "SBN5.PREb1") {
        // Approaches from 1/2 step below with terminal 1 whole step below appoggiatura
        int app = pi - 1;
        int terminal = app - 2; // 1 whole step below appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "SBN1.PREa1") {
        // Approaches from 1 whole step below with terminal 1 whole step above appoggiatura
        int app = pi - 2;
        int terminal = app + 2; // 1 whole step above appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "SBN1.PREa5") {
        // Approaches from 1 whole step below with terminal 1/2 step above appoggiatura
        int app = pi - 2;
        int terminal = app + 1; // 1/2 step above appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "SBN5.PREa5") {
        // Approaches from 1/2 step below with terminal 1/2 step above appoggiatura
        int app = pi - 1;
        int terminal = app + 1; // 1/2 step above appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "SBN5.PREa1") {
        // Approaches from 1/2 step below with terminal 1 whole step above appoggiatura
        int app = pi - 1;
        int terminal = app + 2; // 1 whole step above appoggiatura
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }

    // SBN POST variants
    else if (variant == "SBN1.POSTb1") {
        // Approaches from 1 whole step below followed by principal with terminal 1 whole step below appoggiatura
        int app = pi - 2;
        int terminal = app - 2; // 1 whole step below appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "SBN1.POSTb5") {
        // Approaches from 1 whole step below followed by principal with terminal 1/2 step below appoggiatura
        int app = pi - 2;
        int terminal = app - 1; // 1/2 step below appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "SBN5.POSTb5") {
        // Approaches from 1/2 step below followed by principal with terminal 1/2 step below appoggiatura
        int app = pi - 1;
        int terminal = app - 1; // 1/2 step below appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "SBN5.POSTb1") {
        // Approaches from 1/2 step below followed by principal with terminal 1 whole step below appoggiatura
        int app = pi - 1;
        int terminal = app - 2; // 1 whole step below appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "SBN1.POSTa1") {
        // Approaches from 1 whole step below followed by principal with terminal 1 whole step above appoggiatura
        int app = pi - 2;
        int terminal = app + 2; // 1 whole step above appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "SBN1.POSTa5") {
        // Approaches from 1 whole step below followed by principal with terminal 1/2 step above appoggiatura
        int app = pi - 2;
        int terminal = app + 1; // 1/2 step above appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "SBN5.POSTa5") {
        // Approaches from 1/2 step below followed by principal with terminal 1/2 step above appoggiatura
        int app = pi - 1;
        int terminal = app + 1; // 1/2 step above appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "SBN5.POSTa1") {
        // Approaches from 1/2 step below followed by principal with terminal 1 whole step above appoggiatura
        int app = pi - 1;
        int terminal = app + 2; // 1 whole step above appoggiatura
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }

    // LURP - Long Up Repeat Previous (repetition of preceding tone greater than a 3rd below principal note)
    else if (variant == "LURP") {
        // Basic: repetition of preceding tone greater than a 3rd below principal note
        int app = pi - 4; // Assume 4 semitones below (major 3rd)
        handleMeter(EmbRet, app, pi, pi, durPi, meter);
    }
    else if (variant == "LURP1.PREb1") {
        // Repetition with terminal prefix 1 step below app and terminal 1 step below principal
        int app = pi - 4;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LURP1.PREb5") {
        // Repetition with terminal prefix 1/2 step below app and terminal 1 step below principal
        int app = pi - 4;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LURP5.PREb5") {
        // Repetition with terminal prefix 1/2 step below app and terminal 1/2 step below principal
        int app = pi - 4;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LURP5.PREb1") {
        // Repetition with terminal prefix 1 step below app and terminal 1/2 step below principal
        int app = pi - 4;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LURP1.PREa1") {
        // Repetition with terminal prefix 1 step above app and terminal 1 step below principal
        int app = pi - 4;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LURP1.PREa5") {
        // Repetition with terminal prefix 1/2 step above app and terminal 1 step below principal
        int app = pi - 4;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LURP5.PREa5") {
        // Repetition with terminal prefix 1/2 step above app and terminal 1/2 step below principal
        int app = pi - 4;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LURP5.PREa1") {
        // Repetition with terminal prefix 1 step above app and terminal 1/2 step below principal
        int app = pi - 4;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }

    // LURP POST variants
    else if (variant == "LURP1.POSTb1") {
        // Repetition with terminal postfix 1 step below app and terminal 1 step below principal
        int app = pi - 4;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LURP1.POSTb5") {
        // Repetition with terminal postfix 1/2 step below app and terminal 1 step below principal
        int app = pi - 4;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LURP5.POSTb5") {
        // Repetition with terminal postfix 1/2 step below app and terminal 1/2 step below principal
        int app = pi - 4;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LURP5.POSTb1") {
        // Repetition with terminal postfix 1 step below app and terminal 1/2 step below principal
        int app = pi - 4;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LURP1.POSTa1") {
        // Repetition with terminal postfix 1 step above app and terminal 1 step below principal
        int app = pi - 4;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LURP1.POSTa5") {
        // Repetition with terminal postfix 1/2 step above app and terminal 1 step below principal
        int app = pi - 4;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LURP5.POSTa5") {
        // Repetition with terminal postfix 1/2 step above app and terminal 1/2 step below principal
        int app = pi - 4;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LURP5.POSTa1") {
        // Repetition with terminal postfix 1 step above app and terminal 1/2 step below principal
        int app = pi - 4;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }

    // LDRP - Long Down Repeat Previous (repetition of preceding tone greater than a 3rd above principal note)
    else if (variant == "LDRP") {
        // Basic: repetition of preceding tone greater than a 3rd above principal note
        int app = pi + 4; // Assume 4 semitones above (major 3rd)
        handleMeter(EmbRet, app, pi, pi, durPi, meter);
    }
    else if (variant == "LDRP1.PREb1") {
        // Repetition with terminal prefix 1 step below app and terminal 1 step below principal
        int app = pi + 4;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDRP1.PREb5") {
        // Repetition with terminal prefix 1/2 step below app and terminal 1 step below principal
        int app = pi + 4;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDRP5.PREb5") {
        // Repetition with terminal prefix 1/2 step below app and terminal 1/2 step below principal
        int app = pi + 4;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDRP5.PREb1") {
        // Repetition with terminal prefix 1 step below app and terminal 1/2 step below principal
        int app = pi + 4;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDRP1.PREa1") {
        // Repetition with terminal prefix 1 step above app and terminal 1 step below principal
        int app = pi + 4;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDRP1.PREa5") {
        // Repetition with terminal prefix 1/2 step above app and terminal 1 step below principal
        int app = pi + 4;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDRP5.PREa5") {
        // Repetition with terminal prefix 1/2 step above app and terminal 1/2 step below principal
        int app = pi + 4;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDRP5.PREa1") {
        // Repetition with terminal prefix 1 step above app and terminal 1/2 step below principal
        int app = pi + 4;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }

    // LDRP POST variants
    else if (variant == "LDRP1.POSTb1") {
        // Repetition with terminal postfix 1 step below app and terminal 1 step below principal
        int app = pi + 4;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDRP1.POSTb5") {
        // Repetition with terminal postfix 1/2 step below app and terminal 1 step below principal
        int app = pi + 4;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDRP5.POSTb5") {
        // Repetition with terminal postfix 1/2 step below app and terminal 1/2 step below principal
        int app = pi + 4;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDRP5.POSTb1") {
        // Repetition with terminal postfix 1 step below app and terminal 1/2 step below principal
        int app = pi + 4;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDRP1.POSTa1") {
        // Repetition with terminal postfix 1 step above app and terminal 1 step below principal
        int app = pi + 4;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDRP1.POSTa5") {
        // Repetition with terminal postfix 1/2 step above app and terminal 1 step below principal
        int app = pi + 4;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDRP5.POSTa5") {
        // Repetition with terminal postfix 1/2 step above app and terminal 1/2 step below principal
        int app = pi + 4;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDRP5.POSTa1") {
        // Repetition with terminal postfix 1 step above app and terminal 1/2 step below principal
        int app = pi + 4;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }

    // LUSA - Long Up Step Above (appoggiatura is step above principal after leap > 3rd above)
    else if (variant == "LUSA1") {
        // Appoggiatura is 1 step above principal note and comes after a leap of greater than a 3rd above
        int app = pi + 2; // 1 whole step above principal
        handleMeter(EmbRet, app, pi, pi, durPi, meter);
    }
    else if (variant == "LUSA5") {
        // Appoggiatura is 1/2 step above principal note and comes after a leap of greater than a 3rd above
        int app = pi + 1; // 1/2 step above principal
        handleMeter(EmbRet, app, pi, pi, durPi, meter);
    }
    else if (variant == "LUSA1.PREb1") {
        // With whole step below prefix, is 1 step above principal and is a leap > 3rd above previous note
        int app = pi + 2;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LUSA1.PREb5") {
        // With 1/2 step below prefix, is 1 step above principal and is a leap > 3rd above previous note
        int app = pi + 2;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LUSA5.PREb5") {
        // With 1/2 step below prefix, is 1/2 step above principal and is a leap > 3rd above previous note
        int app = pi + 1;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LUSA5.PREb1") {
        // With whole step below prefix, is 1/2 step above principal and is a leap > 3rd above previous note
        int app = pi + 1;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LUSA1.PREa1") {
        // With whole step above prefix, is 1 step above principal and is a leap > 3rd above previous note
        int app = pi + 2;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LUSA1.PREa5") {
        // With 1/2 step above prefix, is 1 step above principal and is a leap > 3rd above previous note
        int app = pi + 2;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LUSA5.PREa5") {
        // With 1/2 step above prefix, is 1/2 step above principal and is a leap > 3rd above previous note
        int app = pi + 1;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LUSA5.PREa1") {
        // With whole step above prefix, is 1/2 step above principal and is a leap > 3rd above previous note
        int app = pi + 1;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }

    // LUSA POST variants
    else if (variant == "LUSA1.POSTb1") {
        // With whole step below postfix, is 1 step above principal and is a leap > 3rd above previous note
        int app = pi + 2;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LUSA1.POSTb5") {
        // With 1/2 step below postfix, is 1 step above principal and is a leap > 3rd above previous note
        int app = pi + 2;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LUSA5.POSTb5") {
        // With 1/2 step below postfix, is 1/2 step above principal and is a leap > 3rd above previous note
        int app = pi + 1;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LUSA5.POSTb1") {
        // With whole step below postfix, is 1/2 step above principal and is a leap > 3rd above previous note
        int app = pi + 1;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LUSA1.POSTa1") {
        // With whole step above postfix, is 1 step above principal and is a leap > 3rd above previous note
        int app = pi + 2;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LUSA1.POSTa5") {
        // With 1/2 step above postfix, is 1 step above principal and is a leap > 3rd above previous note
        int app = pi + 2;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LUSA5.POSTa5") {
        // With 1/2 step above postfix, is 1/2 step above principal and is a leap > 3rd above previous note
        int app = pi + 1;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LUSA5.POSTa1") {
        // With whole step above postfix, is 1/2 step above principal and is a leap > 3rd above previous note
        int app = pi + 1;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }

    // LUSB - Long Up Step Below (appoggiatura is step below principal after leap > 3rd above)
    else if (variant == "LUSB1") {
        // Appoggiatura is 1 step below principal note and comes after a leap of greater than a 3rd above
        int app = pi - 2; // 1 whole step below principal
        handleMeter(EmbRet, app, pi, pi, durPi, meter);
    }
    else if (variant == "LUSB5") {
        // Appoggiatura is 1/2 step below principal note and comes after a leap of greater than a 3rd above
        int app = pi - 1; // 1/2 step below principal
        handleMeter(EmbRet, app, pi, pi, durPi, meter);
    }
    else if (variant == "LUSB1.PREb1") {
        // With whole step below prefix, is 1 step below principal and is a leap > 3rd above previous note
        int app = pi - 2;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LUSB1.PREb5") {
        // With 1/2 step below prefix, is 1 step below principal and is a leap > 3rd above previous note
        int app = pi - 2;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LUSB5.PREb5") {
        // With 1/2 step below prefix, is 1/2 step below principal and is a leap > 3rd above previous note
        int app = pi - 1;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LUSB5.PREb1") {
        // With whole step below prefix, is 1/2 step below principal and is a leap > 3rd above previous note
        int app = pi - 1;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LUSB1.PREa1") {
        // With whole step above prefix, is 1 step below principal and is a leap > 3rd above previous note
        int app = pi - 2;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LUSB1.PREa5") {
        // With 1/2 step above prefix, is 1 step below principal and is a leap > 3rd above previous note
        int app = pi - 2;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LUSB5.PREa5") {
        // With 1/2 step above prefix, is 1/2 step below principal and is a leap > 3rd above previous note
        int app = pi - 1;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LUSB5.PREa1") {
        // With whole step above prefix, is 1/2 step below principal and is a leap > 3rd above previous note
        int app = pi - 1;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }

    // LUSB POST variants
    else if (variant == "LUSB1.POSTb1") {
        // With whole step below postfix, is 1 step below principal and is a leap > 3rd above previous note
        int app = pi - 2;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LUSB1.POSTb5") {
        // With 1/2 step below postfix, is 1 step below principal and is a leap > 3rd above previous note
        int app = pi - 2;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LUSB5.POSTb5") {
        // With 1/2 step below postfix, is 1/2 step below principal and is a leap > 3rd above previous note
        int app = pi - 1;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LUSB5.POSTb1") {
        // With whole step below postfix, is 1/2 step below principal and is a leap > 3rd above previous note
        int app = pi - 1;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LUSB1.POSTa1") {
        // With whole step above postfix, is 1 step below principal and is a leap > 3rd above previous note
        int app = pi - 2;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LUSB1.POSTa5") {
        // With 1/2 step above postfix, is 1 step below principal and is a leap > 3rd above previous note
        int app = pi - 2;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LUSB5.POSTa5") {
        // With 1/2 step above postfix, is 1/2 step below principal and is a leap > 3rd above previous note
        int app = pi - 1;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LUSB5.POSTa1") {
        // With whole step above postfix, is 1/2 step below principal and is a leap > 3rd above previous note
        int app = pi - 1;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }

    // LDSA - Long Down Step Above (appoggiatura is step above principal preceded by leap > 3rd below)
    else if (variant == "LDSA1") {
        // Appoggiatura is 1 step above principal that is preceded by a note that is a leap > 3rd below principal
        int app = pi + 2; // 1 whole step above principal
        handleMeter(EmbRet, app, pi, pi, durPi, meter);
    }
    else if (variant == "LDSA5") {
        // Appoggiatura is 1/2 step above principal that is preceded by a note that is a leap > 3rd below principal
        int app = pi + 1; // 1/2 step above principal
        handleMeter(EmbRet, app, pi, pi, durPi, meter);
    }
    else if (variant == "LDSA1.PREb1") {
        // Is one whole step above principal preceded by leap > 3rd below, with terminal prefixed one whole step below appoggiatura
        int app = pi + 2;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDSA1.PREb5") {
        // Is one whole step above principal preceded by leap > 3rd below, with terminal prefixed 1/2 step below appoggiatura
        int app = pi + 2;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDSA5.PREb5") {
        // Is 1/2 step above principal preceded by leap > 3rd below, with terminal prefixed 1/2 step below appoggiatura
        int app = pi + 1;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDSA5.PREb1") {
        // Is 1/2 step above principal preceded by leap > 3rd below, with terminal prefixed one whole step below appoggiatura
        int app = pi + 1;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDSA1.PREa1") {
        // Is one whole step above principal preceded by leap > 3rd below, with terminal prefixed one whole step above appoggiatura
        int app = pi + 2;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDSA1.PREa5") {
        // Is one whole step above principal preceded by leap > 3rd below, with terminal prefixed 1/2 step above appoggiatura
        int app = pi + 2;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDSA5.PREa5") {
        // Is 1/2 step above principal preceded by leap > 3rd below, with terminal prefixed 1/2 step above appoggiatura
        int app = pi + 1;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDSA5.PREa1") {
        // Is 1/2 step above principal preceded by leap > 3rd below, with terminal prefixed one whole step above appoggiatura
        int app = pi + 1;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }

    // LDSA POST variants
    else if (variant == "LDSA1.POSTb1") {
        // Is one whole step above principal preceded by leap > 3rd below, with terminal postfixed one whole step below appoggiatura
        int app = pi + 2;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDSA1.POSTb5") {
        // Is one whole step above principal preceded by leap > 3rd below, with terminal postfixed 1/2 step below appoggiatura
        int app = pi + 2;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDSA5.POSTb5") {
        // Is 1/2 step above principal preceded by leap > 3rd below, with terminal postfixed 1/2 step below appoggiatura
        int app = pi + 1;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDSA5.POSTb1") {
        // Is 1/2 step above principal preceded by leap > 3rd below, with terminal postfixed one whole step below appoggiatura
        int app = pi + 1;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDSA1.POSTa1") {
        // Is one whole step above principal preceded by leap > 3rd below, with terminal postfixed one whole step above appoggiatura
        int app = pi + 2;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDSA1.POSTa5") {
        // Is one whole step above principal preceded by leap > 3rd below, with terminal postfixed 1/2 step above appoggiatura
        int app = pi + 2;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDSA5.POSTa5") {
        // Is 1/2 step above principal preceded by leap > 3rd below, with terminal postfixed 1/2 step above appoggiatura
        int app = pi + 1;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDSA5.POSTa1") {
        // Is 1/2 step above principal preceded by leap > 3rd below, with terminal postfixed one whole step above appoggiatura
        int app = pi + 1;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }

    // LDSB - Long Down Step Below (appoggiatura is step below principal preceded by leap > 3rd below)
    else if (variant == "LDSB1") {
        // Appoggiatura is 1 step below principal that is preceded by a note that is a leap > 3rd below principal
        int app = pi - 2; // 1 whole step below principal
        handleMeter(EmbRet, app, pi, pi, durPi, meter);
    }
    else if (variant == "LDSB5") {
        // Appoggiatura is 1/2 step below principal that is preceded by a note that is a leap > 3rd below principal
        int app = pi - 1; // 1/2 step below principal
        handleMeter(EmbRet, app, pi, pi, durPi, meter);
    }
    else if (variant == "LDSB1.PREb1") {
        // Is one whole step below principal preceded by leap > 3rd below, with terminal prefixed one whole step below appoggiatura
        int app = pi - 2;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDSB1.PREb5") {
        // Is one whole step below principal preceded by leap > 3rd below, with terminal prefixed 1/2 step below appoggiatura
        int app = pi - 2;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDSB5.PREb5") {
        // Is 1/2 step below principal preceded by leap > 3rd below, with terminal prefixed 1/2 step below appoggiatura
        int app = pi - 1;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDSB5.PREb1") {
        // Is 1/2 step below principal preceded by leap > 3rd below, with terminal prefixed one whole step below appoggiatura
        int app = pi - 1;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDSB1.PREa1") {
        // Is one whole step below principal preceded by leap > 3rd below, with terminal prefixed one whole step above appoggiatura
        int app = pi - 2;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDSB1.PREa5") {
        // Is one whole step below principal preceded by leap > 3rd below, with terminal prefixed 1/2 step above appoggiatura
        int app = pi - 2;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDSB5.PREa5") {
        // Is 1/2 step below principal preceded by leap > 3rd below, with terminal prefixed 1/2 step above appoggiatura
        int app = pi - 1;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }
    else if (variant == "LDSB5.PREa1") {
        // Is 1/2 step below principal preceded by leap > 3rd below, with terminal prefixed one whole step above appoggiatura
        int app = pi - 1;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, terminal, pi, durPi, meter);
    }

    // LDSB POST variants
    else if (variant == "LDSB1.POSTb1") {
        // Is one whole step below principal preceded by leap > 3rd below, with terminal postfixed one whole step below appoggiatura
        int app = pi - 2;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDSB1.POSTb5") {
        // Is one whole step below principal preceded by leap > 3rd below, with terminal postfixed 1/2 step below appoggiatura
        int app = pi - 2;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDSB5.POSTb5") {
        // Is 1/2 step below principal preceded by leap > 3rd below, with terminal postfixed 1/2 step below appoggiatura
        int app = pi - 1;
        int terminal = app - 1; // 1/2 step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDSB5.POSTb1") {
        // Is 1/2 step below principal preceded by leap > 3rd below, with terminal postfixed one whole step below appoggiatura
        int app = pi - 1;
        int terminal = app - 2; // 1 whole step below app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDSB1.POSTa1") {
        // Is one whole step below principal preceded by leap > 3rd below, with terminal postfixed one whole step above appoggiatura
        int app = pi - 2;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDSB1.POSTa5") {
        // Is one whole step below principal preceded by leap > 3rd below, with terminal postfixed 1/2 step above appoggiatura
        int app = pi - 2;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDSB5.POSTa5") {
        // Is 1/2 step below principal preceded by leap > 3rd below, with terminal postfixed 1/2 step above appoggiatura
        int app = pi - 1;
        int terminal = app + 1; // 1/2 step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }
    else if (variant == "LDSB5.POSTa1") {
        // Is 1/2 step below principal preceded by leap > 3rd below, with terminal postfixed one whole step above appoggiatura
        int app = pi - 1;
        int terminal = app + 2; // 1 whole step above app
        handleMeter3(EmbRet, app, pi, terminal, durPi, meter);
    }

    else {
        // Handle unknown variant
        std::cerr << "Unknown appoggiatura variant: " << variant << '\n';
    }

    return EmbRet;
}

// NEW FUNCTION: Generate a random pool of variants for user selection
std::vector<std::string> generateRandomVariantPool(int poolSize = 10) {
    // Define the complete pool of appoggiatura variants
    const std::vector<std::string> allVariants = {
        // RPNB - Repeat Previous Note Below variants
        "RPNB", "RPNB.PREb1", "RPNB.PREb5", "RPNB.PREa1", "RPNB.PREa5",
        "RPNB.POSTb1", "RPNB.POSTb5", "RPNB.POSTa1", "RPNB.POSTa5",

        // RPNA - Repeat Previous Note Above variants
        "RPNA",
        "RPNA.PREb1", "RPNA.PREb5", "RPNA.PREa5", "RPNA.PREa1",
        "RPNA.POSTb1", "RPNA.POSTb5", "RPNA.POSTa5", "RPNA.POSTa1",

        // SAN - Step Above Note variants
        "SAN1", "SAN5",
        "SAN1.PREb1", "SAN5.PREb5", "SAN1.PREb5", "SAN5.PREb1",
        "SAN1.PREa1", "SAN5.PREa5", "SAN1.PREa5", "SAN5.PREa1",
        "SAN1.POSTa1", "SAN1.POSTa5", "SAN5.POSTa5", "SAN5.POSTa1",
        "SAN1.POSTb1", "SAN1.POSTb5", "SAN5.POSTb5", "SAN5.POSTb1",

        // SBN - Step Below Note variants
        "SBN1", "SBN5",
        "SBN1.PREb1", "SBN1.PREb5", "SBN5.PREb5", "SBN5.PREb1",
        "SBN1.PREa1", "SBN1.PREa5", "SBN5.PREa5", "SBN5.PREa1",
        "SBN1.POSTb1", "SBN1.POSTb5", "SBN5.POSTb5", "SBN5.POSTb1",
        "SBN1.POSTa1", "SBN1.POSTa5", "SBN5.POSTa5", "SBN5.POSTa1",

        // LURP - Long Up Repeat Previous variants
        "LURP",
        "LURP1.PREb1", "LURP1.PREb5", "LURP5.PREb5", "LURP5.PREb1",
        "LURP1.PREa1", "LURP1.PREa5", "LURP5.PREa5", "LURP5.PREa1",
        "LURP1.POSTb1", "LURP1.POSTb5", "LURP5.POSTb5", "LURP5.POSTb1",
        "LURP1.POSTa1", "LURP1.POSTa5", "LURP5.POSTa5", "LURP5.POSTa1",

        // LDRP - Long Down Repeat Previous variants
        "LDRP",
        "LDRP1.PREb1", "LDRP1.PREb5", "LDRP5.PREb5", "LDRP5.PREb1",
        "LDRP1.PREa1", "LDRP1.PREa5", "LDRP5.PREa5", "LDRP5.PREa1",
        "LDRP1.POSTb1", "LDRP1.POSTb5", "LDRP5.POSTb5", "LDRP5.POSTb1",
        "LDRP1.POSTa1", "LDRP1.POSTa5", "LDRP5.POSTa5", "LDRP5.POSTa1",

        // LUSA - Long Up Step Above variants
        "LUSA1", "LUSA5",
        "LUSA1.PREb1", "LUSA1.PREb5", "LUSA5.PREb5", "LUSA5.PREb1",
        "LUSA1.PREa1", "LUSA1.PREa5", "LUSA5.PREa5", "LUSA5.PREa1",
        "LUSA1.POSTb1", "LUSA1.POSTb5", "LUSA5.POSTb5", "LUSA5.POSTb1",
        "LUSA1.POSTa1", "LUSA1.POSTa5", "LUSA5.POSTa5", "LUSA5.POSTa1",

        // LUSB - Long Up Step Below variants
        "LUSB1", "LUSB5",
        "LUSB1.PREb1", "LUSB1.PREb5", "LUSB5.PREb5", "LUSB5.PREb1",
        "LUSB1.PREa1", "LUSB1.PREa5", "LUSB5.PREa5", "LUSB5.PREa1",
        "LUSB1.POSTb1", "LUSB1.POSTb5", "LUSB5.POSTb5", "LUSB5.POSTb1",
        "LUSB1.POSTa1", "LUSB1.POSTa5", "LUSB5.POSTa5", "LUSB5.POSTa1",

        // LDSA - Long Down Step Above variants
        "LDSA1", "LDSA5",
        "LDSA1.PREb1", "LDSA1.PREb5", "LDSA5.PREb5", "LDSA5.PREb1",
        "LDSA1.PREa1", "LDSA1.PREa5", "LDSA5.PREa5", "LDSA5.PREa1",
        "LDSA1.POSTb1", "LDSA1.POSTb5", "LDSA5.POSTb5", "LDSA5.POSTb1",
        "LDSA1.POSTa1", "LDSA1.POSTa5", "LDSA5.POSTa5", "LDSA5.POSTa1",

        // LDSB - Long Down Step Below variants
        "LDSB1", "LDSB5",
        "LDSB1.PREb1", "LDSB1.PREb5", "LDSB5.PREb5", "LDSB5.PREb1",
        "LDSB1.PREa1", "LDSB1.PREa5", "LDSB5.PREa5", "LDSB5.PREa1",
        "LDSB1.POSTb1", "LDSB1.POSTb5", "LDSB5.POSTb5", "LDSB5.POSTb1",
        "LDSB1.POSTa1", "LDSB1.POSTa5", "LDSB5.POSTa5", "LDSB5.POSTa1"
    };

    // Create a copy of all variants and shuffle it
    std::vector<std::string> shuffledVariants = allVariants;

    // Use modern random number generation
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

// NEW FUNCTION: Parse user input for multiple choice selection
std::vector<int> parseUserChoices(const std::string& input, int maxChoice) {
    std::vector<int> choices;
    std::istringstream iss(input);
    std::string token;

    while (iss >> token) {
        try {
            int choice = std::stoi(token);
            if (choice >= 1 && choice <= maxChoice) {
                // Avoid duplicates
                if (std::find(choices.begin(), choices.end(), choice) == choices.end()) {
                    choices.push_back(choice);
                }
            }
        } catch (const std::exception&) {
            // Ignore invalid tokens
        }
    }

    return choices;
}

// NEW FUNCTION: Let user choose from a pool of variants with multiple choice support
std::vector<std::string> getUserVariantChoices() {
    std::cout << "\n=== Enhanced Appoggiatura Variant Selection ===" << std::endl;
    std::cout << "Choose your variant selection method:" << std::endl;
    std::cout << "1. Random selection (original behavior)" << std::endl;
    std::cout << "2. Choose single variant from a random pool" << std::endl;
    std::cout << "3. Choose multiple variants from a random pool" << std::endl;
    std::cout << "Enter your choice (1, 2, or 3): ";

    int choice;
    while (!(std::cin >> choice) || (choice < 1 || choice > 3)) {
        std::cout << "Invalid input. Please enter 1, 2, or 3: ";
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }

    if (choice == 1) {
        std::cout << "Using random variant selection." << std::endl;
        std::cout << "===============================================\n" << std::endl;
        return {"RANDOM"}; // Special flag for random selection
    }

    // Generate random pool of variants
    std::vector<std::string> variantPool = generateRandomVariantPool(15); // Increased pool size

    std::cout << "\nHere are " << variantPool.size() << " randomly selected variants to choose from:" << std::endl;
    std::cout << "=================================================================" << std::endl;

    for (size_t i = 0; i < variantPool.size(); ++i) {
        std::cout << std::setw(2) << (i + 1) << ". " << variantPool[i] << std::endl;
    }

    std::vector<std::string> selectedVariants;

    if (choice == 2) {
        // Single choice
        std::cout << "\nEnter the number of your preferred variant (1-" << variantPool.size() << "): ";

        int variantChoice;
        while (!(std::cin >> variantChoice) || variantChoice < 1 || variantChoice > static_cast<int>(variantPool.size())) {
            std::cout << "Invalid choice. Please enter a number between 1 and " << variantPool.size() << ": ";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }

        selectedVariants.push_back(variantPool[variantChoice - 1]);
        std::cout << "You selected: " << selectedVariants[0] << std::endl;
    } else {
        // Multiple choice
        std::cout << "\nSelect from any number (separated by spaces) from 1 to " << variantPool.size()
                  << " of the variants you would like to randomly apply:" << std::endl;
        std::cout << "Example: '1 3 7 12' to select variants 1, 3, 7, and 12" << std::endl;
        std::cout << "Your choices: ";

        std::cin.ignore(); // Clear the newline from previous input
        std::string userInput;
        std::getline(std::cin, userInput);

        std::vector<int> choices = parseUserChoices(userInput, variantPool.size());

        if (choices.empty()) {
            std::cout << "No valid choices entered. Using random selection instead." << std::endl;
            return {"RANDOM"};
        }

        // Convert choices to variant names
        for (int choice : choices) {
            selectedVariants.push_back(variantPool[choice - 1]);
        }

        std::cout << "You selected " << selectedVariants.size() << " variants:" << std::endl;
        for (size_t i = 0; i < selectedVariants.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << selectedVariants[i] << std::endl;
        }
    }

    std::cout << "===============================================\n" << std::endl;
    return selectedVariants;
}

// ENHANCED applyVariant function with user choice support
std::vector<std::pair<int, int>> applyVariant(int pi, int durPi, std::string& selectedVariant, const std::vector<std::string>& userChoices = {}) {
    if (userChoices.empty() || (userChoices.size() == 1 && userChoices[0] == "RANDOM")) {
        // Original random selection behavior
        const std::vector<std::string> variants = {
            // RPNB - Repeat Previous Note Below variants
            "RPNB", "RPNB.PREb1", "RPNB.PREb5", "RPNB.PREa1", "RPNB.PREa5",
            "RPNB.POSTb1", "RPNB.POSTb5", "RPNB.POSTa1", "RPNB.POSTa5",

            // RPNA - Repeat Previous Note Above variants
            "RPNA",
            "RPNA.PREb1", "RPNA.PREb5", "RPNA.PREa5", "RPNA.PREa1",
            "RPNA.POSTb1", "RPNA.POSTb5", "RPNA.POSTa5", "RPNA.POSTa1",

            // SAN - Step Above Note variants
            "SAN1", "SAN5",
            "SAN1.PREb1", "SAN5.PREb5", "SAN1.PREb5", "SAN5.PREb1",
            "SAN1.PREa1", "SAN5.PREa5", "SAN1.PREa5", "SAN5.PREa1",
            "SAN1.POSTa1", "SAN1.POSTa5", "SAN5.POSTa5", "SAN5.POSTa1",
            "SAN1.POSTb1", "SAN1.POSTb5", "SAN5.POSTb5", "SAN5.POSTb1",

            // SBN - Step Below Note variants
            "SBN1", "SBN5",
            "SBN1.PREb1", "SBN1.PREb5", "SBN5.PREb5", "SBN5.PREb1",
            "SBN1.PREa1", "SBN1.PREa5", "SBN5.PREa5", "SBN5.PREa1",
            "SBN1.POSTb1", "SBN1.POSTb5", "SBN5.POSTb5", "SBN5.POSTb1",
            "SBN1.POSTa1", "SBN1.POSTa5", "SBN5.POSTa5", "SBN5.POSTa1",

            // LURP - Long Up Repeat Previous variants
            "LURP",
            "LURP1.PREb1", "LURP1.PREb5", "LURP5.PREb5", "LURP5.PREb1",
            "LURP1.PREa1", "LURP1.PREa5", "LURP5.PREa5", "LURP5.PREa1",
            "LURP1.POSTb1", "LURP1.POSTb5", "LURP5.POSTb5", "LURP5.POSTb1",
            "LURP1.POSTa1", "LURP1.POSTa5", "LURP5.POSTa5", "LURP5.POSTa1",

            // LDRP - Long Down Repeat Previous variants
            "LDRP",
            "LDRP1.PREb1", "LDRP1.PREb5", "LDRP5.PREb5", "LDRP5.PREb1",
            "LDRP1.PREa1", "LDRP1.PREa5", "LDRP5.PREa5", "LDRP5.PREa1",
            "LDRP1.POSTb1", "LDRP1.POSTb5", "LDRP5.POSTb5", "LDRP5.POSTb1",
            "LDRP1.POSTa1", "LDRP1.POSTa5", "LDRP5.POSTa5", "LDRP5.POSTa1",

            // LUSA - Long Up Step Above variants
            "LUSA1", "LUSA5",
            "LUSA1.PREb1", "LUSA1.PREb5", "LUSA5.PREb5", "LUSA5.PREb1",
            "LUSA1.PREa1", "LUSA1.PREa5", "LUSA5.PREa5", "LUSA5.PREa1",
            "LUSA1.POSTb1", "LUSA1.POSTb5", "LUSA5.POSTb5", "LUSA5.POSTb1",
            "LUSA1.POSTa1", "LUSA1.POSTa5", "LUSA5.POSTa5", "LUSA5.POSTa1",

            // LUSB - Long Up Step Below variants
            "LUSB1", "LUSB5",
            "LUSB1.PREb1", "LUSB1.PREb5", "LUSB5.PREb5", "LUSB5.PREb1",
            "LUSB1.PREa1", "LUSB1.PREa5", "LUSB5.PREa5", "LUSB5.PREa1",
            "LUSB1.POSTb1", "LUSB1.POSTb5", "LUSB5.POSTb5", "LUSB5.POSTb1",
            "LUSB1.POSTa1", "LUSB1.POSTa5", "LUSB5.POSTa5", "LUSB5.POSTa1",

            // LDSA - Long Down Step Above variants
            "LDSA1", "LDSA5",
            "LDSA1.PREb1", "LDSA1.PREb5", "LDSA5.PREb5", "LDSA5.PREb1",
            "LDSA1.PREa1", "LDSA1.PREa5", "LDSA5.PREa5", "LDSA5.PREa1",
            "LDSA1.POSTb1", "LDSA1.POSTb5", "LDSA5.POSTb5", "LDSA5.POSTb1",
            "LDSA1.POSTa1", "LDSA1.POSTa5", "LDSA5.POSTa5", "LDSA5.POSTa1",

            // LDSB - Long Down Step Below variants
            "LDSB1", "LDSB5",
            "LDSB1.PREb1", "LDSB1.PREb5", "LDSB5.PREb5", "LDSB5.PREb1",
            "LDSB1.PREa1", "LDSB1.PREa5", "LDSB5.PREa5", "LDSB5.PREa1",
            "LDSB1.POSTb1", "LDSB1.POSTb5", "LDSB5.POSTb5", "LDSB5.POSTb1",
            "LDSB1.POSTa1", "LDSB1.POSTa5", "LDSB5.POSTa5", "LDSB5.POSTa1"
        };

        // Randomly select a variant
        selectedVariant = variants[rand() % variants.size()];
    } else {
        // Use one of the user's selected variants randomly
        selectedVariant = userChoices[rand() % userChoices.size()];
    }

    // Use the comprehensive implementation with DUPLE meter by default
    return applyAppoggiatura(pi, durPi, DUPLE, selectedVariant);
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
    std::cout << "======================================================\n" << std::endl;

    return percentage;
}

// ENHANCED processFile function with percentage control and user variant choice
void processFile(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream input(inputFile);
    std::ofstream output(outputFile);

    if (!input.is_open() || !output.is_open()) {
        std::cerr << "Error opening files.\n";
        return;
    }

    // Get transformation percentage from user
    double transformationPercentage = getUserTransformationPercentage();

    // Get user's variant choices
    std::vector<std::string> userVariantChoices = getUserVariantChoices();

    // Write header to the output file
    output << std::left << std::setw(11) << "Track"
           << std::setw(11) << "Note"
           << std::setw(20) << "Duration"
           << std::setw(20) << "Label"
           << std::setw(15) << "Variant"
           << "\n";
    output << "-----------------------------------------------------------------------\n";

    // Statistics tracking
    int totalEligibleNotes = 0;
    int transformedNotes = 0;
    std::map<std::string, int> variantUsageCount;

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

        // Check if this label is eligible for transformation
        if (label == "FN" || label == "RLN" || label == "QN" || label == "I3" ||
            label == "I8" || label == "U2R" || label == "HT" || label == "SLP" ||
            label == "RDN" || label == "RN" || label == "LAD" || label == "DN" ||
            label == "DNW" || label == "SN" || label == "LP" || label == "NLN") {

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

                    // Track variant usage
                    variantUsageCount[selectedVariant]++;

                    // Output the transformed notes
                    for (const auto& [transformedNote, transformedDuration] : transformed) {
                        std::string transNote = getNoteName(transformedNote); // Convert MIDI to readable name
                        output << std::left
                               << std::setw(11) << track
                               << std::setw(11) << transNote
                               << std::setw(20) << transformedDuration
                               << std::setw(20) << label
                               << std::setw(15) << selectedVariant
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
                       << std::setw(15) << "ORIGINAL" // Mark as original
                       << "\n";
            }
        } else {
            // Output original data for non-eligible labels
            output << std::left
                   << std::setw(11) << track
                   << std::setw(11) << noteName
                   << std::setw(20) << duration
                   << std::setw(20) << label
                   << std::setw(15) << "" // Empty variant column
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

    if (userVariantChoices.size() == 1 && userVariantChoices[0] != "RANDOM") {
        std::cout << "Variant used: " << userVariantChoices[0] << std::endl;
    } else if (userVariantChoices.size() > 1) {
        std::cout << "Variants used (" << userVariantChoices.size() << " total):" << std::endl;
        for (const auto& [variant, count] : variantUsageCount) {
            std::cout << "  " << variant << ": " << count << " times" << std::endl;
        }
    } else {
        std::cout << "Variant selection: Random" << std::endl;
    }

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
        char programChange[3] = {0x00, static_cast<char>(0xC0), 0x00}; // Delta time, command, program number
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
    srand(static_cast<unsigned int>(time(0)));

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
    std::cout << "=== The Appoggiatura Transformation Tool ==============" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "====This transformation tool takes a note==============" << std::endl;
    std::cout << "====that is appropriately labeled and applies==========" << std::endl;
    std::cout << "==========an appoggiatura to that note.================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "=== Use with labels: FN, RLN, QN, I3, I8, U2R, HT======" << std::endl;
    std::cout << "=== SLP, RDN, RN, LAD, DN, DWN, SN, LP, NLN ===========" << std::endl;
    std::cout << "======== with % Transformation Selection ===================" << std::endl;
    std::cout << "======== Midi File Output and Text Output ==================" << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Enter the path to the input file (.txt format only): ";
    std::getline(std::cin, inputFile);

    // Remove quotes if the user included them
    if (inputFile.size() >= 2 &&
        ((inputFile.front() == '"' && inputFile.back() == '"') ||
         (inputFile.front() == '\'' && inputFile.back() == '\''))) {
        inputFile = inputFile.substr(1, inputFile.size() - 2);
    }

    // Generate unique output filenames with timestamp
    std::string outputFile = "Enhanced_Appoggiatura_Transformation_Multiple_User_Choice_" + std::string(timestamp) + ".txt";
    std::cout << std::endl;
    std::string midiOutputFile = "Enhanced_Appoggiatura_Transformation_Multiple_User_Choice_" + std::string(timestamp) + ".mid";
    std::cout << std::endl;

    std::cout << "Processing file: " << inputFile << std::endl;
    std::cout << std::endl;

    std::cout << "=============================================" << std::endl;
    std::cout << "Output will be written to: " << outputFile << std::endl;
    std::cout << "MIDI output will be written to: " << midiOutputFile << std::endl;
    std::cout << "=============================================" << std::endl;

    // Process the file (now with percentage control and enhanced user variant choice)
    processFile(inputFile, outputFile);

    // Convert the processed file to MIDI
    convertToMidi(outputFile, midiOutputFile);

    std::cout << "\n=== Processing Complete! ===" << std::endl;
    std::cout << "Enhanced transformation with multiple choice variant selection completed successfully." << std::endl;
    std::cout << "Press Enter to exit...";
    std::cin.get();

    return 0;
}
