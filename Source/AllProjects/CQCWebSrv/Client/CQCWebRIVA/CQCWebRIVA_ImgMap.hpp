//
// FILE NAME: CQCWebRIVA_ImgMap.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/16/2017
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
//  This is the header for the class we use to track a RIVA client's image map. Images
//  are a big deal because we need to be efficient about them. The client maintains a
//  map of any images it has cached, with the image path and the serial number. When he
//  connects, he sends us his current map and one of these is set up by the RIVA thread
//  servicing that client. This lets us know when we are about to send him a command to
//  draw an image that he doesn't have or doesn't have the latest version of, so that we
//  can send it to him.
//
//  If he drops images from his map, he lets us know and we update our copy.
//
//  We use a TKeyObjPair class for the actual per-image data, with the key being the image
//  path.
//
// CAVEATS/GOTCHAS:
//
//  1)  We don't maintain image data here. The IV engine already has an elaborate img
//      caching scheme. So we get the image data from it when we need to send an img.
//      We also download images through it so that they get cached up.
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

using TRIVAImgItem = TKeyObjPair<TString, tCIDLib::TCard4>;

// ---------------------------------------------------------------------------
//   CLASS: TRIVAImgMap
//  PREFIX: rimap
// ---------------------------------------------------------------------------
class CQCWEBRIVAEXP TRIVAImgMap
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TRIVAImgMap();

        TRIVAImgMap(const TRIVAImgMap&) = delete;
        TRIVAImgMap(TRIVAImgMap&&) = delete;

        ~TRIVAImgMap();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TRIVAImgMap& operator=(const TRIVAImgMap&) = delete;
        TRIVAImgMap& operator=(TRIVAImgMap&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAddUpdate
        (
            const   TString&                strImgPath
            , const tCIDLib::TCard4         c4SerialNum
        );

        const TRIVAImgItem* pimiFind
        (   const   TString&                strImgPath
        )   const;

        tCIDLib::TVoid Remove
        (
            const   TString&                strImgPath
        );

        tCIDLib::TVoid Reset();


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colImgs
        //      A hash set of our path/serial number pair objects. The key ops object is
        //      set up for case insensitive mode, so that we don't have to worry about
        //      potential dups with different case somehow. This one needs to be
        //      thread safe.
        // -------------------------------------------------------------------
        TKeyedHashSet<TRIVAImgItem, TString, TStringKeyOps> m_colImgs;
};


#pragma CIDLIB_POPPACK
