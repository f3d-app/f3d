#include "vtkF3DImguiConsole.h"

#include "F3DImguiStyle.h"

#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

#include <imgui.h>

#include <algorithm>
#include <array>
#include <cstdint>

struct vtkF3DImguiConsole::Internals
{
  enum class LogType : std::uint8_t
  {
    Log,
    Warning,
    Error,
    Typed,
    Completion
  };

  std::vector<std::pair<LogType, std::string>> Logs;
  std::array<char, 2048> CurrentInput = {};
  bool NewError = false;
  bool NewWarning = false;
  std::pair<size_t, size_t> Completions{ 0,
    0 }; // Index for start and length of completions in Logs
  std::function<std::vector<std::string>(const std::string& pattern)>
    CompletionCallback; // Callback to get the list of commands matching pattern
  std::vector<std::string> CommandHistory;
  std::pair<std::string, int> LastInput; // Last input before navigating history
  int CommandHistoryIndexInv = -1;       // Current inverted index in command history navigation

  /**
   * Clear completions from the logs
   */
  void ClearCompletions()
  {
    if (this->Completions.second > 0)
    {
      this->Logs.erase(this->Logs.begin() + this->Completions.first,
        this->Logs.begin() + this->Completions.second);
      this->Completions.second = 0;
    }
  }
  /**
   * Callback to process text editing events in console
   */
  int TextEditCallback(ImGuiInputTextCallbackData* data)
  {
    this->ClearCompletions();
    switch (data->EventFlag)
    {
      case ImGuiInputTextFlags_CallbackCompletion:
      {
        assert(this->CompletionCallback);
        std::string pattern{ data->Buf };
        std::vector<std::string> candidates =
          this->CompletionCallback(pattern); // List of candidates completion

        if (candidates.size() == 1)
        {
          // Single match. Delete the beginning of the word and replace it entirely so we've got
          // nice casing.
          data->DeleteChars(0, static_cast<int>(pattern.size()));
          data->InsertChars(data->CursorPos, candidates[0].c_str());
        }
        else if (candidates.size() > 1)
        {
          // Multiple matches. Complete as much as we can.
          // So inputting "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as
          // matches.
          size_t matchLen = 0;
          bool allCandidatesMatches = true;
          // Find the common prefix to all candidates
          while (allCandidatesMatches)
          {
            const std::string& first = candidates[0];
            if (first.size() <= matchLen)
            {
              // The first candidate is shorter than the current match length
              allCandidatesMatches = false;
            }
            else
            {
              // Check if all candidates match the current character
              const char target = first[matchLen];
              allCandidatesMatches = std::all_of(candidates.begin(), candidates.end(),
                [matchLen, target](const std::string& s)
                { return s.size() > matchLen && s[matchLen] == target; });
            }
            if (allCandidatesMatches)
            {
              matchLen++;
            }
          }

          if (matchLen > 0)
          {
            // Fill the best we can by now - use the longest common prefix from available candidates
            // (possibly just pattern itself in the worst case)
            data->DeleteChars(0, static_cast<int>(pattern.size()));
            data->InsertChars(
              data->CursorPos, candidates[0].c_str(), candidates[0].c_str() + matchLen);
          }

          this->Completions.first = this->Logs.size();
          this->Completions.second = this->Logs.size() + candidates.size() + 1;
          // Add all candidates to the logs
          this->Logs.emplace_back(
            std::make_pair(Internals::LogType::Completion, "Possible matches:"));
          std::transform(candidates.begin(), candidates.end(), std::back_inserter(this->Logs),
            [](const std::string& candidate)
            { return std::make_pair(Internals::LogType::Completion, candidate); });
        }
        break;
      }
      case ImGuiInputTextFlags_CallbackHistory:
      {
        /* CommandHistoryIndexInv is a reversed index for command history:
        - `-1` represents the current user input (not yet stored in history).
        - `0` corresponds to the most recent command (CommandHistory.size() - 1).
        - `CommandHistory.size() - 1` maps to the oldest command (0 in CommandHistory). */
        const int prevHistoryPos = this->CommandHistoryIndexInv;
        if (prevHistoryPos == -1)
        {
          /* Saving the last input before history navigation */
          this->LastInput = { this->CurrentInput.data(), data->CursorPos };
        }
        const int histSize = static_cast<int>(this->CommandHistory.size());
        if (data->EventKey == ImGuiKey_UpArrow && this->CommandHistoryIndexInv < (histSize - 1))
        {
          this->CommandHistoryIndexInv++;
        }
        else if (data->EventKey == ImGuiKey_DownArrow && this->CommandHistoryIndexInv >= 0)
        {
          this->CommandHistoryIndexInv--;
        }

        if (prevHistoryPos != this->CommandHistoryIndexInv)
        {
          if (this->CommandHistoryIndexInv == -1)
          {
            /* Restoring the last input when navigated back to it */
            data->DeleteChars(0, data->BufTextLen);
            data->InsertChars(0, this->LastInput.first.c_str());
            data->CursorPos = this->LastInput.second;
          }
          else
          {
            /* We should not be able to have negative index here */
            /* Retrieve the command from history */
            std::string historyStr =
              this->CommandHistory[histSize - this->CommandHistoryIndexInv - 1];
            data->DeleteChars(0, data->BufTextLen);
            data->InsertChars(0, historyStr.c_str());
            data->CursorPos = static_cast<int>(historyStr.size());
          }
        }
      }
    }
    return 0;
  }
};

