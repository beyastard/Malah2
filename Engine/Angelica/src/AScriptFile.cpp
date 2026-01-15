#include "pch.h"
#include "AScriptFile.h"

////////////////////////////////////////////////////////////////////////////////////
//
//	Script file is pure text file without the first 4-byte flag of Angelica files. When
//	a script file is opened, it will be loaded into memory and freed when Close() is
//	called. So never forget to close a script file after you opened it.
//
//	Script file has the same comment style as C/C++, that means all content in a line
//	after // or paragraph between /* */ pair will be ignored.
//
//	Token: token is a text string. It only contain characters whose corresponding
//	ASCII codes > 32 except some special characters: , ; ( ) ", you can use these
//	characters and spece to separate tokens. Of course //, /* and */ won't be consided
//	as tokens.
//
//	For example:
//
//		abdad		is a token
//		123.22		is a token
//		[p%^@1]		is a token
//
//	A special case is, all text in a line and between "" or () will be consided as ONE
//	token. examples:
//
//	"293, is a number",	'293, is a number' will be read as a token
//	(213, 222, 10.2),	'213, 222, 10.2' will be read as a token
//	"(%$# QWE)"			'(%$# QWE)' will be read as a token
//
//	Use GetNextToken() or PeekNextToken to next token and it will be stored in
//	m_szToken of A3DScriptFile object. m_szToken is a public member so it can be accessd freely.
//
////////////////////////////////////////////////////////////////////////////////////

namespace
{
    // Helper: case-insensitive character comparison
    bool iequals(char a, char b)
    {
        return
            std::tolower(static_cast<unsigned char>(a)) ==
            std::tolower(static_cast<unsigned char>(b));
    }
}

bool AScriptFile::Open(AFile* file)
{
    if (!file)
        return false;

    // Get file size
    if (!file->Seek(0, AFILE_SEEK_END))
        return false;

    size_t fileSize = file->GetPos();
    if (!file->Seek(0, AFILE_SEEK_SET))
        return false;

    if (fileSize == 0)
    {
        m_scriptContent.clear();
        m_currentPos = 0;
        m_currentLine = 0;
        m_currentToken.clear();

        return true;
    }

    // Read entire file into string
    m_scriptContent.resize(fileSize);
    size_t bytesRead = 0;
    if (!file->Read(m_scriptContent.data(), fileSize, bytesRead) || bytesRead != fileSize)
    {
        m_scriptContent.clear();
        return false;
    }

    // Reset state
    m_currentPos = 0;
    m_currentLine = 0;
    m_currentToken.clear();

    // Restore file pointer
    file->Seek(0, AFILE_SEEK_SET);

    return true;
}

void AScriptFile::Close()
{
    m_scriptContent.clear();
    m_currentPos = 0;
    m_currentLine = 0;
    m_currentToken.clear();
}

void AScriptFile::ResetScriptFile()
{
    m_currentPos = 0;
    m_currentLine = 0;
    m_currentToken.clear();
}

bool AScriptFile::GetNextToken(bool crossLine)
{
    m_currentToken.clear();

    while (m_currentPos < m_scriptContent.size())
    {
        // Skip whitespace and separators
        while (m_currentPos < m_scriptContent.size())
        {
            char ch = m_scriptContent[m_currentPos];
            if (ch > 32 && ch != ';' && ch != ',')
                break;

            m_currentPos++;
            if (ch == '\n')
            {
                if (!crossLine)
                {
                    m_currentPos--; // Don't consume the newline
                    return false;
                }

                m_currentLine++;
            }
        }

        if (m_currentPos >= m_scriptContent.size())
            return false;

        // Skip single-line comments (//)
        if (m_currentPos + 1 < m_scriptContent.size() && m_scriptContent[m_currentPos] == '/' && m_scriptContent[m_currentPos + 1] == '/')
        {

            // Find end of line
            while (m_currentPos < m_scriptContent.size() && m_scriptContent[m_currentPos] != '\n')
                m_currentPos++;

            if (m_currentPos >= m_scriptContent.size())
                return false;

            if (!crossLine)
                return false;

            m_currentPos++; // Skip '\n'
            m_currentLine++;

            // Continue the outer loop to start parsing from the beginning of next line
            continue;
        }

        // Skip multi-line comments (/* */)
        if (m_currentPos + 1 < m_scriptContent.size() && m_scriptContent[m_currentPos] == '/' && m_scriptContent[m_currentPos + 1] == '*')
        {

            bool error = false;
            m_currentPos += 2; // Skip /*

            while (m_currentPos + 1 < m_scriptContent.size())
            {
                if (m_scriptContent[m_currentPos] == '*' && m_scriptContent[m_currentPos + 1] == '/')
                {
                    m_currentPos += 2; // Skip */
                    if (error)
                        return false;

                    break; // Continue the outer loop to start parsing from after the comment
                }

                if (m_scriptContent[m_currentPos] == '\n')
                {
                    if (!crossLine)
                        error = true;

                    m_currentLine++;
                }

                m_currentPos++;
            }

            if (m_currentPos + 1 >= m_scriptContent.size())
                return false; // Unterminated comment

            // Continue the outer loop to start parsing from after the comment
            continue;
        }

        // Parse token
        if (m_currentPos < m_scriptContent.size())
        {
            char startChar = m_scriptContent[m_currentPos];
            if (startChar == '"' || startChar == '(')
            {
                // Quoted token
                char endChar = (startChar == '"') ? '"' : ')';
                m_currentPos++; // Skip opening quote/paren

                while (m_currentPos < m_scriptContent.size())
                {
                    char ch = m_scriptContent[m_currentPos];
                    if (ch == endChar)
                    {
                        m_currentPos++; // Skip closing quote/paren
                        break;
                    }

                    m_currentToken += ch;
                    m_currentPos++;
                }
            }
            else
            {
                // Normal token
                while (m_currentPos < m_scriptContent.size())
                {
                    char ch = m_scriptContent[m_currentPos];
                    if (ch <= 32 || ch == ';' || ch == ',')
                        break;

                    m_currentToken += ch;
                    m_currentPos++;
                }
            }
        }

        return !m_currentToken.empty();
    }

    return false;
}

bool AScriptFile::PeekNextToken(bool crossLine)
{
    // Save current state
    size_t savedPos = m_currentPos;
    int savedLine = m_currentLine;
    std::string savedToken = m_currentToken;

    bool result = GetNextToken(crossLine);

    // Restore state
    m_currentPos = savedPos;
    m_currentLine = savedLine;
    m_currentToken = savedToken;

    return result;
}

bool AScriptFile::SkipLine()
{
    while (m_currentPos < m_scriptContent.size())
    {
        if (m_scriptContent[m_currentPos] == '\n')
        {
            m_currentPos++;
            m_currentLine++;

            return true;
        }

        m_currentPos++;
    }

    return true;
}

bool AScriptFile::MatchToken(std::string_view token, bool caseSensitive)
{
    while (GetNextToken(true))
    {
        if (caseSensitive)
        {
            if (m_currentToken == token)
                return true;
        }
        else
        {
            if (m_currentToken.size() == token.size() && std::equal(m_currentToken.begin(), m_currentToken.end(), token.begin(), iequals))
                return true;
        }
    }

    return false;
}
