#include "vtkF3DImguiConsole.h"

#include <vtkCommand.h>
#include <vtkObjectFactory.h>

#include <imgui.h>

#include <algorithm>
#include <array>
#include <vtkCallbackCommand.h>
#include <vtkNew.h>

struct vtkF3DImguiConsole::Internals
{
  enum class LogType
  {
    Log,
    Warning,
    Error,
    Typed
  };

  std::vector<std::pair<LogType, std::string>> Logs;
  std::array<char, 256> CurrentInput = {};
  bool NewError = false;
  bool NewWarning = false;
  std::vector<std::string> Commands; // List of supported commands
  std::pair<size_t, size_t> Completions{ 0,
    0 }; // Index for start and length of completions in Logs
  /**
   * Clear completions from the logs
   */
  void ClearCompletions()
  {
    if (Completions.second > 0)
    {
      Logs.erase(Logs.begin() + Completions.first, Logs.begin() + Completions.second);
      Completions.second = 0;
    }
  }
  /**
   * Callback to process text editing events in console
   */
  int TextEditCallback(ImGuiInputTextCallbackData* data)
  {
    ClearCompletions();
    switch (data->EventFlag)
    {
      case ImGuiInputTextFlags_CallbackCompletion:
      {
        std::string pattern{ data->Buf };

        // Build a list of candidates
        std::vector<std::string> candidates;
        // Copy all commands that start with the pattern
        std::copy_if(Commands.begin(), Commands.end(), std::back_inserter(candidates),
          [&pattern](const std::string& s)
          {
            return s.rfind(pattern, 0) == 0; // To avoid dependency for C++20 starts_with
          });

        if (candidates.size() == 1)
        {
          // Single match. Delete the beginning of the word and replace it entirely so we've got
          // nice casing.
          data->DeleteChars(0, static_cast<int>(pattern.size()));
          data->InsertChars(data->CursorPos, candidates[0].c_str());
          data->InsertChars(data->CursorPos, " ");
        }
        else if (candidates.size() > 1)
        {
          // Multiple matches. Complete as much as we can.
          // So inputting "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as
          // matches.
          size_t match_len = pattern.size();
          bool all_candidates_matches = true;
          // Find the common prefix to all candidates
          while (all_candidates_matches)
          {
            for (auto& candidate : candidates)
            {
              if (candidate.size() <= match_len ||
                std::tolower(candidate[match_len]) != std::tolower(pattern[match_len]))
              {
                all_candidates_matches = false;
                break;
              }
            }
            if (all_candidates_matches)
            {
              match_len++;
            }
          }

          if (match_len > 0)
          {
            data->DeleteChars(0, static_cast<int>(pattern.size()));
            data->InsertChars(
              data->CursorPos, candidates[0].c_str(), candidates[0].c_str() + match_len);
          }

          Completions.first = Logs.size();
          Completions.second = Logs.size() + candidates.size() + 1;
          // Add all candidates to the logs
          this->Logs.emplace_back(std::make_pair(Internals::LogType::Log, "Possible matches:"));
          std::transform(candidates.begin(), candidates.end(), std::back_inserter(this->Logs),
            [](const std::string& candidate)
            { return std::make_pair(Internals::LogType::Log, candidate); });
        }

        break;
      }
    }
    return 0;
  }
};

vtkStandardNewMacro(vtkF3DImguiConsole);

/**
 * Callback to receive list of commands from the interactor
 */
static void OnCommandsSent(vtkObject*, unsigned long, void* clientData, void* data)
{
  std::vector<std::string>* commands = static_cast<std::vector<std::string>*>(data);
  vtkF3DImguiConsole* console = static_cast<vtkF3DImguiConsole*>(clientData);
  for (const auto& command : *commands)
  {
    console->AddCommand(command);
  }
}

//----------------------------------------------------------------------------
vtkF3DImguiConsole::vtkF3DImguiConsole()
  : Pimpl(new Internals())
{
  vtkNew<vtkCallbackCommand> keyCommandsCallback;
  keyCommandsCallback->SetClientData(this);
  keyCommandsCallback->SetCallback(OnCommandsSent);
  this->AddObserver(vtkF3DImguiConsole::CommandListEvent, keyCommandsCallback);
}

//----------------------------------------------------------------------------
vtkF3DImguiConsole::~vtkF3DImguiConsole() = default;

//----------------------------------------------------------------------------
void vtkF3DImguiConsole::DisplayText(const char* text)
{
  switch (this->GetCurrentMessageType())
  {
    case vtkOutputWindow::MESSAGE_TYPE_ERROR:
      this->Pimpl->Logs.emplace_back(std::make_pair(Internals::LogType::Error, text));
      this->Pimpl->NewError = true;
      break;
    case vtkOutputWindow::MESSAGE_TYPE_WARNING:
    case vtkOutputWindow::MESSAGE_TYPE_GENERIC_WARNING:
      this->Pimpl->Logs.emplace_back(std::make_pair(Internals::LogType::Warning, text));
      this->Pimpl->NewWarning = true;
      break;
    default:
      this->Pimpl->Logs.emplace_back(std::make_pair(Internals::LogType::Log, text));
  }

  // also print text to std::cout
  this->Superclass::DisplayText(text);
}

