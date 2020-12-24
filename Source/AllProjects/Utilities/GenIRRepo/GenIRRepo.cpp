//
// FILE NAME: GenIRREpo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/08/2005
//
// COPYRIGHT: Charmed Quark Systems, Ltd @ 2020
//
//  This software is copyrighted by 'Charmed Quark Systems, Ltd' and 
//  the author (Dean Roddey.) It is licensed under the MIT Open Source 
//  license:
//
//  https://opensource.org/licenses/MIT
//
// DESCRIPTION:
//
//  This is the main file of a program that goes through the UIRT section
//  of the IR repository and, for ever UIRT IRExport file it finds, it will
//  read it in and spit out the equiv files for the other blasters that
//  we support. All of them support translation from the Pronto based
//  IR data we use in the UIRT.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CIDXML.hpp"
#include    "CQCIR.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc
(
        TThread&            thrThis
    ,   tCIDLib::TVoid*     pData
);



// ----------------------------------------------------------------------------
//  Local data
// ----------------------------------------------------------------------------
static const TString strSrcModel(L"USB-UIRT");
static const TString astrRepoDirs[] =
{
    L"GlobalCache"
    , L"R2DI"
};
static const tCIDLib::TCard4 c4RepoDirs = 2;

static tCIDLib::TCard4      c4ErrCount;
static TPathStr             pathIRRepo;
static TPathStr             pathSrcRoot;


// ----------------------------------------------------------------------------
//  Forward references
// ----------------------------------------------------------------------------
tCIDLib::TVoid GenerateFiles(TTextOutStream& strmOut, const TString& strPath);


// ---------------------------------------------------------------------------
//  Do the magic main module code
// ---------------------------------------------------------------------------
CIDLib_MainModule(TThread(L"GenIRRepo", eMainThreadFunc))



// ----------------------------------------------------------------------------
//  Program entry point
// ----------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    TTextOutStream& strmOut = TSysInfo::strmOut();
    try
    {
        strmOut << L"\nCQC IR Repository Generator\n"
                << L"Copyright (c) Charmed Quark Systems\n"
                << kCIDLib::EndLn;

        //
        //  First see if they provided a source tree command line parameter.
        //  This is the top level, under which we expect to find the IR
        //  device specific directories.
        //
        if (TSysInfo::c4CmdLineParmCount() == 1)
            pathIRRepo = TSysInfo::strCmdLineParmAt(0);

        //
        //  If we didn't get a command line parameter, then we are probably
        //  being used internally within the build system, so get the
        //  CID_SRCTREE environment variable, and build up the actual
        //  path relative to that.
        //
        if (pathIRRepo.bIsEmpty())
        {
            TPathStr  pathSrcTree;

            if (TProcEnvironment::bFind(L"CID_SRCTREE", pathSrcTree))
            {
                // We got some thing, so normalize it
                pathSrcTree.Normalize();

                //
                //  Ok, now build up the path to the IR repository. The blaster
                //  specific files are under that.
                //
                pathIRRepo = pathSrcTree;
                pathIRRepo.AddLevel(L"Source\\Miscellaneous\\IRLibraries");
                strmOut << L"Repository: " << pathIRRepo
                        << kCIDLib::NewLn << kCIDLib::EndLn;
            }
        }

        // If still nothing, then give up
        if (pathIRRepo.bIsEmpty())
        {
            strmOut << L"No working directory was provided, stopping..."
                    << kCIDLib::EndLn;
            return tCIDLib::EExitCodes::ConfigError;
        }

        // Now build up the path to the src model that we will convert from
        pathSrcRoot = pathIRRepo;
        pathSrcRoot.AddLevel(strSrcModel);

        // This source root path has to exist
        if (!TFileSys::bExists(pathSrcRoot))
        {
            strmOut << L"The source IR model directory was not found"
                    << kCIDLib::EndLn;
            return tCIDLib::EExitCodes::NotFound;
        }

        //
        //  Make sure that the top level directories for each of the blaster
        //  devices exist, else create them.
        //
        strmOut << L"Generating output for devices:\n";
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RepoDirs; c4Index++)
        {
            strmOut << L"   " << astrRepoDirs[c4Index] << kCIDLib::NewLn;
            if (!TFileSys::bExists(pathIRRepo, astrRepoDirs[c4Index]))
                TFileSys::MakeSubDirectory(astrRepoDirs[c4Index], pathIRRepo);
        }
        strmOut << kCIDLib::EndLn;

        //
        //  Now we call a recursive method that will work it's way through
        //  the UIRT part of the repository tree and convert and spit out
        //  each IR export file it finds.
        //
        //  At each level it gets the relative path, so that it's easy to
        //  build up the equiv path in the other areas.
        //
        GenerateFiles(strmOut, L"");

        if (c4ErrCount)
            strmOut << L"Errors occured, the conversion failed";
        else
            strmOut << L"The conversion completed without errors";
        strmOut << kCIDLib::DNewLn << kCIDLib::FlushIt;
    }

    // Catch any CIDLib runtime errors
    catch(const TError& errToCatch)
    {
        strmOut << L"A CIDLib runtime error occured during processing.\n"
                << L"Error: " << errToCatch << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::RuntimeError;
    }

    //
    //  Kernel errors should never propogate out of CIDLib, but test for
    //  them here just in case.
    //
    catch(const TKrnlError& kerrToCatch)
    {
        strmOut << L"A kernel error occured during processing.\nError="
                << kerrToCatch.errcId() << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::FatalError;
    }

    // Catch a general exception
    catch(...)
    {
        strmOut << L"A general exception occured during processing"
                << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::SystemException;
    }
    return tCIDLib::EExitCodes::Normal;
}


