// ----------------------------------------------------------------------------
//  FILE: CQCWCHelper_CtrlServerBase.hpp
//  DATE: Tue, Jan 19 17:39:55 2021 -0500
//    ID: D2CE20750A902C56-3E26CBF9499D7A90
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

class  TCQCWCHelperServerBase : public TOrbServerBase
{
    public :
        // --------------------------------------------------------------------
        // Public, static data
        // --------------------------------------------------------------------
        static const TString strInterfaceId;
        
        // ------------------------------------------------------------------------
        //  These are the available keys that the helper program will accept in the
        //  parameters string it gets in the initial blob of data passed to it
        //  by the controlling application.
        //                  
        // ------------------------------------------------------------------------
        static const TString strParam_CustAR;


        // --------------------------------------------------------------------
        // Constructors and destructor
        // --------------------------------------------------------------------
        TCQCWCHelperServerBase(const TCQCWCHelperServerBase&) = delete;
        TCQCWCHelperServerBase(TCQCWCHelperServerBase&&) = delete;
        ~TCQCWCHelperServerBase();

        // --------------------------------------------------------------------
        // Public operators
        // --------------------------------------------------------------------
        TCQCWCHelperServerBase& operator=(const TCQCWCHelperServerBase&) = delete;
        TCQCWCHelperServerBase& operator=(TCQCWCHelperServerBase&&) = delete;

        // --------------------------------------------------------------------
        // Public, pure virtual methods
        // --------------------------------------------------------------------
        virtual tCIDLib::TVoid Close() = 0;

        virtual tCIDLib::TVoid Initialize
        (
            const TArea& areaPar
            , const TArea& areaWidget
            , const tCIDLib::TBoolean bVisibility
        ) = 0;

        virtual tCIDLib::TVoid Ping() = 0;

        virtual tCIDLib::TVoid SetArea
        (
            const TArea& areaPar
            , const TArea& areaWidget
        ) = 0;

        virtual tCIDLib::TVoid SetMute
        (
            const tCIDLib::TBoolean bNewState
        ) = 0;

        virtual tCIDLib::TVoid SetVisibility
        (
            const tCIDLib::TBoolean bNewState
        ) = 0;

        virtual tCIDLib::TVoid SetVolume
        (
            const tCIDLib::TCard4 c4ToSet
        ) = 0;

    protected :
        // --------------------------------------------------------------------
        // Hidden constructors
        // --------------------------------------------------------------------
        TCQCWCHelperServerBase();
        TCQCWCHelperServerBase(const TOrbObjId& ooidThis);

        // --------------------------------------------------------------------
        // Protected, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid Dispatch
        (
            const  TString&      strMethodName
            ,      TOrbCmd&      orbcToDispatch
        )   override;

    private :
        // --------------------------------------------------------------------
        // Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TCQCWCHelperServerBase,TOrbServerBase)
};

#pragma CIDLIB_POPPACK

