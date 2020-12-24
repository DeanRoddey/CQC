//
// FILE NAME: ZWaveUSBC_EdUnitDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/11/2007
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
//  This is the header for the ZWaveUSBC_EdUnitDlg.cpp file, which implements
//  the TZWaveEdUnitDlg class. This class handles the replication process,
//  whereby CQC EdUnits the units and groups defined by the primary Z-Wave
//  controller.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveEdUnitDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TZWaveEdUnitDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveEdUnitDlg();

        ~TZWaveEdUnitDlg();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const TString&                strMoniker
            ,       TZWaveUnit&             unitToEdit
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TZWaveEdUnitDlg(const TZWaveEdUnitDlg&);
        tCIDLib::TVoid operator=(const TZWaveEdUnitDlg&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_punitToEdit
        //      A pointer to the incoming unit. We don't modify it until the
        //      server accepts any changes.
        //
        //  m_pwndXXXX
        //      Since we have to interact with these controls a lot, we get
        //      convenience, typed, pointers to them. We don't own these, we
        //      just are looking at them in a convenient way.
        //
        //  m_strMoniker
        //      The moniker of our driver, which the caller provides for us,
        //      so that we can create a client proxy and upload the changes
        //      made.
        // -------------------------------------------------------------------
        TZWaveUnit*     m_punitToEdit;
        TPushButton*    m_pwndCancel;
        TEntryField*    m_pwndName;
        TCheckBox*      m_pwndPolled;
        TNumSpinBox*    m_pwndPollInt;
        TPushButton*    m_pwndSave;
        TString         m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveEdUnitDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