// ----------------------------------------------------------------------------
//  Helper functions
// ----------------------------------------------------------------------------

//
//  Parses a source file, leaving the result in the passed tree parser data
//  structure, which will be passed to WriteFile() below, who can pull the
//  data out and spit it out.
//
tCIDLib::TBoolean bParseSrc(        TTextOutStream& strmOut
                            , const TString&        strFile
                            ,       TXMLTreeParser& xtprsToUse)
{
    tCIDXML::TEntitySrcRef esrSrc(new TFileEntitySrc(strFile));
    const tCIDLib::TBoolean bOk = xtprsToUse.bParseRootEntity
    (
        esrSrc
        , tCIDXML::EParseOpts::None
        , tCIDXML::EParseFlags::Tags
    );

    if (!bOk)
    {
        TStreamIndentJan janIndent(&strmOut, 4);
        const TXMLTreeParser::TErrInfo& erriFirst = xtprsToUse.colErrors()[0];
        strmOut << L"\n[" << erriFirst.strSystemId() << L"/"
                << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                << L"] " << erriFirst.strText() << kCIDLib::EndLn;
        c4ErrCount++;
    }
    return bOk;
}


//
//  We get an XML tree parser object, which was filled in by a call to bParseSrc
//  above. We pull the info out and write it out to the indicated file, doing
//  the data conversion as required by the indicated model type.
//
tCIDLib::TVoid WriteFile(       TTextOutStream&     strmOut
                        , const TString&            strFile
                        ,       TXMLTreeParser&     xtprsSrc
                        , const TString&            strDevice)
{
    try
    {
        // Create an output text stream for the output file
        TTextFileOutStream strmExp
        (
            strFile
            , tCIDLib::ECreateActs::CreateAlways
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
            , tCIDLib::EAccessModes::Excl_OutStream
            , new TUTF8Converter
        );

        const TXMLTreeElement& xtnodeRoot = xtprsSrc.xtdocThis().xtnodeRoot();

        //
        //  Write out the file header. Remove the EDevCat_ prefix from the
        //  device category if it's there.
        //
        TString strCat = xtnodeRoot.xtattrNamed(L"CQCIR:Category").strValue();
        if (strCat.bStartsWithI(L"EDevCat_"))
            strCat.Cut(0, 8);
        const TString& strDescr = xtnodeRoot.xtattrNamed(L"CQCIR:Description").strValue();
        const TString& strMake = xtnodeRoot.xtattrNamed(L"CQCIR:Make").strValue();
        const TString& strModel = xtnodeRoot.xtattrNamed(L"CQCIR:Model").strValue();
        facCQCIR().WriteExpHeader(strmExp, strDevice, strMake, strModel, strDescr, strCat);

        //
        //  Now iterate through the commands and spit them out. For each one
        //  we get out the key and value, convert the value, then write it
        //  out. We set up the parser to only returns tags, so we don't have
        //  to check the types of nodes as we iterate. They will only be element
        //  nodes.
        //
        const TXMLTreeElement& xtnodeCmds
                                = xtnodeRoot.xtnodeChildAtAsElement(0);
        const tCIDLib::TCard4 c4Count = xtnodeCmds.c4ChildCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TXMLTreeElement& xtnodeCur
                                = xtnodeCmds.xtnodeChildAtAsElement(c4Index);

            const TString& strKey = xtnodeCur.xtattrNamed(L"CQCIR:CmdName").strValue();
            const TString& strValue = xtnodeCur.xtattrNamed(L"CQCIR:Data").strValue();

            strmExp << L"      <CQCIR:Cmd CQCIR:CmdName='" << strKey
                    << L"'\n                 CQCIR:Data='";

            // Write out the value in the correctly translated format
            try
            {
                if (strDevice == L"GlobalCache")
                {
                    facCQCIR().ProntoToGC100(strValue, strmExp);
                }
                 else if (strDevice == L"R2DI")
                {
                    facCQCIR().ProntoToR2DI(strValue, strmExp);
                }
                 else
                {
                    strmOut << L"Unknown device type!" << kCIDLib::EndLn;
                    TProcess::ExitProcess(tCIDLib::EExitCodes::RuntimeError);
                }
            }

            catch(const TError& errToCatch)
            {
                c4ErrCount++;
                strmOut << L"  " << errToCatch.strErrText() << kCIDLib::EndLn;
            }

            strmExp << L"'/>\n";
        }

        // Write out the file footer
        facCQCIR().WriteExpFooter(strmExp, strDevice, strMake, strModel);
    }

    catch(const TError& errToCatch)
    {
        {
            TStreamIndentJan janIndent(&strmOut, 4);
            strmOut << L"\nError processing file:\n" << errToCatch;
        }
        strmOut << kCIDLib::EndLn;
    }
}


