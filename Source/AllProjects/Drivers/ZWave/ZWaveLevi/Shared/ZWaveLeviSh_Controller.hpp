//
// FILE NAME: ZWaveLevih_Controller.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/14/2012
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
//  This is the header file for the ZWaveLevi_Controller.cpp file, which
//  implements a derivative of the basic unit class to represent controllers.
//  These are fairly simple since the VRCOP itself we deal with directly.
//  This is more to deal with scene controllers, though we also allow one
//  of these to be marked as the VRCOP which is the one we'll set up
//  async notification associations to.
//
//  Primarly what we are for is to handle incoming scene activiation
//  reports, which we will then convert to a user action event.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZWController
//  PREFIX: unit
//
//  This class represents a controller. We look for all general controller
//  units.
// ---------------------------------------------------------------------------
class ZWLEVISHEXPORT TZWController : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWController();

        TZWController
        (
            const   tCIDLib::TCard4         c4Id
        );

        TZWController
        (
            const   TZWController&          unitSrc
        );

        ~TZWController();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWController& operator=
        (
            const   TZWController&          unitSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TZWController&          unitSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TZWController&          unitSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bStoreFldIds
        (
            const   TVector<TCQCFldDef>&    colFlds
            , const tZWaveLeviSh::TValList& fcolIds
        );

        tCIDLib::TBoolean bStoreGenReport
        (
            const   tCIDLib::TCard4         ac4Values[]
            , const tCIDLib::TCard4         c4ValCnt
            ,       TCQCServerBase&         sdrvLevi
        );

        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        virtual tCIDLib::TBoolean bWantsOptInfo
        (
                    tCIDLib::TStrCollect&   colToFill
        )   const;

        tCIDLib::TCard4 c4QueryFldDefs
        (
                    TVector<TCQCFldDef>&    colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const;

        tCIDLib::TVoid PrepForUse();

        tCIDLib::TVoid SetFieldErrs
        (
                    TCQCServerBase&         sdrvLevi
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsVRCOP() const;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckSendTrigger
        (
            const   tCIDLib::TCard4         c4SceneNum
            ,       TCQCServerBase&         sdrvLevi
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4LastScene
        //      For a scene controller we have to keep up with the id of the
        //      last scene invoked. We only want to send out a user action
        //      if we see a new one. They can't invoke the same one two times
        //      in a row. We init to 0, which is not a valid scene number, so
        //      we'll trigger the first time.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4LastScene;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWController,TZWaveUnit)
        DefPolyDup(TZWController)
};


#pragma CIDLIB_POPPACK


