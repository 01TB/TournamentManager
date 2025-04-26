// 1. Include YOUR project header FIRST
#include "MainForm.h"

// 2. Define Windows lean includes BEFORE including windows.h (Optional but Recommended)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX // Prevents windows.h from defining min() and max() macros

// 3. Include standard / Windows / other library headers AFTER your project header
#include <windows.h> // Only if you actually need AllocConsole or other WinAPI
#include <iostream>  // Only if using std::cout/cerr (Console::WriteLine is preferred for C++/CLI)
#include <limits>    // Only if using std::numeric_limits (Double::MaxValue is fine)
// #include <fstream> // This is NOT needed for System::IO::StreamWriter

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
    // finalMatch, winner, runnerUp initialized later in their respective phases

    // Initialize logs
    eliminationResultsLog = gcnew List<String^>();
    quarterFinalResultsLog = gcnew List<String^>();
    semiFinalResultsLog = gcnew List<String^>();
    finalRankingLog = gcnew List<String^>();
    finalResultLog = "";

    currentPhase = 0;
    SetupEliminationPhaseUI();
}

void MainForm::SetupEliminationPhaseUI() {
    // Optional: Setup console if needed for debugging output
    // AllocConsole();
    // freopen("CONOUT$", "w", stdout);

    pnlPhaseContent->Controls->Clear();
    lblPhaseTitle->Text = "Insertion ELIMINATION";

    for each (List<Player^> ^ group in eliminationGroups) {
        group->Clear();
    }
    eliminationResultsLog->Clear();

    int startX = 20; int currentY = 10; int rowHeight = 30;
    int verticalSpacing = 5; int groupSpacing = 15;
    char groupChar = 'A';

    for (int i = 0; i < 4; ++i) {
        Label^ groupLabel = gcnew Label();
        groupLabel->Text = "Groupe " + Char(groupChar + i).ToString();
        groupLabel->Location = System::Drawing::Point(10, currentY);
        groupLabel->AutoSize = true;
        groupLabel->Font = gcnew System::Drawing::Font(this->Font->FontFamily, 10, FontStyle::Bold);
        pnlPhaseContent->Controls->Add(groupLabel);
        currentY += groupLabel->Height + verticalSpacing;

        List<Player^>^ currentGroupPlayers = eliminationGroups[i];

        for (int j = 1; j <= 4; ++j) {
            String^ playerID = Char(groupChar + i).ToString() + j.ToString();
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
            timeToolTip->SetToolTip(timeTextBox, "Enter time as MM:SS.mmm (e.g., 01:57.123)");
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
            playerPanel->Enabled = (i == 0 && j == 1); // Only enable first row initially
            currentY += playerPanel->Height + verticalSpacing;
        }
        currentY += groupSpacing;
    }

    Button^ processButton = gcnew Button();
    processButton->Name = "btnProcessElimination";
    processButton->Text = "Process Elimination Results";
    processButton->AutoSize = true;
    processButton->Location = System::Drawing::Point((pnlPhaseContent->Width - processButton->Width) / 2, currentY + 10);
    processButton->Click += gcnew System::EventHandler(this, &MainForm::ProcessEliminationResults);
    processButton->Enabled = false;
    pnlPhaseContent->Controls->Add(processButton);

    Control^ firstPanel = pnlPhaseContent->Controls["pnlA1"];
    if (firstPanel) {
        Control^ firstTextBox = firstPanel->Controls["txtTimeA1"];
        if (firstTextBox != nullptr) {
            firstTextBox->Focus();
        }
    }
}


