//
// FILE NAME: MQTTS_IOEvent.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/04/2019
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
//
// CAVEATS/GOTCHAS:
//
// LOG:
//



// ---------------------------------------------------------------------------
///  CLASS: TIOEventPool
// PREFIX: spl
// ---------------------------------------------------------------------------
class TIOEventPool : public TFixedSizePool<TMQTTIOEvent>
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIOEventPool() :

            TFixedSizePool(1024, L"MQTT I/O Event Pool", tCIDLib::EMTStates::Safe)
        {
        }

        TIOEventPool(const TIOEventPool&) = delete;

        ~TIOEventPool()
        {
        }


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIOEventPool& operator=(const TIOEventPool&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        [[nodiscard]] TMQTTIOEvent* pelemMakeNew()   override
        {
            return new TMQTTIOEvent();
        }

        tCIDLib::TVoid PrepElement(TMQTTIOEvent& ioevTar) override
        {
            ioevTar.Reset();
        }
};



// ---------------------------------------------------------------------------
///  CLASS: TDrvEventPool
// PREFIX: spl
// ---------------------------------------------------------------------------
class TDrvEventPool : public TFixedSizePool<TMQTTDrvEvent>
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDrvEventPool() :

            TFixedSizePool(1024, L"MQTT Drv Event Pool", tCIDLib::EMTStates::Safe)
        {
        }

        TDrvEventPool(const TDrvEventPool&) = delete;

        ~TDrvEventPool()
        {
        }


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDrvEventPool& operator=(const TDrvEventPool&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        [[nodiscard]] TMQTTDrvEvent* pelemMakeNew()
        {
            return new TMQTTDrvEvent();
        }

        tCIDLib::TVoid PrepElement(TMQTTDrvEvent& drvevTar)
        {
            drvevTar.Reset();
        }
};



// ---------------------------------------------------------------------------
//  CLASS: TInMsgPool
// PREFIX: spl
// ---------------------------------------------------------------------------
class TInMsgPool : public TSimplePool<TMQTTInMsg>
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInMsgPool() :

            TSimplePool(1024, L"MQTT In Msg Pool", tCIDLib::EMTStates::Safe)
        {
        }

        TInMsgPool(const TInMsgPool&) = delete;

        ~TInMsgPool()
        {
        }


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TInMsgPool& operator=(const TInMsgPool&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4ElemSize(const TMQTTInMsg& msgSrc) override
        {
            return msgSrc.c4BufSize();
        }

        [[nodiscard]] TMQTTInMsg* pelemMakeNew(const tCIDLib::TCard4 c4Size) override
        {
            return new TMQTTInMsg(c4Size);
        }

        tCIDLib::TVoid PrepElement(TMQTTInMsg& msgTar, const tCIDLib::TCard4 c4Size) override
        {
            msgTar.Reset(c4Size);
        }
};



// ---------------------------------------------------------------------------
//  CLASS: TOutMsgPool
// PREFIX: spl
// ---------------------------------------------------------------------------
class TOutMsgPool : public TSimplePool<TMQTTOutMsg>
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TOutMsgPool() :

            TSimplePool(1024, L"MQTT out Msg Pool")
        {
        }

        TOutMsgPool(const TOutMsgPool&) = delete;

        ~TOutMsgPool()
        {
        }


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TOutMsgPool& operator=(const TOutMsgPool&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4ElemSize(const TMQTTOutMsg& msgSrc) override
        {
            return msgSrc.c4BufSize();
        }

        [[nodiscard]] TMQTTOutMsg* pelemMakeNew(const tCIDLib::TCard4 c4Size) override
        {
            return new TMQTTOutMsg(c4Size);
        }

        tCIDLib::TVoid PrepElement(TMQTTOutMsg& msgTar, const tCIDLib::TCard4 c4Size) override
        {
            msgTar.Reset(c4Size);
        }
};


