//  Turns Transformation (C) 2025 <Roderick Harris>
// Enhanced version with percentage control for transformations and multiple choice variant selection
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

// Helper functions for Turn variants - ALL ORIGINAL FUNCTIONS PRESERVED
void handleTurnMeter(std::vector<std::pair<int, int>>& EmbRet, int upper, int principal, int lower, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment = durPi / 4;
        EmbRet.push_back({upper, segment});
        EmbRet.push_back({principal, segment});
        EmbRet.push_back({lower, segment});
        EmbRet.push_back({principal, durPi - 3 * segment}); // Remaining duration
    } else if (meter == TRIPLE) {
        int segment = durPi / 4;
        EmbRet.push_back({upper, segment});
        EmbRet.push_back({principal, segment});
        EmbRet.push_back({lower, segment});
        EmbRet.push_back({principal, durPi - 3 * segment}); // Remaining duration
    }
}

void handleTurnMeterFB(std::vector<std::pair<int, int>>& EmbRet, int upper, int pi, int lower, int underlower, int lower2, int pi2, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment = durPi / 8;
        EmbRet.push_back({upper, segment});
        EmbRet.push_back({pi, segment});
        EmbRet.push_back({lower, segment});
        EmbRet.push_back({underlower, segment});
        EmbRet.push_back({lower, segment});
        EmbRet.push_back({pi, durPi - 5 * segment}); // Remaining duration
    } else if (meter == TRIPLE) {
        int segment = durPi / 5;
        EmbRet.push_back({upper, segment});
        EmbRet.push_back({pi, segment});
        EmbRet.push_back({lower, segment});
        EmbRet.push_back({pi, segment});
        EmbRet.push_back({upper, durPi - 4 * segment}); // Remaining duration
    }
}

void handleTurnMeterBetween(std::vector<std::pair<int, int>>& EmbRet, int pi, int upper, int pi2, int lower, int pi3, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment = durPi / 8;
        EmbRet.push_back({pi, durPi / 2});
        EmbRet.push_back({upper, segment});
        EmbRet.push_back({pi2, segment});
        EmbRet.push_back({lower, segment});
        EmbRet.push_back({pi3, durPi - (durPi / 2 + 3 * segment)}); // Remaining duration
    } else if (meter == TRIPLE) {
        int segment = durPi / 8;
        EmbRet.push_back({pi, segment});
        EmbRet.push_back({upper, durPi / 4});
        EmbRet.push_back({pi2, durPi / 4});
        EmbRet.push_back({lower, durPi / 4});
        EmbRet.push_back({pi3, durPi - (segment + durPi / 4 + durPi / 4 + durPi / 4)}); // Remaining duration
    }
}

void handleTurnMeterTrille(std::vector<std::pair<int, int>>& EmbRet, int upper, int principal, int lower, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment = durPi / 8;
        EmbRet.push_back({upper, segment});
        EmbRet.push_back({principal, segment});
        EmbRet.push_back({upper, segment});
        EmbRet.push_back({principal, segment});
        EmbRet.push_back({lower, segment});
        EmbRet.push_back({principal, durPi - 5 * segment}); // Remaining duration
    } else if (meter == TRIPLE) {
        int segment = durPi / 8;
        EmbRet.push_back({upper, segment});
        EmbRet.push_back({principal, segment});
        EmbRet.push_back({upper, segment});
        EmbRet.push_back({principal, segment});
        EmbRet.push_back({lower, segment});
        EmbRet.push_back({principal, durPi - 5 * segment}); // Remaining duration
    }
}

