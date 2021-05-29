//
// FILE NAME: HostMonC_Window.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/05/2000
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
//  This file implements the main window of the host monitor client.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCHostMonC.hpp"



// ---------------------------------------------------------------------------
//  Magic macro
// ---------------------------------------------------------------------------
RTTIDecls(THostMonCWnd,TCQCDriverClient)


// ---------------------------------------------------------------------------
//  Local const data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCHostMonC_Window
    {
        // -----------------------------------------------------------------------
        //  This is the number of volume info slots we have. If there are more
        //  than this on the host, we scroll among them.
        // -----------------------------------------------------------------------
        constexpr  tCIDLib::TCard4     c4VolSlots = 5;


        // -----------------------------------------------------------------------
        //  These are some indexes of fields in the field polling list. The
        //  results come back in the same order as we put the fields into the
        //  list, so these let us get the results out easily.
        // -----------------------------------------------------------------------
        constexpr   tCIDLib::TCard4     c4IOInd_MemPerUsed = 0;
        constexpr   tCIDLib::TCard4     c4IOInd_FirstVol   = 1;
    }
}


// ---------------------------------------------------------------------------
//   CLASS: THostMonCWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THostMonCWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
THostMonCWnd::THostMonCWnd( const   TCQCDriverObjCfg&   cqcdcThis
                            , const TCQCUserCtx&        cuctxToUse) :

    TCQCDriverClient
    (
        cqcdcThis, L"THostMonCWnd", tCQCKit::EActLevels::Low, cuctxToUse
    )
    , m_bListChanged(kCIDLib::False)
    , m_c4CPUCount(0)
    , m_c4LeftVol(0)
    , m_c4PerMemUsedK(0)
    , m_c4PerMemUsedK2(0)
    , m_c4PhysicalMemK(0)
    , m_pwndMemBar(nullptr)
    , m_strmIn(&m_mbufPollData, 0)
{

}

THostMonCWnd::~THostMonCWnd()
{
}


// ---------------------------------------------------------------------------
//  THostMonCWnd: Public, inherited methods
// ---------------------------------------------------------------------------

// In our case, we aren't allowing any editing of values, so we never have any changes.
tCIDLib::TBoolean THostMonCWnd::bChanges() const
{
    return kCIDLib::False;
}

