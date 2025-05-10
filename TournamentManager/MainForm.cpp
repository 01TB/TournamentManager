// --- START OF FILE MainForm.cpp ---

// 1. Include YOUR project header FIRST
#include "MainForm.h"

// 2. Define Windows lean includes BEFORE including windows.h (Optional but Recommended)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX // Prevents windows.h from defining min() and max() macros

// 3. Include standard / Windows / other library headers AFTER your project header
#include <windows.h> // Only if you actually need AllocConsole or other WinAPI
#include <iostream>  // Only if using std::cout/cerr (Console::WriteLine is preferred for C++/CLI)
// #include <limits> // Not strictly needed if using Double::MaxValue
// #include <fstream> // NOT needed for System::IO::StreamWriter

// 4. Add necessary using namespaces AFTER includes
using namespace TournamentManager; // Your project's namespace
using namespace System;
using namespace System::Windows::Forms;
using namespace System::IO;          // For StreamWriter and SaveFileDialog
using namespace System::Collections::Generic; // For List/Tuple

// --- Constructor Implementation ---
MainForm::MainForm(void) {
    InitializeComponent(); // This MUST be called first
    InitializeTournament(); // Call your setup method
}

// --- Destructor Implementation ---
MainForm::~MainForm() {
    if (components) {
        delete components;
    }
}


// --- Method Implementations ---

void MainForm::InitializeTournament() {
    // Initialize all data structures
    eliminationGroups = gcnew List<List<Player^>^>();
    for (int i = 0; i < 4; ++i) {
        eliminationGroups->Add(gcnew List<Player^>());
    }
    quarterFinalQualifiers = gcnew List<Player^>();
    quarterFinalMatches = gcnew List<Tuple<Player^, Player^>^>();
    quarterFinalWinners = gcnew List<Player^>();
    quarterFinalLosers = gcnew List<Player^>();
    semiFinalMatches = gcnew List<Tuple<Player^, Player^>^>();
    semiFinalWinners = gcnew List<Player^>();
    semiFinalLosers = gcnew List<Player^>();
    // finalMatch, winner, runnerUp initialized later

    minimumTimeThresholdSeconds = -1.0; // Indicate not set yet
    disqualifiedPlayers = gcnew List<Player^>(); // Initialize empty list

    // Initialize logs
    eliminationResultsLog = gcnew List<String^>();
    quarterFinalResultsLog = gcnew List<String^>();
    semiFinalResultsLog = gcnew List<String^>();
    finalRankingLog = gcnew List<String^>();
    finalResultLog = "";

    currentPhase = 0; // Start with Elimination
    SetupEliminationPhaseUI();
}


void MainForm::SetupEliminationPhaseUI() {
    // Optional: Setup console if needed for debugging output
    // AllocConsole();
    // freopen("CONOUT$", "w", stdout);

    pnlPhaseContent->Controls->Clear();
    lblPhaseTitle->Text = "Insertion ELIMINATION";

    // Reset state for this phase
    for each(List<Player^> ^ group in eliminationGroups) group->Clear();
    eliminationResultsLog->Clear();
    disqualifiedPlayers->Clear(); // Clear DQ list from previous attempts
    minimumTimeThresholdSeconds = -1.0; // Reset min time

    int startX = 20; int currentY = 10; int rowHeight = 30;
    int verticalSpacing = 5; int groupSpacing = 15;

    // --- ADD CONTROLS FOR MINIMUM TIME ---
    Label^ minTimeLabel = gcnew Label();
    minTimeLabel->Text = "Temps minimum DQ :";
    minTimeLabel->Location = System::Drawing::Point(startX, currentY);
    minTimeLabel->AutoSize = true;
    pnlPhaseContent->Controls->Add(minTimeLabel);

    TextBox^ minTimeTextBox = gcnew TextBox();
    minTimeTextBox->Name = "txtMinTime";
    minTimeTextBox->Location = System::Drawing::Point(minTimeLabel->Right + 5, currentY - 3);
    minTimeTextBox->Width = 80;
    ToolTip^ minTimeToolTip = gcnew ToolTip();
    minTimeToolTip->SetToolTip(minTimeTextBox, "Enter minimum qualifying time (MM:SS.mmm). Times >= this are disqualified.");
    pnlPhaseContent->Controls->Add(minTimeTextBox);

    Button^ setMinTimeButton = gcnew Button();
    setMinTimeButton->Name = "btnSetMinTime";
    setMinTimeButton->Text = "Set";
    setMinTimeButton->Width = 60;
    setMinTimeButton->Location = System::Drawing::Point(minTimeTextBox->Right + 10, currentY - 5);
    setMinTimeButton->Click += gcnew System::EventHandler(this, &MainForm::SetMinimumTime); // Assign new handler
    pnlPhaseContent->Controls->Add(setMinTimeButton);

    currentY += rowHeight + groupSpacing; // Move down for groups

    // --- PLAYER INPUT ROWS (Loop as before) ---
    char groupCharAscii = 'A'; // Use for calculation, but display might use wide chars
    for (int i = 0; i < 4; ++i) {
        Label^ groupLabel = gcnew Label();
        groupLabel->Text = "Groupe " + System::Char(groupCharAscii + i).ToString(); // Use System::Char for display
        groupLabel->Location = System::Drawing::Point(10, currentY);
        groupLabel->AutoSize = true;
        groupLabel->Font = gcnew System::Drawing::Font(this->Font->FontFamily, 10, FontStyle::Bold);
        pnlPhaseContent->Controls->Add(groupLabel);
        currentY += groupLabel->Height + verticalSpacing;

        List<Player^>^ currentGroupPlayers = eliminationGroups[i];
        for (int j = 1; j <= 4; ++j) {
            String^ playerID = String::Format("{0}{1}", System::Char(groupCharAscii + i), j); // Use Format
            Player^ player = gcnew Player(playerID);
            currentGroupPlayers->Add(player);

            Panel^ playerPanel = gcnew Panel();
            playerPanel->Name = "pnl" + playerID;
            playerPanel->Width = pnlPhaseContent->Width - startX - 10;
            playerPanel->Height = rowHeight;
            playerPanel->Location = System::Drawing::Point(startX, currentY);

            Label^ playerLabel = gcnew Label();
            playerLabel->Text = playerID + " :";
            playerLabel->Location = System::Drawing::Point(0, 5);
            playerLabel->AutoSize = true;
            playerPanel->Controls->Add(playerLabel);

            TextBox^ timeTextBox = gcnew TextBox();
            timeTextBox->Name = "txtTime" + playerID;
            timeTextBox->Tag = player;
            timeTextBox->Location = System::Drawing::Point(playerLabel->Right + 5, 2);
            timeTextBox->Width = 80;
            ToolTip^ timeToolTip = gcnew ToolTip();
            timeToolTip->SetToolTip(timeTextBox, "Enter time as MM:SS.mmm");
            playerPanel->Controls->Add(timeTextBox);

            Button^ validateButton = gcnew Button();
            validateButton->Name = "btnValidate" + playerID;
            validateButton->Tag = playerPanel;
            validateButton->Text = "Validate";
            validateButton->Width = 60;
            validateButton->Location = System::Drawing::Point(timeTextBox->Right + 10, 0);
            validateButton->Click += gcnew System::EventHandler(this, &MainForm::ValidateEliminationTime);
            playerPanel->Controls->Add(validateButton);

            pnlPhaseContent->Controls->Add(playerPanel);

            // *** Initially DISABLE ALL PLAYER rows ***
            playerPanel->Enabled = false; // Will be enabled after min time is set

            currentY += playerPanel->Height + verticalSpacing;
        }
        currentY += groupSpacing;
    }

    // --- Process Results Button (as before, initially disabled) ---
    Button^ processButton = gcnew Button();
    processButton->Name = "btnProcessElimination";
    processButton->Text = "Process Elimination Results";
    processButton->AutoSize = true;
    processButton->Location = System::Drawing::Point((pnlPhaseContent->Width - processButton->Width) / 2, currentY + 10);
    processButton->Click += gcnew System::EventHandler(this, &MainForm::ProcessEliminationResults);
    processButton->Enabled = false;
    pnlPhaseContent->Controls->Add(processButton);

    // Set focus to the minimum time input initially
    minTimeTextBox->Focus();
}