//----------------------------------------------------------------------------
void vtkF3DImguiConsole::ShowConsole()
{
  ImGuiViewport* viewport = ImGui::GetMainViewport();

  constexpr float margin = 30.f;

  this->Pimpl->NewError = false;
  this->Pimpl->NewWarning = false;

  ImGui::SetNextWindowPos(ImVec2(margin, margin));
  ImGui::SetNextWindowSize(
    ImVec2(viewport->WorkSize.x - 2.f * margin, viewport->WorkSize.y - 2.f * margin));
  ImGui::SetNextWindowBgAlpha(0.9f);

  ImGuiWindowFlags winFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

  // Since imgui has focus, it won't propagate the "Escape" key event to VTK
  // So let's handle the console visibility here
  if (ImGui::IsKeyPressed(ImGuiKey_Escape, false) && this->Pimpl->CurrentInput[0] == '\0')
  {
    this->InvokeEvent(vtkF3DImguiConsole::HideEvent);
  }

  ImGui::Begin("Console", nullptr, winFlags);

  // Log window
  const float reservedHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
  if (ImGui::BeginChild(
        "LogRegion", ImVec2(0, -reservedHeight), 0, ImGuiWindowFlags_HorizontalScrollbar))
  {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
    for (const auto& [severity, msg] : this->Pimpl->Logs)
    {
      bool hasColor = true;

      if (this->GetUseColoring())
      {
        switch (severity)
        {
          case Internals::LogType::Error:
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            break;
          case Internals::LogType::Warning:
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
            break;
          case Internals::LogType::Typed:
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 1.0f, 1.0f));
            break;
          default:
            hasColor = false;
        }
      }
      else
      {
        hasColor = false;
      }

      ImGui::TextUnformatted(msg.c_str());
      if (hasColor)
      {
        ImGui::PopStyleColor();
      }
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
      ImGui::SetScrollHereY(1.0f);
    }

    ImGui::PopStyleVar();
  }
  ImGui::EndChild();

  ImGui::Separator();

  // input
  ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue |
    ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion;

  ImGui::Text("> ");
  ImGui::SameLine();

  ImGui::PushItemWidth(-1);

  auto TextEditCallbackStub = [](ImGuiInputTextCallbackData* data) -> int
  {
    vtkF3DImguiConsole::Internals* internals = (vtkF3DImguiConsole::Internals*)data->UserData;
    return internals->TextEditCallback(data);
  };

  bool runCommand = ImGui::InputText("##ConsoleInput", this->Pimpl->CurrentInput.data(),
    sizeof(this->Pimpl->CurrentInput), inputFlags, TextEditCallbackStub, this->Pimpl.get());
  ImGui::PopItemWidth();

  ImGui::SetItemDefaultFocus();

  // if always forcing the focus, it prevents grabbing the scrollbar
  if (!ImGui::IsAnyItemActive())
  {
    ImGui::SetKeyboardFocusHere(-1);
  }

  // do not run the command if nothing is in the input text
  if (runCommand && this->Pimpl->CurrentInput[0] != 0)
  {
    this->Pimpl->Logs.emplace_back(std::make_pair(
      Internals::LogType::Typed, std::string("> ") + this->Pimpl->CurrentInput.data()));
    this->InvokeEvent(vtkF3DImguiConsole::TriggerEvent, this->Pimpl->CurrentInput.data());
    this->Pimpl->CurrentInput = {};
  }

  if (runCommand)
  {
    // No need to show completions after command is run
    this->Pimpl->ClearCompletions();
  }

  ImGui::End();
}

//----------------------------------------------------------------------------
void vtkF3DImguiConsole::ShowBadge()
{
  ImGuiViewport* viewport = ImGui::GetMainViewport();

  if (this->Pimpl->NewError || this->Pimpl->NewWarning)
  {
    constexpr float marginTopRight = 5.f;
    ImVec2 winSize = ImGui::CalcTextSize("!");
    winSize.x += 2.f * (ImGui::GetStyle().WindowPadding.x + ImGui::GetStyle().FramePadding.x);
    winSize.y += 2.f * (ImGui::GetStyle().WindowPadding.y + ImGui::GetStyle().FramePadding.y);

    ImGui::SetNextWindowPos(
      ImVec2(viewport->WorkSize.x - winSize.x - marginTopRight, marginTopRight));
    ImGui::SetNextWindowSize(winSize);

    ImGuiWindowFlags winFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

    ImGui::Begin("ConsoleAlert", nullptr, winFlags);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

    const bool useColoring = this->GetUseColoring();

    if (useColoring)
    {
      ImGui::PushStyleColor(ImGuiCol_Text,
        this->Pimpl->NewError ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
    }

    if (ImGui::Button("!"))
    {
      this->InvokeEvent(vtkF3DImguiConsole::ShowEvent);
    }

    ImGui::PopStyleColor(useColoring ? 2 : 1);

    ImGui::End();
  }
}

//----------------------------------------------------------------------------
void vtkF3DImguiConsole::Clear()
{
  this->Pimpl->Logs.clear();
  this->Pimpl->NewError = false;
  this->Pimpl->NewWarning = false;
}

//----------------------------------------------------------------------------
void vtkF3DImguiConsole::AddCommand(const std::string& command)
{
  this->Pimpl->Commands.push_back(command);
};