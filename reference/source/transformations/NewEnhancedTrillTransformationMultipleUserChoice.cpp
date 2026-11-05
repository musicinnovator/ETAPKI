// Trill Transformation (C) 2025 <Roderick Harris>
// Enhanced version with percentage control for transformations and Multiple Choice Variant Selection
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

void handleMeterShortReg(std::vector<std::pair<int, int>>& EmbRet, int p1, int p2, int p3, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment = durPi / 4;
        EmbRet.push_back({p1, segment});
        EmbRet.push_back({p2, segment});
        EmbRet.push_back({p1, segment});
        EmbRet.push_back({p2, durPi - 3 * segment}); // Use remaining duration
    } else if (meter == TRIPLE) {
        int segment = durPi / 6;
        for (int i = 0; i < 5; ++i) { // Alternate p1 and p2
            EmbRet.push_back({i % 2 == 0 ? p1 : p2, segment});
        }
        EmbRet.push_back({p2, durPi - 5 * segment}); // Remaining duration
    }
}

void handleMeterNormalReg(std::vector<std::pair<int, int>>& EmbRet, int p1, int p2, int p3, int p4, int p5, int pi, int durPi, TimeMeter meter) {
    int segment = durPi / 8;
    if (meter == DUPLE) {
        for (int i = 0; i < 6; ++i) {
            EmbRet.push_back({i % 2 == 0 ? p1 : p2, segment});
        }
        EmbRet.push_back({p2, durPi - 6 * segment}); // Remaining duration
    } else if (meter == TRIPLE) {
        for (int i = 0; i < 6; ++i) {
            EmbRet.push_back({i % 2 == 0 ? p1 : p2, segment});
        }
        EmbRet.push_back({p2, durPi - 6 * segment}); // Remaining duration
    }
}

void handleMeterLongReg(std::vector<std::pair<int, int>>& EmbRet, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE || meter == TRIPLE) {
        int segment = durPi / 8;
        for (int i = 0; i < 7; ++i) {
            EmbRet.push_back({i % 2 == 0 ? p1 : p2, segment});
        }
        EmbRet.push_back({p2, durPi - 7 * segment}); // Remaining duration
    }
}

void handleMeterDelayedNormal(std::vector<std::pair<int, int>>& EmbRet, int p1, int p2, int p3, int p4, int p5, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segmentA = durPi / 4;
        int segmentB = durPi / 8;
        EmbRet.push_back({p1, segmentA});
        EmbRet.push_back({p1, segmentB});
        EmbRet.push_back({p2, segmentB});
        EmbRet.push_back({p1, segmentB});
        EmbRet.push_back({p2, segmentB});
        EmbRet.push_back({p2, durPi - (segmentA + 4 * segmentB)}); // Remaining duration
    } else if (meter == TRIPLE) {
        int segmentA = durPi / 8;
        EmbRet.push_back({p1, segmentA * 2}); // p1 at 1/4
        for (int i = 0; i < 4; i++) {
            EmbRet.push_back({i % 2 == 0 ? p2 : p1, segmentA});
        }
        EmbRet.push_back({p2, durPi - (segmentA * 6)}); // Remaining duration
    }
}

void handleMeterDelayedLong(std::vector<std::pair<int, int>>& EmbRet, int p1, int p2, int p3, int p4, int p5, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE || meter == TRIPLE) {
        int segment = durPi / 8;
        EmbRet.push_back({p1, segment * 2}); // 1/4 duration
        for (int i = 0; i < 5; ++i) {
            EmbRet.push_back({i % 2 == 0 ? p2 : p1, segment});
        }
        EmbRet.push_back({p2, durPi - 7 * segment}); // Remaining duration
    }
}

void handleMeterAscendingShort(std::vector<std::pair<int, int>>& EmbRet, int p1, int p2, int p3, int p4, int p5, int p6, int durPi, TimeMeter meter) {
    int segment = durPi / 8;
    if (meter == DUPLE) {
        for (int i = 0; i < 4; ++i) {
            EmbRet.push_back({i % 2 == 0 ? p1 : p2, segment});
        }
        EmbRet.push_back({p1, durPi / 4});
        EmbRet.push_back({p2, durPi - (4 * segment + durPi / 4)}); // Remaining duration
    } else if (meter == TRIPLE) {
        for (int i = 0; i < 4; ++i) {
            EmbRet.push_back({i % 2 == 0 ? p1 : p2, segment});
        }
        EmbRet.push_back({p1, durPi / 6});
        EmbRet.push_back({p2, durPi - (4 * segment + durPi / 6)}); // Remaining duration
    }
}