tCIDLib::TBoolean THostMonCWnd::bSaveChanges(TString&)
{
    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//  THostMonCWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean THostMonCWnd::bCreated()
{
    TParent::bCreated();

    // Load the dialog resource that defines our content and create the controls
    TDlgDesc dlgdChildren;
    facHostMonC().bCreateDlgDesc(kHMonC::ridDlg_Client, dlgdChildren);

    tCIDCtrls::TWndId widInitFocus;
    CreateDlgItems(dlgdChildren, widInitFocus);

    // Do an initial auto-size to get them in sync with our initial size
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    // Get typed pointers to our important children
    CastChildWnd(*this, kHMonC::ridDlg_Client_PhysMemPercent, m_pwndMemBar);

    // Set percentage ranges on any of the progress bars
    m_pwndMemBar->SetRange(0, 100);

    for (tCIDLib::TCard4 c4Index = 0; c4Index < CQCHostMonC_Window::c4VolSlots; c4Index++)
    {
        TProgressBar* pwndCur = pwndChildAs<TProgressBar>
        (
            kHMonC::ridDlg_Client_Vol_Used1 + (c4Index * 3)
        );
        pwndCur->SetRange(0, 100);
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean THostMonCWnd::bGetConnectData(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    //
    //  This is called when the poll thread get's us reconnected to our
    //  target server and our server side driver comes online. We just
    //  need to get any 'on connect' type data that only has to be read
    //  upon coming online. We'll store it for later use.
    //
    //  Since we just have a few and we only do this when we lose and
    //  regain the connection, we'll just do them via a 'read by name' call.
    //
    TVector<TString> colNames(8);
    colNames.objAdd(TString(kHostMonC::pszFld_CPUCount));
    colNames.objAdd(TString(kHostMonC::pszFld_HostName));
    colNames.objAdd(TString(kHostMonC::pszFld_OSVersion));
    colNames.objAdd(TString(kHostMonC::pszFld_PhysicalMemK));
    colNames.objAdd(TString(kHostMonC::pszFld_CPUType));

    tCIDLib::TCard4 c4Size;
    orbcTarget->ReadFieldsByName(strMoniker(), colNames, c4Size, m_mbufPollData);

    // Set up an input stream over the source
    TBinMBufInStream strmSrc(&m_mbufPollData, c4Size);

    //
    //  And now we can stream the fields out into our members. There will be
    //  a marker value before each actual value, which is used to indicate if
    //  a value was streamed back or if the field is in error. Since our driver
    //  doesn't use any per-field error indicators, we can just eat them.
    //
    tCIDLib::TCard1 c1Marker;
    strmSrc >> c1Marker >> m_c4CPUCount
            >> c1Marker >> m_strHostName
            >> c1Marker >> m_strOSInfo
            >> c1Marker >> m_c4PhysicalMemK
            >> c1Marker >> m_strCPUType;

    //
    //  And now let's build up the field I/O packet that we'll use for our
    //  subsequent polling calls. We have a helper method to do it since we
    //  have to re-do it during polling if we ever get an out of sync
    //  exception.
    //
    SetIOList(orbcTarget);

    //
    //  Do an initial poll of the fields, and then use the results object to
    //  parse the results out.
    //
    orbcTarget->ReadDriverFields
    (
        m_c4DriverListId
        , m_c4DriverId
        , m_c4FieldListId
        , m_fcolFldIds
        , m_fcolSerialNums
        , m_c4ReadBytes
        , m_mbufPollData
    );

    //
    //  Call the helper method that extracts results. It is semi-redundant
    //  in this case, since the addition of the volumes above got them up to
    //  date. But we have to do it for other fields.
    //
    bExtractResults(kCIDLib::True);

    // Indicate we connected ok
    return kCIDLib::True;
}


tCIDLib::TBoolean THostMonCWnd::bDoPoll(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    m_bListChanged = kCIDLib::False;
    try
    {
        orbcTarget->ReadDriverFields
        (
            m_c4DriverListId
            , m_c4DriverId
            , m_c4FieldListId
            , m_fcolFldIds
            , m_fcolSerialNums
            , m_c4ReadBytes
            , m_mbufPollData
        );
    }

    catch(TError& errToCatch)
    {
        //
        //  If it's an out of sync error, reload and try again, else throw.
        //  If the second attempt fails, just let the exception propogate
        //  back to the core driver logic.
        //
        if (errToCatch.eClass() == tCIDLib::EErrClasses::OutOfSync)
        {
            SetIOList(orbcTarget);
            orbcTarget->ReadDriverFields
            (
                m_c4DriverListId
                , m_c4DriverId
                , m_c4FieldListId
                , m_fcolFldIds
                , m_fcolSerialNums
                , m_c4ReadBytes
                , m_mbufPollData
            );

            // We need to re-set our
            m_bListChanged = kCIDLib::True;
        }
         else
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            throw;
        }
    }

    //
    //  Call our helper that pulls the data out and stores it. If we got
    //  out out of sync above, then we tell it to reset the list because
    //  the volume list could have changed.
    //
    return bExtractResults(m_bListChanged);
}


tCIDLib::TVoid THostMonCWnd::Connected()
{
    TString strTmp;

    //
    //  We've connected (or reconnected) so let's update any fields that
    //  display 'on connect' type data. The fields were updated with new
    //  data in the bGetConnectData() callback.
    //
    strChildText(kHMonC::ridDlg_Client_OSInfo, m_strOSInfo);
    strChildText(kHMonC::ridDlg_Client_HostName, m_strHostName);
    strChildText
    (
        kHMonC::ridDlg_Client_PhysMem
        , TString(TLocCardinal64(m_c4PhysicalMemK) + TString(L" KB"))
    );
    strChildText
    (
        kHMonC::ridDlg_Client_CPUInfo
        , m_strCPUType + TString(L" Count=") + TString(TCardinal(m_c4CPUCount))
    );

    // We have to init the volume list
    InitVolDisplay();

    // And do an initial forced display update
    UpdateDisplay(kCIDLib::True);
    m_bListChanged = kCIDLib::False;
}


tCIDLib::TVoid THostMonCWnd::DoCleanup()
{
    // We don't really have any
}


tCIDLib::TVoid THostMonCWnd::LostConnection()
{
}


tCIDLib::TVoid THostMonCWnd::UpdateDisplayData()
{
    // Call a private helper in non-forced mode
    if (m_bListChanged)
        InitVolDisplay();
    UpdateDisplay(m_bListChanged);
    m_bListChanged = kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  THostMonCWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean THostMonCWnd::bExtractResults(const tCIDLib::TBoolean bInit)
{
    // If this is an init mode, clean out the lists
    if (bInit)
    {
        m_colFlds.RemoveAll();
        m_colVolInfo.RemoveAll();
    }

    //
    //  Pull out the data from the formatted buffer. The caller has done
    //  a field read to fill the buffer. First of all we should get a start
    //  object marker, then a version marker.
    //
    m_strmIn.Reset();
    m_strmIn.SetEndIndex(m_c4ReadBytes);
    m_strmIn.CheckForStartMarker(CID_FILE, CID_LINE);
    tCIDLib::TCard1 c1FmtVersion;
    m_strmIn >> c1FmtVersion;
    if (c1FmtVersion != kCQCKit::c1FldFmtVersion)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFIOP_BadFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c1FmtVersion)
            , clsIsA()
        );
    }

    // Remember if we have any changes
    tCIDLib::TBoolean bChanges = kCIDLib::False;

    // Update our poll data fields with the most recent data
    tCIDLib::TCard1 c1Marker;
    tCIDLib::TCard2 c2Id;
    tCIDLib::TCard4 c4Index = 0;
    tCIDLib::TCard4 c4SerialNum;
    TString strTmp;
    while (!m_strmIn.bEndOfStream())
    {
        //
        //  We should see a frame marker before each one, then the field
        //  id, squeeezd into a TCard2 value, and the marker that indicates
        //  whether new data is available of not.
        //
        m_strmIn.CheckForFrameMarker(CID_FILE, CID_LINE);
        m_strmIn >> c2Id >> c1Marker;

        // If the marker says new data, then get it out
        if (c1Marker == kCQCKit::c1FldData_Changed)
        {
            // Get out the new serial number and update our list
            m_strmIn >> c4SerialNum;
            m_fcolSerialNums[c4Index] = c4SerialNum;

            // Now get out the value and store it
            if (c4Index == CQCHostMonC_Window::c4IOInd_MemPerUsed)
            {
                m_strmIn >> m_c4PerMemUsedK2;
            }
             else if (c4Index >= CQCHostMonC_Window::c4IOInd_FirstVol)
            {
                m_strmIn >> strTmp;

                if (bInit)
                {
                    m_colVolInfo.objAdd(TCQCVolPollInfo(strTmp));
                }
                 else
                {
                    m_colVolInfo[c4Index - CQCHostMonC_Window::c4IOInd_FirstVol]
                        .ParseValue(strTmp);
                }
            }

            bChanges = kCIDLib::True;
        }
         else if (c1Marker == kCQCKit::c1FldData_InError)
        {

        }
        c4Index++;
    }
    return bChanges;
}


