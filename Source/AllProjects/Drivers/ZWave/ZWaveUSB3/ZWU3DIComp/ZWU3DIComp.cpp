//
// FILE NAME: ZWU3DIComp.Cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/11/2017
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
//  This is the main module for a little utility that is used by the installer image
//  builder. It parses all of our Z-Wave device info XML files and builds up a new
//  output XML file. That file is an index that provides just the basic info required
//  to let the user browse the available devices on the client side, and to map from
//  the manufacturer ids to the appropriate file on the server side.
//
//  This can also of course be used to test all of the files after making any changes,
//  and to rebuild the index.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//


// ----------------------------------------------------------------------------
//  The driver shared facility brings in anything we need
// ----------------------------------------------------------------------------
#include    "ZWaveUSB3Sh.hpp"


// ----------------------------------------------------------------------------
//  Forward references
// ----------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThread(TThread&, tCIDLib::TVoid*);


// ----------------------------------------------------------------------------
//  Include magic main module code
// ----------------------------------------------------------------------------
CIDLib_MainModule(TThread(L"ZWU3DICompThread", eMainThread))



// ----------------------------------------------------------------------------
//  A simple class we'll use to store the index info we want to keep from each
//  file.
// ----------------------------------------------------------------------------
class TIndexInfo
{
    public :
        TIndexInfo() : c8ManIds(0) {}
        ~TIndexInfo() {}

        tCIDLib::TCard8     c8ManIds;
        tCIDLib::TStrList   colClasses;
        tCIDLib::TBoolList  fcolSecure;
        tCIDLib::TCardList  fcolVersions;
        TString             strCppClass;
        TString             strFile;
        TString             strInstruct;
        TString             strMake;
        TString             strModel;
        TString             strName;;
};

static tCIDLib::ESortComps eCompFunc(const TIndexInfo& ii1, const TIndexInfo& ii2)
{
    // Sort first by make
    tCIDLib::ESortComps eRes = ii1.strMake.eCompareI(ii2.strMake);

    // If that's the same, compare by the model
    if (eRes == tCIDLib::ESortComps::Equal)
        eRes  = ii1.strModel.eCompareI(ii2.strModel);

    return eRes;
}
static tCIDLib::TBoolean bIgnoreDeleted = kCIDLib::True;
enum class EParseRes
{
    Error
    , Keep
    , Discard
};


// ----------------------------------------------------------------------------
//  Global data
// ----------------------------------------------------------------------------
TOutConsole strmCon;