vtkStandardNewMacro(vtkF3DImguiConsole);

//----------------------------------------------------------------------------
vtkF3DImguiConsole::vtkF3DImguiConsole()
  : Pimpl(new Internals())
{
}

//----------------------------------------------------------------------------
vtkF3DImguiConsole::~vtkF3DImguiConsole() = default;

//----------------------------------------------------------------------------
void vtkF3DImguiConsole::DisplayText(const char* text)
{
  MessageTypes type = this->GetCurrentMessageType();
  if (this->GetDisplayStream(type) != StreamType::Null)
  {
    switch (type)
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
  }

  // also print text to std::cout
  this->Superclass::DisplayText(text);
}

//----------------------------------------------------------------------------
void vtkF3DImguiConsole::ShowConsole(bool minimal)
{
  const ImGuiViewport* viewport = ImGui::GetMainViewport();

  constexpr float margin = F3DImguiStyle::GetDefaultMargin();
  const float padding = ImGui::GetStyle().WindowPadding.x + ImGui::GetStyle().FramePadding.x;
  float windowWidth = viewport->WorkSize.x - 2.f * margin;

  ImGui::SetNextWindowPos(ImVec2(margin, margin));
  // explicitly calculate size of minimal console to avoid extra flashing frame
  if (minimal)
  {
    if (this->Pimpl->NewError || this->Pimpl->NewWarning)
    {
      // prevent overlap with console badge in minimal console
      const ImVec2 badgeSize = this->GetBadgeSize();
      windowWidth = viewport->WorkSize.x - badgeSize.x - 3.f * margin;
    }
    ImGui::SetNextWindowSize(ImVec2(windowWidth, ImGui::CalcTextSize(">").y + 2.f * padding));
  }
  else
  {
    // minimal console shouldn't clear console badge
    this->Pimpl->NewError = false;
    this->Pimpl->NewWarning = false;

    ImGui::SetNextWindowSize(ImVec2(windowWidth, viewport->WorkSize.y - 2.f * margin));
  }

  ImGui::SetNextWindowBgAlpha(0.9f);

  ImGuiWindowFlags winFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

  // Since imgui has focus, it won't propagate the "Escape" key event to VTK
  // So let's handle the console visibility here
  if (ImGui::IsKeyPressed(ImGuiKey_Escape, false) && this->Pimpl->CurrentInput[0] == '\0')
  {
    this->Pimpl->CommandHistoryIndexInv = -1; // Reset history navigation on hiding
    this->Pimpl->ClearCompletions();          // Clear completion on hiding
    this->InvokeEvent(vtkF3DImguiConsole::HideEvent);
  }

  ImGui::Begin("Console", nullptr, winFlags);

  // Log window, will only show if not in minimal mode
  if (!minimal)
  {
    const float reservedHeight =
      ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
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
              ImGui::PushStyleColor(ImGuiCol_Text, F3DImguiStyle::GetErrorColor());
              break;
            case Internals::LogType::Warning:
              ImGui::PushStyleColor(ImGuiCol_Text, F3DImguiStyle::GetWarningColor());
              break;
            case Internals::LogType::Typed:
              ImGui::PushStyleColor(ImGuiCol_Text, F3DImguiStyle::GetHighlightColor());
              break;
            case Internals::LogType::Completion:
              ImGui::PushStyleColor(ImGuiCol_Text, F3DImguiStyle::GetCompletionColor());
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
  }

  // input
  ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue |
    ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion |
    ImGuiInputTextFlags_CallbackHistory;

  ImGui::Text(">");
  ImGui::SameLine();

  ImGui::PushItemWidth(-1);

  auto TextEditCallbackStub = [](ImGuiInputTextCallbackData* data) -> int
  {
    vtkF3DImguiConsole::Internals* internals = (vtkF3DImguiConsole::Internals*)data->UserData;
    return internals->TextEditCallback(data);
  };

  bool runCommand = ImGui::InputTextWithHint("##ConsoleInput", "Type a command...",
    this->Pimpl->CurrentInput.data(), sizeof(this->Pimpl->CurrentInput), inputFlags,
    TextEditCallbackStub, this->Pimpl.get());
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
    this->Pimpl->CommandHistory.emplace_back(this->Pimpl->CurrentInput.data());
    this->Pimpl->CommandHistoryIndexInv = -1; // Reset history navigation, looks natural
    this->Pimpl->CurrentInput = {};
  }

  if (runCommand)
  {
    // No need to show completions after command is run
    this->Pimpl->ClearCompletions();

    // exit console immediately after running command if in minimal mode
    if (minimal)
    {
      this->InvokeEvent(vtkF3DImguiConsole::HideEvent);
    }
  }

  ImGui::End();
}

