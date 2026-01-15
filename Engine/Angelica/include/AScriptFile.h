#ifndef _ASCRIPTFILE_H_
#define _ASCRIPTFILE_H_

#include "AFile.h"

class AScriptFile
{
public:
    AScriptFile() = default;
    virtual ~AScriptFile() = default;

    bool Open(AFile* file); // Open an existing script file
    void Close();           // Close file

    void ResetScriptFile();             // Reset pointers
    bool GetNextToken(bool crossLine);  // Get next token and move pointer forward
    bool PeekNextToken(bool crossLine); // Peek next token without moving pointer
    bool SkipLine();                    // Skip current line
    bool MatchToken(std::string_view token, bool caseSensitive); // Search specified token

    [[nodiscard]] bool IsEnd() const noexcept { return m_currentPos >= m_scriptContent.size(); }
    [[nodiscard]] const std::string& GetCurrentToken() const noexcept { return m_currentToken; }
    [[nodiscard]] int GetCurrentLine() const noexcept { return m_currentLine; }

private:
    std::string m_scriptContent; // Entire script content
    size_t m_currentPos = 0;     // Current position in script
    int m_currentLine = 0;       // Current line number (0-based)
    std::string m_currentToken;  // Current token (result of GetNextToken)
};

#endif
