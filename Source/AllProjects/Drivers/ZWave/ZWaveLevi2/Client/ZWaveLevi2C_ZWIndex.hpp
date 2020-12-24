//
// FILE NAME: ZWaveLevi2C_ZWIndex.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/26/2014
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
//  This file implements the Z-Wave device info index. We load this from the index
//  file (which is created via the ZWDIComp program and put into the release.) It
//  is divided into manufacturers. We create a few different views of the data to
//  make it easier to find something.
//
//  We create a small class to hold the actual data we parse from the file, and
//  then an index class that holds the list of device info objects and provides some
//  different sorted views.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWDIInfo
//  PREFIX: zwdii
// ---------------------------------------------------------------------------
class TZWDIInfo
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWDIInfo();

        ~TZWDIInfo();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ParseFromXML
        (
            const   TString&                strMake
            , const TXMLTreeElement&        xtnodeRoot
        );


        // -------------------------------------------------------------------
        //  Public class members
        //
        //  m_strFileName
        //      The base part of the file name that holds the device info for this
        //      device. We only get a subset of it in the index and we need to set
        //      the file name on the unit when it's configured since that's the only
        //      bit that is persisted.
        //
        //  m_strMake
        //  m_strModel
        //      The make/model of the device, which should be a unique key.
        //
        //  m_strName
        //      The overall name of the device.
        //
        //  m_strType
        //      The generic device type, bin sensor, thermo, etc.... This is the
        //      short text from the shared facility's enum, so it can be used to
        //      alt xlat back to the enum if desired.
        // -------------------------------------------------------------------
        TString     m_strFileName;
        TString     m_strMake;
        TString     m_strModel;
        TString     m_strName;
        TString     m_strType;
};



// ---------------------------------------------------------------------------
//   CLASS: TZDIIndex
//  PREFIX: zwdin
// ---------------------------------------------------------------------------
class TZDIIndex
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZDIIndex();

        ~TZDIIndex();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ParseFromXML
        (
                TWindow&                    wndParent
        );


        // -------------------------------------------------------------------
        //  Public class members
        //
        //  m_colMakeModel
        //      The list sorted by make and then model. This is the natural order
        //      of the index file, so this is the one we initially load. All the
        //      others are by ref collections that refer to these elements.
        // -------------------------------------------------------------------
        TVector<TZWDIInfo>   m_colMakeModel;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ResetLists();
};


#pragma CIDLIB_POPPACK