//----------------------------------------------------------------------------
void vtkF3DImguiConsole::ShowBadge()
{
  const ImGuiViewport* viewport = ImGui::GetMainViewport();

  if (this->Pimpl->NewError || this->Pimpl->NewWarning)
  {
    constexpr float margin = F3DImguiStyle::GetDefaultMargin();
    ImVec2 badgeSize = this->GetBadgeSize();

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkSize.x - badgeSize.x - margin, margin));
    ImGui::SetNextWindowSize(badgeSize);
    ImGui::SetNextWindowBgAlpha(0.9f);

    ImGuiWindowFlags winFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

    ImGui::Begin("ConsoleAlert", nullptr, winFlags);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, F3DImguiStyle::GetHighlightColor());

    ImGui::PushStyleColor(ImGuiCol_Text,
      this->Pimpl->NewError ? F3DImguiStyle::GetErrorColor() : F3DImguiStyle::GetWarningColor());

    if (ImGui::Button("!"))
    {
      this->InvokeEvent(vtkF3DImguiConsole::ShowEvent);
    }

    ImGui::PopStyleColor(3);

    ImGui::End();
  }
}

//----------------------------------------------------------------------------
ImVec2 vtkF3DImguiConsole::GetBadgeSize()
{
  const float padding = ImGui::GetStyle().WindowPadding.x + ImGui::GetStyle().FramePadding.x;
  ImVec2 badgeSize = ImGui::CalcTextSize("!");
  badgeSize.x += 2.f * padding;
  badgeSize.y += 2.f * padding;
  return badgeSize;
}

//----------------------------------------------------------------------------
void vtkF3DImguiConsole::Clear()
{
  this->Pimpl->Logs.clear();
  this->Pimpl->NewError = false;
  this->Pimpl->NewWarning = false;
}

//----------------------------------------------------------------------------
void vtkF3DImguiConsole::SetCompletionCallback(
  std::function<std::vector<std::string>(const std::string& pattern)> callback)
{
  this->Pimpl->CompletionCallback = std::move(callback);
}
