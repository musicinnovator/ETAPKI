//  Slide Transformation (C) 2025 <Roderick Harris>
// Enhanced version with percentage control for transformations and user variant choice
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

// Helper functions for slide variants - ALL ORIGINAL FUNCTIONS PRESERVED
void handleMeter(std::vector<std::pair<int, int>>& EmbRet, int pre1, int pre2, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment = durPi / 4;
        EmbRet.push_back({pre1, segment});
        EmbRet.push_back({pre2, segment});
        EmbRet.push_back({pi, segment * 2});
    } else if (meter == TRIPLE) {
        int segment = durPi / 3;
        EmbRet.push_back({pre1, segment});
        EmbRet.push_back({pre2, segment});
        EmbRet.push_back({pi, segment});
    }
}

void handleMeter4(std::vector<std::pair<int, int>>& EmbRet, int pre1, int pre2, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment1 = (durPi * 3) / 8; // 3/8 of durPi
        int segment2 = durPi / 8;      // 1/8 of durPi
        EmbRet.push_back({pre1, segment1});
        EmbRet.push_back({pre2, segment2});
        EmbRet.push_back({pi, durPi / 2});
    } else if (meter == TRIPLE) {
        int segment1 = durPi / 3; // 1/3 of durPi
        int segment2 = durPi / 6; // 1/6 of durPi
        EmbRet.push_back({pre1, segment1});
        EmbRet.push_back({pre2, segment2});
        EmbRet.push_back({pi, durPi / 2});
    }
}

void handleMeter5(std::vector<std::pair<int, int>>& EmbRet, int pre1, int pre2, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment1 = durPi / 4; // 1/4 of durPi
        int segment2 = (durPi * 3) / 8; // 3/8 of durPi
        EmbRet.push_back({pre2, segment1});
        EmbRet.push_back({pre1, segment2});
        EmbRet.push_back({pi, durPi / 2});
    } else if (meter == TRIPLE) {
        int segment1 = durPi / 6; // 1/6 of durPi
        int segment2 = durPi / 3; // 1/3 of durPi
        EmbRet.push_back({pre1, segment1});
        EmbRet.push_back({pre2, segment2});
        EmbRet.push_back({pi, durPi / 2});
    }
}

void handleMeter3(std::vector<std::pair<int, int>>& EmbRet, int pre1, int pre2, int pre3, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment = durPi / 6; // 1/6 of durPi
        EmbRet.push_back({pre1, segment});
        EmbRet.push_back({pre2, segment});
        EmbRet.push_back({pre3, segment});
        EmbRet.push_back({pi, durPi - 3 * segment});  // Remaining duration
    } else if (meter == TRIPLE) {
        int segment = durPi / 8; // 1/8 of durPi
        EmbRet.push_back({pre1, segment});
        EmbRet.push_back({pre2, segment});
        EmbRet.push_back({pre3, segment});
        EmbRet.push_back({pi, durPi - 3 * segment});  // Remaining duration
    }
}

void handleMeter6(std::vector<std::pair<int, int>>& EmbRet, int pre1, int pre2, int pre3, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment1 = durPi / 4; // 1/4 of durPi
        int segment2 = durPi / 8; // 1/8 of durPi
        int segment3 = durPi / 8; // 1/8 of durPi
        EmbRet.push_back({pre1, segment1});
        EmbRet.push_back({pre2, segment2});
        EmbRet.push_back({pre3, segment3});
        EmbRet.push_back({pi, durPi - (segment1 + segment2 + segment3)}); // Remaining duration
    } else if (meter == TRIPLE) {
        int segment = durPi / 8; // 1/8 of durPi
        EmbRet.push_back({pre1, segment * 2});
        EmbRet.push_back({pre2, segment});
        EmbRet.push_back({pre3, segment});
        EmbRet.push_back({pi, durPi - 4 * segment});  // Remaining duration
    }
}

