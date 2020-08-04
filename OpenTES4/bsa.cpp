#include "bsa.h"

BSA::BSA() {}

void BSA::summary()
{
    std::cout << "header" << std::endl;
    std::cout << this->header.fileId << std::endl;
    std::cout << this->header.version << std::endl;
    std::cout << this->header.offset << std::endl;
    std::cout << this->header.archiveFlags << std::endl;
    std::cout << this->header.folderCount << std::endl;
    std::cout << this->header.fileCount << std::endl;
    std::cout << this->header.totalFolderNameLength << std::endl;
    std::cout << this->header.totalFileNameLength << std::endl;
    std::cout << this->header.fileFlags << std::endl;

    std::cout << "folderRecords" << std::endl;
    for (std::size_t i = 0; i < this->header.folderCount; ++i) {
        std::cout << this->folderRecords[i].nameHash << std::endl;
        std::cout << this->folderRecords[i].count << std::endl;
        std::cout << this->folderRecords[i].offset << std::endl;
    }

    std::cout << "fileRecordBlocks" << std::endl;
    std::bitset<32> archiveFlags(this->header.archiveFlags);
    for (std::size_t i = 0; i < this->fileRecordBlocks.size(); ++i) {

        if (archiveFlags.test(0)) {
            std::cout << this->fileRecordBlocks[i].name.data << std::endl;
        }

        for (std::size_t j = 0;
             j < this->fileRecordBlocks[i].fileRecords.size(); ++j) {
            std::cout << "    "
                      << this->fileRecordBlocks[i].fileRecords[j].nameHash
                      << std::endl;
            std::cout << "    " << this->fileRecordBlocks[i].fileRecords[j].size
                      << std::endl;
            std::cout << "    "
                      << this->fileRecordBlocks[i].fileRecords[j].offset
                      << std::endl;
        }
    }

    std::cout << "fileNameBlock" << std::endl;
    if (archiveFlags.test(1)) {
        for (std::size_t i = 0; i < this->header.fileCount; i++) {
            std::cout << this->fileNameBlock.fileNames[i].data << std::endl;
        }
    }
}

std::istream& operator>>(std::istream& is, BSA& bsa)
{
    /* lambda to read datasize byte into dataptr
     * usage :
     * read(is, &int32, sizeof(4));
     * read(is, arrayptr, arraysize);
     */
    auto read = [](std::istream& is, void* dataptr, uint64_t datasize) {
        is.read(reinterpret_cast<char*>(dataptr), datasize);
    };

    // header
    read(is, &bsa.header.fileId, sizeof(bsa.header.fileId));
    read(is, &bsa.header.version, sizeof(bsa.header.version));

    read(is, &bsa.header.offset, sizeof(bsa.header.offset));
    read(is, &bsa.header.archiveFlags, sizeof(bsa.header.archiveFlags));
    read(is, &bsa.header.folderCount, sizeof(bsa.header.folderCount));
    read(is, &bsa.header.fileCount, sizeof(bsa.header.fileCount));
    read(is, &bsa.header.totalFolderNameLength,
        sizeof(bsa.header.totalFolderNameLength));
    read(is, &bsa.header.totalFileNameLength,
        sizeof(bsa.header.totalFileNameLength));
    read(is, &bsa.header.fileFlags, sizeof(bsa.header.fileFlags));

    std::bitset<32> archiveFlags(bsa.header.archiveFlags);

    // folderRecords
    for (std::size_t i = 0; i < bsa.header.folderCount; ++i) {
        BSA::FolderRecord fr;
        read(is, &fr.nameHash, sizeof(fr.nameHash));
        read(is, &fr.count, sizeof(fr.count));
        read(is, &fr.offset, sizeof(fr.offset));
        bsa.folderRecords.push_back(fr);
    }

    // Check if fileCount and folderRecord number of files match.
    uint32_t frfc = 0;
    for (std::size_t i = 0; i < bsa.header.folderCount; ++i) {
        frfc += bsa.folderRecords[i].count;
    }

    if (frfc != bsa.header.fileCount) {
        std::cout << "ERROR : fileCount and folderRecord number of files "
                     "doesn't match"
                  << std::endl;
        return is;
    }

    // fileRecordBlocks
    for (uint32_t i = 0; i < bsa.header.folderCount; ++i) {
        BSA::FileRecordBlocks frb;

        BSA::BZString bzs;
        if (archiveFlags.test(0)) {
            read(is, &bzs.length, sizeof(bzs.length));
            bzs.data = new char[bzs.length];

            read(is, bzs.data, bzs.length);

            // Check valid BZString
            if (bzs.data[bzs.length - 1] != '\0') {
                std::cout << "ERROR : invalid bzstring" << std::endl;
                return is;
            }

        } else {
            bzs.length = 0;
            bzs.data = nullptr;
        }
        frb.name = bzs;

        for (uint32_t j = 0; j < bsa.folderRecords[i].count; ++j) {
            BSA::FileRecord fr;

            read(is, &fr.nameHash, sizeof(fr.nameHash));
            read(is, &fr.size, sizeof(fr.size));
            read(is, &fr.offset, sizeof(fr.offset));

            frb.fileRecords.push_back(fr);
        }

        bsa.fileRecordBlocks.push_back(frb);
    }

    // fileNameBlock
    if (archiveFlags.test(1)) {
        // bit 2 of archiveFlags is set
        // list of lower case file names
        std::string tmpstr;
        for (uint32_t i = 0; i < bsa.header.fileCount; i++) {
            getline(is, tmpstr, '\0');

            BSA::ZString zs;
            zs.data = new char[tmpstr.length()];

            for (std::size_t j = 0; j <= tmpstr.length(); j++) {
                zs.data[j] = tmpstr[j];
            }

            bsa.fileNameBlock.fileNames.push_back(zs);
        }
    }

    // Check if fileRecords and fileNameBlock number of filename match.
    if (archiveFlags.test(1)) {
        std::size_t frbfrs = 0;
        for (std::size_t i = 0; i < bsa.fileRecordBlocks.size(); ++i)
            frbfrs += bsa.fileRecordBlocks.at(i).fileRecords.size();

        if (frbfrs != bsa.fileNameBlock.fileNames.size()) {
            std::cout << "ERROR : fileRecords and fileNameBlock number of "
                         "files doesn't match"
                      << std::endl;
            return is;
        }
    }

    // link fileRecords and fileNameBlock
    if (archiveFlags.test(1)) {
        std::size_t c = 0;
        for (std::size_t i = 0; i < bsa.fileRecordBlocks.size(); ++i) {
            for (std::size_t j = 0;
                 j < bsa.fileRecordBlocks.at(i).fileRecords.size(); ++j) {
                bsa.fileRecordBlocks.at(i).fileRecords.at(j).filename
                    = &bsa.fileNameBlock.fileNames.at(c);
                c++;
            }
        }
    }

    // Read Compressed File block and Uncompressed File block

    return is;
}