String^ MainForm::FindNextPlayerID(String^ currentID) {
    if (String::IsNullOrEmpty(currentID) || currentID->Length < 2) {
        return nullptr;
    }

    try {
        // Keep char groupChar, but be aware of the warning's reason
        char groupChar = currentID[0]; // Warning C4244 still expected here

        int playerNum = Int32::Parse(currentID->Substring(1));

        if (playerNum < 4) {
            // --- MODIFIED LINE AGAIN ---
            // Explicitly pass the ORIGINAL System::Char from the string to Format
            return String::Format("{0}{1}", currentID[0], playerNum + 1);
            // --- END MODIFIED LINE ---
        }
        else if (groupChar < 'D') { // This comparison might also be safer with wchar_t
            // --- SAFER VERSION ---
            // Convert the NEXT character directly
            return String::Format("{0}1", (wchar_t)(currentID[0] + 1));
            // --- END SAFER VERSION ---

            // Original was: return Char(groupChar + 1).ToString() + "1";
        }
        else {
            return nullptr; // D4
        }
    }
    catch (FormatException^ ex) {
        Console::WriteLine("Error parsing player ID: " + ex->Message);
        return nullptr;
    }
    catch (OverflowException^ ex) { // Catch potential overflow from char+1
        Console::WriteLine("Error calculating next group char: " + ex->Message);
        return nullptr;
    }
}


