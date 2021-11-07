//
// FILE NAME: CQCIGKit_SelIRCmdDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/06/2006
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
//  This is the header for the CQCGKit_SelIRCmdDlgDlg.cpp file, which allows
//  the user to select an IR device model/cmd value. It's returned in the
//  standard "model.cmd" format that can be written to the IR device driver's
//  invocation field.
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
//   CLASS: TCQCSelIRCmdDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TCQCSelIRCmdDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCSelIRCmdDlg();

        TCQCSelIRCmdDlg(const TCQCSelIRCmdDlg&) = delete;
        TCQCSelIRCmdDlg(TCQCSelIRCmdDlg&&) = delete;

        ~TCQCSelIRCmdDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCSelIRCmdDlg& operator=(const TCQCSelIRCmdDlg&) = delete;
        TCQCSelIRCmdDlg& operator=(TCQCSelIRCmdDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const TString&                strDriver
            ,       TString&                strToFill
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private data types
        //
        //  We keep a list of string lists. Each slot is the commands for the model
        //  at that index. And the values in that slot are the names of the commands
        //  that model supports. For the models we have a regular string list that
        //  contains those (in the same order.)
        // -------------------------------------------------------------------
        using TCmdList = TVector<tCIDLib::TStrList>;


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

        tCIDLib::TVoid LoadCmds
        (
            const   TString&                strModel
            , const TString&                strSelCmd
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colCmds
        //  m_colModels
        //      To hold the IR model/cmd data we get from the driver. The model list
        //      is a string list that holds the model names in the order we got them.
        //      The commands list is a list of string lists, in the same order as
        //      the models, each of which contains the names of the commands for that
        //      model.
        //
        //      In the models list box we set the original index into the models list
        //      on each item, so that we can map back.
        //
        //  m_pwndXXX
        //      Some typed pointers we get to some of our child widgets we
        //      need to interact with on a typed basis.
        //
        //  m_strData
        //      We load the raw data queried from the IR driver into this
        //      string, where it will be parsed later to load up the lists.
        //
        //  m_strDriver
        //      The IR driver moniker is stored until we can use it to get
        //      the list filled in.
        //
        //  m_strInX
        //      If there is any incoming value, we break it out into model,
        //      command and zone parts for later use. The first two are used
        //      for initial selection. The zone info is put back onto the
        //      selected command.
        //
        //  m_strSelection
        //      We put the selection here until it can be copied back to the
        //      caller's buffer.
        // -------------------------------------------------------------------
        TCmdList            m_colCmds;
        tCIDLib::TStrList   m_colModels;
        TPushButton*        m_pwndCancelButton;
        TMultiColListBox*   m_pwndCmds;
        TMultiColListBox*   m_pwndModels;
        TPushButton*        m_pwndSelectButton;
        TString             m_strData;
        TString             m_strDriver;
        TString             m_strInCmd;
        TString             m_strInModel;
        TString             m_strInZone;
        TString             m_strSelection;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCSelIRCmdDlg, TDlgBox)
};

#pragma CIDLIB_POPPACK




