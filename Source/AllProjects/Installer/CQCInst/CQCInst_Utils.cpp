//
// FILE NAME: CQCInst_Utils.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/04/2000
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
//  This file implements some grunt work utility methods of the facility
//  class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCInst.hpp"



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCInst_Utils
    {
        // -----------------------------------------------------------------------
        //  The options we put into the spin boxes that offer IP ports. These are
        //  just offered as common defaults. They can enter their own.
        // -----------------------------------------------------------------------
        struct TPortOptItem
        {
            tCIDLib::TIPPortNum     ippnToUse;
            tCIDLib::TCh            achText[6];
        };

        constexpr TPortOptItem aitemPorts[] =
        {
                {    80 , L"80"    }
            ,   { 11000 , L"11000" }
            ,   { 11001 , L"11001" }
            ,   { 11002 , L"11002" }
            ,   { 11003 , L"11003" }
            ,   { 11004 , L"11004" }
            ,   { 11005 , L"11005" }
            ,   { 11006 , L"11006" }
            ,   { 11007 , L"11007" }
            ,   { 11008 , L"11008" }
            ,   { 11009 , L"11009" }
            ,   { 12000 , L"12000" }
            ,   { 12001 , L"12001" }
            ,   { 12002 , L"12002" }
            ,   { 12003 , L"12003" }
            ,   { 12004 , L"12004" }
            ,   { 12005 , L"12005" }
            ,   { 12006 , L"12006" }
            ,   { 12007 , L"12007" }
            ,   { 12008 , L"12008" }
            ,   { 12009 , L"12009" }
            ,   { 12100 , L"12100" }
            ,   { 12101 , L"12101" }
            ,   { 12102 , L"12102" }
            ,   { 12103 , L"12103" }
            ,   { 12104 , L"12104" }
            ,   { 12105 , L"12105" }
            ,   { 12106 , L"12106" }
            ,   { 12107 , L"12107" }
            ,   { 12108 , L"12108" }
            ,   { 12109 , L"12109" }
            ,   { 13500 , L"13500" }
            ,   { 13501 , L"13501" }
            ,   { 13502 , L"13502" }
            ,   { 13503 , L"13503" }
            ,   { 13504 , L"13504" }
            ,   { 13505 , L"13505" }
            ,   { 13506 , L"13506" }
            ,   { 13507 , L"13507" }
            ,   { 13508 , L"13508" }
            ,   { 13509 , L"13509" }
            ,   { 13510 , L"13510" }
            ,   { 13511 , L"13511" }
            ,   { 13512 , L"13512" }
            ,   { 13513 , L"13513" }
            ,   { 13514 , L"13514" }
            ,   { 13515 , L"13515" }
            ,   { 13516 , L"13516" }
            ,   { 13517 , L"13517" }
            ,   { 13518 , L"13518" }
            ,   { 13519 , L"13519" }
            ,   { 13520 , L"13520" }
            ,   { 14200 , L"14200" }
            ,   { 14201 , L"14201" }
            ,   { 14202 , L"14202" }
            ,   { 14203 , L"14203" }
            ,   { 14204 , L"14204" }
            ,   { 14205 , L"14205" }
            ,   { 14205 , L"14206" }
            ,   { 14205 , L"14207" }
            ,   { 14205 , L"14208" }
            ,   { 20000 , L"20000" }
            ,   { 20001 , L"20001" }
            ,   { 20002 , L"20002" }
            ,   { 20003 , L"20003" }
            ,   { 20004 , L"20004" }
            ,   { 20005 , L"20005" }
        };
        constexpr tCIDLib::TCard4 c4PortOptCount = tCIDLib::c4ArrayElems(aitemPorts);
    }
}



// ---------------------------------------------------------------------------
//  TMainFrameWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  All the panels present ports as a combo box of list box string items. So we provide
//  a convenience method that validates the port value for them. The user can enter
//  values manually as well as select from the list.
//
//  We have one that just validates and another that returns the port number if it is
//  valid. The first just calls the second and discards the output param.
//
tCIDLib::TBoolean TMainFrameWnd::bValidatePort(const TComboBox& wndSrc, TString& strErrText)
{
    tCIDLib::TIPPortNum ippnRet = 0;
    return bValidatePort(wndSrc, strErrText, ippnRet);
}

