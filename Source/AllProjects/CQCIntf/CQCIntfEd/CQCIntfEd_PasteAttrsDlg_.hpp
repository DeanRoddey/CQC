//
// FILE NAME: CQCIntfEd_PasteAttrsDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/27/2016
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
//  This is the header for the CQCIntfEd_PasteAttrsDlg.cpp file, which allows the user
//  to select which attributes to paste. We load the available attributes into a tree
//  control with check boxes enabled.
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
//   CLASS: TPasteAttrsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TPasteAttrsDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TPasteAttrsDlg();

        TPasteAttrsDlg(const TPasteAttrsDlg&) = delete;

        ~TPasteAttrsDlg();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TPasteAttrsDlg& operator=(const TPasteAttrsDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            , const TCQCIntfAttrPack&       iapAvail
            ,       tCIDMData::TAttrList&   colSelected
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

        tCIDCtrls::EEvResponses eTreeHandler
        (
                    TTreeEventInfo&         wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pcolSelected
        //      A point to the caller's selected list. We put any selected attributes into
        //      this list, which the caller can apply to the widgets.
        //
        //  m_piapAvail
        //      A pointer to the incoming attribute pack that we get the data from.
        //
        //  m_pwndXXX
        //      Some typed pointers we get to some of our child widgets we need to
        //      interact with on a typed basis.
        // -------------------------------------------------------------------
        tCIDMData::TAttrList*   m_pcolSelected;
        const TCQCIntfAttrPack* m_piapAvail;
        TPushButton*            m_pwndApply;
        TPushButton*            m_pwndCancel;
        TTreeView*              m_pwndTree;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TPasteAttrsDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



