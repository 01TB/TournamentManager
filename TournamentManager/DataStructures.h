#pragma once // Prevents the header from being included multiple times

// Include standard libraries if you plan to mix later (optional for current structure)
// #include <string>
// #include <vector>
// #include <algorithm>

// Include necessary .NET namespaces for C++/CLI types used below
using namespace System;
using namespace System::Collections::Generic; // For List<> if you ever use it here
using namespace System::Diagnostics; // For Debug::WriteLine (optional debugging)

// Structure to hold player data
public ref struct Player {
    String^ id;         // Player ID (e.g., "A1", "B3")
    double timeInSeconds; // Store time as total seconds for easy comparison (e.g., 1:57.97 -> 117.97)
    String^ formattedTime; // Store the original formatted time for display (e.g., "01:57.97")

    // Constructor (optional but helpful)
    Player(String^ _id) : id(_id), timeInSeconds(Double::MaxValue), formattedTime("N/A") {} // Initialize with invalid time

    // Static comparison function for sorting List<Player^> (ascending time)
    static int ComparePlayersByTime(Player^ p1, Player^ p2) {
        // Handle cases where one or both players might not have a valid time yet
        if (p1->timeInSeconds == Double::MaxValue && p2->timeInSeconds == Double::MaxValue) {
            return 0; // Consider them equal if both invalid
        }
        else if (p1->timeInSeconds == Double::MaxValue) {
            return 1; // Invalid time is considered "greater" (comes last)
        }
        else if (p2->timeInSeconds == Double::MaxValue) {
            return -1; // Invalid time is considered "greater" (comes last)
        }
        else {
            // Both times are valid, compare normally
            return p1->timeInSeconds.CompareTo(p2->timeInSeconds);
        }
    }
};

// --- Helper Functions ---

// Helper function to try parsing MM:SS.mmm string to total seconds
// Returns true if successful, false otherwise. Sets outSeconds via reference.
static bool TryParseTime(String^ timeStr, double% outSeconds) {
    outSeconds = Double::MaxValue; // Default to invalid time
    if (String::IsNullOrWhiteSpace(timeStr)) return false;

    // Use CultureInfo::InvariantCulture to handle '.' as decimal separator reliably
    System::Globalization::CultureInfo^ culture = System::Globalization::CultureInfo::InvariantCulture;

    array<String^>^ parts = timeStr->Split(':');
    if (parts->Length != 2) {
        // Debug::WriteLine("Time parse failed: Incorrect number of ':' parts.");
        return false;
    }

    int minutes;
    if (!Int32::TryParse(parts[0], minutes) || minutes < 0) {
        // Debug::WriteLine("Time parse failed: Cannot parse minutes or minutes < 0.");
        return false;
    }

    // Now parse seconds and milliseconds (expecting "SS.mmm")
    double secondsWithMs;
    if (!Double::TryParse(parts[1], System::Globalization::NumberStyles::AllowDecimalPoint, culture, secondsWithMs) || secondsWithMs < 0.0 || secondsWithMs >= 60.0) {
        // Debug::WriteLine(String::Format("Time parse failed: Cannot parse seconds+ms part '{0}' or value out of range [0, 60).", parts[1]));
        return false;
    }

    // Validation passed, calculate total seconds
    outSeconds = (minutes * 60.0) + secondsWithMs;
    // Debug::WriteLine(String::Format("Time parse success: '{0}' -> {1} seconds.", timeStr, outSeconds));
    return true;
}


// Helper function to format total seconds back to MM:SS.mmm string
// Ensures 3 digits for milliseconds.
static String^ FormatTime(double totalSeconds) {
    if (totalSeconds == Double::MaxValue || totalSeconds < 0) {
        return "N/A"; // Not Available or Invalid
    }

    // Use TimeSpan for potentially easier/more robust time math
    TimeSpan ts = TimeSpan::FromSeconds(totalSeconds);

    // Extract parts
    int minutes = ts.Minutes + (ts.Hours * 60) + (ts.Days * 24 * 60); // Total minutes
    int seconds = ts.Seconds;
    int milliseconds = ts.Milliseconds;

    // Format with leading zeros: D2 for 2 digits, D3 for 3 digits
    return String::Format("{0:D2}:{1:D2}.{2:D3}", minutes, seconds, milliseconds);
}