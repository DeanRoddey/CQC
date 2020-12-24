//
// FILE NAME: CQCIntfEng_AdjustButton.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/05/2004
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
//  This is the header for the CQCIntfEng_AdjustButton.cpp file, which
//  implements a specialized button that does inc/dec operations.
//
//  This class is only here to insure that old files can be read. This type of
//  widget is no longer supported.
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
//   CLASS: TCQCIntfAdjustButton
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfAdjustButton

    : public TCQCIntfPushButton
    , public MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfAdjustButton();

        TCQCIntfAdjustButton
        (
            const   TCQCIntfAdjustButton&   iwdgSrc
        );

        ~TCQCIntfAdjustButton();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfAdjustButton& operator=
        (
            const   TCQCIntfAdjustButton&   iwdgSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FieldChanged
        (
                    TCQCFldPollInfo&        cfpiAssoc
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        )   override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

        tCIDLib::TVoid ValueUpdate
        (
                    TCQCPollEngine&         polleToUse
            , const tCIDLib::TBoolean       bNoRedraw
            , const tCIDLib::TBoolean       bVarsUpdate
            , const TStdVarsTar&            ctarGlobalVars
            , const TGUIRegion&             grgnClip
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfAdjustButton,TCQCIntfPushButton)
        DefPolyDup(TCQCIntfAdjustButton)
};


#pragma CIDLIB_POPPACK