// ----------------------------------------------------------------------------
//  Private helpers
// ----------------------------------------------------------------------------
EParseRes eParseFile(       TXMLTreeParser& xtprsToUse
                    , const TString&        strPath
                    , const TString&        strFileName
                    ,       TIndexInfo&     iiTar)
{

    if (!xtprsToUse.bParseRootEntity(strPath
                                    , tCIDXML::EParseOpts::Validate
                                    , tCIDXML::EParseFlags::TagsNText))
    {
        const TXMLTreeParser::TErrInfo& errCur = xtprsToUse.colErrors()[0];
        strmCon << L"The parse failed\n"
                << errCur.strText()
                << kCIDLib::NewLn << L"(" << errCur.c4Line()
                << kCIDLib::chPeriod << errCur.c4Column()
                << L") " << errCur.strSystemId() << kCIDLib::NewEndLn;
        return EParseRes::Error;
    }

    //
    //  Pull out the info and update the passed index info object. Everything we need
    //  to know is an attribute of the root.
    //
    tCIDLib::TCard4 c4At;
    const TXMLTreeElement& xtnodeRoot = xtprsToUse.xtdocThis().xtnodeRoot();

    // Unless less asked to include disabled ones, ignore them
    if (bIgnoreDeleted && xtnodeRoot.xtattrNamed(L"Disabled").bValueAs())
        return EParseRes::Discard;

    const TXMLTreeAttr& xtattrCppClass = xtnodeRoot.xtattrNamed(L"UnitClass");
    const TXMLTreeAttr& xtattrMake = xtnodeRoot.xtattrNamed(L"Make");
    const TXMLTreeAttr& xtattrModel = xtnodeRoot.xtattrNamed(L"Model");
    const TXMLTreeAttr& xtattrName = xtnodeRoot.xtattrNamed(L"Name");
    const TXMLTreeAttr& xtattrManIds = xtnodeRoot.xtattrNamed(L"ManIds");

    //
    //  Get the instruction element if available. It's contents is text. It is optional
    //  so it may not be there. If there it will have a single text node child since it
    //  is PCDATA type.
    //
    TString strInstruct;
    const TXMLTreeElement* pxtnodeInstruct = xtnodeRoot.pxtnodeFindElement
    (
        L"Instruct", 0, c4At
    );
    if (pxtnodeInstruct)
        iiTar.strInstruct = pxtnodeInstruct->xtnodeChildAtAsText(0).strText();
    else
        iiTar.strInstruct.Clear();

    iiTar.strCppClass = xtattrCppClass.strValue();
    if (iiTar.strCppClass.bCompareI(L"_NA_"))
        iiTar.strCppClass.Clear();

    iiTar.strFile = strFileName;
    iiTar.strMake = xtattrMake.strValue();
    iiTar.strModel = xtattrModel.strValue();
    iiTar.strName = xtattrName.strValue();
    iiTar.c8ManIds = xtattrManIds.strValue().c8Val();

    // Go through the command classes and save those
    iiTar.colClasses.RemoveAll();
    iiTar.fcolSecure.RemoveAll();
    iiTar.fcolVersions.RemoveAll();
    const TXMLTreeElement& xtnodeCCs = xtnodeRoot.xtnodeFindElement(L"CmdClasses", 0, c4At);
    const tCIDLib::TCard4 c4CCCnt = xtnodeCCs.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CCCnt; c4Index++)
    {
        // Skip non-elements
        if (xtnodeCCs.eChildTypeAt(c4Index) != tCIDXML::ENodeTypes::Element)
            continue;

        const TXMLTreeElement& xtnodeCC = xtnodeCCs.xtnodeChildAtAsElement(c4Index);
        const TXMLTreeAttr& xtattrName = xtnodeCC.xtattrNamed(L"Name");
        const TXMLTreeAttr& xtattrVer = xtnodeCC.xtattrNamed(L"Version");
        const TXMLTreeAttr& xtattrSecure = xtnodeCC.xtattrNamed(L"Secure");

        //
        //  Make sure the class is valid, we don't do this in the DTD since that would
        //  end up with the same list in two places.
        //
        if (tZWaveUSB3Sh::eXlatECClasses(xtattrName.strValue()) == tZWaveUSB3Sh::ECClasses::Count)
        {
            strmCon << L"\nThe parse failed\nInvalid command class '"
                    << xtattrName.strValue()
                    << L"' in file " << strFileName
                    << kCIDLib::NewEndLn;
            return EParseRes::Error;
        }

        iiTar.colClasses.objAdd(xtattrName.strValue());
        iiTar.fcolVersions.c4AddElement(xtattrVer.c4ValueAs());
        iiTar.fcolSecure.c4AddElement(xtattrSecure.bValueAs());
    }

    return EParseRes::Keep;
}



