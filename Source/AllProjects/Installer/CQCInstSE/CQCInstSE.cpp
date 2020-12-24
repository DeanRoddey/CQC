//
// COPYRIGHT: Charmed Quark Systems, Ltd @ 2020
//
//  This software is copyrighted by 'Charmed Quark Systems, Ltd' and
//  the author (Dean Roddey.) It is licensed under the MIT Open Source
//  license:
//
//  https://opensource.org/licenses/MIT
//
#include <windows.h>
#include <Shellapi.h>
#include <iostream>
#include <fstream>

// We need to pull in the shell API for a directory removal call
#pragma comment(lib, "Shell32.lib")


//
//  The list of files and their resource ids. THIS MUST BE kept in sync with the
//  resources in the .RC file.
//
//  The DLLs and pack are version stamped and we get the version defined on the
//  compiler command line, CQC_VERSION and CQC_FULL_VERSION, so it will update
//  the list during compilation correctly to match the version being built.
//
//  We have to play some tracks with stringizing and concatenating macro values
//  with strings in order to build the names. So we define a few macros to do this.
//
//  The first just concatenates three parameters. The second takes the prefix,
//  version value (which it stringizes), and suffix and passes them to the first
//  one. The last one takes those same three values, but we need it so that
//  the version macro is evaluated before it's passed to the 2nd one. So what
//  gets stringized by the 2nd ones is the expanded value, not the name of the
//  macro value.
//
struct TAFile
{
    unsigned            ridLoad;
    const char*         pszName;
};

#define ConcatStrs(f,s,t) f##s##t
#define ConcatStrs2(p,v,s) ConcatStrs(p,#v,s)
#define MakeVerName(p,v,s) ConcatStrs2(p,v,s)

TAFile aFiles[] =
{
    // The EXE MUST be first
    { 1     , "CQCInstUnp.exe" }

  , { 2     , "CIDCommonText_en.CIDMsg" }
  , { 3     , MakeVerName("CIDCrypto_", CQC_VERSION, ".dll") }
  , { 4     , MakeVerName("CIDCrypto_", CQC_VERSION, "_en.CIDMsg") }
  , { 5     , MakeVerName("CIDCtrls_", CQC_VERSION, ".CIDRes") }
  , { 6     , MakeVerName("CIDCtrls_", CQC_VERSION, ".dll") }
  , { 7     , MakeVerName("CIDCtrls_", CQC_VERSION, "_en.CIDMsg") }
  , { 8     , MakeVerName("CIDImage_", CQC_VERSION, ".dll") }
  , { 9     , MakeVerName("CIDImage_", CQC_VERSION, "_en.CIDMsg") }
  , { 10    , MakeVerName("CIDImgFact_", CQC_VERSION, ".dll") }
  , { 11    , MakeVerName("CIDImgFact_", CQC_VERSION, "_en.CIDMsg") }
  , { 12    , MakeVerName("CIDJPEG_", CQC_VERSION, ".dll") }
  , { 13    , MakeVerName("CIDJPEG_", CQC_VERSION, "_en.CIDMsg") }
  , { 14    , MakeVerName("CIDKernel_", CQC_VERSION, ".dll") }
  , { 15    , MakeVerName("CIDKernel_", CQC_VERSION, "_en.CIDMsg") }
  , { 16    , MakeVerName("CIDLib_", CQC_VERSION, ".dll") }
  , { 17    , MakeVerName("CIDLib_", CQC_VERSION, "_en.CIDMsg") }
  , { 18    , MakeVerName("CIDMath_", CQC_VERSION, ".dll") }
  , { 19    , MakeVerName("CIDMath_", CQC_VERSION, "_en.CIDMsg") }
  , { 20    , MakeVerName("CIDPack_", CQC_VERSION, ".dll") }
  , { 21    , MakeVerName("CIDPack_", CQC_VERSION, "_en.CIDMsg") }
  , { 22    , MakeVerName("CIDPNG_", CQC_VERSION, ".dll") }
  , { 23    , MakeVerName("CIDPNG_", CQC_VERSION, "_en.CIDMsg") }
  , { 24    , MakeVerName("CIDGraphDev_", CQC_VERSION, ".CIDRes") }
  , { 25    , MakeVerName("CIDGraphDev_", CQC_VERSION, ".dll") }
  , { 26    , MakeVerName("CIDGraphDev_", CQC_VERSION, "_en.CIDMsg") }
  , { 27    , MakeVerName("CIDZLib_", CQC_VERSION, ".dll") }
  , { 28    , MakeVerName("CIDZLib_", CQC_VERSION, "_en.CIDMsg") }
  , { 29    , MakeVerName("CIDMData_", CQC_VERSION, ".dll") }
  , { 30    , MakeVerName("CIDMData_", CQC_VERSION, "_en.CIDMsg") }
  , { 31    , "CQCApp.ico" }
  , { 32    , MakeVerName("CQCImage_", CQC_FULL_VERSION, ".CIDPack") }
  , { 33    , "CQCInstUnp.CIDRes" }
  , { 34    , "CQCInstUnp_en.CIDMsg" }

  #if     defined(_DEBUG)
  , { 35    , "vcruntime140d.dll" }
  #else
  , { 35    , "vcruntime140.dll" }
  #endif

    // End of list indicator
  , { 0     , 0 }
};