void handleTurnMeterP32(std::vector<std::pair<int, int>>& EmbRet, int upper, int principal, int lower, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment1 = (durPi * 3) / 8;
        int segment2 = durPi / 8;
        EmbRet.push_back({upper, segment1});
        EmbRet.push_back({principal, segment2});
        EmbRet.push_back({lower, segment2});
        EmbRet.push_back({principal, durPi - (segment1 + 2 * segment2)}); // Remaining duration
    } else if (meter == TRIPLE) {
        int segment1 = (durPi * 3) / 8;
        int segment2 = durPi / 8;
        EmbRet.push_back({upper, segment1});
        EmbRet.push_back({principal, segment2});
        EmbRet.push_back({lower, segment2});
        EmbRet.push_back({principal, durPi - (segment1 + 2 * segment2)}); // Remaining duration
    }
}

void handleTurnMeterSnapped(std::vector<std::pair<int, int>>& EmbRet, int upper, int principal, int lower, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment = durPi / 6;
        EmbRet.push_back({upper, segment});
        EmbRet.push_back({principal, segment});
        EmbRet.push_back({lower, segment});
        EmbRet.push_back({principal, durPi - 3 * segment}); // Remaining duration
    } else if (meter == TRIPLE) {
        int segment = durPi / 6;
        EmbRet.push_back({upper, segment});
        EmbRet.push_back({principal, segment});
        EmbRet.push_back({lower, segment});
        EmbRet.push_back({principal, durPi - 3 * segment}); // Remaining duration
    }
}