tCIDLib::TBoolean
TMainFrameWnd::bValidatePort(const TComboBox& wndSrc, TString& strErrText, tCIDLib::TIPPortNum& ippnFound)
{
    ippnFound = 0;
    TString strVal;
    if (!wndSrc.bQueryCurVal(strVal)
    || !strVal.bToCard4(ippnFound, tCIDLib::ERadices::Auto) || !ippnFound || (ippnFound > 0xFFFF))
    {
        strErrText.LoadFromMsg(kCQCInstErrs::errcVal_BadPort, facCQCInst, strVal);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  All the panels present the ports as a combo box of list box string items.
//  So we provide them with a convenience method to get the number back out
//  when it's time for them to store their data.
//
//  They can enter an invalid value, so we return 0 in that case, which is not
//  a valid value and will fail validation.
//
tCIDLib::TIPPortNum TMainFrameWnd::ippnExtractPort(const TComboBox& wndSrc)
{
    tCIDLib::TIPPortNum ippnRet = 0;
    TString strVal;
    if (wndSrc.bQueryCurVal(strVal))
    {
        if (!strVal.bToCard4(ippnRet, tCIDLib::ERadices::Auto))
            ippnRet = 0;
    }
    return ippnRet;
}


//
//  THis is a helper that panels can call get a list of ports loaded up into a combo box of
//  theirs.
//
//  They also pass in the previously set port and we try to select it if it's in the list
//  of available ports. Else we add it to their list.
//
tCIDLib::TVoid
TMainFrameWnd::LoadPortOpts(const   TInstInfoPanel&     panTar
                            ,       TComboBox&          wndTarget
                            , const tCIDLib::TIPPortNum ippnCur)
{
    // See if it's already been loaded. If so, we skip the initial loads
    const tCIDLib::TBoolean bLoadIt(wndTarget.c4ItemCount() == 0);

    TString strFmt;
    tCIDLib::TCard4 c4SelInd = kCIDLib::c4MaxCard;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < CQCInst_Utils::c4PortOptCount; c4Index++)
    {
        if (bLoadIt)
        {
            strFmt.SetFormatted(CQCInst_Utils::aitemPorts[c4Index].ippnToUse);
            wndTarget.c4AddItem(strFmt);
        }
        if (CQCInst_Utils::aitemPorts[c4Index].ippnToUse == ippnCur)
            c4SelInd = c4Index;
    }

    //
    //  If it's not one in the standard list, and we don't find it in the list already,
    //  then add it.
    //
    if (c4SelInd == kCIDLib::c4MaxCard)
    {
        strFmt.SetFormatted(ippnCur);
        c4SelInd = wndTarget.c4FindByText(strFmt);
        if (c4SelInd == kCIDLib::c4MaxCard)
        {
            wndTarget.c4AddItem(strFmt);
            c4SelInd = CQCInst_Utils::c4PortOptCount;
        }
    }
    wndTarget.SelectByIndex(c4SelInd);
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TMainFrameWnd::bFindOldVer()
{
    facCQCInst.LogInstallMsg(L"Check for old version info");

    //
    //  We need to see if we can get the old version info. So lets try to open
    //  the LOCAL_MACHINE\SOFTWARE\Charmed Quark key. If its there, then the
    //  "TargetDir" value holds the installation directory.
    //
    tCIDLib::TBoolean bCreated;
    tCIDLib::TBoolean bHaveOld = kCIDLib::False;

    tCIDKernel::TWRegHandle hkeyCQS = TKrnlWin32Registry::hkeyCreateOrOpenSubKey
    (
        tCIDKernel::ERootKeys::LocalMachine
        , L"SOFTWARE"
        , kCQCKit::pszReg_Key
        , bCreated
        , tCIDKernel::ERegAccFlags::StdOwned
    );

    if (hkeyCQS)
    {
        const tCIDLib::TCard4 c4BufLen = 2048;
        tCIDLib::TCh achBuf[c4BufLen + 1];

        if (TKrnlWin32Registry::bQueryStrValue( hkeyCQS
                                                , kCQCKit::pszReg_Path
                                                , achBuf
                                                , c4BufLen))
        {
            if (TFileSys::bExists(achBuf, tCIDLib::EDirSearchFlags::NormalDirs))
            {
                TCQCVerInstallInfo viiOld;
                if (facCQCInst.bCheckTarget(*this, achBuf, viiOld))
                {
                    //
                    //  Ok, we have the old info now. Save this path as
                    //  both the likely target for install and as the source
                    //  of the old version info.
                    //
                    bHaveOld = kCIDLib::True;
                    const TString strOldPath(achBuf);
                    m_infoInstall.strPath(strOldPath);
                    m_infoInstall.SetOldInfo(viiOld, strOldPath);
                }
            }
        }
            else
        {
            facCQCInst.LogKrnlErr
            (
                CID_FILE
                , CID_LINE
                , kCQCInstMsgs::midStatus_ReadRegPath
                , TKrnlError::kerrLast()
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::CantDo
            );
        }

        // Close the handle now
        if (!TKrnlWin32Registry::bCloseKey(hkeyCQS))
        {
            facCQCInst.LogKrnlErr
            (
                CID_FILE
                , CID_LINE
                , kCQCInstMsgs::midStatus_CloseRegKey
                , TKrnlError::kerrLast()
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::CantDo
            );
            facCQCInst.LogInstallMsg(L"Could not close registry handle");
        }
    }

    //
    //  If we didn't find anything in the registry, let's get the user to
    //  confirm that it's a new install.
    //
    if (!bHaveOld)
    {
        TCQCNewInstCheckDlg dlgNewCheck;
        TString             strPath;
        TCQCVerInstallInfo  viiOld;

        const tCQCInst::ENewInstRes eRes = dlgNewCheck.eRunDlg
        (
            TWindow::wndDesktop(), strPath, viiOld
        );

        if (eRes == tCQCInst::ENewInstRes::Found)
        {
            TString strTmp(L"User selected a target path. Path=");
            strTmp.Append(strPath);
            facCQCInst.LogInstallMsg(strPath);

            //
            //  Ok, we have the old info now. Save this path as both the
            //  likely target for install and as the sourceof the old version
            //  info.
            //
            m_infoInstall.strPath(strPath);
            m_infoInstall.SetOldInfo(viiOld, strPath);
        }
         else if (eRes == tCQCInst::ENewInstRes::Cancel)
        {
            facCQCInst.LogInstallMsg(L"User canceled from new check dialog");
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//
//  Runs final validation stuff that can only be done once the big picture is
//  available. This is called just before the summary panel. We return either
//  the summary panel id, to let the process move forward, or the index of
//  the first offending panel.
//
tCIDLib::TCard4 TMainFrameWnd::c4GlobalValidation()
{
    //
    //  Run through all the panels, and let each one report its port usage.
    //  If we come back with any port count greater than 1 for a panel, then
    //  it is using a port already used on a previous panel.
    //
    tCQCInst::TPortCntList fcolPorts(0xFFFF);
    fcolPorts.SetAll(0);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4PanInd_Summary; c4Index++)
    {
        m_colPanels[c4Index]->ReportPortUsage(fcolPorts);
        for (tCIDLib::TCard4 c4PIndex = 1; c4PIndex < 0xFFFF; c4PIndex++)
        {
            if (fcolPorts[c4PIndex] > 1)
            {
                TOkBox msgbPort
                (
                    facCQCInst.strMsg(kCQCInstErrs::errcVal_PortUsed, TCardinal(c4PIndex))
                );
                msgbPort.ShowIt(*this);
                return c4Index;
            }
        }
    }

    //
    //  Now, go back and actually try to open a socket and bind locally on
    //  each port, to see if some other program is using one of them.
    //
    //  Note that we have to do it both for IPV4 and IPV6 addresses.
    //
    //  Log which protocols we are going to check
    //
    TString strProtos;
    if (TKrnlIP::bIPV4Avail())
    {
        if (!strProtos.bIsEmpty())
            strProtos.Append(L",");
        strProtos.Append(L"IPV4");
    }
    if (TKrnlIP::bIPV6Avail())
    {
        if (!strProtos.bIsEmpty())
            strProtos.Append(L",");
        strProtos.Append(L"IPV6");
    }

    facCQCInst.LogInstallMsg
    (
        L"Checking port availability. Protocols=%(1)", strProtos
    );


    if (TKrnlIP::bIPV4Avail())
    {
        facCQCInst.LogInstallMsg(L"Checking IPV4 port availability");

        for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4PanInd_Summary; c4Index++)
        {
            // We just want the ones from this panel, so reset list
            fcolPorts.SetAll(0);
            m_colPanels[c4Index]->ReportPortUsage(fcolPorts);
            for (tCIDLib::TCard4 c4PIndex = 1; c4PIndex < 0xFFFF; c4PIndex++)
            {
                // If not in use by this panel, skip this one
                if (!fcolPorts[c4PIndex])
                    continue;

                // It's used by someone so try to bind to it
                try
                {
                    TIPEndPoint ipepTest4
                    (
                        tCIDSock::ESpecAddrs::Any
                        , tCIDSock::EAddrTypes::IPV4
                        , c4PIndex
                    );
                    TClientStreamSocket sockTest
                    (
                        tCIDSock::ESockProtos::TCP, ipepTest4.eAddrType()
                    );
                    sockTest.BindListen(ipepTest4);
                    sockTest.Close();
                }

                catch(TError& errToCatch)
                {
                    // Log the error for later diagnosis
                    if (!errToCatch.bLogged())
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }

                    //
                    //  Watch for an in-user error. Others we will ignore.
                    //
                    if (errToCatch.errcKrnlId() == kKrnlErrs::errcGen_Busy)
                    {
                        TOkBox msgbPort
                        (
                            facCQCInst.strMsg
                            (
                                kCQCInstErrs::errcVal_CantOpenPort
                                , TCardinal(c4PIndex)
                                , TString(L"IPV4")
                            )
                        );
                        msgbPort.ShowIt(*this);
                        return c4Index;
                    }
                }
            }
        }

        if (TKrnlIP::bIPV6Avail())
        {
            facCQCInst.LogInstallMsg(L"Checking IPV6 port availability");

            for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4PanInd_Summary; c4Index++)
            {
                // We just want the ones from this panel, so reset list
                fcolPorts.SetAll(0);
                m_colPanels[c4Index]->ReportPortUsage(fcolPorts);
                for (tCIDLib::TCard4 c4PIndex = 1; c4PIndex < 0xFFFF; c4PIndex++)
                {
                    // If not in use by this panel, skip this one
                    if (!fcolPorts[c4PIndex])
                        continue;

                    // It's used by someone so try to bind to it
                    try
                    {
                        TIPEndPoint ipepTest6
                        (
                            tCIDSock::ESpecAddrs::Any
                            , tCIDSock::EAddrTypes::IPV6
                            , c4PIndex
                        );

                        TClientStreamSocket sockTest
                        (
                            tCIDSock::ESockProtos::TCP, ipepTest6.eAddrType()
                        );
                        sockTest.BindListen(ipepTest6);
                        sockTest.Close();
                    }

                    catch(TError& errToCatch)
                    {
                        // Log the error for later diagnosis
                        if (!errToCatch.bLogged())
                        {
                            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                            TModule::LogEventObj(errToCatch);
                        }

                        //
                        //  Watch for an in-user error. Others we will ignore.
                        //
                        if (errToCatch.errcKrnlId() == kKrnlErrs::errcGen_Busy)
                        {
                            TOkBox msgbPort
                            (
                                facCQCInst.strMsg
                                (
                                    kCQCInstErrs::errcVal_CantOpenPort
                                    , TCardinal(c4PIndex)
                                    , TString(L"IPV6")
                                )
                            );
                            msgbPort.ShowIt(*this);
                            return c4Index;
                        }
                    }
                }
            }
        }
    }

    // No problems, so return summary panel
    return m_c4PanInd_Summary;
}

