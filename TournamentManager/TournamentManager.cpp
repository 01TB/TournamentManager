#include "MainForm.h" // Must include your form header

using namespace System;
using namespace System::Windows::Forms;
using namespace TournamentManager; // Use YOUR project's namespace here

[STAThreadAttribute] // Important for Forms!
int main(array<System::String^>^ args)
{
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    // Make sure 'MainForm' here is the EXACT name of your form class
    Application::Run(gcnew MainForm());

    return 0;
}