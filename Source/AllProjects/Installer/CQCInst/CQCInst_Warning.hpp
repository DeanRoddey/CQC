//
// FILE NAME: CQCInst_WarningPanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/03/2018
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
//  There are some failures that we don't want to stop the installation for. So
//  we want to accumulate them and show them before we commit to the upgrade,
//  and let the user decide whether to continue or not. Here we just define a
//  simple class to store warning info. The facility will store them and the
//  final status panel will display them, if any.
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
//   CLASS: TInstWarning
//  PREFIX: warn
// ---------------------------------------------------------------------------
class TInstWarning
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstWarning();

        TInstWarning
        (
            const   TString&                strMsg
        );

        TInstWarning
        (
            const   TString&                strMsg
            , const TString&                strResource
        );

        TInstWarning
        (
            const   TString&                strMsg
            , const TError&                 errToDisplay
        );

        TInstWarning
        (
            const   TString&                strMsg
            , const TString&                strResource
            , const TError&                 errToDisplay
        );

        TInstWarning
        (
            const   TInstWarning&           warnSrc
        );

        TInstWarning(TInstWarning&&) = delete;

        ~TInstWarning();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TInstWarning& operator=
        (
            const   TInstWarning&           warnSrc
        );

        TInstWarning& operator=(TInstWarning&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const tCIDLib::TBoolean bHasExcept() const
        {
            return m_bHasExcept;
        }

        const TError& errException() const
        {
            return m_errException;
        }

        const TString& strMsg() const
        {
            return m_strMsg;
        }

        const TString& strResource() const
        {
            return m_strResource;
        }


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bHasExcept
        //      Indicates whether m_errException is valid or not.
        //
        //  m_errException
        //      If there was an exception involved, it is stored here and m_bHasExcept
        //      is set. We can let the user see it.
        //
        //  m_strMsg
        //      The actual error message for the user's consumption.
        //
        //  m_strResource
        //      If it was some resource (files mostly but maybe repo content and
        //      such) this indicates that.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bHasExcept;
        TError              m_errException;
        TString             m_strMsg;
        TString             m_strResource;
};

#pragma CIDLIB_POPPACK