const TCQCFldDef*
THostMonCWnd::pflddFind(const   TString&            strFind
                        , const tCIDLib::TBoolean   bThrowIfNot) const
{
    const tCIDLib::TCard4 c4Count = m_colFlds.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    const TCQCFldDef* pflddCur;
    for (; c4Index < c4Count; c4Index++)
    {
        pflddCur = &m_colFlds[c4Index];
        if (pflddCur->strName() == strFind)
            break;
    }

    if (c4Index == c4Count)
    {
        if (bThrowIfNot)
        {
            facHostMonC().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kHMonCErrs::errcFld_NotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , strFind
            );
        }
        return 0;
    }
    return pflddCur;
}


tCIDLib::TVoid THostMonCWnd::InitVolDisplay()
{
    //
    //  Update our volume info list box with an entry for each of the volumes
    //  we got. We reset the scroll origin, to put the first one on the left.
    //  If there are more than the number of display slots, we just display
    //  the first set that will fit.
    //
    m_c4LeftVol = 0;
    const tCIDLib::TCard4 c4Count = tCIDLib::MinVal
    (
        m_colVolInfo.c4ElemCount(), CQCHostMonC_Window::c4VolSlots
    );

    tCIDLib::TCard4 c4Index;
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCVolPollInfo& vpiCur = m_colVolInfo[c4Index];

        strChildText
        (
            kHMonC::ridDlg_Client_VolCap1 + (c4Index * 3)
            , TString(TLocCardinal64(vpiCur.c4TotalMB())) + TString(L" MB")
        );

        strChildText
        (
            kHMonC::ridDlg_Client_VolName1 + (c4Index * 3)
            , vpiCur.strVolume()
        );
    }

    // If we didn't update them all, then clear out the remaining ones
    for (; c4Index < CQCHostMonC_Window::c4VolSlots; c4Index++)
    {
        strChildText(kHMonC::ridDlg_Client_VolCap1 + (c4Index * 3), L"");
        strChildText(kHMonC::ridDlg_Client_VolName1 + (c4Index * 3), L"");

        pwndChildAs<TProgressBar>(kHMonC::ridDlg_Client_Vol_Used1 + (c4Index * 3))->SetValue(0);
    }
}