void MainForm::SetMinimumTime(System::Object^ sender, System::EventArgs^ e) {
    Control^ minTimeTextBoxControl = pnlPhaseContent->Controls["txtMinTime"];
    Control^ setMinTimeButtonControl = pnlPhaseContent->Controls["btnSetMinTime"];
    TextBox^ minTimeTextBox = dynamic_cast<TextBox^>(minTimeTextBoxControl);
    Button^ setMinTimeButton = dynamic_cast<Button^>(setMinTimeButtonControl);

    if (minTimeTextBox == nullptr || setMinTimeButton == nullptr) { /* Error handling */ return; }

    double timeSeconds;
    if (TryParseTime(minTimeTextBox->Text->Trim(), timeSeconds) && timeSeconds >= 0) {
        minimumTimeThresholdSeconds = timeSeconds;
        String^ formattedMinTime = FormatTime(minimumTimeThresholdSeconds);
        MessageBox::Show("Temps minimum set to: " + formattedMinTime, "Minimum Time Set", MessageBoxButtons::OK, MessageBoxIcon::Information);

        minTimeTextBox->Enabled = false;
        setMinTimeButton->Enabled = false;

        // Enable the A1 panel
        Control^ firstPlayerPanelControl = pnlPhaseContent->Controls["pnlA1"];
        if (firstPlayerPanelControl != nullptr) {
            firstPlayerPanelControl->Enabled = true;
            for each(Control ^ ctrl in firstPlayerPanelControl->Controls) {
                TextBox^ firstTextBox = dynamic_cast<TextBox^>(ctrl);
                if (firstTextBox != nullptr) { firstTextBox->Focus(); break; }
            }
        }
        else { /* Error handling */ }
    }
    else {
        MessageBox::Show("Invalid time format for Temps minimum.\nPlease use MM:SS.mmm and ensure it's not negative.", "Input Error", MessageBoxButtons::OK, MessageBoxIcon::Warning);
        minimumTimeThresholdSeconds = -1.0;
        minTimeTextBox->Focus(); minTimeTextBox->SelectAll();
    }
}


String^ MainForm::FindNextPlayerID(String^ currentID) {
    if (String::IsNullOrEmpty(currentID) || currentID->Length < 2) {
        return nullptr;
    }
    try {
        // Use System::Char directly for safety
        System::Char groupChar = currentID[0];
        int playerNum = Int32::Parse(currentID->Substring(1));

        if (playerNum < 4) {
            return String::Format("{0}{1}", groupChar, playerNum + 1);
        }
        else if (groupChar < L'D') { // Use wide char literal L'D'
            return String::Format("{0}1", (wchar_t)(groupChar + 1));
        }
        else {
            return nullptr; // D4
        }
    }
    catch (FormatException^ ex) {
        Console::WriteLine("Error parsing player ID: " + ex->Message); return nullptr;
    }
    catch (OverflowException^ ex) {
        Console::WriteLine("Error calculating next group char: " + ex->Message); return nullptr;
    }
}


void MainForm::ValidateEliminationTime(System::Object^ sender, System::EventArgs^ e) {
    Button^ clickedButton = dynamic_cast<Button^>(sender);
    Panel^ playerPanel = dynamic_cast<Panel^>(clickedButton->Tag);
    if (!playerPanel) { /* Error handling */ return; }

    TextBox^ timeTextBox = nullptr; Player^ currentPlayer = nullptr;
    for each(Control ^ ctrl in playerPanel->Controls) {
        timeTextBox = dynamic_cast<TextBox^>(ctrl);
        if (timeTextBox) { currentPlayer = dynamic_cast<Player^>(timeTextBox->Tag); break; }
    }
    if (!timeTextBox || !currentPlayer) { /* Error handling */ return; }

    double timeSeconds;
    if (TryParseTime(timeTextBox->Text->Trim(), timeSeconds)) {
        currentPlayer->timeInSeconds = timeSeconds;
        currentPlayer->formattedTime = FormatTime(timeSeconds);

        // Disqualification Check
        bool justDisqualified = false;
        if (minimumTimeThresholdSeconds >= 0 && timeSeconds >= minimumTimeThresholdSeconds) {
            currentPlayer->isDisqualified = true;
            disqualifiedPlayers->Add(currentPlayer);
            justDisqualified = true;
            Console::WriteLine("Player " + currentPlayer->id + " DISQUALIFIED (Time: " + currentPlayer->formattedTime + " >= Min: " + FormatTime(minimumTimeThresholdSeconds) + ")");
        }
        else {
            currentPlayer->isDisqualified = false;
        }

        playerPanel->Visible = false; // Hide row

        String^ nextPlayerID = FindNextPlayerID(currentPlayer->id);
        if (nextPlayerID != nullptr) {
            Control^ nextPanelControl = pnlPhaseContent->Controls["pnl" + nextPlayerID];
            if (nextPanelControl) {
                nextPanelControl->Enabled = true;
                for each(Control ^ ctrl in nextPanelControl->Controls) {
                    TextBox^ nextTextBox = dynamic_cast<TextBox^>(ctrl);
                    if (nextTextBox) { nextTextBox->Focus(); break; }
                }
            }
            else { /* Error handling: Panel not found */ }
        }
        else { // Last player (D4)
            Control^ processButtonControl = pnlPhaseContent->Controls["btnProcessElimination"];
            Button^ processButton = dynamic_cast<Button^>(processButtonControl);
            if (processButton) {
                processButton->Enabled = true; processButton->Focus();
                MessageBox::Show("All elimination times entered. Click 'Process Elimination Results'.", "Input Complete", MessageBoxButtons::OK, MessageBoxIcon::Information);
            }
            else { /* Error handling: Button not found */ }
        }
    }
    else {
        MessageBox::Show("Invalid time format for " + currentPlayer->id + ".\nPlease use MM:SS.mmm", "Input Error", MessageBoxButtons::OK, MessageBoxIcon::Warning);
        timeTextBox->Focus(); timeTextBox->SelectAll();
    }
}


