#ifndef _ASTRINGTABLE_H_
#define _ASTRINGTABLE_H_

#include <unordered_map>

typedef struct _ASTRING_ENTRY
{
	int pEntryName; // Offset relative to the beginning of m_pCharBuffer
	int pEntryData; // Offset relative to the beginning of m_pCharBuffer
} ASTRING_ENTRY, *PASTRING_ENTRY;

class AStringTable
{
public:
    AStringTable() = default;
    ~AStringTable() = default;

    bool Init(std::wstring_view filename);
    bool Release();

    // Get entry by name (case-insensitive)
    bool GetEntry(std::wstring_view entryName, std::wstring& entryData) const;

    // Get entry by index
    bool GetEntryDataByIndex(size_t index, std::wstring& entryData) const;
    bool GetEntryNameByIndex(size_t index, std::wstring& entryName) const;

    // Add entry (name converted to uppercase)
    bool AddEntry(std::wstring_view entryName, std::wstring_view entryData);

    // Sort entries (for binary search compatibility, though not needed with map)
    bool ResortEntries();

    [[nodiscard]] size_t GetEntryCount() const noexcept { return m_entries.size(); }

protected:
    int CompareTwoEntries(size_t index1, size_t index2) const;

private:
    struct StringEntry
    {
        std::string name;
        std::string data;
    };

    std::vector<StringEntry> m_entries;
    std::unordered_map<std::string, size_t> m_nameToIndex; // For O(1) lookups
    bool m_hasSorted = false;
};

#endif
