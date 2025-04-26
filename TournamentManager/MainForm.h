// --- START OF FILE MainForm.h ---
#pragma once

// Only include headers needed for DECLARATIONS in this file
#include "DataStructures.h" // Needed for Player^, Tuple^ etc.

// IMPORTANT: Move <windows.h>, <iostream>, <limits>, <fstream> to MainForm.cpp

namespace TournamentManager {

    // Bring necessary .NET namespaces into scope for declarations
    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections::Generic; // For List<>, Tuple<>
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;
    // DO NOT put 'using namespace System::IO;' here if only used in implementation

    public ref class MainForm : public System::Windows::Forms::Form
    {
    public:
        MainForm(void); // Constructor Declaration

    protected:
        ~MainForm(); // Destructor Declaration

        // --- Designer Controls ---
    private: System::Windows::Forms::Label^ lblPhaseTitle;
    private: System::Windows::Forms::Panel^ pnlPhaseContent;
           // Add other controls if you add them via the designer

           // --- Tournament State Variables ---
    private:
        List<List<Player^>^>^ eliminationGroups;
        List<Player^>^ quarterFinalQualifiers;
        List<Tuple<Player^, Player^>^>^ quarterFinalMatches;
        List<Player^>^ quarterFinalWinners;
        List<Player^>^ quarterFinalLosers;
        List<Tuple<Player^, Player^>^>^ semiFinalMatches;
        List<Player^>^ semiFinalWinners;
        List<Player^>^ semiFinalLosers;
        Tuple<Player^, Player^>^ finalMatch;
        Player^ tournamentWinner;
        Player^ tournamentRunnerUp;
        // List<Player^>^ allRankedPlayers; // Removed, calculated locally in ranking function

        int currentPhase;

        // Logs
        List<String^>^ eliminationResultsLog;
        List<String^>^ quarterFinalResultsLog;
        List<String^>^ semiFinalResultsLog;
        String^ finalResultLog;
        List<String^>^ finalRankingLog;

        // --- Method Declarations (Declare ONCE) ---
    private:
        void InitializeTournament();
        // Elimination
        void SetupEliminationPhaseUI();
        void ValidateEliminationTime(System::Object^ sender, System::EventArgs^ e);
        String^ FindNextPlayerID(String^ currentID);
        void ProcessEliminationResults(System::Object^ sender, System::EventArgs^ e);
        // Match Phases (Shared)
        void ValidateMatchTime(System::Object^ sender, System::EventArgs^ e);
        String^ FindNextMatchPlayerID(String^ currentPlayerID);
        // Quarterfinals
        void SetupQuarterFinalPhaseUI();
        void ProcessQuarterFinalResults(System::Object^ sender, System::EventArgs^ e);
        // Semifinals
        void SetupSemiFinalPhaseUI();
        void ProcessSemiFinalResults(System::Object^ sender, System::EventArgs^ e);
        // Final
        void SetupFinalPhaseUI();
        void ProcessFinalResults(System::Object^ sender, System::EventArgs^ e);
        // End Game
        void CalculateAndDisplayFinalRanking();
        void SaveResultsToFile();

        // --- Designer Support ---
    private:
        System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
        // Leave InitializeComponent DEFINITION here - the designer manages this.
        void InitializeComponent(void)
        {
            // ... (Designer code - DO NOT REMOVE OR MOVE) ...
            this->lblPhaseTitle = (gcnew System::Windows::Forms::Label());
            this->pnlPhaseContent = (gcnew System::Windows::Forms::Panel());
            this->SuspendLayout();
            //
            // lblPhaseTitle
            //
            this->lblPhaseTitle->Dock = System::Windows::Forms::DockStyle::Top;
            this->lblPhaseTitle->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->lblPhaseTitle->Location = System::Drawing::Point(0, 0);
            this->lblPhaseTitle->Name = L"lblPhaseTitle";
            this->lblPhaseTitle->Padding = System::Windows::Forms::Padding(0, 0, 0, 5);
            this->lblPhaseTitle->Size = System::Drawing::Size(584, 30);
            this->lblPhaseTitle->TabIndex = 0;
            this->lblPhaseTitle->Text = L"Tournament Phase";
            this->lblPhaseTitle->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
            //
            // pnlPhaseContent
            //
            this->pnlPhaseContent->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->pnlPhaseContent->Dock = System::Windows::Forms::DockStyle::Fill;
            this->pnlPhaseContent->Location = System::Drawing::Point(0, 30);
            this->pnlPhaseContent->Name = L"pnlPhaseContent";
            this->pnlPhaseContent->Size = System::Drawing::Size(584, 431);
            this->pnlPhaseContent->TabIndex = 1;
            //
            // MainForm
            //
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(584, 461);
            this->Controls->Add(this->pnlPhaseContent);
            this->Controls->Add(this->lblPhaseTitle);
            this->Name = L"MainForm";
            this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
            this->Text = L"Tournament Manager";
            this->ResumeLayout(false);
        }
#pragma endregion

    }; // End of MainForm class
} // End of namespace
// --- END OF FILE MainForm.h ---