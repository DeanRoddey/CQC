//
// FILE NAME: CQCProtoDev_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/27/2003
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
//  This is the header for the facility class of this facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCProtoDev
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCProtoDev : public TGUIFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCProtoDev();

        TFacCQCProtoDev(const TFacCQCProtoDev&) = delete;
        TFacCQCProtoDev(TFacCQCProtoDev&&) = delete;

        ~TFacCQCProtoDev();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCProtoDev& operator=(const TFacCQCProtoDev&) = delete;
        TFacCQCProtoDev& operator=(TFacCQCProtoDev&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSystem() const;

        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        const TCQCSecToken& sectUser() const
        {
            return m_cuctxToUse.sectUser();
        }

    private :
        // -------------------------------------------------------------------
        //  Private non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoLogon();

        tCIDLib::TBoolean bParseParms
        (
                    tCIDLib::TBoolean&      bIgnoreState
        );

        tCIDLib::TVoid LoadCfg
        (
                    TArea&                  areaFrame
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bSystem
        //      If the /System command line parm is set, then this is set
        //      to true. It's not advertised. It's for us to work on drivers
        //      internally. It causes a generated driver pack to be marked
        //      as a system driver, else it'll be marked as User, which will
        //      will make the manifest and protocol file go into the User
        //      section on the file server when imported.
        //
        //  m_cuctxToUse
        //      We set up a user context for the user account that is logged
        //      in under, which we need to pass to various things.
        //
        //  m_strAppTitle
        //      We use this app title in our own title bar and in any msg
        //      boxes we pop up, so load it once and keep it around.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bSystem;
        TCQCUserCtx         m_cuctxToUse;
        TString             m_strAppTitle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCProtoDev,TGUIFacility)
};


