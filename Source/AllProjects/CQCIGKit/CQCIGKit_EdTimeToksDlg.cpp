//
// FILE NAME: CQCIGKit_EdTimeToksDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/11/2016
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
//  This file implements a formatted time text editor dialog.
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
#include    "CQCIGKit_.hpp"
#include    "CQCIGKit_EdTimeToksDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEdTimeToksDlg,TDlgBox)


// ---------------------------------------------------------------------------
//   CLASS: TEdTimeToksDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdTimeToksDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdTimeToksDlg::TEdTimeToksDlg() :

    m_pwndCancel(nullptr)
    , m_pwndList(nullptr)
    , m_pwndInsert(nullptr)
    , m_pwndPrefab(nullptr)
    , m_pwndValue(nullptr)
    , m_pwndSample(nullptr)
    , m_pwndSave(nullptr)
{
}

TEdTimeToksDlg::~TEdTimeToksDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdTimeToksDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdTimeToksDlg::bRun(const TWindow& wndOwner, TString& strToEdit)
{
    m_strToEdit = strToEdit;
    const tCIDLib::TCard4 c4Res = c4RunDlg(wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_EdTimeToks);

    if (c4Res == kCQCIGKit::ridDlg_EdTimeToks_Save)
    {
        strToEdit = m_strToEdit;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TEdTimeToksDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdTimeToksDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTimeToks_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTimeToks_Insert, m_pwndInsert);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTimeToks_List, m_pwndList);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTimeToks_Prefab, m_pwndPrefab);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTimeToks_Sample, m_pwndSample);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTimeToks_Save, m_pwndSave);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTimeToks_Value, m_pwndValue);

    // If the last area isn't empty, we need to go back to the previous area
    if (!s_areaLast.bIsEmpty())
    {
        //
        //  Do an inital auto-size to get the content resized from our initial size
        //  to the saved size
        //
        AutoAdjustChildren(areaWndSize(), s_areaLast);

        // And now move to the new size
        SetSizePos(s_areaLast, kCIDLib::False);
    }

    //
    //  Set up the list box. We want to center justify the left column so we have to put in
    //  a dummy zero width column first (first one can only be left.)
    //
    tCIDLib::TStrList colCols(3);
    colCols.objAdd(TString::strEmpty());
    colCols.objAdd(L"Token");
    colCols.objAdd(L"Token Description");
    m_pwndList->SetColumns(colCols);

    // Zero zize the first one and size the 2nd one to its column title. Last is auto-sized
    m_pwndList->SetColOpts(0, 0);
    m_pwndList->SetColOpts(1, kCIDLib::c4MaxCard, tCIDLib::EHJustify::Center);
    m_pwndList->AutoSizeCol(1, kCIDLib::True);

    // Load up the incoming value
    m_pwndValue->strWndText(m_strToEdit);

    // And do an initial update of the sample value based on the incoming edit value
    UpdateSample();

    //
    //  Load up the list box now. We have to just do this the old fashioned way, there's
    //  no list or anything.
    //
    colCols[1] = L":";
    colCols[2] = L"The localized time separator character";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"/";
    colCols[2] = L"The localized date separator character";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"a";
    colCols[2] = L"The localized abbreviated day of the week name";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"A";
    colCols[2] = L"The localized full day of the week name";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"D";
    colCols[2] = L"The day of the month as a number";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"d";
    colCols[2] = L"The localized name of the day of the week";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"e";
    colCols[2] = L"Total elapsed seconds";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"E";
    colCols[2] = L"Total elapsed minutes";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"H";
    colCols[2] = L"The hour in 24 hour terms";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"h";
    colCols[2] = L"The hour in 12 hour terms";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"l";
    colCols[2] = L"The milliseconds";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"M";
    colCols[2] = L"The month as a number";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"m";
    colCols[2] = L"The localized abbreviated month name";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"n";
    colCols[2] = L"The localized full month name";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"P";
    colCols[2] = L"The localized AM/PM designation";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"p";
    colCols[2] = L"The single letter AM/PM designation";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"s";
    colCols[2] = L"The seconds";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"S";
    colCols[2] = L"The raw time stamp value (elapsed 100ns intervals)";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"T";
    colCols[2] = L"The localized time zone name";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"t";
    colCols[2] = L"The local time zone offset from UTC, in minutes";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"u";
    colCols[2] = L"The minutes";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"x";
    colCols[2] = L"Elapsed days, hours, and minutes, two digit, e.g. 01:21:04.";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"X";
    colCols[2] = L"Same as 'x', but leaves out the days if the elapsed days are zero.";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"Y";
    colCols[2] = L"The year as a 4 digit value";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"y";
    colCols[2] = L"The trailing two digits of the year";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"z";
    colCols[2] = L"Elapsed minutes and seconds, two digit, e.g. 03:15:08.";
    m_pwndList->c4AddItem(colCols, 0);

    colCols[1] = L"Z";
    colCols[2] = L"Same as 'z' but will no hour if zero.";
    m_pwndList->c4AddItem(colCols, 0);


    // Set up our handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEdTimeToksDlg::eClickHandler);
    m_pwndInsert->pnothRegisterHandler(this, &TEdTimeToksDlg::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TEdTimeToksDlg::eLBHandler);
    m_pwndPrefab->pnothRegisterHandler(this, &TEdTimeToksDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdTimeToksDlg::eClickHandler);
    m_pwndValue->pnothRegisterHandler(this, &TEdTimeToksDlg::eEFHandler);

    return bRet;
}


