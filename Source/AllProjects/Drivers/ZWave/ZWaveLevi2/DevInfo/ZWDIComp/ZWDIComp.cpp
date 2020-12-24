//
// FILE NAME: ZWDIComp.Cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/23/2014
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
//  builder. It parses all of the Z-Wave device info XML files and builds up a new
//  output XML file. That file is an index that provides just the basic info required
//  for the V2 Z-Wave client side driver to let the user browse the available devices
//  by manufacturer, model, and type. This way, it doesn't have to parse all of the
//  files just to present the list.
//
//  This can also of course be used to test all of the files after making any changes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//


// ----------------------------------------------------------------------------
//  Includes. The XML header brings in all we need here
// ----------------------------------------------------------------------------
#include    "CIDXML.hpp"


// ----------------------------------------------------------------------------
//  Forward references
// ----------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThread(TThread&, tCIDLib::TVoid*);


// ----------------------------------------------------------------------------
//  Include magic main module code
// ----------------------------------------------------------------------------
CIDLib_MainModule(TThread(L"ZWDICompThread", eMainThread))



// ----------------------------------------------------------------------------
//  A simple class we'll use to store the index info we want to keep from each
//  file.
// ----------------------------------------------------------------------------
class TIndexInfo
{
    public :
        TIndexInfo() {}
        ~TIndexInfo() {}

        TString     strFile;
        TString     strMake;
        TString     strModel;
        TString     strName;
        TString     strGType;
};

static tCIDLib::ESortComps
eCompFunc(const TIndexInfo& ii1, const TIndexInfo& ii2)
{
    // Sort first by make
    tCIDLib::ESortComps eRes = ii1.strMake.eCompareI(ii2.strMake);

    // If that's the same, compare by the model
    if (eRes == tCIDLib::ESortComps::Equal)
        eRes  = ii1.strModel.eCompareI(ii2.strModel);

    return eRes;
}


// ----------------------------------------------------------------------------
//  Global data
// ----------------------------------------------------------------------------
TOutConsole strmCon;



// ----------------------------------------------------------------------------
//  Private helpers
// ----------------------------------------------------------------------------
tCIDLib::TBoolean bParseFile(       TXMLTreeParser& xtprsToUse
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
        return kCIDLib::False;
    }

    //
    //  Pull out the info and update the passed index info object. Everything we need
    //  to know is an attribut of the root.
    //
    const TXMLTreeElement& xtnodeRoot = xtprsToUse.xtdocThis().xtnodeRoot();

    const TXMLTreeAttr& xtattrMake = xtnodeRoot.xtattrNamed(L"Make");
    const TXMLTreeAttr& xtattrModel = xtnodeRoot.xtattrNamed(L"Model");
    const TXMLTreeAttr& xtattrName = xtnodeRoot.xtattrNamed(L"Name");
    const TXMLTreeAttr& xtattrGType = xtnodeRoot.xtattrNamed(L"GType");

    iiTar.strFile = strFileName;
    iiTar.strMake = xtattrMake.strValue();
    iiTar.strModel = xtattrModel.strValue();
    iiTar.strName = xtattrName.strValue();
    iiTar.strGType = xtattrGType.strValue();

    return kCIDLib::True;
}



// ----------------------------------------------------------------------------
//  The program entry point
// ----------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    strmCon << L"CQC Z-Wave Device Info Compiler" << kCIDLib::NewEndLn;

    // Get the base CQC source tree
    TString strCQCDataDidr;
    if (!TProcEnvironment::bFind(L"CQC_DATADIR", strCQCDataDidr))
    {
        strmCon << L"   Could not find CQC data directory info" << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::BadEnvironment;
    }

    //
    //  Build up the source path to the files. The build process puts them all
    //  under the CQCData\Local\ZWInfo directory.
    //
    TPathStr pathSrcDir = strCQCDataDidr;
    pathSrcDir.AddLevel(L"Local");
    pathSrcDir.AddLevel(L"ZWInfo");

    if (!TFileSys::bIsDirectory(pathSrcDir))
    {
        strmCon << L"   Source path doesn't exists Path="
                << pathSrcDir
                << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::BadParameters;
    }

    // Make that directory our current one, so the parser will naturally find the DTD
    TFileSys::SetCurrentDir(pathSrcDir);

    // The target is the same directory as where the source files are from.
    TPathStr pathOutFile(pathSrcDir);
    pathOutFile.AddLevel(L"ZWDevInfoIndex.xml");

    try
    {
        // Iterate all of the ZWInfo files and parse them
        TDirIter diterSrc;

        pathSrcDir.AddLevel(L"*.ZWInfo");
        TFindBuf fndbCur;
        if (!diterSrc.bFindFirst(pathSrcDir, fndbCur, tCIDLib::EDirSearchFlags::NormalFiles))
        {
            strmCon << L"   No ZWInfo files found in source directory" << kCIDLib::NewEndLn;
            return tCIDLib::EExitCodes::NotFound;
        }

        TVector<TIndexInfo> colIndex;
        TIndexInfo          iiCur;
        TString             strName;
        TXMLTreeParser      xtprsToUse;
        do
        {
            fndbCur.pathFileName().bQueryName(strName);

            strmCon << L"    Parsing: " << strName << kCIDLib::EndLn;
            if (!bParseFile(xtprsToUse, fndbCur.pathFileName(), strName, iiCur))
                return tCIDLib::EExitCodes::RuntimeError;

            // It worked so add it to our list
            colIndex.objAdd(iiCur);

        }   while (diterSrc.bFindNext(fndbCur));


        // OK, let's sort the list now and output the index file
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
                facCIDXML().EscapeFor(iiCur.strMake, strmTar, tCIDXML::EEscTypes::Attribute);
                strmTar << L"'>\n";

                strLastMake = iiCur.strMake;
            }

            strmTar << L"      <Dev Model='";
            facCIDXML().EscapeFor(iiCur.strModel, strmTar, tCIDXML::EEscTypes::Attribute);

            strmTar << L"' Name='";
            facCIDXML().EscapeFor(iiCur.strName, strmTar, tCIDXML::EEscTypes::Attribute);

            strmTar << L"' Type='";
            facCIDXML().EscapeFor(iiCur.strGType, strmTar, tCIDXML::EEscTypes::Attribute);

            strmTar << L"' File='";
            facCIDXML().EscapeFor(iiCur.strFile, strmTar, tCIDXML::EEscTypes::Attribute);

            strmTar << L"'/>\n";
        }

        // Close off the last make, and the main element
        strmTar << L"    </Make>\n</ZWIndex>\n";
    }

    catch(const TError& errToCatch)
    {
        strmCon << L"   An exception occurred. Error=\n"
                << errToCatch
                << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::RuntimeError;
    }

    return tCIDLib::EExitCodes::Normal;
}