void handleMeterAscendingNormal(std::vector<std::pair<int, int>>& EmbRet, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment = durPi / 8;
        for (int i = 0; i < 7; ++i) {
            EmbRet.push_back({i % 2 == 0 ? p1 : p2, segment}); // Alternate p1 and p2
        }
        EmbRet.push_back({p2, durPi - 7 * segment}); // Remaining duration
    } else if (meter == TRIPLE) {
        int segment = durPi / 8;
        for (int i = 0; i < 7; ++i) {
            EmbRet.push_back({i % 2 == 0 ? p1 : p2, segment}); // Alternate p1 and p2
        }
        EmbRet.push_back({p2, durPi - 7 * segment}); // Remaining duration
    }
}

void handleMeterTerminalShort(std::vector<std::pair<int, int>>& EmbRet, int p1, int p2, int p3, int pi, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment = durPi / 4;
        EmbRet.push_back({p1, segment});
        EmbRet.push_back({p2, segment});
        EmbRet.push_back({p1, segment});
        EmbRet.push_back({p2, durPi - 3 * segment}); // Remaining duration
    } else if (meter == TRIPLE) {
        int segment = durPi / 6;
        for (int i = 0; i < 5; ++i) {
            EmbRet.push_back({i % 2 == 0 ? p1 : p2, segment});
        }
        EmbRet.push_back({p2, durPi - 5 * segment}); // Remaining duration
    }
}

void handleMeterAscendingLong(std::vector<std::pair<int, int>>& EmbRet, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8, int p9, int p10, int p11, int p12, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment = durPi / 16;
        for (int i = 0; i < 15; ++i) {
            EmbRet.push_back({i % 2 == 0 ? p1 : p2, segment}); // Alternate between p1 and p2 for 15 segments
        }
        EmbRet.push_back({p2, durPi - 15 * segment}); // Remaining time for the last segment
    } else if (meter == TRIPLE) {
        int segment = durPi / 12;
        for (int i = 0; i < 11; ++i) {
            EmbRet.push_back({i % 2 == 0 ? p1 : p2, segment}); // Alternate between p1 and p2 for 11 segments
        }
        EmbRet.push_back({p2, durPi - 11 * segment}); // Remaining time for the last segment
    }
}

void handleMeterTerminalNormal(std::vector<std::pair<int, int>>& EmbRet, int p1, int p2, int p3, int pi, int p4, int p5, int p6, int p7, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment = durPi / 8;
        for (int i = 0; i < 7; ++i) {
            EmbRet.push_back({i % 2 == 0 ? p1 : p2, segment});
        }
        EmbRet.push_back({p2, durPi - 7 * segment});
    } else if (meter == TRIPLE) {
        int segment = durPi / 12;
        for (int i = 0; i < 11; ++i) { // Alternate p1 and p2
            EmbRet.push_back({i % 2 == 0 ? p1 : p2, segment});
        }
        EmbRet.push_back({p2, durPi - 11 * segment}); // Remaining duration
    }
}

void handleMeterTerminalLong(std::vector<std::pair<int, int>>& EmbRet, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8, int p9, int p10, int p11, int p12, int p13, int p14, int p15, int p16, int durPi, TimeMeter meter) {
    if (meter == DUPLE) {
        int segment = durPi / 16;
        for (int i = 0; i < 15; ++i) {
            EmbRet.push_back({i % 2 == 0 ? p1 : p2, segment}); // Alternate p1 and p2
        }
        EmbRet.push_back({p2, durPi - 15 * segment}); // Remaining duration
    } else if (meter == TRIPLE) {
        int segment = durPi / 12;
        for (int i = 0; i < 11; ++i) {
            EmbRet.push_back({i % 2 == 0 ? p1 : p2, segment}); // Alternate p1 and p2
        }
        EmbRet.push_back({p2, durPi - 11 * segment}); // Remaining duration
    }
}

