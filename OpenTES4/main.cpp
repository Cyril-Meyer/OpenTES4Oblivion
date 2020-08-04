#include <fstream>
#include <iostream>

#include "bsa.h"

int main()
{
    std::cout << "OpenTES4Oblivion" << std::endl;

    BSA bsaMisc;
    BSA bsaMeshes;

    std::ifstream bsaFileMisc("Oblivion - Misc.bsa", std::ios::binary);

    if (bsaFileMisc.is_open()) {
        std::cout << "Oblivion - Misc.bsa" << std::endl;

        bsaFileMisc >> bsaMisc;

        //        bsaMisc.summary();

        bsaFileMisc.close();
    }

    std::ifstream bsaFileMeshes("Oblivion - Meshes.bsa", std::ios::binary);

    if (bsaFileMeshes.is_open()) {
        std::cout << "Oblivion - Meshes.bsa" << std::endl;

        bsaFileMeshes >> bsaMeshes;

        // bsaMeshes.summary();

        bsaFileMeshes.close();
    }

    return 0;
}
