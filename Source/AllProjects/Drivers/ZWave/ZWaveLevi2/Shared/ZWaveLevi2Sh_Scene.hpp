//
// FILE NAME: ZWaveLevi2Sh_Scene.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/01/2014
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
//  This is the header file for the ZWaveLevi2Sh_Scene.cpp file, which implements
//  the TZWCmdClass derivatives for scene related command classes.
//
//  See the parent class for details.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZWCCSceneActConf
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCCSceneActConf : public TZWCmdClass
{
    public :
        // -------------------------------------------------------------------
        //  Consructors and Destructor
        // -------------------------------------------------------------------
        TZWCCSceneActConf
        (
            const   TZWDevClass&            zwdcInst
        );

        ~TZWCCSceneActConf();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHandleGenReport
        (
            const   tCIDLib::TCardList&     fcolVals
            , const TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
            , const tCIDLib::TCard4         c4FromInst
        );

        tCIDLib::TVoid InitVals
        (
            const   TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TZWCCSceneActConf();
        TZWCCSceneActConf(const TZWCCSceneActConf&);
        tCIDLib::TVoid operator=(const TZWCCSceneActConf&);


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4LastScene
        //      We have to keep up with the id of the last scene invoked. We only
        //      want to send out a user action if we see a new one. They can't
        //      invoke the same one two times in a row. We init to 0, which is not
        //      a valid scene number, so we'll trigger the first time.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4LastScene;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCSceneActConf, TZWCmdClass)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCSceneActivation
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCCSceneActivation : public TZWCmdClass
{
    public :
        // -------------------------------------------------------------------
        //  Consructors and Destructor
        // -------------------------------------------------------------------
        TZWCCSceneActivation
        (
            const   TZWDevClass&            zwdcInst
        );

        ~TZWCCSceneActivation();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHandleGenReport
        (
            const   tCIDLib::TCardList&     fcolVals
            , const TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
            , const tCIDLib::TCard4         c4FromInst
        );

        tCIDLib::TVoid InitVals
        (
            const   TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TZWCCSceneActivation();
        TZWCCSceneActivation(const TZWCCSceneActivation&);
        tCIDLib::TVoid operator=(const TZWCCSceneActivation&);


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4LastScene
        //      We have to keep up with the id of the last scene invoked. We only
        //      want to send out a user action if we see a new one. They can't
        //      invoke the same one two times in a row. We init to 0, which is not
        //      a valid scene number, so we'll trigger the first time.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4LastScene;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCSceneActivation, TZWCmdClass)
};



#pragma CIDLIB_POPPACK


