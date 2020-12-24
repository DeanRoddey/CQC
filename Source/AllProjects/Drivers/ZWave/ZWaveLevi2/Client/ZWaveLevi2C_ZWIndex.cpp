//
// FILE NAME: ZWaveLevi2C_ZWIndex.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/26/2014
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
//  This file implements the device info index.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLevi2C_.hpp"



// ---------------------------------------------------------------------------a
//   CLASS: TZWDIInfo
//  PREFIX: zwdii
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWDIInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWDIInfo::TZWDIInfo()
{
}

TZWDIInfo::~TZWDIInfo()
{
}


// ---------------------------------------------------------------------------
//  TZWDIInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TZWDIInfo::ParseFromXML(const   TString&            strMake
                        , const TXMLTreeElement&    xtnodeDev)
{
    // Let's find our info, all of which is just in attributes
    m_strFileName   = xtnodeDev.xtattrNamed(L"File").strValue();
    m_strName       = xtnodeDev.xtattrNamed(L"Name").strValue();
    m_strModel      = xtnodeDev.xtattrNamed(L"Model").strValue();
    m_strType       = xtnodeDev.xtattrNamed(L"Type").strValue();

    m_strMake       = strMake;
}



// ---------------------------------------------------------------------------
//   CLASS: TZDIIndex
//  PREFIX: zwdin
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZDIIndex: Constructors and Destructor
// ---------------------------------------------------------------------------
TZDIIndex::TZDIIndex()
{
}

TZDIIndex::~TZDIIndex()
{
}


// ---------------------------------------------------------------------------
//  TZDIIndex: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZDIIndex::ParseFromXML(TWindow& wndParent)
{
    // Build up the path to the file
    TPathStr pathIndex(facCQCKit().strLocalDataDir());
    pathIndex.AddLevel(L"ZWInfo");
    pathIndex.AddLevel(L"ZWDevInfoIndex.xml");

    if (!TFileSys::bExists(pathIndex))
    {
        TErrBox msgbNotFnd(L"The device info index file was not found");
        msgbNotFnd.ShowIt(wndParent);
        return;
    }

    try
    {
        TXMLTreeParser  xtprsInd;

        //
        //  Tell it to store tags only. Since we don't use a DTD for this, we don't
        //  want to have to deal with text nodes and we don't use element text for
        //  any info, so this works well for us.
        //
        if (!xtprsInd.bParseRootEntity( pathIndex
                                        , tCIDXML::EParseOpts::None
                                        , tCIDXML::EParseFlags::Tags))
        {
            const TXMLTreeParser::TErrInfo& erriFirst = xtprsInd.colErrors()[0];

            TTextStringOutStream strmOut(512);
            strmOut << L"[" << erriFirst.strSystemId() << L"/"
                    << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                    << L"] " << erriFirst.strText() << kCIDLib::FlushIt;

            facZWaveLevi2C().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZW2CErrs::errcZWInd_InvalidXML
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , strmOut.strData()
            );
        }

        //
        //  It worked, so let's process it. We first go through the manufacturer
        //  elements and for each one we iterate the device elemens within it.
        //
        const TXMLTreeElement&  xtnodeRoot = xtprsInd.xtdocThis().xtnodeRoot();
        const tCIDLib::TCard4   c4MakeCnt = xtnodeRoot.c4ChildCount();
        tCIDLib::TStrList       colFails;
        TString                 strTmp;
        TZWDIInfo               zwdiiCur;

        for (tCIDLib::TCard4 c4MakeInd = 0; c4MakeInd < c4MakeCnt; c4MakeInd++)
        {
            const TXMLTreeElement& xtnodeMake = xtnodeRoot.xtnodeChildAtAsElement(c4MakeInd);

            // Get the make name out since we pass it in to each object
            const TString& strMake = xtnodeMake.xtattrNamed(L"Name").strValue();

            //
            //  And now let's iterate each of it's children in turn, processing each
            //  one as an index object.
            //
            const tCIDLib::TCard4 c4ModCnt = xtnodeMake.c4ChildCount();
            for (tCIDLib::TCard4 c4ModInd = 0; c4ModInd < c4ModCnt; c4ModInd++)
            {
                const TXMLTreeElement& xtnodeModel
                (
                    xtnodeMake.xtnodeChildAtAsElement(c4ModInd)
                );

                try
                {
                    zwdiiCur.ParseFromXML(strMake, xtnodeModel);

                    // It worked, so add it to the list
                    m_colMakeModel.objAdd(zwdiiCur);
                }

                catch(TError& errToCatch)
                {
                    colFails.objAdd(errToCatch.strErrText());
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        TExceptDlg dlgFailed
        (
            wndParent
            , wndParent.strWndText()
            , L"An exception occurred while parsing device info index"
            , errToCatch
        );

        // Make sure the lists are cleared
        ResetLists();
    }
}



// ---------------------------------------------------------------------------
//  TZDIIndex: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Just clears all of the lists out
tCIDLib::TVoid TZDIIndex::ResetLists()
{
    // Clear all of the by ref lists

    // Remove the real objects now
    m_colMakeModel.RemoveAll();
}