// ----------------------------------------------------------------------------
//  The program entry point
// ----------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    strmCon << L"CQC Z-Wave USB3 Device Info Compiler" << kCIDLib::NewEndLn;

    // Get the base CQC source tree
    TString strCQCDataDir;
    if (!TProcEnvironment::bFind(L"CQC_DATADIR", strCQCDataDir))
    {
        strmCon << L"   Could not find CQC data directory" << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::BadEnvironment;
    }

    // And the CQC projects root directory
    TString strCQCAllDir;
    if (!TProcEnvironment::bFind(L"CQC_ALL", strCQCAllDir))
    {
        strmCon << L"   Could not find CQC project root directory" << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::BadEnvironment;
    }

    // Check for any supported command line parameters
    TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
    for (; cursParms; ++cursParms)
    {
        if (cursParms->bCompareI(L"/KeepDeleted"))
            bIgnoreDeleted = kCIDLib::False;
        else
        {
            strmCon << L"Unknown command line parameter: " << *cursParms
                    << kCIDLib::NewEndLn;
            return tCIDLib::EExitCodes::BadParameters;
        }
    }


    //
    //  First thing we want to do is to copy all of the zwdi files and the DTD to
    //  the output directory over under CQCData\DataServer\ZWU3Info. This way we don't
    //  have to have a separate copy project, which I always forget to run first.
    //
    TPathStr pathTarDir = strCQCDataDir;
    pathTarDir.AddLevel(L"DataServer");
    pathTarDir.AddLevel(L"ZWU3Info");

    TPathStr pathSrcDir = strCQCAllDir;
    pathSrcDir.AddLevel(L"Drivers");
    pathSrcDir.AddLevel(L"ZWave");
    pathSrcDir.AddLevel(L"ZWaveUSB3");
    pathSrcDir.AddLevel(L"DevInfo");

    if (!TFileSys::bIsDirectory(pathSrcDir))
    {
        strmCon << L"   Source path doesn't exists Path="
                << pathSrcDir
                << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::BadParameters;
    }

    // If the target directory doesn't exist, create it
    TFileSys::MakePath(pathTarDir);


    try
    {
        //
        //  Clean any existing content from the directory. This insures we never
        //  get any old stuff hanging around.
        //
        TFileSys::CleanPath(pathTarDir, tCIDLib::ETreeDelModes::Shallow);

        //
        //  Now let's iterate the source directory and parse each one, copying
        //  the files to the target if valid.
        //
        TString strTarFile = pathTarDir;
        const tCIDLib::TCard4 c4TarPathLen = pathTarDir.c4Length();
        TString strNameExt;
        TFindBuf fndbCur;
        TDirIter diterSrc;

        if (diterSrc.bFindFirst(pathSrcDir, kCIDLib::pszAllFilesSpec, fndbCur, tCIDLib::EDirSearchFlags::NormalFiles))
        {
            do
            {
                // Build up the target file name
                fndbCur.pathFileName().bQueryNameExt(strNameExt);
                strTarFile.CapAt(c4TarPathLen);
                strTarFile.Append(kCIDLib::chPathSep);
                strTarFile.Append(strNameExt);

                // And copy it, default is to overwrite
                TFileSys::CopyFile(fndbCur.pathFileName(), strTarFile);

            }   while (diterSrc.bFindNext(fndbCur));
        }
    }

    catch(const TError& errToCatch)
    {
        strmCon << L"   An exception occurred during copy phase. Error=\n"
                << errToCatch
                << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::RuntimeError;
    }


    //
    //  Now we can do the build. Let's make the previous target directory the new source
    //
    pathSrcDir = pathTarDir;

    // Make that directory our current one, so the parser will naturally find the DTD
    TFileSys::SetCurrentDir(pathSrcDir);

    // The target is the same directory as where the source files are from.
    TPathStr pathOutFile(pathSrcDir);
    pathOutFile.AddLevel(L"ZWU3DevInfoIndex.xml");

    try
    {
        // Iterate all of the ZWInfo files and parse them
        TDirIter diterSrc;

        pathSrcDir.AddLevel(L"*.ZWDI");
        TFindBuf fndbCur;
        if (!diterSrc.bFindFirst(pathSrcDir, fndbCur, tCIDLib::EDirSearchFlags::NormalFiles))
        {
            strmCon << L"   No ZWDI files found in source directory" << kCIDLib::NewEndLn;
            return tCIDLib::EExitCodes::NotFound;
        }

        tCIDLib::TStrList   colDisabled;
        TVector<TIndexInfo> colIndex;
        TIndexInfo          iiCur;
        TString             strName;
        TXMLTreeParser      xtprsToUse;
        do
        {
            fndbCur.pathFileName().bQueryName(strName);

            strmCon << L"    Parsing: " << strName;
            EParseRes eRes = eParseFile(xtprsToUse, fndbCur.pathFileName(), strName, iiCur);
            if (eRes == EParseRes::Error)
            {
                strmCon << L" (An error occurred)" << kCIDLib::NewEndLn;
                return tCIDLib::EExitCodes::RuntimeError;
            }
            strmCon << kCIDLib::EndLn;

            //
            //  It worked so add it to our list if a keeper, else add the name to
            //  the disabled list.
            //
            if (eRes == EParseRes::Keep)
                colIndex.objAdd(iiCur);
            else
                colDisabled.objAdd(strName);

        }   while (diterSrc.bFindNext(fndbCur));


        // OK, let's sort the list now (by make) and output the index file
        colIndex.Sort(&eCompFunc);

        TTextFileOutStream strmTar
        (
            pathOutFile
            , tCIDLib::ECreateActs::CreateAlways
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
            , tCIDLib::EAccessModes::Excl_Write
            , new TUTF8Converter
        );

        strmTar << L"<?xml version='1.0' encoding='UTF-8'?>\n"
                << L"<ZWIndex>\n";


        TString strLastMake;
        const tCIDLib::TCard4 c4IICnt = colIndex.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4IICnt; c4Index++)
        {
            const TIndexInfo& iiCur = colIndex[c4Index];

            //
            //  If the make has changed, and this isn't the first index, close off
            //  the previous make. Either way start another.
            //
            if (strLastMake != iiCur.strMake)
            {
                if (c4Index)
                    strmTar << L"    </Make>\n";

                strmTar << L"    <Make Name='";
                facCIDXML().EscapeFor(iiCur.strMake, strmTar, tCIDXML::EEscTypes::Attribute, TString::strEmpty());
                strmTar << L"'>\n";

                strLastMake = iiCur.strMake;
            }

            strmTar << L"      <Dev Model='";
            facCIDXML().EscapeFor(iiCur.strModel, strmTar, tCIDXML::EEscTypes::Attribute, TString::strEmpty());

            strmTar << L"' Name='";
            facCIDXML().EscapeFor(iiCur.strName, strmTar, tCIDXML::EEscTypes::Attribute, TString::strEmpty());

            strmTar << L"' File='";
            facCIDXML().EscapeFor(iiCur.strFile, strmTar, tCIDXML::EEscTypes::Attribute, TString::strEmpty());

            strmTar << L"'\n           CppClass='" << iiCur.strCppClass;
            strmTar << L"' ManIds='0x" << TCardinal64(iiCur.c8ManIds, tCIDLib::ERadices::Hex);
            strmTar << L"'>\n";

            // DO the classes
            strmTar << L"        <Classes>\n";
            const tCIDLib::TCard4 c4ClsCnt = iiCur.colClasses.c4ElemCount();
            for (tCIDLib::TCard4 c4ClsInd = 0; c4ClsInd < c4ClsCnt; c4ClsInd++)
            {
                strmTar << L"            <Class Name=\""
                        << iiCur.colClasses[c4ClsInd]
                        << L"\" Version=\""
                        << iiCur.fcolVersions[c4ClsInd]
                        << L"\" Secure=\""
                        << (iiCur.fcolSecure[c4ClsInd] ? L"Yes" : L"No")
                        << L"\"/>\n";
            }
            strmTar << L"       </Classes>\n";

            // If there are any instructions write them out
            if (!iiCur.strInstruct.bIsEmpty())
            {
                strmTar << L"       <Instruct>";
                facCIDXML().EscapeFor
                (
                    iiCur.strInstruct, strmTar, tCIDXML::EEscTypes::ElemText, TString::strEmpty()
                );
                strmTar << L"</Instruct>\n";
            }

            strmTar << L"       </Dev>\n";
        }

        // Close off the last make, and the main element
        strmTar << L"    </Make>\n</ZWIndex>\n";


        // If any disabled ones, indicate that
        const tCIDLib::TCard4 c4DisabledCnt = colDisabled.c4ElemCount();
        if (c4DisabledCnt)
        {
            strmCon << L"\n\nDisabled devices:\n";
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4DisabledCnt; c4Index++)
                strmCon << L"    " << colDisabled[c4Index] << kCIDLib::NewLn;
            strmCon << kCIDLib::NewEndLn;
        }
    }

    catch(const TError& errToCatch)
    {
        strmCon << L"   An exception occurred during build. Error=\n"
                << errToCatch
                << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::RuntimeError;
    }

    return tCIDLib::EExitCodes::Normal;
}