// Main function from pasted.txt, renamed to applyTrill
std::vector<std::pair<int, int>> applyTrill(int pi, int durPi, TimeMeter meter, const std::string& variant) {
    if (durPi <= 0) {
        throw std::invalid_argument("Duration (durPi) must be greater than 0");
    }
    if (meter != DUPLE && meter != TRIPLE) {
        throw std::invalid_argument("Invalid TimeMeter");
    }

    // Short Reg Trills - Baroque and Classical --------------------------------------------------
    std::vector<std::pair<int, int>> EmbRet;
    if (variant == "BTrRs1") {
        handleMeterShortReg(EmbRet, pi + 2, pi, pi + 2, pi,  durPi, meter);
    } else if (variant == "BTrRs5") {
        handleMeterShortReg(EmbRet, pi + 1, pi , pi + 1, pi , durPi, meter);
    } else if (variant == "CTrRs1") {
        handleMeterShortReg(EmbRet, pi, pi + 2, pi, pi + 2 , durPi, meter);
    } else if (variant == "CTrRs5") {
        handleMeterShortReg(EmbRet, pi, pi + 1, pi, pi + 1 , durPi, meter);
    } else {  // Unknown variant
        std::cerr << "Unknown variant: " << variant << '\n';
    }
    // Normal Reg Trills - Baroque and Classical --------------------------------------------------
    if (variant == "BTrRn1") {
        handleMeterNormalReg(EmbRet, pi + 2, pi, pi + 2, pi, pi + 2, pi,  durPi, meter);
    } else if (variant == "BTrRn5") {
        handleMeterNormalReg(EmbRet, pi + 1, pi, pi + 1, pi, pi + 1, pi,  durPi, meter);
    } else if (variant == "CTrRn1") {
        handleMeterNormalReg(EmbRet, pi, pi + 5, pi, pi + 5, pi, pi + 5, durPi, meter);
    } else if (variant == "CTrRn5") {
        handleMeterNormalReg(EmbRet, pi, pi + 1, pi, pi + 1, pi, pi + 1, durPi, meter);
    } else {  // Unknown variant
        std::cerr << "Unknown variant: " << variant << '\n';
    }

    //Long Reg Trills - Baroque and Classical --------------------------------------------------

    if (variant == "BTrRl1") {
        handleMeterLongReg(EmbRet, pi + 2, pi, pi + 2, pi, pi + 2, pi, pi + 2, pi,  durPi, meter);
    } else if (variant == "BTrRl5") {
       handleMeterLongReg(EmbRet, pi + 1, pi, pi + 1, pi, pi + 1, pi, pi + 1, pi,  durPi, meter);
    } else if (variant == "CTrRl1") {
        handleMeterLongReg(EmbRet, pi, pi + 2, pi, pi + 2, pi , pi + 2, pi, pi + 2 ,durPi, meter);
    } else if (variant == "CTrRl5") {
        handleMeterLongReg(EmbRet, pi, pi + 1, pi, pi + 1, pi, pi + 1, pi, pi + 1 ,durPi, meter);
    } else {  // Unknown variant
        std::cerr << "Unknown variant: " << variant << '\n';
    }

    // Delayed Normal Trills - Baroque and Classical --------------------------------------------------
    if (variant == "BTrDen1") {
        handleMeterDelayedNormal(EmbRet, pi + 2, pi, pi + 2, pi, pi + 2, pi,  durPi, meter);
    } else if (variant == "BTrDen5") {
        handleMeterDelayedNormal(EmbRet, pi + 1, pi, pi + 1, pi, pi + 1, pi,  durPi, meter);
    } else if (variant == "CTrDen1") {
        handleMeterDelayedNormal(EmbRet, pi, pi + 5, pi, pi + 5, pi, pi + 5, durPi, meter);
    } else if (variant == "CTrDen5") {
        handleMeterDelayedNormal(EmbRet, pi, pi + 1, pi, pi + 1, pi, pi + 1, durPi, meter);
    } else {  // Unknown variant
        std::cerr << "Unknown variant: " << variant << '\n';
    }

    //Delayed Long Trills - Baroque and Classical --------------------------------------------------

    if (variant == "BTrDel1") {
        handleMeterDelayedLong(EmbRet, pi + 2, pi, pi + 2, pi, pi + 2, pi ,  durPi, meter);
    } else if (variant == "BTrDel5") {
        handleMeterDelayedLong(EmbRet, pi + 1, pi, pi + 1, pi, pi + 1, pi, durPi, meter);
    } else if (variant == "CTrDel1") {
        handleMeterDelayedLong(EmbRet, pi, pi + 2, pi, pi + 2, pi , pi + 2, durPi, meter);
    } else if (variant == "CTrDel5") {
        handleMeterDelayedLong(EmbRet, pi, pi + 1, pi, pi + 1, pi, pi + 1, durPi, meter);
    } else {  // Unknown variant
        std::cerr << "Unknown variant: " << variant << '\n';
    }

    // Ascending Short  Trills - Baroque and Classical --------------------------------------------------

    if (variant == "BTrAs1 ") {
        handleMeterAscendingShort(EmbRet, pi -2, pi, pi + 2, pi, pi + 2, pi,  durPi, meter);
    } else if (variant == "BTrAs5 ") {
        handleMeterAscendingShort(EmbRet, pi -1, pi, pi + 2, pi, pi + 2, pi,  durPi, meter);
    } else if (variant == "CTrAs1 ") {
        handleMeterAscendingShort(EmbRet, pi -2, pi, pi + 2, pi, pi , pi + 2,  durPi, meter);
    } else if (variant == "CTrAs5 ") {
        handleMeterAscendingShort(EmbRet, pi -1, pi, pi + 2, pi, pi , pi + 2,  durPi, meter);
    } else {  // Unknown variant
        std::cerr << "Unknown variant: " << variant << '\n';
    }
    // Ascending Normal Trills - Baroque and Classical --------------------------------------------------
    if (variant == "BTrAn1") {
        handleMeterAscendingNormal(EmbRet, pi - 2, pi, pi + 2, pi, pi + 2, pi, pi + 2, pi,  durPi, meter);
    } else if (variant == "BTrAn5") {
        handleMeterAscendingNormal(EmbRet, pi - 1, pi, pi + 2, pi, pi + 2, pi, pi + 2, pi,  durPi, meter);
    } else if (variant == "CTrAn1") {
        handleMeterAscendingNormal(EmbRet, pi - 2, pi, pi + 2, pi, pi, pi + 2, pi, pi + 2, durPi, meter);
    } else if (variant == "CTrAn5") {
        handleMeterAscendingNormal(EmbRet, pi - 1, pi, pi + 2, pi, pi , pi + 2 , pi, pi + 2,  durPi, meter);
    } else {  // Unknown variant
        std::cerr << "Unknown variant: " << variant << '\n';
    }

    //Ascending Long Trills - Baroque and Classical --------------------------------------------------

    if (variant == "BTrAl1") {
        handleMeterAscendingLong(EmbRet, pi - 2, pi, pi + 2, pi,  pi + 2, pi, pi + 2, pi, pi + 2, pi, pi + 2, pi, durPi, meter);
    } else if (variant == "BTrAl5") {
       handleMeterAscendingLong(EmbRet, pi - 1, pi, pi + 2, pi,  pi + 2, pi, pi + 2, pi, pi + 2, pi, pi + 2, pi, durPi, meter);
    } else if (variant == "CTrAl1") {
        handleMeterAscendingLong(EmbRet, pi - 2, pi, pi + 2, pi, pi, pi + 2, pi, pi + 2, pi, pi + 2, pi, pi + 2, durPi, meter);
    } else if (variant == "CTrAl5") {
        handleMeterAscendingLong(EmbRet, pi - 1, pi, pi + 2, pi, pi, pi + 2, pi, pi + 2, pi, pi + 2, pi, pi + 2, durPi, meter);
    } else {  // Unknown variant
        std::cerr << "Unknown variant: " << variant << '\n';
    }

    // Descending Short  Trills - Baroque and Classical --------------------------------------------------

    if (variant == "BTrDs1 ") {
        handleMeterAscendingShort(EmbRet, pi + 2, pi, pi - 2, pi, pi + 2, pi,  durPi, meter);
    } else if (variant == "BTrDs5 ") {
        handleMeterAscendingShort(EmbRet, pi + 1, pi, pi - 2, pi, pi + 2, pi,  durPi, meter);
    } else if (variant == "CTrDs1 ") {
        handleMeterAscendingShort(EmbRet, pi + 2, pi, pi - 2, pi, pi , pi + 2,  durPi, meter);
    } else if (variant == "CTrDs5 ") {
        handleMeterAscendingShort(EmbRet, pi + 1, pi, pi - 2, pi, pi , pi + 2,  durPi, meter);
    } else {  // Unknown variant
        std::cerr << "Unknown variant: " << variant << '\n';
    }
    // Descending Normal Trills - Baroque and Classical --------------------------------------------------
    if (variant == "BTrDn1") {
        handleMeterAscendingNormal(EmbRet, pi + 2, pi, pi - 2, pi, pi + 2, pi, pi + 2, pi,  durPi, meter);
    } else if (variant == "BTrDn5") {
        handleMeterAscendingNormal(EmbRet, pi + 1, pi, pi - 2, pi, pi + 2, pi, pi + 2, pi,  durPi, meter);
    } else if (variant == "CTrDn1") {
        handleMeterAscendingNormal(EmbRet, pi + 2, pi, pi - 2, pi, pi, pi + 2, pi, pi + 2, durPi, meter);
    } else if (variant == "CTrDn5") {
        handleMeterAscendingNormal(EmbRet, pi + 1, pi, pi - 2, pi, pi , pi + 2 , pi, pi + 2,  durPi, meter);
    } else {  // Unknown variant
        std::cerr << "Unknown variant: " << variant << '\n';
    }

    //Descending Long Trills - Baroque and Classical --------------------------------------------------

    if (variant == "BTrDl1") {
        handleMeterAscendingLong(EmbRet, pi + 2, pi, pi - 2, pi,  pi + 2, pi, pi + 2, pi, pi + 2, pi, pi + 2, pi, durPi, meter);
    } else if (variant == "BTrDl5") {
       handleMeterAscendingLong(EmbRet, pi + 1, pi, pi - 2, pi,  pi + 2, pi, pi + 2, pi, pi + 2, pi, pi + 2, pi, durPi, meter);
    } else if (variant == "CTrDl1") {
        handleMeterAscendingLong(EmbRet, pi + 2, pi, pi - 2, pi, pi, pi + 2, pi, pi + 2, pi, pi + 2, pi, pi + 2, durPi, meter);
    } else if (variant == "CTrDl5") {
        handleMeterAscendingLong(EmbRet, pi + 1, pi, pi - 2, pi, pi, pi + 2, pi, pi + 2, pi, pi + 2, pi, pi + 2, durPi, meter);
    } else {  // Unknown variant
        std::cerr << "Unknown variant: " << variant << '\n';
    }

        // Terminal Short  Trills - Baroque and Classical --------------------------------------------------

    if (variant == "BTrTs1 ") {
        handleMeterTerminalShort(EmbRet, pi + 2, pi, pi - 2, pi,  durPi, meter);
    } else if (variant == "BTrTs5 ") {
        handleMeterTerminalShort(EmbRet, pi + 1, pi, pi - 2, pi,  durPi, meter);
    } else if (variant == "CTrTs1 ") {
        handleMeterTerminalShort(EmbRet, pi + 2, pi, pi - 2, pi,  durPi, meter);
    } else if (variant == "CTrTs5 ") {
        handleMeterTerminalShort(EmbRet, pi + 1, pi, pi - 2, pi,  durPi, meter);
    } else {  // Unknown variant
        std::cerr << "Unknown variant: " << variant << '\n';
    }
    // Terminal Normal Trills - Baroque and Classical --------------------------------------------------
    if (variant == "BTrTn1") {
        handleMeterTerminalNormal(EmbRet, pi + 2, pi, pi + 2, pi, pi + 2, pi, pi - 2, pi,  durPi, meter);
    } else if (variant == "BTrTn5") {
        handleMeterTerminalNormal(EmbRet, pi + 1, pi, pi +1, pi, pi + 1, pi, pi - 2, pi,  durPi, meter);
    } else if (variant == "CTrTn1") {
        handleMeterTerminalNormal(EmbRet, pi, pi + 2, pi, pi + 2,pi, pi + 2, pi - 2 , pi, durPi, meter);
    } else if (variant == "CTrTn5") {
        handleMeterTerminalNormal(EmbRet, pi, pi + 1, pi, pi + 1,pi, pi + 1, pi - 2 , pi, durPi, meter);
    } else {  // Unknown variant
        std::cerr << "Unknown variant: " << variant << '\n';
    }

    //Terminal Long Trills - Baroque and Classical --------------------------------------------------

    if (variant == "BTrTl1") {
        handleMeterTerminalLong(EmbRet, pi + 2, pi, pi + 2, pi, pi + 2, pi, pi + 2, pi, pi + 2, pi, pi + 2, pi, pi + 2, pi, pi - 2, pi,  durPi, meter);
    } else if (variant == "BTrTl5") {
       handleMeterTerminalLong(EmbRet, pi + 1, pi, pi + 1, pi, pi + 1, pi, pi + 1, pi, pi + 1, pi, pi + 1, pi, pi + 1, pi, pi - 2, pi,  durPi, meter);
    } else if (variant == "CTrTl1") {
        handleMeterTerminalLong(EmbRet, pi , pi+ 2, pi , pi + 2, pi , pi + 2, pi , pi + 2, pi , pi + 2, pi  , pi + 2, pi , pi + 2, pi - 2, pi,  durPi, meter);
    } else if (variant == "CTrTl5") {
        handleMeterTerminalLong(EmbRet, pi , pi + 1, pi , pi + 1, pi , pi + 1, pi , pi + 1, pi , pi + 1, pi  , pi + 1, pi , pi + 1, pi - 2, pi,  durPi, meter);
    } else {  // Unknown variant
        std::cerr << "Unknown variant: " << variant << '\n';
    }

    return EmbRet;
}