void handleMeter7(std::vector<std::pair<int, int>>& EmbRet, int pre1, int pre2, int pre3, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment1 = durPi / 8; // 1/8 of durPi
        int segment2 = durPi / 4; // 1/4 of durPi
        int segment3 = durPi / 8; // 1/8 of durPi
        EmbRet.push_back({pre1, segment1});
        EmbRet.push_back({pre2, segment2});
        EmbRet.push_back({pre3, segment3});
        EmbRet.push_back({pi, durPi - (segment1 + segment2 + segment3)});  // Remaining duration
    } else if (meter == TRIPLE) {
        int segment = durPi / 8; // 1/8 of durPi
        EmbRet.push_back({pre1, segment});
        EmbRet.push_back({pre2, segment * 2}); // 2/8
        EmbRet.push_back({pre3, segment});
        EmbRet.push_back({pi, durPi - (segment * 4)});  // Remaining duration
    }
}

void handleMeter8(std::vector<std::pair<int, int>>& EmbRet, int pre1, int pre2, int pre3, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment1 = durPi / 8; // 1/8 of durPi
        int segment2 = durPi / 8; // 1/8 of durPi
        int segment3 = durPi / 4; // 1/4 of durPi
        EmbRet.push_back({pre1, segment1});
        EmbRet.push_back({pre2, segment2});
        EmbRet.push_back({pre3, segment3});
        EmbRet.push_back({pi, durPi - (segment1 + segment2 + segment3)});  // Remaining duration
    } else if (meter == TRIPLE) {
        int segment = durPi / 12; // 1/12 of durPi
        EmbRet.push_back({pre1, segment * 2});
        EmbRet.push_back({pre2, segment * 2});
        EmbRet.push_back({pre3, segment * 4});
        EmbRet.push_back({pi, durPi - (segment * 8)});  // Remaining duration
    }
}