void MainForm::ProcessEliminationResults(System::Object^ sender, System::EventArgs^ e) {
    Button^ processButton = dynamic_cast<Button^>(sender);
    if (processButton) processButton->Enabled = false;

    if (minimumTimeThresholdSeconds < 0) { // Check if min time was set
        MessageBox::Show("Error: Minimum time was not set. Cannot process.", "Processing Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
        if (processButton) processButton->Enabled = true; return;
    }

    Console::WriteLine("\n--- Elimination Phase Results ---");
    Console::WriteLine("Minimum Time for Qualification: " + FormatTime(minimumTimeThresholdSeconds));
    eliminationResultsLog->Clear();
    eliminationResultsLog->Add("--- Elimination Phase Results ---");
    eliminationResultsLog->Add("Minimum Time for Qualification: " + FormatTime(minimumTimeThresholdSeconds));
    quarterFinalQualifiers->Clear();
    char groupCharAscii = 'A'; bool dataComplete = true;

    for each(List<Player^> ^ groupPlayers in eliminationGroups) {
        for each(Player ^ p in groupPlayers) { // Basic check if time was entered
            if (p->timeInSeconds == Double::MaxValue) {
                MessageBox::Show("Error: Player " + p->id + " missing time.", "Processing Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
                if (processButton) processButton->Enabled = true; dataComplete = false; break;
            }
        }
        if (!dataComplete) break;

        groupPlayers->Sort(gcnew Comparison<Player^>(Player::ComparePlayersByTime));
        String^ groupLogHeader = "Groupe " + System::Char(groupCharAscii).ToString() + " Ranking:";
        Console::WriteLine(groupLogHeader); eliminationResultsLog->Add(groupLogHeader);

        int qualifiersFoundInGroup = 0;
        for (int i = 0; i < groupPlayers->Count; ++i) {
            Player^ currentPlayer = groupPlayers[i];
            String^ rankSuffix = (i == 0) ? "er" : "e";
            String^ rankStr = (i + 1).ToString() + rankSuffix;
            String^ displayTime = currentPlayer->formattedTime;
            if (String::IsNullOrEmpty(displayTime) || displayTime == "N/A") displayTime = FormatTime(currentPlayer->timeInSeconds);
            String^ dqIndicator = currentPlayer->isDisqualified ? " (DQ)" : "";
            String^ playerLog = String::Format("  {0} ({1}) - {2}{3}", currentPlayer->id, rankStr, displayTime, dqIndicator);
            Console::WriteLine(playerLog); eliminationResultsLog->Add(playerLog);

            if (!currentPlayer->isDisqualified && qualifiersFoundInGroup < 2) {
                quarterFinalQualifiers->Add(currentPlayer); qualifiersFoundInGroup++;
                String^ qualMsg = "   -> Qualifies for Quarterfinals";
                Console::WriteLine(qualMsg); eliminationResultsLog->Add(qualMsg);
            }
        }
        Console::WriteLine(""); eliminationResultsLog->Add("");
        groupCharAscii++;
    }

    if (dataComplete) {
        if (quarterFinalQualifiers->Count != 8) {
            String^ warningMsg = "WARNING: Found " + quarterFinalQualifiers->Count + " qualifiers instead of 8 due to disqualifications.";
            Console::WriteLine(warningMsg); eliminationResultsLog->Add(warningMsg);
        }
        System::Windows::Forms::DialogResult proceedResult = MessageBox::Show("Elimination phase complete. Proceed to Quarterfinals?", "Phase Complete", MessageBoxButtons::YesNo, MessageBoxIcon::Question);
        if (proceedResult == System::Windows::Forms::DialogResult::Yes) {
            if (quarterFinalQualifiers->Count % 2 != 0 || quarterFinalQualifiers->Count < 2) {
                MessageBox::Show("Error: Cannot proceed to Quarterfinals with " + quarterFinalQualifiers->Count + " qualifiers. Need an even number >= 2.", "Qualification Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
                if (processButton) processButton->Enabled = true; // Allow reprocessing maybe?
            }
            else {
                currentPhase = 1; SetupQuarterFinalPhaseUI();
            }
        }
        else {
            MessageBox::Show("Quarterfinals setup cancelled.", "Action Cancelled", MessageBoxButtons::OK, MessageBoxIcon::Information);
            if (processButton) processButton->Enabled = true;
        }
    }
    else {
        if (processButton) processButton->Enabled = true;
    }
}


void MainForm::SetupQuarterFinalPhaseUI() {
    pnlPhaseContent->Controls->Clear();
    lblPhaseTitle->Text = "Insertion QUART DE FINALE (1/4)";
    quarterFinalMatches->Clear(); quarterFinalWinners->Clear(); quarterFinalLosers->Clear(); quarterFinalResultsLog->Clear();

    // Check for valid number of qualifiers (must be even, >= 2)
    if (quarterFinalQualifiers->Count % 2 != 0 || quarterFinalQualifiers->Count < 2) {
        MessageBox::Show("Error: Cannot setup Quarterfinals with " + quarterFinalQualifiers->Count + " qualifiers.", "Setup Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
        return; // Cannot proceed
    }
    if (quarterFinalQualifiers->Count == 0) { // Handle case where everyone was DQ'd
        MessageBox::Show("No players qualified for Quarterfinals.", "Tournament End", MessageBoxButtons::OK, MessageBoxIcon::Information);
        lblPhaseTitle->Text = "TOURNOI TERMINE (No Qualifiers)";
        // Maybe show DQ list here?
        CalculateAndDisplayFinalRanking(); // Will show only DQ list
        SaveResultsToFile();
        return;
    }


    // --- Define the Matches ---
    // Adjust logic if fewer than 8 qualifiers. Simple pairing for now.
    for (int i = 0; i < quarterFinalQualifiers->Count; i += 2) {
        quarterFinalMatches->Add(Tuple::Create(quarterFinalQualifiers[i], quarterFinalQualifiers[i + 1]));
    }


    int startX = 20; int currentY = 10; int rowHeight = 30; int verticalSpacing = 5; int matchSpacing = 15;

    for (int i = 0; i < quarterFinalMatches->Count; ++i) {
        Label^ matchLabel = gcnew Label(); matchLabel->Text = String::Format("Match {0}: {1} vs {2}", i + 1, quarterFinalMatches[i]->Item1->id, quarterFinalMatches[i]->Item2->id);
        /* Location/AutoSize/Font */ matchLabel->Location = System::Drawing::Point(startX - 10, currentY); matchLabel->AutoSize = true; matchLabel->Font = gcnew System::Drawing::Font(this->Font, FontStyle::Bold);
        pnlPhaseContent->Controls->Add(matchLabel); currentY += matchLabel->Height + verticalSpacing;

        array<Player^>^ playersInMatch = { quarterFinalMatches[i]->Item1, quarterFinalMatches[i]->Item2 };
        for each(Player ^ player in playersInMatch) {
            player->formattedTime = "N/A"; player->timeInSeconds = Double::MaxValue; // Reset time
            Panel^ playerPanel = gcnew Panel(); playerPanel->Name = "pnl" + player->id; /* Size/Location */
            playerPanel->Width = pnlPhaseContent->Width - startX - 10; playerPanel->Height = rowHeight; playerPanel->Location = System::Drawing::Point(startX, currentY);
            // Add Label, TextBox, ToolTip, Button
            Label^ playerLabel = gcnew Label(); playerLabel->Text = player->id + " :"; /*...*/ playerPanel->Controls->Add(playerLabel); playerLabel->Location = System::Drawing::Point(0, 5); playerLabel->AutoSize = true;
            TextBox^ timeTextBox = gcnew TextBox(); timeTextBox->Name = "txtTime" + player->id; timeTextBox->Tag = player; /*...*/ playerPanel->Controls->Add(timeTextBox); timeTextBox->Location = System::Drawing::Point(playerLabel->Right + 5, 2); timeTextBox->Width = 80;
            ToolTip^ timeToolTip = gcnew ToolTip(); timeToolTip->SetToolTip(timeTextBox, "Enter time as MM:SS.mmm");
            Button^ validateButton = gcnew Button(); validateButton->Name = "btnValidate" + player->id; validateButton->Tag = playerPanel; validateButton->Text = "Validate"; /*...*/ validateButton->Width = 60; validateButton->Location = System::Drawing::Point(timeTextBox->Right + 10, 0);
            validateButton->Click += gcnew System::EventHandler(this, &MainForm::ValidateMatchTime);
            playerPanel->Controls->Add(validateButton);

            pnlPhaseContent->Controls->Add(playerPanel);
            currentY += playerPanel->Height + verticalSpacing;
            playerPanel->Enabled = false;
        }
        currentY += matchSpacing;
    }

    Button^ processButton = gcnew Button(); processButton->Name = "btnProcessQF"; /* Text/Location/AutoSize */
    processButton->Text = "Process Quarterfinal Results"; processButton->AutoSize = true; processButton->Location = System::Drawing::Point((pnlPhaseContent->Width - processButton->Width) / 2, currentY + 10);
    processButton->Click += gcnew System::EventHandler(this, &MainForm::ProcessQuarterFinalResults); processButton->Enabled = false;
    pnlPhaseContent->Controls->Add(processButton);

    // Enable first player row if matches exist
    if (quarterFinalMatches->Count > 0) {
        String^ firstPlayerID = quarterFinalMatches[0]->Item1->id;
        Control^ firstPanelControl = pnlPhaseContent->Controls["pnl" + firstPlayerID];
        if (firstPanelControl) {
            firstPanelControl->Enabled = true;
            for each(Control ^ ctrl in firstPanelControl->Controls) {
                TextBox^ firstTextBox = dynamic_cast<TextBox^>(ctrl);
                if (firstTextBox) { firstTextBox->Focus(); break; }
            }
        }
    }
    else { // Should not happen if check at start is correct, but safety
        processButton->Enabled = true; // Allow immediate (empty) processing? Or hide?
    }
}

void MainForm::ProcessQuarterFinalResults(System::Object^ sender, System::EventArgs^ e) {
    Button^ processButton = dynamic_cast<Button^>(sender); if (processButton) processButton->Enabled = false;
    quarterFinalResultsLog->Clear(); quarterFinalWinners->Clear(); quarterFinalLosers->Clear(); bool dataComplete = true;

    for each(Tuple<Player^, Player^> ^ match in quarterFinalMatches) {
        if (match->Item1->timeInSeconds == Double::MaxValue || match->Item2->timeInSeconds == Double::MaxValue) {
            MessageBox::Show("Error: Not all QF players have times.", "Processing Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
            if (processButton) processButton->Enabled = true; dataComplete = false; break;
        }
    }
    if (!dataComplete) return;

    Console::WriteLine("\n--- Quarterfinal Phase Results ---"); quarterFinalResultsLog->Add("--- Quarterfinal Phase Results ---");

    for each(Tuple<Player^, Player^> ^ match in quarterFinalMatches) {
        Player^ p1 = match->Item1; Player^ p2 = match->Item2; Player^ winner; Player^ loser;
        if (p1->timeInSeconds < p2->timeInSeconds) { winner = p1; loser = p2; }
        else { winner = p2; loser = p1; }
        quarterFinalWinners->Add(winner); quarterFinalLosers->Add(loser);
        String^ matchLog = String::Format("{0} ({1}) vs {2} ({3}) -> Winner: {4}", p1->id, p1->formattedTime, p2->id, p2->formattedTime, winner->id);
        Console::WriteLine(matchLog); quarterFinalResultsLog->Add(matchLog);
    }
    Console::WriteLine(""); quarterFinalResultsLog->Add("");

    // Check if enough winners for next phase
    if (quarterFinalWinners->Count < 2) {
        MessageBox::Show("Not enough winners (" + quarterFinalWinners->Count + ") to proceed to Semifinals. Tournament ends.", "Tournament End", MessageBoxButtons::OK, MessageBoxIcon::Information);
        lblPhaseTitle->Text = "TOURNOI TERMINE";
        CalculateAndDisplayFinalRanking();
        SaveResultsToFile();
        return;
    }
    if (quarterFinalWinners->Count % 2 != 0) {
        MessageBox::Show("Error: Odd number of winners (" + quarterFinalWinners->Count + ") for Semifinals. Cannot proceed.", "Setup Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
        if (processButton) processButton->Enabled = true; // Allow user to potentially fix something? Unlikely.
        return;
    }

    System::Windows::Forms::DialogResult proceedResult = MessageBox::Show("Quarterfinals complete. Proceed to Semifinals?", "Phase Complete", MessageBoxButtons::YesNo, MessageBoxIcon::Question);
    if (proceedResult == System::Windows::Forms::DialogResult::Yes) {
        currentPhase = 2; SetupSemiFinalPhaseUI();
    }
    else { if (processButton) processButton->Enabled = true; }
}


void MainForm::SetupSemiFinalPhaseUI() {
    pnlPhaseContent->Controls->Clear(); lblPhaseTitle->Text = "Insertion DEMI FINALE (1/2)";
    semiFinalMatches->Clear(); semiFinalWinners->Clear(); semiFinalLosers->Clear(); semiFinalResultsLog->Clear();

    // Check prerequisite (must be even number >= 2)
    if (quarterFinalWinners->Count < 2 || quarterFinalWinners->Count % 2 != 0) {
        MessageBox::Show("Error: Cannot setup Semifinals with " + quarterFinalWinners->Count + " winners.", "Setup Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
        return;
    }

    // Define Matches (Simple pairing)
    for (int i = 0; i < quarterFinalWinners->Count; i += 2) {
        semiFinalMatches->Add(Tuple::Create(quarterFinalWinners[i], quarterFinalWinners[i + 1]));
    }

    int startX = 20; int currentY = 10; int rowHeight = 30; int verticalSpacing = 5; int matchSpacing = 15;

    for (int i = 0; i < semiFinalMatches->Count; ++i) {
        Label^ matchLabel = gcnew Label(); matchLabel->Text = String::Format("Match {0}: {1} vs {2}", i + 1, semiFinalMatches[i]->Item1->id, semiFinalMatches[i]->Item2->id);
        /* Location/AutoSize/Font */ matchLabel->Location = System::Drawing::Point(startX - 10, currentY); matchLabel->AutoSize = true; matchLabel->Font = gcnew System::Drawing::Font(this->Font, FontStyle::Bold);
        pnlPhaseContent->Controls->Add(matchLabel); currentY += matchLabel->Height + verticalSpacing;

        array<Player^>^ playersInMatch = { semiFinalMatches[i]->Item1, semiFinalMatches[i]->Item2 };
        for each(Player ^ player in playersInMatch) {
            player->formattedTime = "N/A"; player->timeInSeconds = Double::MaxValue; // Reset time
            Panel^ playerPanel = gcnew Panel(); playerPanel->Name = "pnl" + player->id; /* Size/Location */
            playerPanel->Width = pnlPhaseContent->Width - startX - 10; playerPanel->Height = rowHeight; playerPanel->Location = System::Drawing::Point(startX, currentY);
            // Add Label, TextBox, ToolTip, Button
            Label^ playerLabel = gcnew Label(); playerLabel->Text = player->id + " :"; /*...*/ playerPanel->Controls->Add(playerLabel); playerLabel->Location = System::Drawing::Point(0, 5); playerLabel->AutoSize = true;
            TextBox^ timeTextBox = gcnew TextBox(); timeTextBox->Name = "txtTime" + player->id; timeTextBox->Tag = player; /*...*/ playerPanel->Controls->Add(timeTextBox); timeTextBox->Location = System::Drawing::Point(playerLabel->Right + 5, 2); timeTextBox->Width = 80;
            ToolTip^ timeToolTip = gcnew ToolTip(); timeToolTip->SetToolTip(timeTextBox, "Enter time as MM:SS.mmm");
            Button^ validateButton = gcnew Button(); validateButton->Name = "btnValidate" + player->id; validateButton->Tag = playerPanel; validateButton->Text = "Validate"; /*...*/ validateButton->Width = 60; validateButton->Location = System::Drawing::Point(timeTextBox->Right + 10, 0);
            validateButton->Click += gcnew System::EventHandler(this, &MainForm::ValidateMatchTime);
            playerPanel->Controls->Add(validateButton);

            pnlPhaseContent->Controls->Add(playerPanel);
            currentY += playerPanel->Height + verticalSpacing;
            playerPanel->Enabled = false;
        }
        currentY += matchSpacing;
    }

    Button^ processButton = gcnew Button(); processButton->Name = "btnProcessSF"; /* Text/Location/AutoSize */
    processButton->Text = "Process Semifinal Results"; processButton->AutoSize = true; processButton->Location = System::Drawing::Point((pnlPhaseContent->Width - processButton->Width) / 2, currentY + 10);
    processButton->Click += gcnew System::EventHandler(this, &MainForm::ProcessSemiFinalResults); processButton->Enabled = false;
    pnlPhaseContent->Controls->Add(processButton);

    // Enable first player
    if (semiFinalMatches->Count > 0) {
        String^ firstPlayerID = semiFinalMatches[0]->Item1->id;
        Control^ firstPanelControl = pnlPhaseContent->Controls["pnl" + firstPlayerID];
        if (firstPanelControl) {
            firstPanelControl->Enabled = true;
            for each(Control ^ ctrl in firstPanelControl->Controls) {
                TextBox^ firstTextBox = dynamic_cast<TextBox^>(ctrl);
                if (firstTextBox) { firstTextBox->Focus(); break; }
            }
        }
    }
    else {
        processButton->Enabled = true; // Should not happen if checks are correct
    }
}


void MainForm::ProcessSemiFinalResults(System::Object^ sender, System::EventArgs^ e) {
    Button^ processButton = dynamic_cast<Button^>(sender); if (processButton) processButton->Enabled = false;
    semiFinalResultsLog->Clear(); semiFinalWinners->Clear(); semiFinalLosers->Clear(); bool dataComplete = true;

    for each(Tuple<Player^, Player^> ^ match in semiFinalMatches) {
        if (match->Item1->timeInSeconds == Double::MaxValue || match->Item2->timeInSeconds == Double::MaxValue) {
            MessageBox::Show("Error: Not all SF players have times.", "Processing Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
            if (processButton) processButton->Enabled = true; dataComplete = false; break;
        }
    }
    if (!dataComplete) return;

    Console::WriteLine("\n--- Semifinal Phase Results ---"); semiFinalResultsLog->Add("--- Semifinal Phase Results ---");

    for each(Tuple<Player^, Player^> ^ match in semiFinalMatches) {
        Player^ p1 = match->Item1; Player^ p2 = match->Item2; Player^ winner; Player^ loser;
        if (p1->timeInSeconds < p2->timeInSeconds) { winner = p1; loser = p2; }
        else { winner = p2; loser = p1; }
        semiFinalWinners->Add(winner); semiFinalLosers->Add(loser);
        String^ matchLog = String::Format("{0} ({1}) vs {2} ({3}) -> Winner: {4}", p1->id, p1->formattedTime, p2->id, p2->formattedTime, winner->id);
        Console::WriteLine(matchLog); semiFinalResultsLog->Add(matchLog);
    }
    Console::WriteLine(""); semiFinalResultsLog->Add("");

    // Check if exactly 2 winners for Final
    if (semiFinalWinners->Count != 2) {
        MessageBox::Show("Incorrect number of winners (" + semiFinalWinners->Count + ") for Final. Tournament ends.", "Tournament End", MessageBoxButtons::OK, MessageBoxIcon::Information);
        lblPhaseTitle->Text = "TOURNOI TERMINE";
        CalculateAndDisplayFinalRanking();
        SaveResultsToFile();
        return;
    }


    System::Windows::Forms::DialogResult proceedResult = MessageBox::Show("Semifinals complete. Proceed to Final?", "Phase Complete", MessageBoxButtons::YesNo, MessageBoxIcon::Question);
    if (proceedResult == System::Windows::Forms::DialogResult::Yes) {
        currentPhase = 3; SetupFinalPhaseUI();
    }
    else { if (processButton) processButton->Enabled = true; }
}


void MainForm::SetupFinalPhaseUI() {
    pnlPhaseContent->Controls->Clear(); lblPhaseTitle->Text = "Insertion FINALE";
    finalResultLog = ""; // Clear previous log only

    // Check prerequisite
    if (semiFinalWinners->Count != 2) {
        MessageBox::Show("Error: Cannot setup Final with " + semiFinalWinners->Count + " winners.", "Setup Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
        return;
    }

    finalMatch = Tuple::Create(semiFinalWinners[0], semiFinalWinners[1]);
    int startX = 20; int currentY = 10; int rowHeight = 30; int verticalSpacing = 5;

    Label^ matchLabel = gcnew Label(); matchLabel->Text = String::Format("FINALE: {0} vs {1}", finalMatch->Item1->id, finalMatch->Item2->id);
    /* Set Location/AutoSize/Font */ matchLabel->Location = System::Drawing::Point(startX - 10, currentY); matchLabel->AutoSize = true; matchLabel->Font = gcnew System::Drawing::Font(this->Font, FontStyle::Bold);
    pnlPhaseContent->Controls->Add(matchLabel); currentY += matchLabel->Height + verticalSpacing * 2;

    array<Player^>^ finalists = { finalMatch->Item1, finalMatch->Item2 };
    for each(Player ^ player in finalists) {
        player->formattedTime = "N/A"; player->timeInSeconds = Double::MaxValue; // Reset time
        Panel^ playerPanel = gcnew Panel(); playerPanel->Name = "pnl" + player->id; /* Size/Location */
        playerPanel->Width = pnlPhaseContent->Width - startX - 10; playerPanel->Height = rowHeight; playerPanel->Location = System::Drawing::Point(startX, currentY);
        // Add Label, TextBox, ToolTip, Button
        Label^ playerLabel = gcnew Label(); playerLabel->Text = player->id + " :"; /*...*/ playerPanel->Controls->Add(playerLabel); playerLabel->Location = System::Drawing::Point(0, 5); playerLabel->AutoSize = true;
        TextBox^ timeTextBox = gcnew TextBox(); timeTextBox->Name = "txtTime" + player->id; timeTextBox->Tag = player; /*...*/ playerPanel->Controls->Add(timeTextBox); timeTextBox->Location = System::Drawing::Point(playerLabel->Right + 5, 2); timeTextBox->Width = 80;
        ToolTip^ timeToolTip = gcnew ToolTip(); timeToolTip->SetToolTip(timeTextBox, "Enter time as MM:SS.mmm");
        Button^ validateButton = gcnew Button(); validateButton->Name = "btnValidate" + player->id; validateButton->Tag = playerPanel; validateButton->Text = "Validate"; /*...*/ validateButton->Width = 60; validateButton->Location = System::Drawing::Point(timeTextBox->Right + 10, 0);
        validateButton->Click += gcnew System::EventHandler(this, &MainForm::ValidateMatchTime);
        playerPanel->Controls->Add(validateButton);

        pnlPhaseContent->Controls->Add(playerPanel);
        currentY += playerPanel->Height + verticalSpacing;
        playerPanel->Enabled = false;
    }

    Button^ processButton = gcnew Button(); processButton->Name = "btnProcessFinal"; /* Text/Location/AutoSize */
    processButton->Text = "Process Final Results && Generate Ranking"; processButton->AutoSize = true; processButton->Location = System::Drawing::Point((pnlPhaseContent->Width - processButton->Width) / 2, currentY + 20);
    processButton->Click += gcnew System::EventHandler(this, &MainForm::ProcessFinalResults); processButton->Enabled = false;
    pnlPhaseContent->Controls->Add(processButton);

    // Enable first finalist
    String^ firstPlayerID = finalMatch->Item1->id;
    Control^ firstPanelControl = pnlPhaseContent->Controls["pnl" + firstPlayerID];
    if (firstPanelControl) {
        firstPanelControl->Enabled = true;
        for each(Control ^ ctrl in firstPanelControl->Controls) {
            TextBox^ firstTextBox = dynamic_cast<TextBox^>(ctrl);
            if (firstTextBox) { firstTextBox->Focus(); break; }
        }
    }
}


void MainForm::ProcessFinalResults(System::Object^ sender, System::EventArgs^ e) {
    Button^ processButton = dynamic_cast<Button^>(sender); if (processButton) processButton->Enabled = false;
    finalResultLog = ""; // Clear previous log only
    bool dataComplete = true; Player^ p1 = finalMatch->Item1; Player^ p2 = finalMatch->Item2;

    if (p1->timeInSeconds == Double::MaxValue || p2->timeInSeconds == Double::MaxValue) {
        MessageBox::Show("Error: Not all Final players have times.", "Processing Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
        if (processButton) processButton->Enabled = true; return;
    }

    Console::WriteLine("\n--- Final Phase Result ---"); finalResultLog = "--- Final Phase Result ---\n";
    if (p1->timeInSeconds < p2->timeInSeconds) { tournamentWinner = p1; tournamentRunnerUp = p2; }
    else { tournamentWinner = p2; tournamentRunnerUp = p1; }
    String^ matchLog = String::Format("FINALE: {0} ({1}) vs {2} ({3}) -> Winner: {4}", p1->id, p1->formattedTime, p2->id, p2->formattedTime, tournamentWinner->id);
    Console::WriteLine(matchLog); finalResultLog += matchLog + "\n";

    currentPhase = 4; // Finished
    CalculateAndDisplayFinalRanking();
    SaveResultsToFile();

    MessageBox::Show("Tournament Finished!\n\nWinner: " + tournamentWinner->id + "\n\nFinal rankings displayed and saved.", "Tournament Complete", MessageBoxButtons::OK, MessageBoxIcon::Information);
    lblPhaseTitle->Text = "TOURNOI TERMINE";
}


String^ MainForm::FindNextMatchPlayerID(String^ currentPlayerID) {
    List<Player^>^ activePlayers = nullptr;
    if (currentPhase == 1 && quarterFinalMatches) { // QF
        activePlayers = gcnew List<Player^>();
        // --- FIX: Replace 'var' with explicit type ---
        for each(Tuple<Player^, Player^> ^ match in quarterFinalMatches) {
            activePlayers->Add(match->Item1); activePlayers->Add(match->Item2);
        }
        // --- End Fix ---
    }
    else if (currentPhase == 2 && semiFinalMatches) { // SF
        activePlayers = gcnew List<Player^>();
        // --- FIX: Replace 'var' with explicit type ---
        for each(Tuple<Player^, Player^> ^ match in semiFinalMatches) {
            activePlayers->Add(match->Item1); activePlayers->Add(match->Item2);
        }
        // --- End Fix ---
    }
    else if (currentPhase == 3 && finalMatch) { // Final
        activePlayers = gcnew List<Player^>();
        activePlayers->Add(finalMatch->Item1); activePlayers->Add(finalMatch->Item2);
    }
    else return nullptr;

    // ...(rest of the function remains the same)...
    if (!activePlayers || activePlayers->Count == 0) return nullptr;
    int currentIndex = -1;
    for (int i = 0; i < activePlayers->Count; ++i) if (activePlayers[i]->id == currentPlayerID) { currentIndex = i; break; }
    if (currentIndex != -1 && currentIndex < activePlayers->Count - 1) return activePlayers[currentIndex + 1]->id;
    else return nullptr;
}


void MainForm::ValidateMatchTime(System::Object^ sender, System::EventArgs^ e) {
    Button^ clickedButton = dynamic_cast<Button^>(sender); Panel^ playerPanel = dynamic_cast<Panel^>(clickedButton->Tag);
    if (!playerPanel) { /* Error handling */ return; }

    TextBox^ timeTextBox = nullptr; Player^ currentPlayer = nullptr;
    for each(Control ^ ctrl in playerPanel->Controls) {
        timeTextBox = dynamic_cast<TextBox^>(ctrl);
        if (timeTextBox) { currentPlayer = dynamic_cast<Player^>(timeTextBox->Tag); break; }
    }
    if (!timeTextBox || !currentPlayer) { /* Error handling */ return; }

    double timeSeconds;
    if (TryParseTime(timeTextBox->Text->Trim(), timeSeconds)) {
        currentPlayer->timeInSeconds = timeSeconds; currentPlayer->formattedTime = FormatTime(timeSeconds);
        playerPanel->Visible = false;

        String^ nextPlayerID = FindNextMatchPlayerID(currentPlayer->id);
        if (nextPlayerID) {
            Control^ nextPanelControl = pnlPhaseContent->Controls["pnl" + nextPlayerID];
            if (nextPanelControl) {
                nextPanelControl->Enabled = true;
                for each(Control ^ ctrl in nextPanelControl->Controls) {
                    TextBox^ nextTextBox = dynamic_cast<TextBox^>(ctrl);
                    if (nextTextBox) { nextTextBox->Focus(); break; }
                }
            }
            else { /* Error handling: Panel not found */ }
        }
        else { // Last player for this phase
            String^ processButtonName = "";
            if (currentPhase == 1) processButtonName = "btnProcessQF";
            else if (currentPhase == 2) processButtonName = "btnProcessSF";
            else if (currentPhase == 3) processButtonName = "btnProcessFinal";
            if (!String::IsNullOrEmpty(processButtonName)) {
                Control^ processButtonControl = pnlPhaseContent->Controls[processButtonName];
                Button^ processButton = dynamic_cast<Button^>(processButtonControl);
                if (processButton) {
                    processButton->Enabled = true; processButton->Focus();
                    MessageBox::Show("All times entered. Click '" + processButton->Text + "'.", "Input Complete", MessageBoxButtons::OK, MessageBoxIcon::Information);
                }
                else { /* Error handling: Button not found */ }
            }
        }
    }
    else {
        MessageBox::Show("Invalid time format for " + currentPlayer->id + ".\nPlease use MM:SS.mmm", "Input Error", MessageBoxButtons::OK, MessageBoxIcon::Warning);
        timeTextBox->Focus(); timeTextBox->SelectAll();
    }
}


void MainForm::CalculateAndDisplayFinalRanking() {
    finalRankingLog->Clear(); // Clear previous ranking before calculating new one
    Console::WriteLine("\n------ FINAL TOURNAMENT RANKING (Non-Disqualified) ------"); finalRankingLog->Add("------ FINAL TOURNAMENT RANKING (Non-Disqualified) ------");

    List<Player^>^ eliminationLosersForRanking = gcnew List<Player^>();
    List<String^>^ qualifierIDs = gcnew List<String^>();
    if (quarterFinalQualifiers) { for each(Player ^ p in quarterFinalQualifiers) qualifierIDs->Add(p->id); }
    if (eliminationGroups) {
        for each(List<Player^> ^ group in eliminationGroups) {
            if (group) {
                for each(Player ^ p in group) {
                    if (p && !qualifierIDs->Contains(p->id) && p->timeInSeconds != Double::MaxValue && !p->isDisqualified) {
                        eliminationLosersForRanking->Add(p);
                    }
                }
            }
        }
    }

    Comparison<Player^>^ sortByTime = gcnew Comparison<Player^>(Player::ComparePlayersByTime);
    if (semiFinalLosers) semiFinalLosers->Sort(sortByTime);
    if (quarterFinalLosers) quarterFinalLosers->Sort(sortByTime);
    if (eliminationLosersForRanking) eliminationLosersForRanking->Sort(sortByTime);

    List<Player^>^ rankedList = gcnew List<Player^>();
    if (tournamentWinner) rankedList->Add(tournamentWinner);
    if (tournamentRunnerUp) rankedList->Add(tournamentRunnerUp);
    if (semiFinalLosers) rankedList->AddRange(semiFinalLosers);
    if (quarterFinalLosers) rankedList->AddRange(quarterFinalLosers);
    if (eliminationLosersForRanking) rankedList->AddRange(eliminationLosersForRanking);

    int currentRank = 1;
    for (int i = 0; i < rankedList->Count; ++i) {
        bool isTied = (i > 0 && rankedList[i]->timeInSeconds != Double::MaxValue && rankedList[i]->timeInSeconds == rankedList[i - 1]->timeInSeconds);
        if (!isTied) currentRank = i + 1;
        String^ rankSuffix = (currentRank == 1) ? "er" : "e";
        String^ rankDisplay = currentRank.ToString() + rankSuffix;
        String^ rankLog = String::Format("{0} : {1} ({2})", rankDisplay, rankedList[i]->id, rankedList[i]->formattedTime);
        Console::WriteLine(rankLog); finalRankingLog->Add(rankLog);
    }
    Console::WriteLine("------------------------------------"); finalRankingLog->Add("------------------------------------");

    // DISQUALIFIED LIST
    // --- ADD DISQUALIFIED LIST TO OUTPUT ---
    Console::WriteLine("\n------ DISQUALIFIED PLAYERS (Temps Minimum) ------");
    finalRankingLog->Add("");
    finalRankingLog->Add("------ DISQUALIFIED PLAYERS (Temps Minimum) ------");
    if (disqualifiedPlayers != nullptr && disqualifiedPlayers->Count > 0) {
        // --- FIX: Replace lambda sort with static function call ---
        disqualifiedPlayers->Sort(gcnew Comparison<Player^>(Player::ComparePlayersByID));
        // --- End Fix ---

        for each(Player ^ dqPlayer in disqualifiedPlayers) {
            String^ displayTime = dqPlayer->formattedTime; if (String::IsNullOrEmpty(displayTime) || displayTime == "N/A") displayTime = FormatTime(dqPlayer->timeInSeconds);
            String^ dqLog = String::Format("  - {0} (Elimination Time: {1})", dqPlayer->id, displayTime);
            Console::WriteLine(dqLog); finalRankingLog->Add(dqLog);
        }
    }
    else {
        Console::WriteLine("  (None)"); finalRankingLog->Add("  (None)");
    }
    Console::WriteLine("---------------------------------------------");
    finalRankingLog->Add("---------------------------------------------");
}


void MainForm::SaveResultsToFile() {
    SaveFileDialog^ saveFileDialog = gcnew SaveFileDialog();
    saveFileDialog->Filter = "Text File|*.txt"; saveFileDialog->Title = "Save Tournament Results"; saveFileDialog->FileName = "tournament_results.txt";

    if (saveFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
        String^ filename = saveFileDialog->FileName;
        StreamWriter^ writer = nullptr;
        try {
            writer = gcnew StreamWriter(filename);
            writer->WriteLine("====== TOURNAMENT RESULTS ======"); writer->WriteLine();
            // Final Ranking (Includes DQ list at the end now)
            if (finalRankingLog && finalRankingLog->Count > 0) { for each(String ^ line in finalRankingLog) writer->WriteLine(line); }
            else { writer->WriteLine("------ FINAL RANKING (Not Available) ------"); } writer->WriteLine();
            // Phase Results
            writer->WriteLine("------ FINAL ------"); writer->WriteLine(String::IsNullOrEmpty(finalResultLog) ? "(No Data)" : finalResultLog); writer->WriteLine();
            writer->WriteLine("------ SEMI-FINALS ------"); if (semiFinalResultsLog && semiFinalResultsLog->Count > 0) { for each(String ^ line in semiFinalResultsLog) writer->WriteLine(line); }
            else { writer->WriteLine("(No Data)"); } writer->WriteLine();
            writer->WriteLine("------ QUARTER-FINALS ------"); if (quarterFinalResultsLog && quarterFinalResultsLog->Count > 0) { for each(String ^ line in quarterFinalResultsLog) writer->WriteLine(line); }
            else { writer->WriteLine("(No Data)"); } writer->WriteLine();
            writer->WriteLine("------ ELIMINATION PHASE ------"); if (eliminationResultsLog && eliminationResultsLog->Count > 0) { for each(String ^ line in eliminationResultsLog) writer->WriteLine(line); }
            else { writer->WriteLine("(No Data)"); } writer->WriteLine();
            writer->WriteLine("==============================");
            MessageBox::Show("Results saved successfully to:\n" + filename, "Save Complete", MessageBoxButtons::OK, MessageBoxIcon::Information);
        }
        catch (Exception^ ex) {
            MessageBox::Show("Error saving results: " + ex->Message, "File Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
        finally {
            if (writer != nullptr) { writer->Close(); }
        }
    }
    else {
        MessageBox::Show("Save operation cancelled.", "Cancelled", MessageBoxButtons::OK, MessageBoxIcon::Warning);
    }
}

// --- END OF FILE MainForm.cpp ---