// NEW FUNCTION: Generate random pool of variants for user selection
std::vector<std::string> generateRandomVariantPool(int poolSize) {
    // Define the complete pool of variants
    const std::vector<std::string> allVariants = {
        // Regular Trills - Baroque and Classical - Short , Normal and Long
        "BTrRs1", "BTrRs5", "CTrRs1", "CTrRs5",
        "BTrRn1", "BTrRn5", "CTrRn1", "CTrRn5",
        "BTrRl1", "BTrRl5", "CTrRl1", "CTrRl5",

        "BTrDen1", "BTrDen5", "CTrDen1", "CTrDen5",
        "BTrDel1", "BTrDel5", "CTrDel1", "CTrDel5",

        "BTrAs1 ", "BTrAs5 ", "CTrAs1 ", "CTrAs5 ",
        "BTrAn1", "BTrAn5", "CTrAn1", "CTrAn5",
        "BTrAl1", "BTrAl5", "CTrAl1", "CTrAl5",

        "BTrDs1 ", "BTrDs5 ", "CTrDs1 ", "CTrDs5 ",
        "BTrDn1", "BTrDn5", "CTrDn1", "CTrDn5",
        "BTrDl1", "BTrDl5", "CTrDl1", "CTrDl5",

        "BTrTs1 ", "BTrTs5 ", "CTrTs1 ", "CTrTs5 ",
        "BTrTn1", "BTrTn5", "CTrTn1", "CTrTn5",
        "BTrTl1", "BTrTl5", "CTrTl1", "CTrTl5"
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

// NEW FUNCTION: Parse user input for multiple choice selection
std::vector<int> parseMultipleChoiceInput(const std::string& input, int maxChoice) {
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

    // Remove duplicates while preserving order
    std::vector<int> uniqueChoices;
    std::set<int> seen;
    for (int choice : choices) {
        if (seen.find(choice) == seen.end()) {
            uniqueChoices.push_back(choice);
            seen.insert(choice);
        }
    }

    return uniqueChoices;
}

// ENHANCED FUNCTION: Let user choose multiple variants from a pool
std::vector<std::string> getUserMultipleVariantChoice() {
    std::cout << "\n=== Trill Variant Selection ===" << std::endl;
    std::cout << "Choose your variant selection method:" << std::endl;
    std::cout << "1. Random selection (original behavior)" << std::endl;
    std::cout << "2. Choose from a random pool of variants" << std::endl;
    std::cout << "3. Select multiple variants from a random pool" << std::endl;
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
        std::cout << std::setw(2) << (i + 1) << ". " << variantPool[i] << std::endl;
    }

    if (choice == 2) {
        // Single choice selection (original behavior)
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
        // Multiple choice selection (NEW FEATURE)
        std::cout << "\nSelect from any number (separated by spaces) from 1 to " << variantPool.size()
                  << " of the variants you would like to randomly apply:" << std::endl;
        std::cout << "Example: '1 3 5 7' to select variants 1, 3, 5, and 7" << std::endl;
        std::cout << "Your selection: ";

        std::cin.ignore(); // Clear the input buffer
        std::string userInput;
        std::getline(std::cin, userInput);

        std::vector<int> selectedIndices = parseMultipleChoiceInput(userInput, variantPool.size());

        if (selectedIndices.empty()) {
            std::cout << "No valid selections made. Using random selection instead." << std::endl;
            return {"RANDOM"};
        }

        // Convert indices to variant names
        std::vector<std::string> selectedVariants;
        std::cout << "\nYou selected the following variants:" << std::endl;
        for (int index : selectedIndices) {
            std::string variant = variantPool[index - 1];
            selectedVariants.push_back(variant);
            std::cout << "- " << variant << std::endl;
        }

        std::cout << "\nThese " << selectedVariants.size() << " variants will be randomly applied during transformation." << std::endl;
        std::cout << "======================================\n" << std::endl;

        return selectedVariants;
    }
}

// ENHANCED applyVariant function with multiple choice support
std::vector<std::pair<int, int>> applyVariant(int pi, int durPi, std::string& selectedVariant, const std::vector<std::string>& userChoices = {}) {
    if (userChoices.empty() || (userChoices.size() == 1 && userChoices[0] == "RANDOM")) {
        // Original random selection behavior - use Trill variants
        const std::vector<std::string> variants = {
            // Regular Trills - Baroque and Classical - Short , Normal and Long
            "BTrRs1", "BTrRs5", "CTrRs1", "CTrRs5",
            "BTrRn1", "BTrRn5", "CTrRn1", "CTrRn5",
            "BTrRl1", "BTrRl5", "CTrRl1", "CTrRl5",

            "BTrDen1", "BTrDen5", "CTrDen1", "CTrDen5",
            "BTrDel1", "BTrDel5", "CTrDel1", "CTrDel5",

            "BTrAs1 ", "BTrAs5 ", "CTrAs1 ", "CTrAs5 ",
            "BTrAn1", "BTrAn5", "CTrAn1", "CTrAn5",
            "BTrAl1", "BTrAl5", "CTrAl1", "CTrAl5",

            "BTrDs1 ", "BTrDs5 ", "CTrDs1 ", "CTrDs5 ",
            "BTrDn1", "BTrDn5", "CTrDn1", "CTrDn5",
            "BTrDl1", "BTrDl5", "CTrDl1", "CTrDl5",

            "BTrTs1 ", "BTrTs5 ", "CTrTs1 ", "CTrTs5 ",
            "BTrTn1", "BTrTn5", "CTrTn1", "CTrTn5",
            "BTrTl1", "BTrTl5", "CTrTl1", "CTrTl5"
        };

        // Randomly select a trill variant
        selectedVariant = variants[rand() % variants.size()];

        // Use the comprehensive trill implementation with DUPLE meter by default
        return applyTrill(pi, durPi, DUPLE, selectedVariant);
    } else {
        // Use one of the user's selected variants randomly
        selectedVariant = userChoices[rand() % userChoices.size()];

        // Use the comprehensive trill implementation with DUPLE meter by default
        return applyTrill(pi, durPi, DUPLE, selectedVariant);
    }
}

// Simple Trill Variant Pool - ORIGINAL FUNCTION NAME PRESERVED
std::vector<std::pair<int, int>> applyTrillVariant(int pi, int durPi, std::string& selectedVariant) {
    // Define the pool of variants - expanded to include all options from the implementation
    const std::vector<std::string> variants = {
        // Regular Trills - Baroque and Classical - Short , Normal and Long
        "BTrRs1", "BTrRs5", "CTrRs1", "CTrRs5",
        "BTrRn1", "BTrRn5", "CTrRn1", "CTrRn5",
        "BTrRl1", "BTrRl5", "CTrRl1", "CTrRl5",

        "BTrDen1", "BTrDen5", "CTrDen1", "CTrDen5",
        "BTrDel1", "BTrDel5", "CTrDel1", "CTrDel5",

        "BTrAs1 ", "BTrAs5 ", "CTrAs1 ", "CTrAs5 ",
        "BTrAn1", "BTrAn5", "CTrAn1", "CTrAn5",
        "BTrAl1", "BTrAl5", "CTrAl1", "CTrAl5",

        "BTrDs1 ", "BTrDs5 ", "CTrDs1 ", "CTrDs5 ",
        "BTrDn1", "BTrDn5", "CTrDn1", "CTrDn5",
        "BTrDl1", "BTrDl5", "CTrDl1", "CTrDl5",

        "BTrTs1 ", "BTrTs5 ", "CTrTs1 ", "CTrTs5 ",
        "BTrTn1", "BTrTn5", "CTrTn1", "CTrTn5",
        "BTrTl1", "BTrTl5", "CTrTl1", "CTrTl5"
    };
    // Randomly select a variant
    selectedVariant = variants[rand() % variants.size()];

    // Use the comprehensive implementation with DUPLE meter by default
    return applyTrill(pi, durPi, DUPLE, selectedVariant);
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
    std::cout << "\n=== Trill Transformation Settings ===" << std::endl;
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

// ENHANCED processFile function with percentage control and multiple variant choice
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
    std::vector<std::string> userVariantChoices = getUserMultipleVariantChoice();

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

        // Check if this label is eligible for transformation
        if (label == "RLN" || label == "CS" || label == "I3" || label == "I8" ||
            label == "U2R" || label == "BM" || label == "SPU" || label == "SPD" ||
            label == "CH" || label == "CW" || label == "CD" || label == "HT" ||
            label == "FM" || label == "RN" || label == "LAD" || label == "DN" ||
            label == "DNW" || label == "SN" || label == "LNSN" || label == "SAN" ||
            label == "SMP" || label == "DLP3") {

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
        std::cout << "Random trill variants used for transformations" << std::endl;
    } else {
        std::cout << "Selected variants pool size: " << userVariantChoices.size() << std::endl;
        std::cout << "Variant usage breakdown:" << std::endl;
        for (const auto& [variant, count] : variantUsageCount) {
            double percentage = transformedNotes > 0 ?
                (static_cast<double>(count) / transformedNotes) * 100.0 : 0.0;
            std::cout << "  " << variant << ": " << count << " times ("
                      << std::fixed << std::setprecision(1) << percentage << "%)" << std::endl;
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
    std::map<int, int> trackTimes; // Track the current time for each track

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

            // Get the current time for this track (initialize to 0 if new track)
            int& currentTime = trackTimes[track];

            // Create note-on event
            MidiEvent noteOn{track, noteNumber, currentTime, duration, true};
            trackEvents[track].push_back(noteOn);

            // Create note-off event
            MidiEvent noteOff{track, noteNumber, currentTime + duration, 0, false};
            trackEvents[track].push_back(noteOff);

            // Update the current time for this track
            currentTime += duration;

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
    std::cout << "========= The Trill Transformation Tool ===============" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "====This transformation tool takes a note==============" << std::endl;
    std::cout << "====that is appropriately labeled and applies==========" << std::endl;
    std::cout << "==========a trill to that note.=========================" << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "=== Use with labels: RLN, CS, I3, I8, U2R =============" << std::endl;
    std::cout << "=== BM, SPU, SPD, CH, CW, CD, HT, BM, SPU, SPD, =======" << std::endl;
    std::cout << "=== BM, SPU, SPD, CH, CW, CD, HT, FM, RN, LAD,  =======" << std::endl;
    std::cout << "=== DN, DNW, SN, LNSN, SAN, SMP, DLP3 =================" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "======== with % Transformation Selection ===============" << std::endl;
    std::cout << "======== Midi File Output and Text Output ==============" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Enter the path to the input file: ";
    std::cout << std::endl;  std::getline(std::cin, inputFile);

    // Remove quotes if the user included them
    if (inputFile.size() >= 2 &&
        ((inputFile.front() == '"' && inputFile.back() == '"') ||
         (inputFile.front() == '\'' && inputFile.back() == '\''))) {
        inputFile = inputFile.substr(1, inputFile.size() - 2);
         }

    // Generate unique output filenames with timestamp
    std::cout << "========================================================" << std::endl;
    std::cout << std::endl;
    std::string outputFile = "Trill_Transformation_Multiple_User_Choice_" + std::string(timestamp) + ".txt";
    std::cout << std::endl;
    std::string midiOutputFile = "Trill_Transformation_Multiple_User_Choice_" + std::string(timestamp) + ".mid";
    std::cout << "========================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Processing file: " << inputFile << std::endl;
    std::cout << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Output will be written to: " << outputFile << std::endl;
    std::cout << std::endl;
    std::cout << "MIDI output will be written to: " << midiOutputFile << std::endl;
    std::cout << "========================================================" << std::endl;

    // Process the file (now with percentage control and multiple variant choice)
    processFile(inputFile, outputFile);

    // Convert the processed file to MIDI
    convertToMidi(outputFile, midiOutputFile);

    std::cout << "Processing complete!" << std::endl;
    std::cout << "Press Enter to exit...";
    std::cin.get();

    return 0;
}