//
// FILE NAME: CQCImgLoader_ExtractIcons.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/23/2006
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
//  This file holds the private methods of the facilty class that are related
//  to the extraction of icons from various system modules.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ----------------------------------------------------------------------------
//  Includes
// ----------------------------------------------------------------------------
#include    "CQCImgLoader.hpp"



tCIDLib::TVoid TFacCQCImgLoader::ExtractIcons()
{
    struct XlatInfo
    {
        const tCIDLib::TCh*     pszIcon;
        const tCIDLib::TCh*     pszRelPath;
        const tCIDLib::TCh*     pszName;
        const tCIDLib::TCh*     pszLib;
    };

    XlatInfo aList[] =
    {
        { L"#268"   , L"General\\Icons" , L"Accessibility"      , L"Shell32" }
      , { L"#1641"  , L"General\\Icons" , L"Address Book"       , L"C:\\Program Files\\Common Files\\System\\wab32res.dll" }
      , { L"#165"   , L"General\\Icons" , L"Backup"             , L"Shell32" }
      , { L"#274"   , L"General\\Icons" , L"Blue Cog"           , L"Shell32" }
      , { L"#100"   , L"General\\Icons" , L"Briefcase"          , L"syncapp.exe" }
      , { L"SC"     , L"General\\Icons" , L"Calculator"         , L"C:\\Windows\\System32\\calc.exe" }
      , { L"#500"   , L"General\\Icons" , L"Card Deck"          , L"sol.exe" }
      , { L"#22"    , L"General\\Icons" , L"Check Mark"         , L"Shell32" }
      , { L"ICON_1" , L"General\\Icons" , L"Chrome Ball"        , L"C:\\Program Files\\Windows NT\\Pinball\\pinball.exe" }
      , { L"#200"   , L"General\\Icons" , L"Date Time"          , L"timedate.cpl" }
      , { L"#152"   , L"General\\Icons" , L"Document"           , L"Shell32" }
      , { L"#154"   , L"General\\Icons" , L"Driver"             , L"Shell32" }
      , { L"#200"   , L"General\\Icons" , L"Exclamation"        , L"netplwiz" }
      , { L"#128"   , L"General\\Icons" , L"Film Reel"          , L"C:\\Program Files\\Movie Maker\\moviemk.exe" }
      , { L"#102"   , L"General\\Icons" , L"File Transfer"      , L"rtcshare.exe" }
      , { L"#4"     , L"General\\Icons" , L"Folder"             , L"Shell32" }
      , { L"#155"   , L"General\\Icons" , L"Font"               , L"Shell32" }
      , { L"#39"    , L"General\\Icons" , L"Font Folder"        , L"Shell32" }
      , { L"#200"   , L"General\\Icons" , L"Globe"              , L"intl.cpl" }
      , { L"#279"   , L"General\\Icons" , L"Group"              , L"Shell32" }
      , { L"#1001"  , L"General\\Icons" , L"Information"        , L"Shell32" }
      , { L"#512"   , L"General\\Icons" , L"Internet Explorer"  , L"Shell32" }
      , { L"#103"   , L"General\\Icons" , L"Keys"               , L"migpwd.exe" }
      , { L"#23"    , L"General\\Icons" , L"Magnifying Glass"   , L"Shell32" }
      , { L"#227"   , L"General\\Icons" , L"Media Document"     , L"Shell32" }
      , { L"#28750" , L"General\\Icons" , L"Media Document 2"   , L"wmp" }
      , { L"#225"   , L"General\\Icons" , L"Music Document"     , L"Shell32" }
      , { L"#237"   , L"General\\Icons" , L"Music Folder"       , L"Shell32" }
      , { L"#200"   , L"General\\Icons" , L"Not Allowed"        , L"Shell32" }
      , { L"#2"     , L"General\\Icons" , L"Note Pad"           , L"NotePad.exe" }
      , { L"#254"   , L"General\\Icons" , L"Open Envelope"      , L"explorer.exe" }
      , { L"#5"     , L"General\\Icons" , L"Open Folder"        , L"Shell32" }
      , { L"#2"     , L"General\\Icons" , L"Paint Brushes"      , L"mspaint.exe" }
      , { L"#102"   , L"General\\Icons" , L"Phone Call"         , L"C:\\Program Files\\Windows NT\\dialer.exe" }
      , { L"#275"   , L"General\\Icons" , L"Pie Chart"          , L"Shell32" }
      , { L"#24"    , L"General\\Icons" , L"Question Mark"      , L"Shell32" }
      , { L"#254"   , L"General\\Icons" , L"Recycler"           , L"Shell32" }
      , { L"#21"    , L"General\\Icons" , L"Schedule"           , L"Shell32" }
      , { L"#46"    , L"General\\Icons" , L"Search Folder"      , L"Shell32" }
      , { L"#134"   , L"General\\Icons" , L"Search Document"    , L"Shell32" }
      , { L"#200"   , L"General\\Icons" , L"Shield"             , L"wscui.cpl" }
      , { L"#173"   , L"General\\Icons" , L"Star"               , L"Shell32" }
      , { L"#1912"  , L"General\\Icons" , L"User Search"        , L"C:\\Program Files\\Common Files\\System\\wab32res.dll" }
      , { L"#224"   , L"General\\Icons" , L"Video Document"     , L"Shell32" }
      , { L"#238"   , L"General\\Icons" , L"Video Folder"       , L"Shell32" }
      , { L"#14"    , L"General\\Icons" , L"World"              , L"Shell32" }
      , { L"#1606"  , L"General\\Icons" , L"Web Search"         , L"netshell" }
      , { L"#71"    , L"General\\Icons" , L"Windows Player"     , L"C:\\Program Files\\Windows Media Player\\setup_wm.exe" }
      , { L"#240"   , L"General\\Icons" , L"X Mark"             , L"Shell32" }
      , { L"#111"   , L"General\\Icons" , L"X Mark 2"           , L"explorer.exe" }
      , { L"#1"     , L"General\\Icons" , L"Zip File"           , L"E:\\WinZip\\WinZip32.exe" }

      , { L"#309"   , L"Hardware\\Icons" , L"Camera"             , L"Shell32" }
      , { L"#295"   , L"Hardware\\Icons" , L"CD-R Disc"          , L"Shell32" }
      , { L"#294"   , L"Hardware\\Icons" , L"CD-ROM Disc"        , L"Shell32" }
      , { L"#296"   , L"Hardware\\Icons" , L"CD-RW Disc"         , L"Shell32" }
      , { L"#310"   , L"Hardware\\Icons" , L"Cell Phone"         , L"Shell32" }
      , { L"#16"    , L"Hardware\\Icons" , L"Computer System"    , L"Shell32" }
      , { L"#250"   , L"Hardware\\Icons" , L"Designer"           , L"Shell32" }
      , { L"#302"   , L"Hardware\\Icons" , L"Digital Disc"       , L"Shell32" }
      , { L"#12"    , L"Hardware\\Icons" , L"Disc Drive"         , L"Shell32" }
      , { L"#222"   , L"Hardware\\Icons" , L"DVD Disc"           , L"Shell32" }
      , { L"#291"   , L"Hardware\\Icons" , L"DVD Drive"          , L"Shell32" }
      , { L"#297"   , L"Hardware\\Icons" , L"DVD-RAM Disc"       , L"Shell32" }
      , { L"#298"   , L"Hardware\\Icons" , L"DVD-R Disc"         , L"Shell32" }
      , { L"#304"   , L"Hardware\\Icons" , L"DVD-ROM Disc"       , L"Shell32" }
      , { L"#318"   , L"Hardware\\Icons" , L"DVD-RW Disc"        , L"Shell32" }
      , { L"#7"     , L"Hardware\\Icons" , L"Floppy Drive"       , L"Shell32" }
      , { L"#314"   , L"Hardware\\Icons" , L"HandHeld"           , L"Shell32" }
      , { L"#9"     , L"Hardware\\Icons" , L"Hard Drive"         , L"Shell32" }
      , { L"#100"   , L"Hardware\\Icons" , L"Hardware"           , L"hdwwiz.cpl" }
      , { L"#102"   , L"Hardware\\Icons" , L"Controller"         , L"joy.cpl" }
      , { L"#200"   , L"Hardware\\Icons" , L"Keyboard"           , L"main.cpl" }
      , { L"#48"    , L"Hardware\\Icons" , L"Lock"               , L"Shell32" }
      , { L"#2"     , L"Hardware\\Icons" , L"Mouse"              , L"setupapi" }
      , { L"#41"    , L"Hardware\\Icons" , L"Music Disc"         , L"Shell32" }
      , { L"#303"   , L"Hardware\\Icons" , L"Muted Speaker"      , L"sndvol32.exe" }
      , { L"#175"   , L"Hardware\\Icons" , L"Network Connection" , L"Shell32" }
      , { L"#10"    , L"Hardware\\Icons" , L"Network Drive"      , L"Shell32" }
      , { L"#101"   , L"Hardware\\Icons" , L"Network Dish"       , L"mstsc.exe" }
      , { L"#19"    , L"Hardware\\Icons" , L"Network"            , L"Shell32" }
      , { L"#132"   , L"Hardware\\Icons" , L"Network2"           , L"mobsync.exe" }
      , { L"#196"   , L"Hardware\\Icons" , L"PhoneFax"           , L"Shell32" }
      , { L"#299"   , L"Hardware\\Icons" , L"Portable Media"     , L"Shell32" }
      , { L"#202"   , L"Hardware\\Icons" , L"Power"              , L"powercfg.cpl" }
      , { L"#245"   , L"Hardware\\Icons" , L"Printer"            , L"Shell32" }
      , { L"#315"   , L"Hardware\\Icons" , L"Scanner"            , L"Shell32" }
      , { L"#304"   , L"Hardware\\Icons" , L"Speaker"            , L"sndvol32.exe" }
      , { L"#277"   , L"Hardware\\Icons" , L"Speaker Music"      , L"Shell32" }
      , { L"#307"   , L"Hardware\\Icons" , L"Tablet"             , L"Shell32" }
      , { L"#500"   , L"Hardware\\Icons" , L"Tools"              , L"main.cpl" }
      , { L"#317"   , L"Hardware\\Icons" , L"Video Camera"       , L"Shell32" }
      , { L"#153"   , L"Hardware\\Icons" , L"Serial Port"        , L"irprops.cpl" }
      , { L"#161"   , L"Hardware\\Icons" , L"Touch Screen"       , L"irprops.cpl" }
      , { L"#300"   , L"Hardware\\Icons" , L"Volume Slider"      , L"sndvol32.exe" }
    };
    const tCIDLib::TCard4 c4Count = tCIDLib::c4ArrayElems(aList);

    XlatInfo aSmlList[] =
    {
        { L"#255"   , L"General\\Icons"  , L"Back Arrow"         , L"Shell32" }
      , { L"#246"   , L"General\\Icons"  , L"Right Arrow"        , L"Shell32" }
      , { L"#290"   , L"General\\Icons"  , L"Right Arrow 2"      , L"Shell32" }
      , { L"#1927"  , L"General\\Icons"  , L"Red Check"          , L"E:\\WinZip\\WinZip32.exe" }
    };
    const tCIDLib::TCard4 c4SmlCount = tCIDLib::c4ArrayElems(aSmlList);

    XlatInfo aSmlList2[] =
    {
        { L"#2255"  , L"General\\Icons"  , L"Red Ball"           , L"E:\\WinZip\\WinZip32.exe" }
      , { L"#2256"  , L"General\\Icons"  , L"Gray Ball"          , L"E:\\WinZip\\WinZip32.exe" }
      , { L"#2257"  , L"General\\Icons"  , L"Green Ball"         , L"E:\\WinZip\\WinZip32.exe" }
    };
    const tCIDLib::TCard4 c4SmlCount2 = tCIDLib::c4ArrayElems(aSmlList2);

    TPathStr pathApp;
    TPathStr pathIntf;
    if (!TProcEnvironment::bFind(L"CIDAPPIMGPATH", pathApp)
    ||  !TProcEnvironment::bFind(L"CQCINTFIMGPATH", pathIntf))
    {
        m_conOut << L"One of the target paths was not found in the environment"
                 << kCIDLib::EndLn;
        return;
    }
    pathApp.Normalize();
    pathIntf.Normalize();

    TGraphDesktopDev            gdevTmp;
    TPathStr                    pathOut;
    TClrPalette                 palClrs;
    TPixelArray                 pixaBits;
    tCIDGraphDev::TIconHandle  hicoTmp;
    TString                     strTmp;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const tCIDLib::TCh* pszCat;
        for (tCIDLib::TCard4 c4FInd = 0; c4FInd < 4; c4FInd++)
        {
            if (c4FInd == 0)
            {
                pszCat = L"intf img";
                pathOut = pathIntf;
                pathOut.AddLevel(aList[c4Index].pszRelPath);
                pathOut.AddLevel(aList[c4Index].pszName);

                hicoTmp = TIcon::hicoLoadRaw
                (
                    aList[c4Index].pszLib
                    , aList[c4Index].pszIcon
                    , TSize(48, 48)
                    , 32
                );
            }
             else if (c4FInd == 1)
            {
                pszCat = L"big app image";
                pathOut = pathApp;
                pathOut.AddLevel(L"Lrg_");
                pathOut.Append(aList[c4Index].pszName);
                hicoTmp = TIcon::hicoLoadRaw
                (
                    aList[c4Index].pszLib
                    , aList[c4Index].pszIcon
                    , TSize(48, 48)
                    , 32
                );
            }
             else if (c4FInd == 2)
            {
                pszCat = L"med app image";
                pathOut = pathApp;
                pathOut.AddLevel(L"Med_");
                pathOut.Append(aList[c4Index].pszName);
                hicoTmp = TIcon::hicoLoadRaw
                (
                    aList[c4Index].pszLib
                    , aList[c4Index].pszIcon
                    , TSize(32, 32)
                    , 32
                );
            }
             else if (c4FInd == 3)
            {
                pszCat = L"small app image";
                pathOut = pathApp;
                pathOut.AddLevel(L"Sml_");
                pathOut.Append(aList[c4Index].pszName);
                hicoTmp = TIcon::hicoLoadRaw
                (
                    aList[c4Index].pszLib
                    , aList[c4Index].pszIcon
                    , TSize(16, 16)
                    , 32
                );
            }

            if (hicoTmp)
            {
                TIcon icoTmp(hicoTmp);
                TBitmap bmpTmp(icoTmp.bmpImageCopy());
                bmpTmp.QueryImgData(gdevTmp, pixaBits, palClrs, kCIDLib::True);
                TPNGImage imgNew(pixaBits);

                pathOut.AppendExt(L"png");
                TBinFileOutStream strmTar
                (
                    pathOut
                    , tCIDLib::ECreateActs::CreateAlways
                    , tCIDLib::EFilePerms::Default
                    , tCIDLib::EFileFlags::SequentialScan
                );

                strmTar << imgNew << kCIDLib::FlushIt;
            }
             else
            {
                m_conOut << L"Failed to load " << pszCat << L". "
                         << aList[c4Index].pszName << L". Continue?(Y/N): "
                         << kCIDLib::FlushIt;

                m_conIn >> strTmp;
                strTmp.ToUpper();
                if (strTmp != L"Y")
                    return;
            }
        }
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SmlCount; c4Index++)
    {
        pathOut = pathApp;
        pathOut.AddLevel(L"Sml_");
        pathOut.Append(aSmlList[c4Index].pszName);
        hicoTmp = TIcon::hicoLoadRaw
        (
            aSmlList[c4Index].pszLib
            , aSmlList[c4Index].pszIcon
            , TSize(16, 16)
            , 32
        );

        if (hicoTmp)
        {
            TIcon icoTmp(hicoTmp);
            TBitmap bmpTmp(icoTmp.bmpImageCopy());
            bmpTmp.QueryImgData(gdevTmp, pixaBits, palClrs, kCIDLib::True);
            TPNGImage imgNew(pixaBits);

            pathOut.AppendExt(L"png");
            TBinFileOutStream strmTar
            (
                pathOut
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
            );
            strmTar << imgNew << kCIDLib::FlushIt;
        }
         else
        {
            m_conOut  << L"Failed to load small app image. "
                      << aSmlList[c4Index].pszName << L". Continue?(Y/N): "
                     << kCIDLib::FlushIt;

            m_conIn >> strTmp;
            strTmp.ToUpper();
            if (strTmp != L"Y")
                return;
        }
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SmlCount2; c4Index++)
    {
        pathOut = pathApp;
        pathOut.AddLevel(L"Sml_");
        pathOut.Append(aSmlList2[c4Index].pszName);
        hicoTmp = TIcon::hicoLoadRaw
        (
            aSmlList2[c4Index].pszLib
            , aSmlList2[c4Index].pszIcon
            , TSize(16, 15)
            , 32
        );

        if (hicoTmp)
        {
            TIcon icoTmp(hicoTmp);
            TBitmap bmpTmp(icoTmp.bmpImageCopy());
            bmpTmp.QueryImgData(gdevTmp, pixaBits, palClrs, kCIDLib::True);
            TPNGImage imgNew(pixaBits);

            pathOut.AppendExt(L"png");
            TBinFileOutStream strmTar
            (
                pathOut
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
            );
            strmTar << imgNew << kCIDLib::FlushIt;
        }
         else
        {
            m_conOut << L"Failed to load small app image. "
                     << aSmlList2[c4Index].pszName << L". Continue?(Y/N): "
                     << kCIDLib::FlushIt;

            m_conIn >> strTmp;
            strTmp.ToUpper();
            if (strTmp != L"Y")
                return;
        }
    }
}