// Remember our position for next time
tCIDLib::TVoid TEdTimeToksDlg::Destroyed()
{
    s_areaLast = areaWnd();

    TParent::Destroyed();
}


// ---------------------------------------------------------------------------
//  TEdTimeToksDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TEdTimeToksDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdTimeToks_Cancel)
    {
        EndDlg(kCQCIGKit::ridDlg_EdTimeToks_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdTimeToks_Insert)
    {
        // We insert the selected token and update the sample
        InsertToken(m_pwndList->c4CurItem());
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdTimeToks_Prefab)
    {
        //
        //  This will display a dropdown menu to the right of the button that lets them
        //  pick some common formats.
        //
        ShowPrefabMenu();
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdTimeToks_Save)
    {
        // Save the edited value and end with a save
        m_strToEdit = m_pwndValue->strWndText();
        EndDlg(kCQCIGKit::ridDlg_EdTimeToks_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TEdTimeToksDlg::eEFHandler(TEFChangeInfo& wnotEvent)
{
    // On a change just update the sample text
    if (wnotEvent.eEvent() == tCIDCtrls::EEFEvents::Changed)
        UpdateSample();

    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TEdTimeToksDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    // On an invoke, insert the selected token into the entry field at the current position
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        InsertToken(wnotEvent.c4Index());
    return tCIDCtrls::EEvResponses::Handled;
}


// Inserts the indicated token into the edit value and updates the sample
tCIDLib::TVoid TEdTimeToksDlg::InsertToken(const tCIDLib::TCard4 c4Index)
{
    //
    //  Get the character from the column and we can just create the token from
    //  that and not have a big switch statement.
    //
    TString strChar;
    m_pwndList->QueryColText(c4Index, 1, strChar);
    TString strToken(L"%(");
    strToken.Append(strChar);
    strToken.Append(L")");

    // And let's insert that in the edit value and update the sample
    m_pwndValue->InsertText(strToken);
    UpdateSample();
}


//
//  Called when the user presses the > button, which we use to drop down a menu so that they
//  can select from some commonly used time formats.
//
tCIDLib::TVoid TEdTimeToksDlg::ShowPrefabMenu()
{
    TPopupMenu menuPrefab(L"Time Token Dialog Prefab");
    menuPrefab.Create();

    // Load up the prefab formats we support
    menuPrefab.AddActionItem(TString::strEmpty(), L"24HHMM"    , 1);
    menuPrefab.AddActionItem(TString::strEmpty(), L"24HHMMSS"  , 2);
    menuPrefab.AddActionItem(TString::strEmpty(), L"Unix Time" , 3);
    menuPrefab.AddActionItem(TString::strEmpty(), L"DDMMYY"    , 4);
    menuPrefab.AddActionItem(TString::strEmpty(), L"DDMMYYYY"  , 5);
    menuPrefab.AddActionItem(TString::strEmpty(), L"Full Date" , 6);
    menuPrefab.AddActionItem(TString::strEmpty(), L"MMDDYY"    , 7);
    menuPrefab.AddActionItem(TString::strEmpty(), L"MMDDYYYY"  , 8);
    menuPrefab.AddActionItem(TString::strEmpty(), L"YYMMDD"    , 9);
    menuPrefab.AddActionItem(TString::strEmpty(), L"YYYYMMDD"  , 10);

    // Get the area of the prefab button and convert to screen coordinates
    TArea areaMenu;
    ToScreenCoordinates(m_pwndPrefab->areaWnd(), areaMenu);

    // Now pop it up, upper left justified with the top/right of the button
    const tCIDLib::TCard4 c4Res = menuPrefab.c4Process(*this, areaMenu.pntUR());

    // If a selection was made so let's do it
    if (c4Res)
    {
        switch(c4Res)
        {
            case 1 :
                m_strToEdit = TTime::str24HHMM();
                break;

            case 2 :
                m_strToEdit = TTime::str24HHMMSS();
                break;

            case 3 :
                m_strToEdit = TTime::strCTime();
                break;

            case 4 :
                m_strToEdit = TTime::strDDMMYY();
                break;

            case 5 :
                m_strToEdit = TTime::strDDMMYYYY();
                break;

            case 6 :
                m_strToEdit = TTime::strFullDate();
                break;

            case 7 :
                m_strToEdit = TTime::strMMDDYYYY();
                break;

            case 8 :
                m_strToEdit = TTime::strMMDDYY();
                break;

            case 9 :
                m_strToEdit = TTime::strYYMMDD();
                break;

            case 10 :
                m_strToEdit = TTime::strYYYYMMDD();
                break;

            default :
            {
                TErrBox msgbErr(L"Unknonwn prefab format selection");
                msgbErr.ShowIt(*this);
                return;
            }
        };

        m_pwndValue->strWndText(m_strToEdit);
        UpdateSample();
    }
}


// Updates the sample text based on the current edit value
tCIDLib::TVoid TEdTimeToksDlg::UpdateSample()
{
    // Get a time object with the current time, and format it using our current edit value
    TTime tmSample(tCIDLib::ESpecialTimes::CurrentTime);

    // The value widget has the text pattern
    m_strToEdit = m_pwndValue->strWndText();

    // Use that to format the time and set that on the sample widget
    TString strSample;
    tmSample.FormatToStr(strSample, m_strToEdit);
    m_pwndSample->strWndText(strSample);
}


// ---------------------------------------------------------------------------
//  TEdTimeToksDlg: Private, static data
// ---------------------------------------------------------------------------
TArea TEdTimeToksDlg::s_areaLast;