int main()
{
    std::cout   << "CQC Self-Extractor\n"
                   "Copyright (r) Charmed Quark Systems, Ltd\n\n" << std::flush;

    //
    //  Get a temporary directory to extract to, make it one larger than max path
    //  so that we can double terminate it below if needed for the shell op.
    //
    char szPath[MAX_PATH + 1];
    if (!::GetTempPath(MAX_PATH, szPath))
    {
        std::cout << "Could not get a temporary path to extract to\n\n"
                     "Press enter to close";

        std::cin.get();
        return 1;
    }

    // Create a subdirectory beneath it if not already there
    strcat_s(szPath, MAX_PATH, "CQCInstUnp");

    // See if this target directory exists
    WIN32_FIND_DATA FileData;
    HANDLE hFind = ::FindFirstFile(szPath, &FileData);

    // Remember if we found it, and then close the search handle
    const bool bTarExists(hFind != (HANDLE)-1);
    ::FindClose(hFind);

    // If it does exist, we want to remove it first, to make sure we start fresh
    if (bTarExists)
    {
        //
        //  The target directory has to be doubly null terminated since it can
        //  hold multiple strings in this call, so add a second null to the target
        //  path string. It won't hurt anything, since everyone else will see the
        //  first one and be happy.
        //
        szPath[strlen(szPath) + 1] = 0;

        // Fill in the file ops structure. Make it a totally silent operation
        SHFILEOPSTRUCT FileOp =
        {
            NULL
            , FO_DELETE
            , szPath
            , NULL
            , FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT
            , 0
            , 0
            , ""
        };

        if (::SHFileOperation(&FileOp))
        {
            std::cout << "The target directory could not be cleaned out. Aborting...\n\n"
                            "Press enter to close";
            std::cin.get();
            return 1;
        }
    }

    // And now create (or recreate) the directory
    if (!::CreateDirectory(szPath, 0))
    {
        std::cout << "Could not create the temporary directory to extract into\n\n"
                     "Press enter to close";
        std::cin.get();
        return 1;
    }


    //
    //  And one for app images under that. We don't have to look for this one
    //  one first, since we know we cleaned out the top level directory above.
    //  So we always have to create this one.
    //
    {
        char szTmpPath[MAX_PATH];
        strcpy_s(szTmpPath, MAX_PATH, szPath);
        strcat_s(szTmpPath, MAX_PATH, "\\AppImages");

        if (!::CreateDirectory(szTmpPath, 0))
        {
            std::cout << "Could not create the temporary app img directory...\n\n"
                         "Press enter to close";
            std::cin.get();
            return 1;
        }
    }

    // Looks like we are ok, so extract the files
    std::cout   << "\nExtracting installer files...\n  Target Path="
                << szPath << "\n\n" << std::flush;

    int index = 0;
    char szCurFile[MAX_PATH];
    while(aFiles[index].pszName)
    {
        std::cout << "  Extracting: " << aFiles[index].pszName << "\n";

        // Get the binary resource info for this file
        HRSRC hCur = ::FindResource(NULL, MAKEINTRESOURCE(aFiles[index].ridLoad), RT_RCDATA);
        if (!hCur)
        {
            std::cout << "Could not create extract resource: " << aFiles[index].pszName
                      << "\n\nPress enter to close";
            std::cin.get();
            break;
        }

        // Get access to the raw data of the resource and its size
        unsigned int szCur = ::SizeofResource(NULL, hCur);
        HGLOBAL hCurData = ::LoadResource(NULL, hCur);
        void* pCurData = ::LockResource(hCurData);

        // Create an output binary file for this guy and write the bytes
        strcpy_s(szCurFile, szPath);
        strcat_s(szCurFile, "\\");
        strcat_s(szCurFile, aFiles[index].pszName);

        std::ofstream outfl(szCurFile, std::ios::binary);
        outfl.write((const char*)pCurData, szCur);
        outfl.close();

        index++;
    }

    // The first one has to be the exectuable, so let's run that one
    strcpy_s(szCurFile, szPath);
    strcat_s(szCurFile, "\\");
    strcat_s(szCurFile, aFiles[0].pszName);

    PROCESS_INFORMATION ProcInfo = {0};
    STARTUPINFO Startup = {0};
    Startup.cb = sizeof(STARTUPINFO);

    if (!::CreateProcess
    (
        szCurFile
        , ""
        , 0
        , 0
        , 0
        , 0
        , 0
        , szPath
        , &Startup
        , &ProcInfo))
    {
        std::cout << "The extracted CQC installer package could not be run\n\n"
                     "Press enter to close";
        std::cin.get();
        return 1;
    }

    return 0;
}

