//
// FILE NAME: CQCIGKit_LatLongDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/05/2005
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
//  This is the header for the CQCClient_LatLongDlg.cpp file, which implements
//  a dialog that gets latitude and longitude values from the user, as two
//  floating point values.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TLatLongDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TLatLongDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TLatLongDlg();

        TLatLongDlg(const TLatLongDlg&) = delete;

        ~TLatLongDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=(const TLatLongDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       tCIDLib::TFloat8&       f8Lat
            ,       tCIDLib::TFloat8&       f8Long
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_f8Lat
        //  m_f8Long
        //      Places to store the results until we can get them back into
        //      the caller's buffer, and to hold incoming values.
        //
        //  m_pwndXXX
        //      Since we have to interact with some controls a lot, we get
        //      convenience, typed, pointers to them. We don't own these, we
        //      just are looking at them a convenient way.
        // -------------------------------------------------------------------
        tCIDLib::TFloat8    m_f8Lat;
        tCIDLib::TFloat8    m_f8Long;
        TPushButton*        m_pwndAcceptButton;
        TPushButton*        m_pwndCancelButton;
        TMultiColListBox*   m_pwndCityList;
        TEntryField*        m_pwndLat;
        TEntryField*        m_pwndLong;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TLatLongDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