void MainForm::ValidateEliminationTime(System::Object^ sender, System::EventArgs^ e) {
    Button^ clickedButton = dynamic_cast<Button^>(sender);
    Panel^ playerPanel = dynamic_cast<Panel^>(clickedButton->Tag);
    if (!playerPanel) { /* Error handling */ return; }

    TextBox^ timeTextBox = nullptr; Player^ currentPlayer = nullptr;
    for each (Control ^ ctrl in playerPanel->Controls) {
        timeTextBox = dynamic_cast<TextBox^>(ctrl);
        if (timeTextBox) { currentPlayer = dynamic_cast<Player^>(timeTextBox->Tag); break; }
    }
    if (!timeTextBox || !currentPlayer) { /* Error handling */ return; }

    double timeSeconds;
    if (TryParseTime(timeTextBox->Text->Trim(), timeSeconds)) {
        currentPlayer->timeInSeconds = timeSeconds;
        currentPlayer->formattedTime = FormatTime(timeSeconds);
        playerPanel->Visible = false;

        String^ nextPlayerID = FindNextPlayerID(currentPlayer->id);
        Console::WriteLine("Next player : " + nextPlayerID);
        if (nextPlayerID) {
            Control^ nextPanelControl = pnlPhaseContent->Controls["pnl" + nextPlayerID];
            if (nextPanelControl) {

                Console::WriteLine("DEBUG: Trying to enable panel for nextPlayerID: " + nextPlayerID);
                if (nextPanelControl == nullptr) {
                    Console::WriteLine("DEBUG: ERROR - nextPanelControl lookup returned null!");
                }
                else if (nextPanelControl == nullptr) {
                    Console::WriteLine("DEBUG: ERROR - dynamic_cast to Panel^ failed!");
                }
                else {
                    Console::WriteLine("DEBUG: Found nextPanel: " + nextPanelControl->Name + ". Attempting to enable...");
                }

                nextPanelControl->Enabled = true;
                for each (Control ^ ctrl in nextPanelControl->Controls) {
                    TextBox^ nextTextBox = dynamic_cast<TextBox^>(ctrl);
                    if (nextTextBox) { nextTextBox->Focus(); break; }
                }
            }
            else {
                Console::WriteLine("No nextPanelControl found!!!");
            }
        }
        else { // Last player (D4)
            Control^ processButtonControl = pnlPhaseContent->Controls["btnProcessElimination"];
            Button^ processButton = dynamic_cast<Button^>(processButtonControl);
            if (processButton) {
                processButton->Enabled = true; processButton->Focus();
                MessageBox::Show("All elimination times entered. Click 'Process Elimination Results'.", "Input Complete", MessageBoxButtons::OK, MessageBoxIcon::Information);
            }
            else { /* Error handling */ }
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

    // Use Console or Debug for output
    Console::WriteLine("\n--- Elimination Phase Results ---");
    eliminationResultsLog->Clear();
    quarterFinalQualifiers->Clear();
    char groupChar = 'A'; bool dataComplete = true;

    for each (List<Player^> ^ groupPlayers in eliminationGroups) {
        for each (Player ^ p in groupPlayers) {
            if (p->timeInSeconds == Double::MaxValue) {
                MessageBox::Show("Error: Player " + p->id + " missing time.", "Processing Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
                if (processButton) processButton->Enabled = true; dataComplete = false; break;
            }
        }
        if (!dataComplete) break;

        groupPlayers->Sort(gcnew Comparison<Player^>(Player::ComparePlayersByTime));
        String^ groupLogHeader = "Groupe " + Char(groupChar).ToString() + " Ranking:";
        Console::WriteLine(groupLogHeader); eliminationResultsLog->Add(groupLogHeader);

        for (int i = 0; i < groupPlayers->Count; ++i) {
            String^ rankSuffix = (i == 0) ? "er" : "e"; // Simplified French suffix
            String^ rankStr = (i + 1).ToString() + rankSuffix;
            String^ displayTime = groupPlayers[i]->formattedTime;
            if (String::IsNullOrEmpty(displayTime) || displayTime == "N/A") displayTime = FormatTime(groupPlayers[i]->timeInSeconds);

            String^ playerLog = String::Format("  {0} ({1}) - {2}", groupPlayers[i]->id, rankStr, displayTime);
            Console::WriteLine(playerLog); eliminationResultsLog->Add(playerLog);

            if (i < 2) {
                quarterFinalQualifiers->Add(groupPlayers[i]);
                String^ qualMsg = "   -> Qualifies for Quarterfinals";
                Console::WriteLine(qualMsg); eliminationResultsLog->Add(qualMsg);
            }
        }
        Console::WriteLine(""); eliminationResultsLog->Add("");
        groupChar++;
    }

    if (dataComplete) {
        System::Windows::Forms::DialogResult proceedResult = MessageBox::Show("Elimination phase complete. Proceed to Quarterfinals?", "Phase Complete", MessageBoxButtons::YesNo, MessageBoxIcon::Question);
        if (proceedResult == System::Windows::Forms::DialogResult::Yes) {
            currentPhase = 1; SetupQuarterFinalPhaseUI();
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

    if (quarterFinalQualifiers->Count != 8) { /* Error handling */ return; }

    // Define Matches (adjust indices based on actual qualifier order if needed)
    quarterFinalMatches->Add(Tuple::Create(quarterFinalQualifiers[0], quarterFinalQualifiers[2])); // A1 vs B1
    quarterFinalMatches->Add(Tuple::Create(quarterFinalQualifiers[1], quarterFinalQualifiers[3])); // A2 vs B2
    quarterFinalMatches->Add(Tuple::Create(quarterFinalQualifiers[4], quarterFinalQualifiers[6])); // C1 vs D1
    quarterFinalMatches->Add(Tuple::Create(quarterFinalQualifiers[5], quarterFinalQualifiers[7])); // C2 vs D2

    int startX = 20; int currentY = 10; int rowHeight = 30; int verticalSpacing = 5; int matchSpacing = 15;

    for (int i = 0; i < quarterFinalMatches->Count; ++i) {
        Label^ matchLabel = gcnew Label();
        matchLabel->Text = String::Format("Match {0}: {1} vs {2}", i + 1, quarterFinalMatches[i]->Item1->id, quarterFinalMatches[i]->Item2->id);
        matchLabel->Location = System::Drawing::Point(startX - 10, currentY); matchLabel->AutoSize = true; matchLabel->Font = gcnew System::Drawing::Font(this->Font, FontStyle::Bold);
        pnlPhaseContent->Controls->Add(matchLabel); currentY += matchLabel->Height + verticalSpacing;

        array<Player^>^ playersInMatch = { quarterFinalMatches[i]->Item1, quarterFinalMatches[i]->Item2 };
        for each (Player ^ player in playersInMatch) {
            // Reset time for this phase
            player->formattedTime = "N/A"; player->timeInSeconds = Double::MaxValue;

            Panel^ playerPanel = gcnew Panel(); playerPanel->Name = "pnl" + player->id; /* Set Size/Location */
            playerPanel->Width = pnlPhaseContent->Width - startX - 10; playerPanel->Height = rowHeight; playerPanel->Location = System::Drawing::Point(startX, currentY);

            Label^ playerLabel = gcnew Label(); playerLabel->Text = player->id + " :"; /* Set Location/AutoSize */
            playerLabel->Location = System::Drawing::Point(0, 5); playerLabel->AutoSize = true; playerPanel->Controls->Add(playerLabel);

            TextBox^ timeTextBox = gcnew TextBox(); timeTextBox->Name = "txtTime" + player->id; timeTextBox->Tag = player; /* Set Location/Width */
            timeTextBox->Location = System::Drawing::Point(playerLabel->Right + 5, 2); timeTextBox->Width = 80; playerPanel->Controls->Add(timeTextBox);
            ToolTip^ timeToolTip = gcnew ToolTip(); timeToolTip->SetToolTip(timeTextBox, "Enter time as MM:SS.mmm");

            Button^ validateButton = gcnew Button(); validateButton->Name = "btnValidate" + player->id; validateButton->Tag = playerPanel; validateButton->Text = "Validate"; /* Set Location/Width */
            validateButton->Width = 60; validateButton->Location = System::Drawing::Point(timeTextBox->Right + 10, 0);
            validateButton->Click += gcnew System::EventHandler(this, &MainForm::ValidateMatchTime); // Shared handler
            playerPanel->Controls->Add(validateButton);

            pnlPhaseContent->Controls->Add(playerPanel);
            currentY += playerPanel->Height + verticalSpacing;
            playerPanel->Enabled = false; // Disable all initially
        }
        currentY += matchSpacing;
    }

    Button^ processButton = gcnew Button(); processButton->Name = "btnProcessQF"; /* Set Text/Location/AutoSize */
    processButton->Text = "Process Quarterfinal Results"; processButton->AutoSize = true; processButton->Location = System::Drawing::Point((pnlPhaseContent->Width - processButton->Width) / 2, currentY + 10);
    processButton->Click += gcnew System::EventHandler(this, &MainForm::ProcessQuarterFinalResults); processButton->Enabled = false;
    pnlPhaseContent->Controls->Add(processButton);

    // Enable first player row
    String^ firstPlayerID = quarterFinalMatches[0]->Item1->id;
    Control^ firstPanelControl = pnlPhaseContent->Controls["pnl" + firstPlayerID];
    if (firstPanelControl) {
        firstPanelControl->Enabled = true;
        for each (Control ^ ctrl in firstPanelControl->Controls) {
            TextBox^ firstTextBox = dynamic_cast<TextBox^>(ctrl);
            if (firstTextBox) { firstTextBox->Focus(); break; }
        }
    }
}

void MainForm::ProcessQuarterFinalResults(System::Object^ sender, System::EventArgs^ e) {
    Button^ processButton = dynamic_cast<Button^>(sender); if (processButton) processButton->Enabled = false;
    quarterFinalResultsLog->Clear(); quarterFinalWinners->Clear(); quarterFinalLosers->Clear(); bool dataComplete = true;

    for each (Tuple<Player^, Player^> ^ match in quarterFinalMatches) {
        if (match->Item1->timeInSeconds == Double::MaxValue || match->Item2->timeInSeconds == Double::MaxValue) {
            MessageBox::Show("Error: Not all QF players have times.", "Processing Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
            if (processButton) processButton->Enabled = true; dataComplete = false; break;
        }
    }
    if (!dataComplete) return;

    Console::WriteLine("\n--- Quarterfinal Phase Results ---"); quarterFinalResultsLog->Add("--- Quarterfinal Phase Results ---");

    for each (Tuple<Player^, Player^> ^ match in quarterFinalMatches) {
        Player^ p1 = match->Item1; Player^ p2 = match->Item2; Player^ winner; Player^ loser;
        if (p1->timeInSeconds < p2->timeInSeconds) { winner = p1; loser = p2; }
        else { winner = p2; loser = p1; }
        quarterFinalWinners->Add(winner); quarterFinalLosers->Add(loser);
        String^ matchLog = String::Format("{0} ({1}) vs {2} ({3}) -> Winner: {4}", p1->id, p1->formattedTime, p2->id, p2->formattedTime, winner->id);
        Console::WriteLine(matchLog); quarterFinalResultsLog->Add(matchLog);
    }
    Console::WriteLine(""); quarterFinalResultsLog->Add("");

    System::Windows::Forms::DialogResult proceedResult = MessageBox::Show("Quarterfinals complete. Proceed to Semifinals?", "Phase Complete", MessageBoxButtons::YesNo, MessageBoxIcon::Question);
    if (proceedResult == System::Windows::Forms::DialogResult::Yes) {
        currentPhase = 2; SetupSemiFinalPhaseUI();
    }
    else { if (processButton) processButton->Enabled = true; }
}


void MainForm::SetupSemiFinalPhaseUI() {
    pnlPhaseContent->Controls->Clear(); lblPhaseTitle->Text = "Insertion DEMI FINALE (1/2)";
    semiFinalMatches->Clear(); semiFinalWinners->Clear(); semiFinalLosers->Clear(); semiFinalResultsLog->Clear();
    if (quarterFinalWinners->Count != 4) { /* Error handling */ return; }

    semiFinalMatches->Add(Tuple::Create(quarterFinalWinners[0], quarterFinalWinners[1])); // Winner QF1 vs Winner QF2
    semiFinalMatches->Add(Tuple::Create(quarterFinalWinners[2], quarterFinalWinners[3])); // Winner QF3 vs Winner QF4

    int startX = 20; int currentY = 10; int rowHeight = 30; int verticalSpacing = 5; int matchSpacing = 15;

    for (int i = 0; i < semiFinalMatches->Count; ++i) {
        Label^ matchLabel = gcnew Label(); matchLabel->Text = String::Format("Match {0}: {1} vs {2}", i + 1, semiFinalMatches[i]->Item1->id, semiFinalMatches[i]->Item2->id);
        /* Set Location/AutoSize/Font */ matchLabel->Location = System::Drawing::Point(startX - 10, currentY); matchLabel->AutoSize = true; matchLabel->Font = gcnew System::Drawing::Font(this->Font, FontStyle::Bold);
        pnlPhaseContent->Controls->Add(matchLabel); currentY += matchLabel->Height + verticalSpacing;

        array<Player^>^ playersInMatch = { semiFinalMatches[i]->Item1, semiFinalMatches[i]->Item2 };
        for each (Player ^ player in playersInMatch) {
            player->formattedTime = "N/A"; player->timeInSeconds = Double::MaxValue; // Reset time
            Panel^ playerPanel = gcnew Panel(); playerPanel->Name = "pnl" + player->id; /* Size/Location */
            playerPanel->Width = pnlPhaseContent->Width - startX - 10; playerPanel->Height = rowHeight; playerPanel->Location = System::Drawing::Point(startX, currentY);

            // Add Label, TextBox, ToolTip, Button like in QF setup
            Label^ playerLabel = gcnew Label(); playerLabel->Text = player->id + " :"; /*...*/ playerPanel->Controls->Add(playerLabel); playerLabel->Location = System::Drawing::Point(0, 5); playerLabel->AutoSize = true;
            TextBox^ timeTextBox = gcnew TextBox(); timeTextBox->Name = "txtTime" + player->id; timeTextBox->Tag = player; /*...*/ playerPanel->Controls->Add(timeTextBox); timeTextBox->Location = System::Drawing::Point(playerLabel->Right + 5, 2); timeTextBox->Width = 80;
            ToolTip^ timeToolTip = gcnew ToolTip(); timeToolTip->SetToolTip(timeTextBox, "Enter time as MM:SS.mmm");
            Button^ validateButton = gcnew Button(); validateButton->Name = "btnValidate" + player->id; validateButton->Tag = playerPanel; validateButton->Text = "Validate"; /*...*/ validateButton->Width = 60; validateButton->Location = System::Drawing::Point(timeTextBox->Right + 10, 0);
            validateButton->Click += gcnew System::EventHandler(this, &MainForm::ValidateMatchTime); // Shared handler
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
    String^ firstPlayerID = semiFinalMatches[0]->Item1->id;
    Control^ firstPanelControl = pnlPhaseContent->Controls["pnl" + firstPlayerID];
    if (firstPanelControl) {
        firstPanelControl->Enabled = true;
        for each (Control ^ ctrl in firstPanelControl->Controls) {
            TextBox^ firstTextBox = dynamic_cast<TextBox^>(ctrl);
            if (firstTextBox) { firstTextBox->Focus(); break; }
        }
    }
}


void MainForm::ProcessSemiFinalResults(System::Object^ sender, System::EventArgs^ e) {
    Button^ processButton = dynamic_cast<Button^>(sender); if (processButton) processButton->Enabled = false;
    semiFinalResultsLog->Clear(); semiFinalWinners->Clear(); semiFinalLosers->Clear(); bool dataComplete = true;

    for each (Tuple<Player^, Player^> ^ match in semiFinalMatches) {
        if (match->Item1->timeInSeconds == Double::MaxValue || match->Item2->timeInSeconds == Double::MaxValue) {
            MessageBox::Show("Error: Not all SF players have times.", "Processing Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
            if (processButton) processButton->Enabled = true; dataComplete = false; break;
        }
    }
    if (!dataComplete) return;

    Console::WriteLine("\n--- Semifinal Phase Results ---"); semiFinalResultsLog->Add("--- Semifinal Phase Results ---");

    for each (Tuple<Player^, Player^> ^ match in semiFinalMatches) {
        Player^ p1 = match->Item1; Player^ p2 = match->Item2; Player^ winner; Player^ loser;
        if (p1->timeInSeconds < p2->timeInSeconds) { winner = p1; loser = p2; }
        else { winner = p2; loser = p1; }
        semiFinalWinners->Add(winner); semiFinalLosers->Add(loser);
        String^ matchLog = String::Format("{0} ({1}) vs {2} ({3}) -> Winner: {4}", p1->id, p1->formattedTime, p2->id, p2->formattedTime, winner->id);
        Console::WriteLine(matchLog); semiFinalResultsLog->Add(matchLog);
    }
    Console::WriteLine(""); semiFinalResultsLog->Add("");

    System::Windows::Forms::DialogResult proceedResult = MessageBox::Show("Semifinals complete. Proceed to Final?", "Phase Complete", MessageBoxButtons::YesNo, MessageBoxIcon::Question);
    if (proceedResult == System::Windows::Forms::DialogResult::Yes) {
        currentPhase = 3; SetupFinalPhaseUI();
    }
    else { if (processButton) processButton->Enabled = true; }
}


void MainForm::SetupFinalPhaseUI() {
    pnlPhaseContent->Controls->Clear(); lblPhaseTitle->Text = "Insertion FINALE";
    finalResultLog = ""; finalRankingLog->Clear();
    if (semiFinalWinners->Count != 2) { /* Error handling */ return; }

    finalMatch = Tuple::Create(semiFinalWinners[0], semiFinalWinners[1]);
    int startX = 20; int currentY = 10; int rowHeight = 30; int verticalSpacing = 5;

    Label^ matchLabel = gcnew Label(); matchLabel->Text = String::Format("FINALE: {0} vs {1}", finalMatch->Item1->id, finalMatch->Item2->id);
    /* Set Location/AutoSize/Font */ matchLabel->Location = System::Drawing::Point(startX - 10, currentY); matchLabel->AutoSize = true; matchLabel->Font = gcnew System::Drawing::Font(this->Font, FontStyle::Bold);
    pnlPhaseContent->Controls->Add(matchLabel); currentY += matchLabel->Height + verticalSpacing * 2;

    array<Player^>^ finalists = { finalMatch->Item1, finalMatch->Item2 };
    for each (Player ^ player in finalists) {
        player->formattedTime = "N/A"; player->timeInSeconds = Double::MaxValue; // Reset time
        Panel^ playerPanel = gcnew Panel(); playerPanel->Name = "pnl" + player->id; /* Size/Location */
        playerPanel->Width = pnlPhaseContent->Width - startX - 10; playerPanel->Height = rowHeight; playerPanel->Location = System::Drawing::Point(startX, currentY);

        // Add Label, TextBox, ToolTip, Button like in QF/SF setup
        Label^ playerLabel = gcnew Label(); playerLabel->Text = player->id + " :"; /*...*/ playerPanel->Controls->Add(playerLabel); playerLabel->Location = System::Drawing::Point(0, 5); playerLabel->AutoSize = true;
        TextBox^ timeTextBox = gcnew TextBox(); timeTextBox->Name = "txtTime" + player->id; timeTextBox->Tag = player; /*...*/ playerPanel->Controls->Add(timeTextBox); timeTextBox->Location = System::Drawing::Point(playerLabel->Right + 5, 2); timeTextBox->Width = 80;
        ToolTip^ timeToolTip = gcnew ToolTip(); timeToolTip->SetToolTip(timeTextBox, "Enter time as MM:SS.mmm");
        Button^ validateButton = gcnew Button(); validateButton->Name = "btnValidate" + player->id; validateButton->Tag = playerPanel; validateButton->Text = "Validate"; /*...*/ validateButton->Width = 60; validateButton->Location = System::Drawing::Point(timeTextBox->Right + 10, 0);
        validateButton->Click += gcnew System::EventHandler(this, &MainForm::ValidateMatchTime); // Shared handler
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
        for each (Control ^ ctrl in firstPanelControl->Controls) {
            TextBox^ firstTextBox = dynamic_cast<TextBox^>(ctrl);
            if (firstTextBox) { firstTextBox->Focus(); break; }
        }
    }
}


void MainForm::ProcessFinalResults(System::Object^ sender, System::EventArgs^ e) {
    Button^ processButton = dynamic_cast<Button^>(sender); if (processButton) processButton->Enabled = false;
    finalResultLog = ""; bool dataComplete = true; Player^ p1 = finalMatch->Item1; Player^ p2 = finalMatch->Item2;

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
    // Optionally clear or update pnlPhaseContent to show ranking
}


String^ MainForm::FindNextMatchPlayerID(String^ currentPlayerID) {
    List<Player^>^ activePlayers = nullptr;
    if (currentPhase == 1) { // QF
        activePlayers = gcnew List<Player^>();
        for each (Tuple<Player^, Player^> ^ match in quarterFinalMatches) { activePlayers->Add(match->Item1); activePlayers->Add(match->Item2); }
    }
    else if (currentPhase == 2) { // SF
        activePlayers = gcnew List<Player^>();
        for each (Tuple<Player^, Player^> ^ match in semiFinalMatches) { activePlayers->Add(match->Item1); activePlayers->Add(match->Item2); }
    }
    else if (currentPhase == 3) { // Final
        activePlayers = gcnew List<Player^>();
        if (finalMatch) { activePlayers->Add(finalMatch->Item1); activePlayers->Add(finalMatch->Item2); }
    }
    else return nullptr;

    if (!activePlayers || activePlayers->Count == 0) return nullptr;

    int currentIndex = -1;
    for (int i = 0; i < activePlayers->Count; ++i) if (activePlayers[i]->id == currentPlayerID) { currentIndex = i; break; }

    if (currentIndex != -1 && currentIndex < activePlayers->Count - 1) return activePlayers[currentIndex + 1]->id;
    else return nullptr; // Last player in sequence
}


void MainForm::ValidateMatchTime(System::Object^ sender, System::EventArgs^ e) {
    Button^ clickedButton = dynamic_cast<Button^>(sender); Panel^ playerPanel = dynamic_cast<Panel^>(clickedButton->Tag);
    if (!playerPanel) { /* Error handling */ return; }

    TextBox^ timeTextBox = nullptr; Player^ currentPlayer = nullptr;
    for each (Control ^ ctrl in playerPanel->Controls) {
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
                for each (Control ^ ctrl in nextPanelControl->Controls) {
                    TextBox^ nextTextBox = dynamic_cast<TextBox^>(ctrl);
                    if (nextTextBox) { nextTextBox->Focus(); break; }
                }
            }
            else { /* Error handling */ }
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
                else { /* Error handling */ }
            }
        }
    }
    else {
        MessageBox::Show("Invalid time format for " + currentPlayer->id + ".\nPlease use MM:SS.mmm", "Input Error", MessageBoxButtons::OK, MessageBoxIcon::Warning);
        timeTextBox->Focus(); timeTextBox->SelectAll();
    }
}


void MainForm::CalculateAndDisplayFinalRanking() {
    finalRankingLog->Clear();
    Console::WriteLine("\n------ FINAL TOURNAMENT RANKING ------"); finalRankingLog->Add("------ FINAL TOURNAMENT RANKING ------");

    List<Player^>^ eliminationLosers = gcnew List<Player^>();
    List<String^>^ qualifierIDs = gcnew List<String^>();
    for each (Player ^ p in quarterFinalQualifiers) { qualifierIDs->Add(p->id); }
    for each (List<Player^> ^ group in eliminationGroups) {
        for each (Player ^ p in group) {
            if (!qualifierIDs->Contains(p->id) && p->timeInSeconds != Double::MaxValue) eliminationLosers->Add(p);
        }
    }

    Comparison<Player^>^ sortByTime = gcnew Comparison<Player^>(Player::ComparePlayersByTime);
    if (semiFinalLosers) semiFinalLosers->Sort(sortByTime);
    if (quarterFinalLosers) quarterFinalLosers->Sort(sortByTime);
    if (eliminationLosers) eliminationLosers->Sort(sortByTime);

    List<Player^>^ rankedList = gcnew List<Player^>();
    if (tournamentWinner) rankedList->Add(tournamentWinner);
    if (tournamentRunnerUp) rankedList->Add(tournamentRunnerUp);
    if (semiFinalLosers) rankedList->AddRange(semiFinalLosers);
    if (quarterFinalLosers) rankedList->AddRange(quarterFinalLosers);
    if (eliminationLosers) rankedList->AddRange(eliminationLosers);

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
}


void MainForm::SaveResultsToFile() {
    SaveFileDialog^ saveFileDialog = gcnew SaveFileDialog();
    saveFileDialog->Filter = "Text File|*.txt"; saveFileDialog->Title = "Save Tournament Results"; saveFileDialog->FileName = "tournament_results.txt";

    if (saveFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
        String^ filename = saveFileDialog->FileName;
        StreamWriter^ writer = nullptr; // Initialize to nullptr
        try {
            writer = gcnew StreamWriter(filename);
            writer->WriteLine("====== TOURNAMENT RESULTS ======"); writer->WriteLine();
            // Final Ranking
            if (finalRankingLog && finalRankingLog->Count > 0) { for each (String ^ line in finalRankingLog) writer->WriteLine(line); }
            else { writer->WriteLine("------ FINAL RANKING (Not Available) ------"); } writer->WriteLine();
            // Phase Results
            writer->WriteLine("------ FINAL ------"); writer->WriteLine(String::IsNullOrEmpty(finalResultLog) ? "(No Data)" : finalResultLog); writer->WriteLine();
            writer->WriteLine("------ SEMI-FINALS ------"); if (semiFinalResultsLog && semiFinalResultsLog->Count > 0) { for each (String ^ line in semiFinalResultsLog) writer->WriteLine(line); }
            else { writer->WriteLine("(No Data)"); } writer->WriteLine();
            writer->WriteLine("------ QUARTER-FINALS ------"); if (quarterFinalResultsLog && quarterFinalResultsLog->Count > 0) { for each (String ^ line in quarterFinalResultsLog) writer->WriteLine(line); }
            else { writer->WriteLine("(No Data)"); } writer->WriteLine();
            writer->WriteLine("------ ELIMINATION PHASE ------"); if (eliminationResultsLog && eliminationResultsLog->Count > 0) { for each (String ^ line in eliminationResultsLog) writer->WriteLine(line); }
            else { writer->WriteLine("(No Data)"); } writer->WriteLine();
            writer->WriteLine("==============================");
            MessageBox::Show("Results saved successfully to:\n" + filename, "Save Complete", MessageBoxButtons::OK, MessageBoxIcon::Information);
        }
        catch (Exception^ ex) {
            MessageBox::Show("Error saving results: " + ex->Message, "File Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
        finally {
            if (writer != nullptr) { writer->Close(); } // Use nullptr check
        }
    }
    else {
        MessageBox::Show("Save operation cancelled.", "Cancelled", MessageBoxButtons::OK, MessageBoxIcon::Warning);
    }
}




// --- END OF FILE MainForm.cpp ---