// Main function to apply slide variants - ORIGINAL FUNCTION NAME AND ALL VARIANTS PRESERVED
std::vector<std::pair<int, int>> applySlideVariants(int pi, int durPi, TimeMeter meter, const std::string& variant) {
    if (durPi <= 0) {
        throw std::invalid_argument("Duration (durPi) must be greater than 0");
    }
    if (meter != DUPLE && meter != TRIPLE) {
        throw std::invalid_argument("Invalid TimeMeter");
    }

    std::vector<std::pair<int, int>> EmbRet;

    // Basic STT slide variants (2-note patterns)
    if (variant == "STTM2m") {  // Slide note start m3 below principal note, then up a M2, and then resolve
        int pre1 = pi - 3;  // m3 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "STTm2M") {  // Slide note start m3 below principal note, then up a m2, and then resolve
        int pre1 = pi - 3;  // m3 below principal
        int pre2 = pre1 + 1;  // m2 up from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "STTm3m") {  // Slide note start M3 below principal note, then up a m3, and then resolve
        int pre1 = pi - 4;  // M3 below principal
        int pre2 = pre1 + 3;  // m3 up from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "STTM2M") {  // Slide note start M3 below principal note, then up a M2, and then resolve
        int pre1 = pi - 4;  // M3 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);

    // Dotted DSTT slide variants (2-note patterns)
    } else if (variant == "DSTTM2m") {  // Slide note start dotted m3 below principal note, then up a M2, and then resolve
        int pre1 = pi - 3;  // m3 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        handleMeter4(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "DSTTm2M") {  // Slide note start dotted m3 below principal note, then up a m2, and then resolve
        int pre1 = pi - 3;  // m3 below principal
        int pre2 = pre1 + 1;  // m2 up from pre1
        handleMeter4(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "DSTTm3m") {  // Slide note start dotted M3 below principal note, then up a m3, and then resolve
        int pre1 = pi - 4;  // M3 below principal
        int pre2 = pre1 + 3;  // m3 up from pre1
        handleMeter4(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "DSTTM2M") {  // Slide note start dotted M3 below principal note, then up a M2, and then resolve
        int pre1 = pi - 4;  // M3 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        handleMeter4(EmbRet, pre1, pre2, pi, durPi, meter);

    // Inverted ISTT slide variants (2-note patterns)
    } else if (variant == "ISTTM2m") {  // Slide note starts M3 above principal note, then down a M2, and then resolve
        int pre1 = pi + 4;  // M3 above principal
        int pre2 = pre1 - 2;  // M2 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "ISTTm2M") {  // Slide note starts m3 above principal note, then down a m2, and then resolve
        int pre1 = pi + 3;  // m3 above principal
        int pre2 = pre1 - 1;  // m2 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "ISTTM3m") {  // Slide note starts M3 above principal notes, then down a m3, and then resolve
        int pre1 = pi + 4;  // M3 above principal
        int pre2 = pre1 - 3;  // m3 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "ISTTM2M") {  // Slide note starts m3 above principal note, then down a M2, and then resolve
        int pre1 = pi + 3;  // m3 above principal
        int pre2 = pre1 - 2;  // M2 down from pre1
        handleMeter(EmbRet, pre1, pre2, pi, durPi, meter);

    // Dotted Inverted DISTT slide variants (2-note patterns)
    } else if (variant == "DISTTM2m") {  // Slide note starts dotted M3 above principal note, then down a M2, and then resolve
        int pre1 = pi + 4;  // M3 above principal
        int pre2 = pre1 - 2;  // M2 down from pre1
        handleMeter4(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "DISTTm2M") {  // Slide note starts dotted m3 above principal note, then down a m2, and then resolve
        int pre1 = pi + 3;  // m3 above principal
        int pre2 = pre1 - 1;  // m2 down from pre1
        handleMeter4(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "DISTTm3m") {  // Slide note starts dotted M3 above principal notes, then down a m3, and then resolve
        int pre1 = pi + 4;  // M3 above principal
        int pre2 = pre1 - 3;  // m3 down from pre1
        handleMeter4(EmbRet, pre1, pre2, pi, durPi, meter);
    } else if (variant == "DISTTM2M") {  // Slide note starts dotted m3 above principal note, then down a M2, and then resolve
        int pre1 = pi + 3;  // m3 above principal
        int pre2 = pre1 - 2;  // M2 down from pre1
        handleMeter4(EmbRet, pre1, pre2, pi, durPi, meter);

    // Three-tone TTS slide variants (3-note patterns)
    } else if (variant == "TTSM2m2M") {  // Three tone Slide note starts P4 below principal note, then up a M2, then up a m2, and then resolve
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 1;  // m2 up from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSm3M2M") {  // Three tone Slide note starts A4 below principal note, then up a m3, then up a m2, and then resolve
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 3;  // m3 up from pre1
        int pre3 = pre2 + 1;  // m2 up from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSm2M2M") {  // Three tone Slide note starts P4 below principal note, then up a m2, then up a M2, and then resolve
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 1;  // m2 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSM2M2m") {  // Three tone Slide note starts P4 below principal note, then up M2, then up a M2, and then resolve
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSM2M2M") {  // Three tone Slide note starts A4 below principal note, then up M2, then up a M2, and then resolve
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSm2m3m") {  // Three tone Slide note starts P4 below principal note, then up a m2, then up a m3, and then resolve
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 1;  // m2 up from pre1
        int pre3 = pre2 + 3;  // m3 up from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSm3M2m") {  // Three tone Slide note starts A4 below principal note, then up a m3, then up a M2, and then resolve
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 3;  // m3 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSM2m3m") {  // Three tone Slide note starts A4 below principal note, then up M2, then up a m3, and then resolve
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 3;  // m3 up from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);

    // First position dotted TTSd1 variants (3-note patterns)
    } else if (variant == "TTSd1M2m2M") {  // Three tone Slide note starts dotted P4 below principal note, then up a M2, then up a m2, and then resolve
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 1;  // m2 up from pre2
        handleMeter6(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd1m3M2M") {  // Three tone Slide note starts dotted A4 below principal note, then up a m3, then up a m2, and then resolve
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 3;  // m3 up from pre1
        int pre3 = pre2 + 1;  // m2 up from pre2
        handleMeter6(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd1m2M2M") {  // Three tone Slide note starts dotted P4 below principal note, then up a m2, then up a M2, and then resolve
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 1;  // m2 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter6(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd1M2M2m") {  // Three tone Slide note starts dotted P4 below principal note, then up M2, then up a M2, and then resolve
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter6(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd1M2M2M") {  // Three tone Slide note starts dotted A4 below principal note, then up M2, then up a M2, and then resolve
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter6(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd1m2m3m") {  // Three tone Slide note starts dotted P4 below principal note, then up a m2, then up a m3, and then resolve
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 1;  // m2 up from pre1
        int pre3 = pre2 + 3;  // m3 up from pre2
        handleMeter6(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd1m3M2m") {  // Three tone Slide note starts dotted A4 below principal note, then up a m3, then up a M2, and then resolve
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 3;  // m3 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter6(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd1M2m3m") {  // Three tone Slide note starts dotted A4 below principal note, then up M2, then up a m3, and then resolve
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 3;  // m3 up from pre2
        handleMeter6(EmbRet, pre1, pre2, pre3, pi, durPi, meter);

    // Second position dotted TTSd2 variants (3-note patterns)
    } else if (variant == "TTSd2M2m2M") {  // Three tone Slide note starts P4 below principal note, then up a dotted M2, then up a m2, and then resolve
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 1;  // m2 up from pre2
        handleMeter7(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd2m3M2M") {  // Three tone Slide note starts A4 below principal note, then up a dotted m3, then up a m2, and then resolve
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 3;  // m3 up from pre1
        int pre3 = pre2 + 1;  // m2 up from pre2
        handleMeter7(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd2m2M2M") {  // Three tone Slide note starts P4 below principal note, then up a dotted m2, then up a M2, and then resolve
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 1;  // m2 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter7(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd2M2M2m") {  // Three tone Slide note starts P4 below principal note, then up a dotted M2, then up a M2, and then resolve
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter7(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd2M2M2M") {  // Three tone Slide note starts A4 below principal note, then up a dotted M2, then up a M2, and then resolve
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter7(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd2m2m3m") {  // Three tone Slide note starts P4 below principal note, then up a dotted m2, then up a m3, and then resolve
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 1;  // m2 up from pre1
        int pre3 = pre2 + 3;  // m3 up from pre2
        handleMeter7(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd2m3M2m") {  // Three tone Slide note starts A4 below principal note, then up a dotted m3, then up a M2, and then resolve
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 3;  // m3 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter7(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd2M2m3m") {  // Three tone Slide note starts A4 below principal note, then up a dotted M2, then up a m3, and then resolve
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 3;  // m3 up from pre2
        handleMeter7(EmbRet, pre1, pre2, pre3, pi, durPi, meter);

    // Third position dotted TTSd3 variants (3-note patterns)
    } else if (variant == "TTSd3M2m2M") {  // Three tone Slide note starts P4 below principal note, then up a M2, then up a dotted m2, and then resolve
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 1;  // m2 up from pre2
        handleMeter8(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd3m3M2M") {  // Three tone Slide note starts A4 below principal note, then up a m3, then up a dotted m2, and then resolve
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 3;  // m3 up from pre1
        int pre3 = pre2 + 1;  // m2 up from pre2
        handleMeter8(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd3m2M2M") {  // Three tone Slide note starts P4 below principal note, then up a m2, then up a dotted M2, and then resolve
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 1;  // m2 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter8(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd3M2M2m") {  // Three tone Slide note starts P4 below principal note, then up a M2, then up a dotted M2, and then resolve
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter8(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd3M2M2M") {  // Three tone Slide note starts A4 below principal note, then up a M2, then up a dotted M2, and then resolve
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter8(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd3m2m3m") {  // Three tone Slide note starts P4 below principal note, then up a m2, then up a dotted m3, and then resolve
        int pre1 = pi - 5;  // P4 below principal
        int pre2 = pre1 + 1;  // m2 up from pre1
        int pre3 = pre2 + 3;  // m3 up from pre2
        handleMeter8(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd3m3M2m") {  // Three tone Slide note starts A4 below principal note, then up a m3, then up a dotted M2, and then resolve
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 3;  // m3 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter8(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTSd3M2m3m") {  // Three tone Slide note starts A4 below principal note, then up a M2, then up a dotted m3, and then resolve
        int pre1 = pi - 6;  // A4 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 3;  // m3 up from pre2
        handleMeter8(EmbRet, pre1, pre2, pre3, pi, durPi, meter);

    // Inverted TTIT variants (3-note patterns)
    } else if (variant == "TTITM2M2M") {  // Three tone Slide Inverted note starts M2 below principal note, then up a M2, then up a M2, and then resolve
        int pre1 = pi - 2;  // M2 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTITM2M2m") {  // Three tone Slide Inverted note starts m3 below principal note, then up a M2, then up a M2, and then resolve
        int pre1 = pi - 3;  // m3 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTITM2m3M") {  // Three tone Slide Inverted note start m3 below principal note, then up a M2, then up a m3, and then resolve
        int pre1 = pi - 3;  // m3 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 3;  // m3 up from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTITm2M2m") {  // Three tone Slide Inverted note starts M2 below principal note, then up a m2, then up a M2, and then resolve
        int pre1 = pi - 2;  // M2 below principal
        int pre2 = pre1 + 1;  // m2 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTITm3M2M") {  // Three tone Slide Inverted note starts m3 below principal note, then up a m3, then up a M2, and then resolve
        int pre1 = pi - 3;  // m3 below principal
        int pre2 = pre1 + 3;  // m3 up from pre1
        int pre3 = pre2 + 2;  // M2 up from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTITm3m2m") {  // Three tone Slide Inverted note starts m3 below principal note, then up a m3, then up a m2, and then resolve
        int pre1 = pi - 3;  // m3 below principal
        int pre2 = pre1 + 3;  // m3 up from pre1
        int pre3 = pre2 + 1;  // m2 up from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "TTITM2m2m") {  // Three tone Slide Inverted note starts M2 below principal note, then up a M2, then up a m2, and then resolve
        int pre1 = pi - 2;  // M2 below principal
        int pre2 = pre1 + 2;  // M2 up from pre1
        int pre3 = pre2 + 1;  // m2 up from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);

    // Inverted above ITTIT variants (3-note patterns)
    } else if (variant == "ITTITM2M2M") {  // Inverted Three tone Slide Inverted note starts M2 above principal note, then down a M2, then down a M2, and then resolve
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 2;  // M2 down from pre1
        int pre3 = pre2 - 2;  // M2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "ITTITm2M3m") {  // Inverted Three tone Slide Inverted note starts M2 above principal note, then down a m2, then down a M3, and then resolve
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 1;  // m2 down from pre1
        int pre3 = pre2 - 4;  // M3 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "ITTITm3m2M") {  // Inverted Three tone Slide Inverted note starts M2 above principal note, then down a m3, then down a m2, and then resolve
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 3;  // m3 down from pre1
        int pre3 = pre2 - 1;  // m2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "ITTITm3m2m") {  // Inverted Three tone Slide Inverted note starts m3 above principal note, then down a m3, then down a m2, and then resolve
        int pre1 = pi + 3;  // m3 above principal
        int pre2 = pre1 - 3;  // m3 down from pre1
        int pre3 = pre2 - 1;  // m2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "ITTITM2M3m") {  // Inverted Three tone Slide Inverted note starts m3 above principal note, then down a M2, then down a M3, and then resolve
        int pre1 = pi + 3;  // m3 above principal
        int pre2 = pre1 - 2;  // M2 down from pre1
        int pre3 = pre2 - 4;  // M3 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "ITTITM2m2M") {  // Inverted Three tone Slide Inverted note starts m2 above principal note, then down a M2, then down a m2, and then resolve
        int pre1 = pi + 1;  // m2 above principal
        int pre2 = pre1 - 2;  // M2 down from pre1
        int pre3 = pre2 - 1;  // m2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "ITTITM2m2m") {  // Inverted Three tone Slide Inverted note starts M2 above principal note, then down a M2, then down a m2, and then resolve
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 2;  // M2 down from pre1
        int pre3 = pre2 - 1;  // m2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "ITTITM2m3m") {  // Inverted Three tone Slide Inverted note starts M2 above principal note, then down a M2, then down a m3, and then resolve
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 2;  // M2 down from pre1
        int pre3 = pre2 - 3;  // m3 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "ITTITm2M2M") {  // Inverted Three tone Slide Inverted note starts m2 above principal note, then down a m2, then down a M2, and then resolve
        int pre1 = pi + 1;  // m2 above principal
        int pre2 = pre1 - 1;  // m2 down from pre1
        int pre3 = pre2 - 2;  // M2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "ITTITm2m3M") {  // Inverted Three tone Slide Inverted note starts M2 above principal note, then down a m2, then down a m3, and then resolve
        int pre1 = pi + 2;  // M2 above principal
        int pre2 = pre1 - 1;  // m2 down from pre1
        int pre3 = pre2 - 3;  // m3 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "ITTITm3M2M") {  // Inverted Three tone Slide Inverted note starts m3 above principal note, then down a m3, then down a M2, and then resolve
        int pre1 = pi + 3;  // m3 above principal
        int pre2 = pre1 - 3;  // m3 down from pre1
        int pre3 = pre2 - 2;  // M2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);

    // Inverted three-tone above ITTS variants (3-note patterns)
    } else if (variant == "ITTSM2M2m") {  // Inverted Three tone Above Slide note starts P4 above principal note, then down a M2, then down a M2, and then resolve
        int pre1 = pi + 5;  // P4 above principal
        int pre2 = pre1 - 2;  // M2 down from pre1
        int pre3 = pre2 - 2;  // M2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "ITTSm2M2M") {  // Inverted Three tone Above Slide note starts P4 above principal note, then down a m2, then down a M2, and then resolve
        int pre1 = pi + 5;  // P4 above principal
        int pre2 = pre1 - 1;  // m2 down from pre1
        int pre3 = pre2 - 2;  // M2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "ITTSm2m3m") {  // Inverted Three tone Above Slide note starts P4 above principal note, then down a m2, then down a m3, and then resolve
        int pre1 = pi + 5;  // P4 above principal
        int pre2 = pre1 - 1;  // m2 down from pre1
        int pre3 = pre2 - 3;  // m3 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "ITTSm3M2m") {  // Inverted Three tone Above Slide note starts A4 above principal note, then down a m3, then down a M2, and then resolve
        int pre1 = pi + 6;  // A4 above principal
        int pre2 = pre1 - 3;  // m3 down from pre1
        int pre3 = pre2 - 2;  // M2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "ITTSm3m2M") {  // Inverted Three tone Above Slide note starts A4 above principal note, then down a m3, then down a m2, and then resolve
        int pre1 = pi + 6;  // A4 above principal
        int pre2 = pre1 - 3;  // m3 down from pre1
        int pre3 = pre2 - 1;  // m2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "ITTSM2M2M") {  // Inverted Three tone Above Slide note starts A4 above principal note, then down a M2, then down a M2, and then resolve
        int pre1 = pi + 6;  // A4 above principal
        int pre2 = pre1 - 2;  // M2 down from pre1
        int pre3 = pre2 - 2;  // M2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "ITTSM2m2m") {  // Inverted Three tone Above Slide note starts M3 above principal note, then down a M2, then down a m2, and then resolve
        int pre1 = pi + 4;  // M3 above principal
        int pre2 = pre1 - 2;  // M2 down from pre1
        int pre3 = pre2 - 1;  // m2 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);
    } else if (variant == "ITTSM2m3m") {  // Inverted Three tone Above Slide note starts A4 above principal note, then down a M2, then down a m3, and then resolve
        int pre1 = pi + 6;  // A4 above principal
        int pre2 = pre1 - 2;  // M2 down from pre1
        int pre3 = pre2 - 3;  // m3 down from pre2
        handleMeter3(EmbRet, pre1, pre2, pre3, pi, durPi, meter);

    } else {
        // Handle unknown variant
        std::cerr << "Unknown slide variant: " << variant << '\n';
    }

    return EmbRet;
}

// NEW FUNCTION: Generate random pool of variants for user selection
std::vector<std::string> generateRandomVariantPool(int poolSize) {
    // Define the complete pool of slide variants - all options from the implementation
    const std::vector<std::string> allVariants = {
        // Basic STT variants (2-note patterns)
        "STTM2m", "STTm2M", "STTm3m", "STTM2M",

        // Dotted DSTT variants (2-note patterns)
        "DSTTM2m", "DSTTm2M", "DSTTm3m", "DSTTM2M",

        // Inverted ISTT variants (2-note patterns)
        "ISTTM2m", "ISTTm2M", "ISTTM3m", "ISTTM2M",

        // Dotted Inverted DISTT variants (2-note patterns)
        "DISTTM2m", "DISTTm2M", "DISTTm3m", "DISTTM2M",

        // Three-tone TTS variants (3-note patterns)
        "TTSM2m2M", "TTSm3M2M", "TTSm2M2M", "TTSM2M2m", "TTSM2M2M",
        "TTSm2m3m", "TTSm3M2m", "TTSM2m3m",

        // First position dotted TTSd1 variants (3-note patterns)
        "TTSd1M2m2M", "TTSd1m3M2M", "TTSd1m2M2M", "TTSd1M2M2m", "TTSd1M2M2M",
        "TTSd1m2m3m", "TTSd1m3M2m", "TTSd1M2m3m",

        // Second position dotted TTSd2 variants (3-note patterns)
        "TTSd2M2m2M", "TTSd2m3M2M", "TTSd2m2M2M", "TTSd2M2M2m", "TTSd2M2M2M",
        "TTSd2m2m3m", "TTSd2m3M2m", "TTSd2M2m3m",

        // Third position dotted TTSd3 variants (3-note patterns)
        "TTSd3M2m2M", "TTSd3m3M2M", "TTSd3m2M2M", "TTSd3M2M2m", "TTSd3M2M2M",
        "TTSd3m2m3m", "TTSd3m3M2m", "TTSd3M2m3m",

        // Inverted TTIT variants (3-note patterns)
        "TTITM2M2M", "TTITM2M2m", "TTITM2m3M", "TTITm2M2m", "TTITm3M2M",
        "TTITm3m2m", "TTITM2m2m",

        // Inverted above ITTIT variants (3-note patterns)
        "ITTITM2M2M", "ITTITm2M3m", "ITTITm3m2M", "ITTITm3m2m", "ITTITM2M3m",
        "ITTITM2m2M", "ITTITM2m2m", "ITTITM2m3m", "ITTITm2M2M", "ITTITm2m3M", "ITTITm3M2M",

        // Inverted three-tone above ITTS variants (3-note patterns)
        "ITTSM2M2m", "ITTSm2M2M", "ITTSm2m3m", "ITTSm3M2m", "ITTSm3m2M",
        "ITTSM2M2M", "ITTSM2m2m", "ITTSM2m3m"
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

// ENHANCED getUserVariantChoice function with multiple choice support
std::vector<std::string> getUserVariantChoice() {
    std::cout << "\n=== Slide Variant Selection ===" << std::endl;
    std::cout << "Choose your variant selection method:" << std::endl;
    std::cout << "1. Random selection (original behavior)" << std::endl;
    std::cout << "2. Choose from a random pool of variants" << std::endl;
    std::cout << "3. Select multiple variants from displayed choices" << std::endl;
    std::cout << "Enter your choice (1, 2, or 3): ";

    int choice;
    while (!(std::cin >> choice) || (choice < 1 || choice > 3)) {
        std::cout << "Invalid input. Please enter 1, 2, or 3: ";
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }

    if (choice == 1) {
        std::cout << "Using random variant selection." << std::endl;
        std::cout << "======================================\n" << std::endl;
        return {"RANDOM"}; // Special flag for random selection
    }

    // Generate random pool of variants
    std::vector<std::string> variantPool = generateRandomVariantPool(10);

    std::cout << "\nHere are 10 randomly selected variants to choose from:" << std::endl;
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
            // Basic STT variants (2-note patterns)
            "STTM2m", "STTm2M", "STTm3m", "STTM2M",

            // Dotted DSTT variants (2-note patterns)
            "DSTTM2m", "DSTTm2M", "DSTTm3m", "DSTTM2M",

            // Inverted ISTT variants (2-note patterns)
            "ISTTM2m", "ISTTm2M", "ISTTM3m", "ISTTM2M",

            // Dotted Inverted DISTT variants (2-note patterns)
            "DISTTM2m", "DISTTm2M", "DISTTm3m", "DISTTM2M",

            // Three-tone TTS variants (3-note patterns)
            "TTSM2m2M", "TTSm3M2M", "TTSm2M2M", "TTSM2M2m", "TTSM2M2M",
            "TTSm2m3m", "TTSm3M2m", "TTSM2m3m",

            // First position dotted TTSd1 variants (3-note patterns)
            "TTSd1M2m2M", "TTSd1m3M2M", "TTSd1m2M2M", "TTSd1M2M2m", "TTSd1M2M2M",
            "TTSd1m2m3m", "TTSd1m3M2m", "TTSd1M2m3m",

            // Second position dotted TTSd2 variants (3-note patterns)
            "TTSd2M2m2M", "TTSd2m3M2M", "TTSd2m2M2M", "TTSd2M2M2m", "TTSd2M2M2M",
            "TTSd2m2m3m", "TTSd2m3M2m", "TTSd2M2m3m",

            // Third position dotted TTSd3 variants (3-note patterns)
            "TTSd3M2m2M", "TTSd3m3M2M", "TTSd3m2M2M", "TTSd3M2M2m", "TTSd3M2M2M",
            "TTSd3m2m3m", "TTSd3m3M2m", "TTSd3M2m3m",

            // Inverted TTIT variants (3-note patterns)
            "TTITM2M2M", "TTITM2M2m", "TTITM2m3M", "TTITm2M2m", "TTITm3M2M",
            "TTITm3m2m", "TTITM2m2m",

            // Inverted above ITTIT variants (3-note patterns)
            "ITTITM2M2M", "ITTITm2M3m", "ITTITm3m2M", "ITTITm3m2m", "ITTITM2M3m",
            "ITTITM2m2M", "ITTITM2m2m", "ITTITM2m3m", "ITTITm2M2M", "ITTITm2m3M", "ITTITm3M2M",

            // Inverted three-tone above ITTS variants (3-note patterns)
            "ITTSM2M2m", "ITTSm2M2M", "ITTSm2m3m", "ITTSm3M2m", "ITTSm3m2M",
            "ITTSM2M2M", "ITTSM2m2m", "ITTSM2m3m"
        };

        // Randomly select a variant
        selectedVariant = variants[rand() % variants.size()];
    } else {
        // Randomly select from user's chosen variants
        selectedVariant = userChoices[rand() % userChoices.size()];
    }

    // Use the comprehensive implementation with DUPLE meter by default
    return applySlideVariants(pi, durPi, DUPLE, selectedVariant);
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
    std::cout << "\n=== Slide Transformation Settings ===" << std::endl;
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
        if (label == "SAN" || label == "RLN" || label == "SMP" || label == "Mmd7" ||
            label == "I8" || label == "U2R" || label == "HT" || label == "MmAug6" ||
            label == "RDN" || label == "RN" || label == "MmAug4" || label == "Mmm3" || label == "LAD") {

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
    std::cout << "========= The Slide Transformation Tool ===============" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "====This transformation tool takes a note==============" << std::endl;
    std::cout << "====that is appropriately labeled and applies==========" << std::endl;
    std::cout << "==========a slide to that note.========" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "=== Use with labels: SAN, RLN, U2R, BM, EM, ============" << std::endl;
    std::cout << "=== MmAug6, MmAug4, Mmm3, Mmd7, I8, HT, RDN, ===========" << std::endl;
    std::cout << "=== SMP, RN, LAD, DNW, LNSN, SMP, DBC, DDN, ============" << std::endl;
    std::cout << "=== LNR, LNSAS, LNSAL, DI, SPCM, SPDM, SSN, ============" << std::endl;
    std::cout << "======  SVN, ANS, ANL, FTB, and CDB ====================" << std::endl;
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
    std::string outputFile = "Slide_transformation_Multiple_User_Choice_" + std::string(timestamp) + ".txt";
    std::cout << std::endl;
    std::string midiOutputFile = "Slide_transformation_Multiple_User_Choice_" + std::string(timestamp) + ".mid";

    std::cout << "Processing file: " << inputFile << std::endl;

    std::cout << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "Output will be written to: " << outputFile << std::endl;
    std::cout << std::endl;
    std::cout << "MIDI output will be written to: " << midiOutputFile << std::endl;
    std::cout << "=======================================================" << std::endl;

    // FIXED: Pass user variant choices to processFile
    processFile(inputFile, outputFile, userVariantChoices);

    // Convert the processed file to MIDI
    convertToMidi(outputFile, midiOutputFile);

    std::cout << "Processing complete!" << std::endl;
    std::cout << "Press Enter to exit...";
    std::cin.get();

    return 0;
}