tCIDLib::TVoid THostMonCWnd::SetIOList(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    //
    //  We need to our the driver id list and the id for our driver, and
    //  the list of fields.
    //
    tCQCKit::EDrvStates eState;
    const tCIDLib::TCard4 c4Count = orbcTarget->c4QueryFields
    (
        strMoniker()
        , eState
        , m_colFlds
        , m_c4FieldListId
        , m_c4DriverId
        , m_c4DriverListId
    );

    //
    //  Now load up the field list with the fields in the order that we
    //  want them to be returned in the results. To make it easier to get
    //  the results out, we have an enum above that defines the order and
    //  we add them to the packet in that order. For each one, put a zero
    //  value into the serial numbers list.
    //
    const TCQCFldDef* pflddCur;
    m_fcolFldIds.RemoveAll();
    m_fcolSerialNums.RemoveAll();

    // Put in the fields first that we know will always be present
    pflddCur = pflddFind(kHostMonC::pszFld_PerMemUsed);
    m_fcolFldIds.c4AddElement(pflddCur->c4Id());
    m_fcolSerialNums.c4AddElement(0);

    //
    //  Now set up the volume list, which is a variable number of fields.
    //  So we find all of the fields that start with "Volume".
    //
    TString strName;
    tCIDLib::TCard4 c4Vol = 1;
    while (kCIDLib::True)
    {
        strName = L"Volume";
        strName.AppendFormatted(c4Vol);

        //
        //  See if this one is there. If not, then we are done so break
        //  out, else store this one.
        //
        pflddCur = pflddFind(strName, kCIDLib::False);
        if (!pflddCur)
            break;

        m_fcolFldIds.c4AddElement(pflddCur->c4Id());
        m_fcolSerialNums.c4AddElement(0);

        c4Vol++;
    }
}


tCIDLib::TVoid THostMonCWnd::UpdateDisplay(const tCIDLib::TBoolean bForce)
{
    //
    //  For any fields that have changed, update our display and mark them
    //  as having been viewed.
    //
    if (bForce || (m_c4PerMemUsedK != m_c4PerMemUsedK2))
    {
        m_c4PerMemUsedK = m_c4PerMemUsedK2;
        m_pwndMemBar->SetValue(m_c4PerMemUsedK2);
    }

    // Do any volumes that changed
    const tCIDLib::TCard4 c4Count = m_colVolInfo.c4ElemCount();
    tCIDLib::TCard4 c4Index = m_c4LeftVol;
    for (; c4Index < c4Count; c4Index++)
    {
        TCQCVolPollInfo& vpiCur = m_colVolInfo[c4Index];
        if (!bForce && !vpiCur.bChanged())
            continue;

        // This one has changed or we are forced so update it
        TProgressBar* pwndUsed = pwndChildAs<TProgressBar>
        (
            kHMonC::ridDlg_Client_Vol_Used1 + ((c4Index - m_c4LeftVol) * 3)
        );

        pwndUsed->SetValue(vpiCur.c4PercentUsed2());
        vpiCur.c4PercentUsed(vpiCur.c4PercentUsed2());
    }
}

