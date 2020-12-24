//
// FILE NAME: CQCIntfView_StateData.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/08/2002
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
//  The interface designer doesn't have very complex state data requirements.
//  It just needs to remember its last open position and the id of the last
//  template it was viewing, etc... This class holds that data. It is
//  persisted in the standard client object store. We derive from a common
//  frame window state class in CQCGKit, which does most of the work.
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
//   CLASS: TCQCIntfViewState
//  PREFIX: fst
// ---------------------------------------------------------------------------
class TCQCIntfViewState : public TCQCFrameState
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfViewState();

        TCQCIntfViewState
        (
            const   TArea&                  areaFrame
            , const TString&                strIntfId
        );

        TCQCIntfViewState
        (
            const   TString&                strIntfId
        );

        TCQCIntfViewState
        (
            const   TCQCIntfViewState&      fstSrc
        );

        ~TCQCIntfViewState();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfViewState& operator=
        (
            const   TCQCIntfViewState&      fstSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ClearLastTmpl();

        const TString& strLastTmpl() const;

        const TString& strLastTmpl
        (
            const   TString&                strIntfId
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strIntfId
        //      This is the id of the interface we were last showing. We use
        //      this to go back to the interface server and get it again on
        //      startup.
        // -------------------------------------------------------------------
        TString m_strIntfId;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfViewState,TCQCFrameState)
};

#pragma CIDLIB_POPPACK



