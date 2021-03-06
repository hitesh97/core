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
#ifndef INCLUDED_CUI_SOURCE_INC_NUMFMT_HXX
#define INCLUDED_CUI_SOURCE_INC_NUMFMT_HXX



#include <rtl/ustring.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/langbox.hxx>
#include <svx/fontlb.hxx>
#include <tools/color.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/field.hxx>
#include <vcl/window.hxx>



class SvxNumberFormatShell;
class SvxNumberInfoItem;
class vector;



class SvxNumberPreview : public vcl::Window
{
private:
    OUString        aPrevStr;
    Color           aPrevCol;
    sal_Int32       mnPos;
    sal_Unicode     mnChar;
    void            InitSettings( bool bForeground, bool bBackground );

protected:
    virtual void    Paint( vcl::RenderContext& /*rRenderContext*/, const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    StateChanged( StateChangedType nStateChange ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

public:
    SvxNumberPreview(vcl::Window* pParent, WinBits nStyle = WB_BORDER);

    void            NotifyChange( const OUString& rPrevStr, const Color* pColor = NULL );
};



class SvxNumberFormatTabPage : public SfxTabPage
{
    using SfxTabPage::DeactivatePage;
    static const sal_uInt16 pRanges[];

public:
    virtual ~SvxNumberFormatTabPage();
    virtual void dispose() SAL_OVERRIDE;

    static VclPtr<SfxTabPage>      Create( vcl::Window* pParent,
                                    const SfxItemSet* rAttrSet );
    // Returns area information.
    static const sal_uInt16* GetRanges() { return pRanges; }

    virtual bool            FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void            Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual sfxpg           DeactivatePage  ( SfxItemSet* pSet = NULL ) SAL_OVERRIDE;

    void                    SetInfoItem( const SvxNumberInfoItem& rItem );
    void                    SetNumberFormatList( const SvxNumberInfoItem& rItem )
                                { SetInfoItem( rItem ); }

    void                    SetOkHdl( const Link<>& rOkHandler );
    void                    HideLanguage(bool nFlag=true);
    virtual bool            PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void            PageCreated(const SfxAllItemSet& aSet) SAL_OVERRIDE;
private:
    SvxNumberFormatTabPage( vcl::Window* pParent,
                            const SfxItemSet& rCoreAttrs );
    VclPtr<FixedText>              m_pFtCategory;
    VclPtr<ListBox>                m_pLbCategory;
    VclPtr<FixedText>              m_pFtFormat;
    VclPtr<ListBox>                m_pLbCurrency;
    VclPtr<SvxFontListBox>         m_pLbFormat;
    VclPtr<FixedText>              m_pFtLanguage;
    VclPtr<SvxLanguageBox>         m_pLbLanguage;
    VclPtr<CheckBox>               m_pCbSourceFormat;
    VclPtr<SvxNumberPreview>       m_pWndPreview;
    VclPtr<FixedText>              m_pFtOptions;
    VclPtr<FixedText>              m_pFtDecimals;
    VclPtr<NumericField>           m_pEdDecimals;
    VclPtr<CheckBox>               m_pBtnNegRed;
    VclPtr<FixedText>              m_pFtLeadZeroes;
    VclPtr<NumericField>           m_pEdLeadZeroes;
    VclPtr<CheckBox>               m_pBtnThousand;

    VclPtr<VclContainer>           m_pFormatCodeFrame;
    VclPtr<Edit>                   m_pEdFormat;
    VclPtr<PushButton>             m_pIbAdd;
    VclPtr<PushButton>             m_pIbInfo;
    VclPtr<PushButton>             m_pIbRemove;

    VclPtr<FixedText>              m_pFtComment;
    VclPtr<Edit>                   m_pEdComment;
    Timer                   aResetWinTimer;


    SvxNumberInfoItem*      pNumItem;
    SvxNumberFormatShell*   pNumFmtShell;
    sal_uLong               nInitFormat;
    Link<>                  fnOkHdl;

    bool                bNumItemFlag; ///< for handling with DocShell
    bool                bOneAreaFlag;
    short                   nFixedCategory;

    OUString sAutomaticEntry;
    OUString sThousandSeparator;
    OUString sEngineeringNotation;

    VclPtr<vcl::Window>            pLastActivWindow;

    void    Init_Impl();
    void    FillCurrencyBox();
    void    FillFormatListBox_Impl( std::vector<OUString>& rEntries );
    void    UpdateOptions_Impl( bool bCheckCatChange );
    void    UpdateFormatListBox_Impl( bool bCat, bool bUpdateEdit );
    void    UpdateThousandEngineeringText();
    void    Obstructing();
    void    EnableBySourceFormat_Impl();
    void    SetCategory( sal_uInt16 nPos );
    OUString  GetExpColorString( Color*& rpPreviewColor, const OUString& aFormatStr, short nTmpCatPos );
    void    MakePreviewText( const OUString& rFormat );
    void    ChangePreviewText( sal_uInt16 nPos );
    void    AddAutomaticLanguage_Impl(LanguageType eAutoLang, bool bSelect);
    // Handler
    DECL_LINK( LostFocusHdl_Impl, Edit* );
    DECL_LINK( DoubleClickHdl_Impl, SvxFontListBox* );
    DECL_LINK( SelFormatHdl_Impl, void * );
    DECL_LINK( ClickHdl_Impl, PushButton* );
    DECL_LINK( EditHdl_Impl, Edit* );
    DECL_LINK( OptHdl_Impl, void * );
    DECL_LINK_TYPED(TimeHdl_Impl, Timer *, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
