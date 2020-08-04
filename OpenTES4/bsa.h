#ifndef BSA_H
#define BSA_H

#include <bitset>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

class BSA {
public:
    BSA();

    /**
     * @brief summary
     * print class information using std::cout
     */
    void summary();

    /**
     * @brief The BString struct
     * A string prefixed with a byte length. NOT zero terminated.
     */
    struct BString {
        int8_t length = 0;
        char* data = nullptr;
    };

    /**
     * @brief The BZString struct
     * A string prefixed with a byte length and terminated with a zero (\x00).
     */
    struct BZString {
        int8_t length = 0;
        char* data = nullptr;
    };

    /**
     * @brief The ZString struct
     * A string terminated with a zero (\x00).
     */
    struct ZString {
        char* data = nullptr;
    };

    struct Header {
        int8_t fileId[4];
        uint32_t version;
        uint32_t offset;
        uint32_t archiveFlags;
        uint32_t folderCount;
        uint32_t fileCount;
        uint32_t totalFolderNameLength;
        uint32_t totalFileNameLength;
        uint32_t fileFlags;
    };

    struct FolderRecord {
        uint64_t nameHash;
        uint32_t count;
        uint32_t offset;
    };

    struct FileRecord {
        uint64_t nameHash;
        uint32_t size;
        uint32_t offset;
        ZString* filename = nullptr;
    };

    struct FileRecordBlocks {
        BZString name; // Only if bit 1 of archive flags is set
        std::vector<FileRecord> fileRecords;
    };

    struct FileNameBlock {
        std::vector<ZString> fileNames;
    };

    struct FileBlock {
        BString name; // Only if bit 9 of archive flags is set
        uint32_t originalSize; // only if compressed file block (else 0)
        uint8_t* data;
    };

    Header header;
    std::vector<FolderRecord> folderRecords;
    std::vector<FileRecordBlocks> fileRecordBlocks;
    FileNameBlock fileNameBlock; // Only if bit 2 of archive flags is set
    std::vector<FileBlock> files;
};

std::istream& operator>>(std::istream& is, BSA& bsa);

#endif // BSA_H
