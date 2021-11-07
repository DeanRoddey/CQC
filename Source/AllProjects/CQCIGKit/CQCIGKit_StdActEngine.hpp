//
// FILE NAME: CQCIGKit_StdActEngine.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/18/2009
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
//  This is the header for the CQCIGKit_StdActEngine.cpp file, which
//  implements a derivative of the standard action engine, extended to
//  work correctly for GUI based apps.
//
//  This is not for use in the interface system. It's for things like key
//  mapped actions, where it's run from the GUI, but only uses the standard
//  non-GUI type targets.
//
//  We just need to overrride the command error where we just display the
//  error in a dialog, and the wait callback, where we do a modal loop until
//  the processing is completed.
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
//   CLASS: TCQCStdGUIActEngine
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class CQCIGKITEXPORT TCQCStdGUIActEngine : public TCQCStdActEngine
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCStdGUIActEngine
        (
            const   TWindow* const          pwndOwner
            ,       TWindow* const          pwndModal
            , const TString&                strTitle
            , const TCQCUserCtx&            cuctxToUse
        );

        TCQCStdGUIActEngine(const TCQCStdGUIActEngine&) = delete;
        TCQCStdGUIActEngine(TCQCStdGUIActEngine&&) = delete;

        ~TCQCStdGUIActEngine();


        // -------------------------------------------------------------------
        //  Public methods
        // -------------------------------------------------------------------
        TCQCStdGUIActEngine& operator=(const TCQCStdGUIActEngine&) = delete;
        TCQCStdGUIActEngine& operator=(TCQCStdGUIActEngine&&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CmdInvokeErr
        (
            const   TError&                 errToCatch
            , const tCIDLib::TCard4         c4Index
        )   final;

        tCIDLib::TVoid WaitEnd
        (
                    tCIDLib::TBoolean&      bDoneFlag
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndModal
        //      This is the window the caller wants us to be modal to while
        //      the action is running, which may not always be the same as
        //      the owner.
        //
        //  m_pwndOwner
        //      This is the window the caller wants us to use as the owner
        //      in any error popups.
        //
        //  m_strTitle
        //      The title text the calling app wants us to use in any error
        //      popups.
        // -------------------------------------------------------------------
        TWindow*        m_pwndModal;
        const TWindow*  m_pwndOwner;
        TString         m_strTitle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCStdGUIActEngine, TCQCStdActEngine)
};

#pragma CIDLIB_POPPACK