// Main function to apply turn variants - ORIGINAL FUNCTION NAME AND ALL VARIANTS PRESERVED
std::vector<std::pair<int, int>> applyTurnVariants(int pi, int durPi, TimeMeter meter, const std::string& variant) {
    if (durPi <= 0) {
        throw std::invalid_argument("Duration (durPi) must be greater than 0");
    }
    if (meter != DUPLE && meter != TRIPLE) {
        throw std::invalid_argument("Invalid TimeMeter");
    }

    std::vector<std::pair<int, int>> EmbRet;

    // Basic Turn variants
    if (variant == "Turn") {  // whole step above, the principal note, and the lower auxiliary (1/2 step below) are played rapidly, resolve and hold for duration
        int upper = pi + 2;  // whole step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeter(EmbRet, upper, pi, lower, pi, durPi, meter);
    } else if (variant == "FTurnF") {  // 1/2 step above, the principal note, and the lower auxiliary (1 whole step below) are played, resolve and hold for duration
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeter(EmbRet, upper, pi, lower, pi, durPi, meter);
    } else if (variant == "FTurnS") {  // 1 whole step above, the principal note, and the lower auxiliary (1 whole step below) are played, resolve and hold for duration
        int upper = pi + 2;  // 1 whole step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeter(EmbRet, upper, pi, lower, pi, durPi, meter);
    } else if (variant == "BTurnF") {  // whole step above, the principal note, and the lower auxiliary (1/2 step below) are played, resolve and hold for duration
        int upper = pi + 2;  // whole step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeter(EmbRet, upper, pi, lower, pi, durPi, meter);
    } else if (variant == "BTurnS") {  // whole step above, the principal note, and the lower auxiliary (1 whole below) are played, resolve and hold for duration
        int upper = pi + 2;  // whole step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeter(EmbRet, upper, pi, lower, pi, durPi, meter);

    // Front and Back Turn variants
    } else if (variant == "FBTurnF") {  // 1/2 step above, the principle note, and the lower turn is 1/2 step below the principal
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 1;  // 1/2 step below
        int underlower = pi - 4;
        handleTurnMeterFB(EmbRet, upper, pi, lower, underlower, lower, pi, durPi, meter);
    } else if (variant == "FBTurnS") {  // 1/2 step above, the principle note, and the lower turn is 1 whole step below the principal
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 2;
        int underlower = pi - 4; // 1 whole step below
        handleTurnMeterFB(EmbRet, upper, pi, lower, underlower, lower, pi, durPi, meter);
    } else if (variant == "FBTurnFF") {  // front and back of turn - both pre and post principal note are 1/2 step above and below
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 1;
        int underlower = pi - 3;// 1/2 step below
        handleTurnMeterFB(EmbRet, upper, pi, lower, underlower, lower, pi, durPi, meter);
    } else if (variant == "FBTurnSS") {  // front and back of turn - both pre and post principal note are 1 whole step above and below
        int upper = pi + 2;  // 1 whole step above
        int lower = pi - 2;
        int underlower = pi - 4;// 1 whole step below
        handleTurnMeterFB(EmbRet, upper, pi, lower, underlower, lower, pi, durPi, meter);
    } else if (variant == "FBTurnFS") {  // front and back of turn, the front is 1/2 step above, the back is 1 whole step below
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 2;
        int underlower = pi - 4;// 1 whole step below
        handleTurnMeterFB(EmbRet, upper, pi, lower, underlower, lower, pi, durPi, meter);
    } else if (variant == "FBTurnSF") {  // front and back of turn, the front is 1 whole step above, the back is 1/2 step below
        int upper = pi + 2;  // 1 whole step above
        int lower = pi - 1;
        int underlower = pi - 4;// 1/2 step below
        handleTurnMeterFB(EmbRet, upper, pi, lower, underlower, lower, pi, durPi, meter);

    // Between Notes Turn variants
    } else if (variant == "BNT") {  // Turn between principal notes, regular
        int upper = pi + 2;  // whole step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterBetween(EmbRet, pi, upper, pi, lower, pi,  durPi, meter);
    } else if (variant == "BNFTurnF") {  // Turn between Notes 1/2 step above, the principal note, and the lower auxiliary (1/2 step below) are played, resolve and hold for duration
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterBetween(EmbRet, pi, upper, pi, lower, pi,  durPi, meter);
    } else if (variant == "BNFTurnS") {  // Turn between Notes 1 whole step above, the principal note, and the lower auxiliary (1 whole step below) are played, resolve and hold for duration
        int upper = pi + 2;  // 1 whole step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterBetween(EmbRet, pi, upper, pi, lower, pi,  durPi, meter);
    } else if (variant == "BNBTurn") {  // Between principal notes, 1 whole step above, the principal note, and the lower auxiliary (1 whole step below) are played, resolve and hold for duration
        int upper = pi + 1;  // 1 whole step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterBetween(EmbRet, pi, upper, pi, lower, pi,  durPi, meter);
    } else if (variant == "BNBTurnS") {  // Between principal notes, whole step above, the principal note, and the lower auxiliary (1 whole below) are played, resolve and hold for duration
        int upper = pi + 1;  // whole step above
        int lower = pi - 3;  // 1 whole step below
        handleTurnMeterBetween(EmbRet, pi, upper, pi, lower, pi,  durPi, meter);
    } else if (variant == "BNFBTurnF") {  // Between principal notes,1/2 step above, the principle note, and the lower turn is 1/2 step below the principal
        int upper = pi + 3;  // 1/2 step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterBetween(EmbRet, pi, upper, pi, lower, pi,  durPi, meter);
    } else if (variant == "BNFBTurnS") {  // Between principal notes, 1/2 step above, the principle note, and the lower turn is 1 whole step below the principal
        int upper = pi + 3;  // 1/2 step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterBetween(EmbRet, pi, upper, pi, lower, pi,  durPi, meter);
    } else if (variant == "BNFBTurnFF") {  // Between principal notes,front and back of turn - both pre and post principal note are 1/2 step above and below
        int upper = pi + 2;  // 1/2 step above
        int lower = pi - 3;  // 1/2 step below
        handleTurnMeterBetween(EmbRet, pi, upper, pi, lower, pi,  durPi, meter);
    } else if (variant == "BNFBTurnSS") {  // Between principal notes, front and back of turn - both pre and post principal note are 1 whole step above and below
        int upper = pi + 4;  // 1 whole step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterBetween(EmbRet, pi, upper, pi, lower, pi,  durPi, meter);
    } else if (variant == "BNFBTurnFS") {  // Between principal notes,front and back of turn, the front is 1/2 step above, the back is 1 whole step below
        int upper = pi + 2;  // 1/2 step above
        int lower = pi - 4;  // 1 whole step below
        handleTurnMeterBetween(EmbRet, pi, upper, pi, lower, pi,  durPi, meter);
    } else if (variant == "BNFBTurnSF") {  // Between principal notes,front and back of turn, the front is 1 whole step above, the back is 1/2 step below
        int upper = pi + 3;  // 1 whole step above
        int lower = pi - 3;  // 1/2 step below
        handleTurnMeterBetween(EmbRet, pi, upper, pi, lower, pi,  durPi, meter);

    // Trilled Turn variants
    } else if (variant == "TT") {  // Trilled regular turn
        int upper = pi + 2;  // whole step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterTrille(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "TFTurnF") {  // Trilled 1/2 step above, the principal note, and the lower auxiliary (1 whole step below) are played, resolve and hold for duration
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterTrille(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "TFTurnS") {  // Trilled 1 whole step above, the principal note, and the lower auxiliary (1 whole step below) are played, resolve and hold for duration
        int upper = pi + 2;  // 1 whole step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterTrille(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "TBTurnF") {  // Trilled whole step above, the principal note, and the lower auxiliary (1/2 step below) are played, resolve and hold for duration
        int upper = pi + 2;  // whole step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterTrille(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "TBTurnS") {  // Trilled whole step above, the principal note, and the lower auxiliary (1 whole below) are played, resolve and hold for duration
        int upper = pi + 2;  // whole step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterTrille(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "TFBTurnF") {  // Trilled 1/2 step above, the principle note, and the lower turn is 1/2 step below the principal
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterTrille(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "TFBTurnS") {  // Trilled 1/2 step above, the principle note, and the lower turn is 1 whole step below the principal
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterTrille(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "TFBTurnFF") {  // Trilled front and back of turn - both pre and post principal note are 1/2 step above and below
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterTrille(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "TFBTurnSS") {  // Trilled front and back of turn - both pre and post principal note are 1 whole step above and below
        int upper = pi + 2;  // 1 whole step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterTrille(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "TFBTurnFS") {  // Trilled front and back of turn, the front is 1/2 step above, the back is 1 whole step below
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterTrille(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "TFBTurnSF") {  // Trilled front and back of turn, the front is 1 whole step above, the back is 1/2 step below
        int upper = pi + 2;  // 1 whole step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterTrille(EmbRet, upper, pi, lower, durPi, meter);

    // P32 Turn variants
    } else if (variant == "P32T") {  // P32 Turn regular turn
        int upper = pi + 2;  // whole step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterP32(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "P32FTurnF") {  // P32 Turn 1/2 step above, the principal note, and the lower auxiliary (1 whole step below) are played, resolve and hold for duration
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterP32(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "P32FTurnS") {  // P32 Turn 1 whole step above, the principal note, and the lower auxiliary (1 whole step below) are played, resolve and hold for duration
        int upper = pi + 2;  // 1 whole step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterP32(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "P32BTurnF") {  // P32 Turn whole step above, the principal note, and the lower auxiliary (1/2 step below) are played, resolve and hold for duration
        int upper = pi + 2;  // whole step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterP32(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "P32BTurnS") {  // P32 Turn whole step above, the principal note, and the lower auxiliary (1 whole below) are played, resolve and hold for duration
        int upper = pi + 2;  // whole step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterP32(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "P32FBTurnF") {  // P32 Turn 1/2 step above, the principle note, and the lower turn is 1/2 step below the principal
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterP32(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "P32FBTurnS") {  // P32 Turn 1/2 step above, the principle note, and the lower turn is 1 whole step below the principal
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterP32(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "P32FBTurnFF") {  // P32 Turn front and back of turn - both pre and post principal note are 1/2 step above and below
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterP32(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "P32FBTurnSS") {  // P32 Turn front and back of turn - both pre and post principal note are 1 whole step above and below
        int upper = pi + 2;  // 1 whole step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterP32(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "P32FBTurnFS") {  // P32 Turn front and back of turn, the front is 1/2 step above, the back is 1 whole step below
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterP32(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "P32FBTurnSF") {  // P32 Turn front and back of turn, the front is 1 whole step above, the back is 1/2 step below
        int upper = pi + 2;  // 1 whole step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterP32(EmbRet, upper, pi, lower, durPi, meter);

    // Snapped Turn variants
    } else if (variant == "ST") {  // Snapped turn regular turn
        int upper = pi + 2;  // whole step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterSnapped(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "SFTurnF") {  // Snapped turn 1/2 step above, the principal note, and the lower auxiliary (1 whole step below) are played, resolve and hold for duration
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterSnapped(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "SFTurnS") {  // Snapped turn Turn 1 whole step above, the principal note, and the lower auxiliary (1 whole step below) are played, resolve and hold for duration
        int upper = pi + 2;  // 1 whole step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterSnapped(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "SBTurnF") {  // Snapped turn whole step above, the principal note, and the lower auxiliary (1/2 step below) are played, resolve and hold for duration
        int upper = pi + 2;  // whole step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterSnapped(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "SBTurnS") {  // Snapped turn Turn whole step above, the principal note, and the lower auxiliary (1 whole below) are played, resolve and hold for duration
        int upper = pi + 2;  // whole step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterSnapped(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "SFBTurnF") {  // Snapped turn 1/2 step above, the principle note, and the lower turn is 1/2 step below the principal
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterSnapped(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "SFBTurnS") {  // Snapped turn 1/2 step above, the principle note, and the lower turn is 1 whole step below the principal
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterSnapped(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "SFBTurnFF") {  // Snapped turn front and back of turn - both pre and post principal note are 1/2 step above and below
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterSnapped(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "SFBTurnSS") {  // Snapped turn front and back of turn - both pre and post principal note are 1 whole step above and below
        int upper = pi + 2;  // 1 whole step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterSnapped(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "SFBTurnFS") {  // Snapped turn front and back of turn, the front is 1/2 step above, the back is 1 whole step below
        int upper = pi + 1;  // 1/2 step above
        int lower = pi - 2;  // 1 whole step below
        handleTurnMeterSnapped(EmbRet, upper, pi, lower, durPi, meter);
    } else if (variant == "SFBTurnSF") {  // Snapped turn front and back of turn, the front is 1 whole step above, the back is 1/2 step below
        int upper = pi + 2;  // 1 whole step above
        int lower = pi - 1;  // 1/2 step below
        handleTurnMeterSnapped(EmbRet, upper, pi, lower, durPi, meter);

    } else {
        // Handle unknown variant
        std::cerr << "Unknown turn variant: " << variant << '\n';
    }

    return EmbRet;
}

// NEW FUNCTION: Generate random pool of Turn variants for user selection
std::vector<std::string> generateRandomVariantPool(int poolSize) {
    // Define the complete pool of Turn variants
    const std::vector<std::string> allVariants = {
        // Basic Turn variants
        "Turn", "FTurnF", "FTurnS", "BTurnF", "BTurnS",
        // Front and Back Turn variants
        "FBTurnF", "FBTurnS", "FBTurnFF", "FBTurnSS", "FBTurnFS", "FBTurnSF",
        // Between Notes Turn variants
        "BNT", "BNFTurnF", "BNFTurnS", "BNBTurn", "BNBTurnS",
        "BNFBTurnF", "BNFBTurnS", "BNFBTurnFF", "BNFBTurnSS", "BNFBTurnFS", "BNFBTurnSF",
        // Trilled Turn variants
        "TT", "TFTurnF", "TFTurnS", "TBTurnF", "TBTurnS",
        "TFBTurnF", "TFBTurnS", "TFBTurnFF", "TFBTurnSS", "TFBTurnFS", "TFBTurnSF",
        // P32 Turn variants
        "P32T", "P32FTurnF", "P32FTurnS", "P32BTurnF", "P32BTurnS",
        "P32FBTurnF", "P32FBTurnS", "P32FBTurnFF", "P32FBTurnSS", "P32FBTurnFS", "P32FBTurnSF",
        // Snapped Turn variants
        "ST", "SFTurnF", "SFTurnS", "SBTurnF", "SBTurnS",
        "SFBTurnF", "SFBTurnS", "SFBTurnFF", "SFBTurnSS", "SFBTurnFS", "SFBTurnSF"
    };

    // Randomly select variants from the pool
    std::vector<std::string> selectedVariants;
    std::vector<std::string> tempPool = allVariants; // Copy for shuffling

    // Shuffle the pool
    std::random_shuffle(tempPool.begin(), tempPool.end());

    // Select the requested number of variants
    int actualPoolSize = std::min(poolSize, static_cast<int>(tempPool.size()));
    for (int i = 0; i < actualPoolSize; ++i) {
        selectedVariants.push_back(tempPool[i]);
    }

    return selectedVariants;
}

// NEW FUNCTION: Parse multiple selections from user input (space-separated numbers)
std::vector<int> parseMultipleSelections(const std::string& input, int maxChoice) {
    std::vector<int> selections;
    std::istringstream iss(input);
    std::string token;

    while (iss >> token) {
        try {
            int choice = std::stoi(token);
            if (choice >= 1 && choice <= maxChoice) {
                // Check for duplicates
                if (std::find(selections.begin(), selections.end(), choice) == selections.end()) {
                    selections.push_back(choice);
                }
            } else {
                std::cout << "Warning: " << choice << " is out of range (1-" << maxChoice << ") and will be ignored." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Warning: '" << token << "' is not a valid number and will be ignored." << std::endl;
        }
    }

    return selections;
}

// ENHANCED FUNCTION: Let user choose from a pool of Turn variants with multiple choice support
std::vector<std::string> getUserVariantChoices() {
    std::cout << "\n=== Turn Variant Selection ===" << std::endl;
    std::cout << "Choose your variant selection method:" << std::endl;
    std::cout << "1. Random selection (original behavior)" << std::endl;
    std::cout << "2. Choose from a random pool of variants" << std::endl;
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
        std::cout << "======================================\n" << std::endl;
        return {"RANDOM"}; // Special flag for random selection
    }

    // Generate random pool of variants
    std::vector<std::string> variantPool = generateRandomVariantPool(15);

    std::cout << "\nHere are 15 randomly selected variants to choose from:" << std::endl;
    std::cout << "======================================================" << std::endl;

    for (size_t i = 0; i < variantPool.size(); ++i) {
        std::cout << (i + 1) << ". " << variantPool[i] << std::endl;
    }

    if (choice == 2) {
        // Single selection (original behavior)
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
    } else {
        // Multiple selection (new feature)
        std::cout << "\nSelect from any number (separated by spaces) from 1 to " << variantPool.size()
                  << " of the variants you would like to randomly apply:" << std::endl;
        std::cout << "Example: 1 3 7 12 (to select variants 1, 3, 7, and 12)" << std::endl;
        std::cout << "Your selection: ";

        std::cin.ignore(); // Clear the input buffer
        std::string input;
        std::getline(std::cin, input);

        std::vector<int> selections = parseMultipleSelections(input, static_cast<int>(variantPool.size()));

        if (selections.empty()) {
            std::cout << "No valid selections made. Using random selection instead." << std::endl;
            return {"RANDOM"};
        }

        // Convert selections to variant names
        std::vector<std::string> selectedVariants;
        for (int selection : selections) {
            selectedVariants.push_back(variantPool[selection - 1]);
        }

        std::cout << "\nYou selected " << selectedVariants.size() << " variants:" << std::endl;
        for (size_t i = 0; i < selectedVariants.size(); ++i) {
            std::cout << (i + 1) << ". " << selectedVariants[i] << std::endl;
        }
        std::cout << "These variants will be randomly applied to transformations." << std::endl;
        std::cout << "======================================\n" << std::endl;

        return selectedVariants;
    }
}

// ENHANCED applyVariant function with multiple choice support
std::vector<std::pair<int, int>> applyVariant(int pi, int durPi, std::string& selectedVariant, const std::vector<std::string>& userChoices = {}) {
    if (userChoices.empty() || (userChoices.size() == 1 && userChoices[0] == "RANDOM")) {
        // Original random selection behavior - use Turn variants
        const std::vector<std::string> variants = {
            // Basic Turn variants
            "Turn", "FTurnF", "FTurnS", "BTurnF", "BTurnS",

            // Front and Back Turn variants
            "FBTurnF", "FBTurnS", "FBTurnFF", "FBTurnSS", "FBTurnFS", "FBTurnSF",

            // Between Notes Turn variants
            "BNT", "BNFTurnF", "BNFTurnS", "BNBTurn", "BNBTurnS",
            "BNFBTurnF", "BNFBTurnS", "BNFBTurnFF", "BNFBTurnSS", "BNFBTurnFS", "BNFBTurnSF",

            // Trilled Turn variants
            "TT", "TFTurnF", "TFTurnS", "TBTurnF", "TBTurnS",
            "TFBTurnF", "TFBTurnS", "TFBTurnFF", "TFBTurnSS", "TFBTurnFS", "TFBTurnSF",

            // P32 Turn variants
            "P32T", "P32FTurnF", "P32FTurnS", "P32BTurnF", "P32BTurnS",
            "P32FBTurnF", "P32FBTurnS", "P32FBTurnFF", "P32FBTurnSS", "P32FBTurnFS", "P32FBTurnSF",

            // Snapped Turn variants
            "ST", "SFTurnF", "SFTurnS", "SBTurnF", "SBTurnS",
            "SFBTurnF", "SFBTurnS", "SFBTurnFF", "SFBTurnSS", "SFBTurnFS", "SFBTurnSF"
        };

        // Randomly select a turn variant
        selectedVariant = variants[rand() % variants.size()];

        // Use the comprehensive turn implementation with DUPLE meter by default
        return applyTurnVariants(pi, durPi, DUPLE, selectedVariant);
    } else {
        // Use the user's selected variants - randomly choose from their selection
        selectedVariant = userChoices[rand() % userChoices.size()];

        // Use the comprehensive turn implementation with DUPLE meter by default
        return applyTurnVariants(pi, durPi, DUPLE, selectedVariant);
    }
}

// Simple Turn Variant Pool - ORIGINAL FUNCTION NAME PRESERVED
std::vector<std::pair<int, int>> applyTurnVariant(int pi, int durPi, std::string& selectedVariant) {
    // Define the pool of turn variants - all options from the implementation
    const std::vector<std::string> variants = {
        // Basic Turn variants
        "Turn", "FTurnF", "FTurnS", "BTurnF", "BTurnS",

        // Front and Back Turn variants
        "FBTurnF", "FBTurnS", "FBTurnFF", "FBTurnSS", "FBTurnFS", "FBTurnSF",

        // Between Notes Turn variants
        "BNT", "BNFTurnF", "BNFTurnS", "BNBTurn", "BNBTurnS",
        "BNFBTurnF", "BNFBTurnS", "BNFBTurnFF", "BNFBTurnSS", "BNFBTurnFS", "BNFBTurnSF",

        // Trilled Turn variants
        "TT", "TFTurnF", "TFTurnS", "TBTurnF", "TBTurnS",
        "TFBTurnF", "TFBTurnS", "TFBTurnFF", "TFBTurnSS", "TFBTurnFS", "TFBTurnSF",

        // P32 Turn variants
        "P32T", "P32FTurnF", "P32FTurnS", "P32BTurnF", "P32BTurnS",
        "P32FBTurnF", "P32FBTurnS", "P32FBTurnFF", "P32FBTurnSS", "P32FBTurnFS", "P32FBTurnSF",

        // Snapped Turn variants
        "ST", "SFTurnF", "SFTurnS", "SBTurnF", "SBTurnS",
        "SFBTurnF", "SFBTurnS", "SFBTurnFF", "SFBTurnSS", "SFBTurnFS", "SFBTurnSF"
    };

    // Randomly select a variant
    selectedVariant = variants[rand() % variants.size()];

    // Apply the selected variant using the main function
    return applyTurnVariants(pi, durPi, DUPLE, selectedVariant); // Default to DUPLE meter
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
    std::cout << "\n=== Turn Transformation Settings ===" << std::endl;
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

// ENHANCED processFile function with percentage control and multiple choice variant support
void processFile(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream input(inputFile);
    std::ofstream output(outputFile);

    if (!input.is_open() || !output.is_open()) {
        std::cerr << "Error opening files.\n";
        return;
    }

    // Get transformation percentage from user
    double transformationPercentage = getUserTransformationPercentage();

    // Get user variant choices (can be multiple)
    std::vector<std::string> userVariantChoices = getUserVariantChoices();

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

        // Check if this label is eligible for transformation - ORIGINAL LABELS PRESERVED
        if (label == "I8" || label == "U2R" || label == "SPD" || label == "CH" ||
            label == "CW" || label == "CD" || label == "HT" || label == "FM" ||
            label == "SLP" || label == "RN" || label == "LAD" || label == "DNW" || label == "SAN"||
            label == "RTR2" || label == "TNTDN"||label == "SMP" || label == "LP"||
            label == "DHT" || label == "LNR"||label == "TNTLN" || label == "TNTTN"||label == "RTD2") {

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

    if (userVariantChoices.size() == 1 && userVariantChoices[0] == "RANDOM") {
        std::cout << "Random turn variants used for transformations" << std::endl;
    } else {
        std::cout << "User-selected variants used for transformations:" << std::endl;
        for (const auto& [variant, count] : variantUsageCount) {
            std::cout << "  " << variant << ": " << count << " times" << std::endl;
        }
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
    std::cout << "========= The Turn Transformation Tool ================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "====This transformation tool takes a note==============" << std::endl;
    std::cout << "====that is appropriately labeled and applies==========" << std::endl;
    std::cout << "==========a turn to that note.=========================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "=== Use with labels: I8, U2R, SPD, CH, CW ==============" << std::endl;
    std::cout << "=== CD, HT, FM, SLP, RN, LAD, DNW, SAN, RTR2 ===========" << std::endl;
    std::cout << "=== TNTDN, SMP, LP, DHT, LNR, TNTLN, TNTTN, RTD2 =======" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "======== with % Transformation Selection ===============" << std::endl;
    std::cout << "======== Midi File Output and Text Output ==============" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Enter the path to the input file: ";
        std::getline(std::cin, inputFile);

    // Remove quotes if the user included them
    if (inputFile.size() >= 2 &&
        ((inputFile.front() == '"' && inputFile.back() == '"') ||
         (inputFile.front() == '\'' && inputFile.back() == '\''))) {
        inputFile = inputFile.substr(1, inputFile.size() - 2);
         }

    // Generate unique output filenames with timestamp
    std::string outputFile = "Turns_Transformation_Multiple_Choice_" + std::string(timestamp) + ".txt";
    std::string midiOutputFile = "Turns_transformation_Multiple_Choice_" + std::string(timestamp) + ".mid";

    std::cout << "Processing file: " << inputFile << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Output will be written to: " << outputFile << std::endl;
    std::cout << std::endl;
    std::cout << "MIDI output will be written to: " << midiOutputFile << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << std::endl;

    // Process the file (now with percentage control and multiple choice variant support)
    processFile(inputFile, outputFile);

    // Convert the processed file to MIDI
    convertToMidi(outputFile, midiOutputFile);

    std::cout << "Processing complete!" << std::endl;
    std::cout << std::endl;
    std::cout << "Press Enter to exit...";
    std::cin.get();

    return 0;
}