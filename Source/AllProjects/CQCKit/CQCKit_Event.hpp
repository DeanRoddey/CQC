//
// FILE NAME: CQCKit_Event.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/09/2005
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
//  This is the header for the CQCKit_Event.cpp file, which provides an
//  in-memory representation of a CQC event. These are sent out when things
//  happen, and other applications can receive them and react.
//
//  On the wire these are encrypted for security purposes since they are broadcast.
//  If you format one to a text stream, you'll get the structured text version
//  nicely formatted. For binary streaming, we stream the text to buffer based
//  text stream with a UTF-8 converter. On streaming back in, the text is parsed
//  back out again.
//
//  The format is a set of blocks, which in theory could be nested so the
//  structure is a tree structure. We provide access to these values via a
//  path, like /lvl1/lvl2/value, where all but the last component represent
//  non-terminal nodes in the tree. The last component is the key of a value
//  inside the last NT node.
//
//  A standard set of events for drivers are supported. There is a method in
//  the driver base class to send these out conveniently and this class
//  in turn provides a method to build them up so that all the details are
//  hidden. There is an enumeration in the IDL for this facility that
//  provides the class names for these types of events.
//
//  The events we currently support are in this form (leaving out some internal
//  only information):
//
//  Field value change event
//
//        cqsl.header
//        {
//            class=cqsl.fldchange
//            source=cqsl.field:moniker.field
//        }
//        cqsl.fldval
//        {
//            val=[-/+xxx]
//            trunc=[yes|no]
//        }
//
//      So it indicates a field change and source indicates the field that
//      changed. The event specific block indicates the new value, and if
//      the value in val= has been truncated due to length or not.
//
//      These are only sent if the the driver is configured to send them,
//      either on any change or if the new value meets satisfies an
//      expression.
//
//
//  Load Change
//
//        cqsl.header
//        {
//            class=cqsl.loadchange
//            source=cqsl.field:moniker.field
//        }
//        cqsl.loadinfo
//        {
//            state=[off|on]
//            loadnum=[1...x]
//            name=xxx
//        }
//
//      This type of event is sent by lighting type drivers to indicate that
//      a load has changed state. The off/on state is sent. Only on/off events
//      should trigger this, not dimming events, and hence the field should
//      always be a boolean on/off field. The load number is passed
//      as well in case it is of use.
//
//
//  Lock Status
//
//        cqsl.header
//        {
//            class=cqsl.lockstatus
//            source=cqsl.dev:moniker
//        }
//        cqsl.lockinfo
//        {
//            state=[locked|unlocked]
//            lockid=[some indentifier, driver specific]
//            code=[optional code if available]
//            type=[manual|other|pad|remote]
//        }
//
//      This type of event is sent by devices that manage locks, when the
//      lock is somehow locked or unlocked, either manually or through some
//      sort of control means. If the type is 'pad', i.e. the operation was
//      done through the keypad, then the entered code is in the code value,
//      if the lock provides it. Some models may not.
//
//
//  Motion Detected
//
//        cqsl.header
//        {
//            class=cqsl.motion
//            source=cqsl.field:moniker.field
//        }
//        cqsl.motioninfo
//        {
//            type=[start|end]
//            sensornum=[1...x]
//            name=xxx
//        }
//
//      This one is sent from drivers that support motion detectors. It will
//      send the sensor number and whether the event is a start motion or
//      end motion event. The sensor number is sent as well in case it is
//      of use.
//
//
//  Presence Event
//
//        cqsl.header
//        {
//            class=cqsl.presence
//            source=cqsl.dev:moniker
//        }
//        cqsl.presenceinfo
//        {
//            type=[enter|exit]
//            idinfo=[extra identity info]
//            area=[area id]
//        }
//
//      This one is sent from drivers that support presence reporting. It
//      indicates whether it is an enter or exit type event. The idinfo field
//      is to allow arbitrary identity info to be passed along to be reacted
//      to. An area identifier can be provided as well to indicate the specific
//      area that was exited or entered.
//
//
//  User action event
//
//        cqsl.header
//        {
//            class=cqsl.useract
//            source=cqsl.dev:moniker
//        }
//        cqsl.actinfo
//        {
//            evtype=xxx
//            evdata=yyy
//        }
//
//      This one is for things like the user pressing a button on the device
//      so that the system can react to it. The evtype and evdata are purely
//      up to the particular driver that sends the event.
//
//
//  Zone Alarm
//
//        cqsl.header
//        {
//            class=cqsl.zonealarm
//            source=cqsl.field:moniker.field
//        }
//        cqsl.zoneinfo
//        {
//            state=[secure|notready|violated]
//            zonenum=[1..x]
//            name=xxx
//        }
//
//      This one is sent when a zone goes into or out of violated state.
//      The zone number is indicated and the new state it has gone into.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TCQCEvent
// PREFIX: cev
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCEvent : public TObject, public MFormattable, public MStreamable
{
    public  :
        // -------------------------------------------------------------------
        //  Some aliases to make the code below more readable.
        // -------------------------------------------------------------------
        using TBlockCol = TBasicTreeCol<TString>;
        using TBlockNode =  TBlockCol::TNodeNT;
        using TKeyNode = TBlockCol::TNodeT;

        enum class ETokenTypes
        {
            Block
            , Key
            , EOB
            , EOS
        };


        // -------------------------------------------------------------------
        //  Some common event strings
        // -------------------------------------------------------------------
        static const TString strHdr_ActInfo;
        static const TString strHdr_FldVal;
        static const TString strHdr_Std;
        static const TString strPath_ActData;
        static const TString strPath_ActType;
        static const TString strPath_Class;
        static const TString strPath_FldValue;
        static const TString strPath_LoadName;
        static const TString strPath_LoadNum;
        static const TString strPath_LoadState;
        static const TString strPath_LockCode;
        static const TString strPath_LockId;
        static const TString strPath_LockState;
        static const TString strPath_LockType;
        static const TString strPath_MotionName;
        static const TString strPath_MotionNum;
        static const TString strPath_MotionType;
        static const TString strPath_PresArea;
        static const TString strPath_PresIdInfo;
        static const TString strPath_PresType;
        static const TString strPath_Source;
        static const TString strPath_SysID;
        static const TString strPath_Version;
        static const TString strPath_ZoneName;
        static const TString strPath_ZoneNum;
        static const TString strPath_ZoneStatus;
        static const TString strSrc_DevPref;
        static const TString strSrc_FldPref;

        static const TString strVal_LockState_Locked;
        static const TString strVal_LockState_Unlocked;

        static const TString strVal_LockType_Manual;
        static const TString strVal_LockType_Other;
        static const TString strVal_LockType_Pad;
        static const TString strVal_LockType_Remote;


        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCEvent();

        TCQCEvent
        (
            const   TCQCEvent&              cevSrc
        );

        TCQCEvent
        (
                    TCQCEvent&&             cevSrc
        );

        ~TCQCEvent();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCEvent& operator=
        (
            const   TCQCEvent&              cevSrc
        );

        TCQCEvent& operator=
        (
                    TCQCEvent&&             cevSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TCQCEvent&              cevSrc
        )   const;

        tCIDLib::TBoolean operator!=(const TCQCEvent& cevSrc) const
        {
            return !operator==(cevSrc);
        }



        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid Reset();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddBlock
        (
            const   TString&                strParent
            , const TString&                strName
        );

        tCIDLib::TVoid AddValue
        (
            const   TString&                strParent
            , const TString&                strName
            , const TString&                strValue
        );

        tCIDLib::TBoolean bIsDrvEv
        (
            const   tCQCKit::EStdDrvEvs     eToCheck
        )   const;

        tCIDLib::TBoolean bIsDrvEvForDev
        (
            const   tCQCKit::EStdDrvEvs     eToCheck
            , const TString&                strDev
        )   const;

        tCIDLib::TBoolean bIsDrvEvForField
        (
            const   tCQCKit::EStdDrvEvs     eToCheck
            , const TString&                strFldName
        )   const;

        tCIDLib::TBoolean bIsDrvEvVal
        (
            const   tCQCKit::EStdDrvEvs     eToCheck
            , const TString&                strFrom
            , const TString&                strVal1
        )   const;

        tCIDLib::TBoolean bIsDrvEvVals
        (
            const   tCQCKit::EStdDrvEvs     eToCheck
            , const TString&                strFrom
            , const TString&                strVal1
            , const TString&                strVal2
        )   const;

        tCIDLib::TBoolean bIsThisDrvEv
        (
            const   tCQCKit::EStdDrvEvs     eToCheck
            , const TString&                strVal1
        )   const;

        tCIDLib::TBoolean bIsThisDrvEv
        (
            const   tCQCKit::EStdDrvEvs     eToCheck
            , const TString&                strVal1
            , const TString&                strVal2
        )   const;

        tCIDLib::TBoolean bIsThisDrvEvV2
        (
            const   tCQCKit::EStdDrvEvs     eToCheck
            , const TString&                strVal2
        )   const;

        tCIDLib::TBoolean bIsOfClass
        (
            const   TString&                strClass
        )   const;

        tCIDLib::TBoolean bQueryFldSrc
        (
            COP     TString&                strMoniker
            , COP   TString&                strField
        )   const;

        tCIDLib::TBoolean bQuerySrc
        (
            COP     TString&                strToFill
        )   const;

        tCIDLib::TBoolean bValueAs
        (
            const   TString&                strPath
        )   const;

        tCIDLib::TBoolean bValueEquals
        (
            const   TString&                strPath
            , const TString&                strCompVal
        )   const;

        tCIDLib::TBoolean bValueExists
        (
            const   TString&                strPath
        )   const;

        tCIDLib::TBoolean bValueExists
        (
            const   TString&                strPath
            ,       TString&                strValue
        )   const;

        tCIDLib::TVoid BuildStdDrvEvent
        (
            const   tCQCKit::EStdDrvEvs     eEvent
            , const TString&                strMoniker
            , const TString&                strField
            , const TString&                strVal1
            , const TString&                strVal2
            , const TString&                strVal3
            , const TString&                strVal4
        );

        [[nodiscard]] tCIDLib::TCard4 c4ValueAs
        (
            const   TString&                strPath
            , const tCIDLib::ERadices       eRadix = tCIDLib::ERadices::Dec
        )   const;

        [[nodiscard]] tCIDLib::TCard8 c8ValueAs
        (
            const   TString&                strPath
            , const tCIDLib::ERadices       eRadix = tCIDLib::ERadices::Dec
        )   const;

        [[nodiscard]] tCIDLib::TFloat8 f8ValueAs
        (
            const   TString&                strPath
        )   const;

        [[nodiscard]] tCIDLib::TInt4 i4ValueAs
        (
            const   TString&                strPath
            , const tCIDLib::ERadices       eRadix = tCIDLib::ERadices::Dec
        )   const;

        tCIDLib::TVoid ParseFrom
        (
                    TTextInStream&          strmSrc
        );

        [[nodiscard]] const TString& strClass() const;

        [[nodiscard]] const TString& strSource() const;

        const TString& strValue
        (
            const   TString&                strPath
        )   const;

        tCIDLib::TVoid SetValue
        (
            const   TString&                strPath
            , const TString&                strValue
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FormatTo
        (
                    TTextOutStream&         strmTar
        )   const final;

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
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AppendLevel
        (
                    TString&                strAddTo
            , const TString&                strToAdd
        )   const;

        tCIDLib::TBoolean bGetNextLine
        (
                    TTextInStream&          strmSrc
            ,       TString&                strToFill
            , const tCIDLib::TBoolean       bEOSOk = kCIDLib::False
        );

        tCIDLib::TVoid BuildHeader
        (
            const   TString&                strClass
            , const TString&                strSource
        );

        ETokenTypes eFindNext
        (
                    TTextInStream&          strmSrc
            ,       TString&                strName
            ,       TString&                strValue
            , const tCIDLib::TBoolean       bEOSOk = kCIDLib::False
        );

        tCIDLib::TVoid FormatBlock
        (
                        TTextOutStream&     strmTar
            , const     TBlockNode&         nodePar
        )   const;

        tCIDLib::TVoid LoadInfo() const;

        tCIDLib::TVoid ParseBlock
        (
                    TTextInStream&          strmSrc
            ,       TString&                strTmp1
            ,       TString&                strTmp2
            ,       TString&                strCur
        );

        const TString& strCheckValNode
        (
            const   TString&                strPath
        )   const;

        tCIDLib::TVoid ThrowCvtErr
        (
            const   TString&                strPath
            , const tCIDLib::TCh* const     pszType
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4LineNum
        //      Used in parsing, to keep up with the current line number.
        //
        //  m_colBlocks
        //      We use a tree collection to hold the potentially nested set of blocks
        //      that follow the header. Since we are basing our format on the xAP
        //      format, we set it to non-case sensitive paths, since xAP is not case
        //      sensitive. And we tell it not to sort nodes by name, since order is
        //      important for us.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4LineNum;
        TBlockCol       m_colBlocks;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCEvent,TObject)
};

#pragma CIDLIB_POPPACK

