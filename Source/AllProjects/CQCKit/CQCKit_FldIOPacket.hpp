//
// FILE NAME: CQCKit_FldIOPacket.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/08/2001
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
//  This is the header for the CQCKit_FldIOPacket.cpp file, which implements
//  the TFldIOPacket class. When a client wants to use the 'scatter/gather'
//  type field read/write methods provided by the CQCServer IDL, it must
//  provide information on the fields it wants to read, and the drivers that
//  those fields belong to. The information provided must be, in order of
//  their hierarchical importance:
//
//      1.  Driver list id
//      2.  The driver id (for each driver involved)
//      3.  The field list id (for each driver involved)
//      4.  The field id (for each field involved)
//      5.  The field serial number (for each field involved)
//
//  Basically, in order to read a set of arbitrary fields from a particular
//  CQCServer, the client must gather the indentifiers, cache them, and pass
//  them back in when it wants to read the fields.
//
//  All of this info is available to clients. It is assumed that they will
//  build up this list of info and use it over and over as they poll or write
//  data.
//
//  The sequence of events would be:
//
//      1.  Call CQCServer's c4QueryDriverIds() method. This will return the
//          driver list id, and the list of driver ids.
//      2.  For each driver of interest, call c4QueryFields() to get the field
//          list id and the list of fields.
//
//  The two list ids are used as stamps. They must be passed back in in order
//  to do the field read/write and will be checked to make sure that the list
//  of drivers, or fields within those drivers, haven't changed. If they have,
//  then an exception is thrown to tell the client it must re-query its field
//  info.
//
//  Each field has a 'serial number' that is bumped each time it's value is
//  changed, so we can track changes in individual fields. The latest serial
//  numbers for each field is returned if the field changed since the last
//  read. On each subsequent call, if the serial number hasn't changed, then
//  the field value hasn't changed and therefore no data is returned for that
//  field. This vastly improves polling efficiency. The serial number is set
//  to zero when a new item is added to the I/O list, which guarantees a read
//  of the value on the first call, since zero is never a valid serial number.
//
//  Since the methods that use this class are targeted towards a particular
//  CQCServer, it can only read fields from drivers on a single CQCServer at
//  once. It can read from multiple drivers on that CQCServer though.
//
//  We create a small class, TFldIOData, which holds the per-field info that
//  we need to track.
//
// CAVEATS/GOTCHAS:
//
//  1)  Note that you don't have to have any fields in the I/O list. You can
//      have just drivers without any of their fields. By doing a poll call
//      to the CQCServer with just drivers, effectively you are just checking
//      to see if any changes have occured in the driver list or in the field
//      list of any of the drivers, which will cause the usual out of sync
//      exception.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TFldIOData
//  PREFIX: fiod
// ---------------------------------------------------------------------------
class CQCKITEXPORT TFldIOData : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TFldIOData();

        TFldIOData
        (
            const  tCIDLib::TCard4          c4FieldId
            , const tCQCKit::EFldTypes      eFldType
        );

        TFldIOData(const TFldIOData&) = default;
        TFldIOData(TFldIOData&&) = default;

        ~TFldIOData();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFldIOData& operator=(const TFldIOData&) = default;
        TFldIOData& operator=(TFldIOData&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4FieldId() const;

        tCIDLib::TCard4 c4FieldId
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4SerialNum() const;

        tCIDLib::TCard4 c4SerialNum
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCQCKit::EFldTypes eFldType() const;

        tCQCKit::EFldTypes eFldType
        (
            const   tCQCKit::EFldTypes      eToSet
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


        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FieldId;
        tCIDLib::TCard4     m_c4SerialNum;
        tCQCKit::EFldTypes  m_eFldType;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TFldIOData,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TFldIOPacket
//  PREFIX: fiop
// ---------------------------------------------------------------------------
class CQCKITEXPORT TFldIOPacket : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFldIOPacket();

        TFldIOPacket
        (
            const   tCIDLib::TCard4         c4DriverListId
        );

        TFldIOPacket(const TFldIOPacket&) = default;
        TFldIOPacket(TFldIOPacket&&) = default;

        ~TFldIOPacket();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFldIOPacket& operator=(const TFldIOPacket&) = default;
        TFldIOPacket& operator=(TFldIOPacket&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsEmpty() const;

        tCIDLib::TBoolean bNoFields() const;

        tCIDLib::TCard4 c4AddField
        (
            const   tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldId
            , const tCQCKit::EFldTypes      eType
        );

        tCIDLib::TCard4 c4AddDriver
        (
            const   tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FldListId
        );

        tCIDLib::TCard4 c4AddOrFindField
        (
            const   tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FldListId
            , const tCIDLib::TCard4         c4FieldId
            , const tCQCKit::EFldTypes      eType
        );

        tCIDLib::TCard4 c4DriverCount() const;

        tCIDLib::TCard4 c4DriverIdAt
        (
            const   tCIDLib::TCard4         c4DrvIndex
        )   const;

        tCIDLib::TCard4 c4DriverIndexForId
        (
            const   tCIDLib::TCard4         c4DriverId
        )   const;

        tCIDLib::TCard4 c4DriverListId() const;

        tCIDLib::TCard4 c4FieldCount
        (
            const   tCIDLib::TCard4         c4DrvId
        )   const;

        tCIDLib::TCard4 c4FieldCountAt
        (
            const   tCIDLib::TCard4         c4DrvIndex
        )   const;

        tCIDLib::TCard4 c4FieldIndexForId
        (
            const   tCIDLib::TCard4         c4DrvIndex
            , const tCIDLib::TCard4         c4FldId
        )   const;

        tCIDLib::TCard4 c4FieldListIdAt
        (
            const   tCIDLib::TCard4         c4DrvIndex
        )   const;

        tCIDLib::TCard4 c4PacketId() const;

        const TFldIOData& fiodAt
        (
            const   tCIDLib::TCard4         c4DrvIndex
            , const tCIDLib::TCard4         c4FldIndex
        )   const;

        TFldIOData& fiodAt
        (
            const   tCIDLib::TCard4         c4DrvIndex
            , const tCIDLib::TCard4         c4FldIndex
        );

        const TFldIOData& fiodById
        (
            const   tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldId
        )   const;

        TFldIOData& fiodById
        (
            const   tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldId
        );

        tCIDLib::TVoid RemoveDriverAt
        (
            const   tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid RemoveFieldAt
        (
            const   tCIDLib::TCard4         c4DrvAt
            , const tCIDLib::TCard4         c4FldAt
        );

        tCIDLib::TVoid Reset
        (
            const   tCIDLib::TCard4         c4DriverListId
        );

        tCIDLib::TVoid SetSerialNum
        (
            const   tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldId
            , const tCIDLib::TCard4         c4SerialNum
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
        //   CLASS: TDriverInfo
        //  PREFIX: drvi
        //
        //  TDriverInfo contains the info we need to track for a single driver.
        //  We have to keep its driver id, field list id, and the list of
        //  field data objects.
        // -------------------------------------------------------------------
        using TFldList = TVector<TFldIOData>;

        class TDriverInfo : public TObject, public MStreamable
        {
            public :
                // -----------------------------------------------------------
                //  Constructors and Destructor
                // -----------------------------------------------------------
                TDriverInfo();

                TDriverInfo
                (
                    const   tCIDLib::TCard4 c4DriverId
                    , const tCIDLib::TCard4 c4FldListId
                );

                TDriverInfo(const TDriverInfo&) = default;
                TDriverInfo(TDriverInfo&&) = default;

                ~TDriverInfo();


                // -----------------------------------------------------------
                //  Public operators
                // -----------------------------------------------------------
                TDriverInfo& operator=(const TDriverInfo&) = default;
                TDriverInfo& operator=(TDriverInfo&&) = default;


                // -----------------------------------------------------------
                //  Public, non-virtual methods
                // -----------------------------------------------------------
                tCIDLib::TCard4 c4AddField
                (
                    const   tCIDLib::TCard4 c4FieldId
                    , const tCQCKit::EFldTypes eType
                );

                tCIDLib::TCard4 c4AddOrFindField
                (
                    const   tCIDLib::TCard4 c4FieldId
                    , const tCQCKit::EFldTypes eType
                );

                tCIDLib::TCard4 c4DriverId() const;

                tCIDLib::TCard4 c4FieldCount() const;

                tCIDLib::TCard4 c4FieldListId() const;

                tCIDLib::TCard4 c4FieldIndexForId
                (
                    const   tCIDLib::TCard4         c4FldId
                )   const;

                const TFldIOData& fiodAt
                (
                    const   tCIDLib::TCard4 c4Index
                )   const;

                TFldIOData& fiodAt
                (
                    const   tCIDLib::TCard4 c4Index
                );

                const TFldIOData& fiodById
                (
                    const   tCIDLib::TCard4 c4FieldId
                )   const;

                TFldIOData& fiodById
                (
                    const   tCIDLib::TCard4 c4FieldId
                );

                tCIDLib::TVoid RemoveFieldAt
                (
                    const   tCIDLib::TCard4 c4At
                );

                tCIDLib::TVoid Reset
                (
                    const   tCIDLib::TCard4 c4DriverId
                    ,       tCIDLib::TCard4 c4FldListId
                );


            protected :
                // -----------------------------------------------------------
                //  Protected, inherited methods
                // -----------------------------------------------------------
                tCIDLib::TVoid StreamFrom
                (
                            TBinInStream&           strmToReadFrom
                )   final;

                tCIDLib::TVoid StreamTo
                (
                            TBinOutStream&          strmToWriteTo
                )   const final;


            private :
                // -----------------------------------------------------------
                //  Private data members
                //
                //  m_c4DriverId
                //      The id for the driver represented by this object.
                //
                //  m_c4FldListId
                //      The field list id for this driver.
                //
                //  m_colFlds
                //      A collection of field data objects, which is just a
                //      trivial class that holds a field id, the serial
                //      number (which is updated on each read so that we can
                //      keep up with what has changed), and the data type of
                //      the field.
                // -----------------------------------------------------------
                tCIDLib::TCard4     m_c4DriverId;
                tCIDLib::TCard4     m_c4FldListId;
                TFldList            m_colFldIds;


                // -----------------------------------------------------------
                //  Magic macros
                // -----------------------------------------------------------
                RTTIDefs(TDriverInfo,TObject)
        };
        using TDriverList = TVector<TDriverInfo>;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4DriverListId
        //      The current driver list id for this packet. This will be
        //      checked by the server to see if our view of the drivers
        //      and their fields is still in sync at the highest level.
        //
        //  m_c4PacketId
        //      This is bumped every time we are reset or the list is changed
        //      in some way. This is used by the results class to know when
        //      it is out of date relative to us.
        //
        //  m_pcolDriverList
        //      The list of TDriverInfo objects that are to have fields in
        //      this field I/O packet.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4DriverListId;
        tCIDLib::TCard4     m_c4PacketId;
        TDriverList         m_colDriverList;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TFldIOPacket,TObject)
};

#pragma CIDLIB_POPPACK


