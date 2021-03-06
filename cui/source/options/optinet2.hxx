/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTINET2_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTINET2_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <vcl/field.hxx>
#include <vcl/group.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/stdctrl.hxx>
#include <svtools/svtabbx.hxx>
#include <sfx2/tabdlg.hxx>
#include <unotools/configitem.hxx>

#include <svtools/headbar.hxx>

namespace svx {
    class SecurityOptionsDialog;
}


// class SvxNoSpaceEdit --------------------------------------------------

class SvxNoSpaceEdit : public Edit
{
private:
    bool bOnlyNumeric;
public:
    SvxNoSpaceEdit(vcl::Window* pParent, WinBits nStyle)
        : Edit(pParent, nStyle)
        , bOnlyNumeric(false)
    {}
    virtual void KeyInput(const KeyEvent& rKEvent) SAL_OVERRIDE;
    virtual void Modify() SAL_OVERRIDE;
    virtual bool set_property(const OString &rKey, const OString &rValue) SAL_OVERRIDE;
};

// class SvxProxyTabPage -------------------------------------------------

class SvxProxyTabPage : public SfxTabPage
{
private:

    VclPtr<ListBox>        m_pProxyModeLB;

    VclPtr<FixedText>      m_pHttpProxyFT;
    VclPtr<SvxNoSpaceEdit> m_pHttpProxyED;
    VclPtr<FixedText>      m_pHttpPortFT;
    VclPtr<SvxNoSpaceEdit> m_pHttpPortED;

    VclPtr<FixedText>      m_pHttpsProxyFT;
    VclPtr<SvxNoSpaceEdit> m_pHttpsProxyED;
    VclPtr<FixedText>      m_pHttpsPortFT;
    VclPtr<SvxNoSpaceEdit> m_pHttpsPortED;

    VclPtr<FixedText>      m_pFtpProxyFT;
    VclPtr<SvxNoSpaceEdit> m_pFtpProxyED;
    VclPtr<FixedText>      m_pFtpPortFT;
    VclPtr<SvxNoSpaceEdit> m_pFtpPortED;

    VclPtr<FixedText>      m_pNoProxyForFT;
    VclPtr<Edit>           m_pNoProxyForED;
    VclPtr<FixedText>      m_pNoProxyDescFT;

    const OUString aProxyModePN;
    const OUString aHttpProxyPN;
    const OUString aHttpPortPN;
    const OUString aHttpsProxyPN;
    const OUString aHttpsPortPN;
    const OUString aFtpProxyPN;
    const OUString aFtpPortPN;
    const OUString aNoProxyDescPN;

    css::uno::Reference< css::uno::XInterface > m_xConfigurationUpdateAccess;

    void EnableControls_Impl(bool bEnable);
    void ReadConfigData_Impl();
    void ReadConfigDefaults_Impl();
    void RestoreConfigDefaults_Impl();

    DECL_LINK( ProxyHdl_Impl, ListBox * );
    DECL_LINK( LoseFocusHdl_Impl, Edit * );

public:
    SvxProxyTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxProxyTabPage();
    virtual void dispose() SAL_OVERRIDE;
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet );
    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;
};

// #98647# class SvxScriptExecListBox ------------------------------------
class SvxScriptExecListBox : public ListBox
{ // for adding tooltips to ListBox
public:
    SvxScriptExecListBox( vcl::Window* pParent, WinBits nStyle = WB_BORDER )
            :ListBox(pParent, nStyle) {}
    SvxScriptExecListBox( vcl::Window* pParent, const ResId& rResId )
            :ListBox(pParent, rResId) {}

protected:
    virtual void RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;
};

// class SvxSecurityTabPage ---------------------------------------------

class SvtSecurityOptions;
class CertPathDialog;
class SvxSecurityTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    VclPtr<PushButton>         m_pSecurityOptionsPB;

    VclPtr<CheckBox>           m_pSavePasswordsCB;
    VclPtr<PushButton>         m_pShowConnectionsPB;

    VclPtr<CheckBox>           m_pMasterPasswordCB;
    VclPtr<FixedText>          m_pMasterPasswordFT;
    VclPtr<PushButton>         m_pMasterPasswordPB;

    VclPtr<VclContainer>       m_pMacroSecFrame;
    VclPtr<PushButton>         m_pMacroSecPB;

    VclPtr<VclContainer>       m_pCertFrame;
    VclPtr<PushButton>         m_pCertPathPB;

    VclPtr<VclContainer>       m_pTSAURLsFrame;
    VclPtr<PushButton>         m_pTSAURLsPB;

    SvtSecurityOptions*         mpSecOptions;
    VclPtr<svx::SecurityOptionsDialog> mpSecOptDlg;

    VclPtr<CertPathDialog> mpCertPathDlg;

    OUString            m_sPasswordStoringDeactivateStr;

    DECL_LINK(SecurityOptionsHdl, void *);
    DECL_LINK(SavePasswordHdl, void* );
    DECL_LINK(MasterPasswordHdl, void *);
    DECL_LINK(MasterPasswordCBHdl, void* );
    DECL_LINK(ShowPasswordsHdl, void *);
    DECL_LINK(MacroSecPBHdl, void* );
    DECL_LINK(CertPathPBHdl, void* );
    DECL_LINK(TSAURLsPBHdl, void* );

    void                InitControls();

                SvxSecurityTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual     ~SvxSecurityTabPage();
    virtual void dispose() SAL_OVERRIDE;

protected:
    virtual void        ActivatePage( const SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual sfxpg       DeactivatePage( SfxItemSet* pSet = 0 ) SAL_OVERRIDE;

public:
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet );
    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;
};

struct SvxEMailTabPage_Impl;
class SvxEMailTabPage : public SfxTabPage
{
    VclPtr<VclContainer> m_pMailContainer;
    VclPtr<FixedImage>   m_pMailerURLFI;
    VclPtr<Edit>         m_pMailerURLED;
    VclPtr<PushButton>   m_pMailerURLPB;
    VclPtr<VclContainer> m_pSuppressHiddenContainer;
    VclPtr<FixedImage>   m_pSuppressHiddenFI;
    VclPtr<CheckBox>     m_pSuppressHidden;

    OUString      m_sDefaultFilterName;

    SvxEMailTabPage_Impl* pImpl;

    DECL_LINK(  FileDialogHdl_Impl, PushButton* ) ;

public:
    SvxEMailTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxEMailTabPage();
    virtual void        dispose() SAL_OVERRIDE;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;
};

#endif // INCLUDED_CUI_SOURCE_OPTIONS_OPTINET2_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