//
//  This method is a recursive one, which is started off at the root of the
//  UIRT part of the tree. It recurses down into all the directories of the
//  UIRT section. For each file, we build up the equiv path into each of the
//  other blaster parts of the tree, and generate the equiv files in those
//  locations.
//
tCIDLib::TVoid GenerateFiles(TTextOutStream& strmOut, const TString& strRelPath)
{
    TPathStr        pathTmp;
    TPathStr        pathSrcFile;
    TDirIter        diterLevel;
    TFindBuf        fndbSearch;
    TXMLTreeParser  xtprsSrc;


    // If no files at this level, then we are done
    pathTmp = pathSrcRoot;
    pathTmp.AddLevel(strRelPath);
    if (!diterLevel.bFindFirst( pathTmp
                                , kCIDLib::pszAllFilesSpec
                                , fndbSearch
                                , tCIDLib::EDirSearchFlags::All))
    {
        return;
    }

    do
    {
        // If it's a dir, then recurse first
        if (fndbSearch.bIsDirectory())
        {
            //
            //  We need to generate this same directory in each of the
            //  other sections, if they don't already exist.
            //
            fndbSearch.pathFileName().bRemovePath();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RepoDirs; c4Index++)
            {
                pathTmp = pathIRRepo;
                pathTmp.AddLevel(astrRepoDirs[c4Index]);
                pathTmp.AddLevel(strRelPath);

                if (!TFileSys::bExists(pathTmp, fndbSearch.pathFileName()))
                    TFileSys::MakeSubDirectory(fndbSearch.pathFileName(), pathTmp);
            }

            // Create the src relative path to pass to the recursion
            pathTmp = strRelPath;
            pathTmp.AddLevel(fndbSearch.pathFileName());
            GenerateFiles(strmOut, pathTmp);
        }

        // If it's a file, then we need to process it if it's an .IRExport
        fndbSearch.pathFileName().bQueryExt(pathTmp);
        if (fndbSearch.bIsNormalFile() && (pathTmp == L"IRExport"))
        {
            fndbSearch.pathFileName().bRemovePath();
            strmOut << L"    Parsing file: " << fndbSearch.pathFileName()
                    << kCIDLib::EndLn;

            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RepoDirs; c4Index++)
            {
                // Create the source file name
                pathSrcFile = pathSrcRoot;
                pathSrcFile.AddLevel(strRelPath);
                pathSrcFile.AddLevel(fndbSearch.pathFileName());

                // Create the target file name
                pathTmp = pathIRRepo;
                pathTmp.AddLevel(astrRepoDirs[c4Index]);
                pathTmp.AddLevel(strRelPath);
                pathTmp.AddLevel(fndbSearch.pathFileName());

                // Parse the input file. If it works, spit out the output
                if (bParseSrc(strmOut, pathSrcFile, xtprsSrc))
                    WriteFile(strmOut, pathTmp, xtprsSrc, astrRepoDirs[c4Index]);
            }
        }
    }   while (diterLevel.bFindNext(fndbSearch));
}

