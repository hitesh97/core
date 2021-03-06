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

#include <hintids.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/i18n/WordType.hpp>
#include <viewopt.hxx>
#include <viewsh.hxx>
#include <SwPortionHandler.hxx>
#include <porhyph.hxx>
#include <inftxt.hxx>
#include <itrform2.hxx>
#include <guess.hxx>
#include <splargs.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::i18n;

Reference< XHyphenatedWord >  SwTxtFormatInfo::HyphWord(
                                const OUString &rTxt, const sal_Int32 nMinTrail )
{
    if( rTxt.getLength() < 4 || m_pFnt->IsSymbol(m_pVsh) )
        return 0;
    Reference< XHyphenator >  xHyph = ::GetHyphenator();
    Reference< XHyphenatedWord > xHyphWord;

    if( xHyph.is() )
        xHyphWord = xHyph->hyphenate( OUString(rTxt),
                            g_pBreakIt->GetLocale( m_pFnt->GetLanguage() ),
                            rTxt.getLength() - nMinTrail, GetHyphValues() );
    return xHyphWord;

}

/**
 * We format a row for interactive hyphenation
 */
bool SwTxtFrm::Hyphenate( SwInterHyphInfo &rHyphInf )
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),"swapped frame at SwTxtFrm::Hyphenate" );

    if( !g_pBreakIt->GetBreakIter().is() )
        return false;

    // We lock it, to start with
    OSL_ENSURE( !IsLocked(), "SwTxtFrm::Hyphenate: this is locked" );

    // The frame::Frame must have a valid SSize!
    Calc();
    GetFormatted();

    bool bRet = false;
    if( !IsEmpty() )
    {
        // We always need to enable hyphenation
        // Don't be afraid: the SwTxtIter saves the old row in the hyphenate
        TxtFrmLockGuard aLock( this );

        if ( IsVertical() )
            SwapWidthAndHeight();

        SwTxtFormatInfo aInf( this, true ); // true for interactive hyph!
        SwTxtFormatter aLine( this, &aInf );
        aLine.CharToLine( rHyphInf.nStart );

        // If we're within the first word of a row, it could've been hyphenated
        // in the row earlier.
        // That's why we go one row back.
        if( aLine.Prev() )
        {
            SwLinePortion *pPor = aLine.GetCurr()->GetFirstPortion();
            while( pPor->GetPortion() )
                pPor = pPor->GetPortion();
            if( pPor->GetWhichPor() == POR_SOFTHYPH ||
                pPor->GetWhichPor() == POR_SOFTHYPHSTR )
                aLine.Next();
        }

        const sal_Int32 nEnd = rHyphInf.GetEnd();
        while( !bRet && aLine.GetStart() < nEnd )
        {
            bRet = aLine.Hyphenate( rHyphInf );
            if( !aLine.Next() )
                break;
        }

        if ( IsVertical() )
            SwapWidthAndHeight();
    }
    return bRet;
}

/**
 * We format a row for interactive hyphenation
 * We can assume that we've already formatted.
 * We just reformat the row, the hyphenator will be prepared like
 * the UI expects it to be.
 * TODO: We can of course optimize this a lot.
 */
void SetParaPortion( SwTxtInfo *pInf, SwParaPortion *pRoot )
{
    OSL_ENSURE( pRoot, "SetParaPortion: no root anymore" );
    pInf->m_pPara = pRoot;
}

bool SwTxtFormatter::Hyphenate( SwInterHyphInfo &rHyphInf )
{
    SwTxtFormatInfo &rInf = GetInfo();

    // We never need to hyphenate anything in the last row
    // Except for, if it contains a FlyPortion or if it's the
    // last row of the Master
    if( !GetNext() && !rInf.GetTxtFly().IsOn() && !pFrm->GetFollow() )
        return false;

    sal_Int32 nWrdStart = nStart;

    // We need to retain the old row
    // E.g.: The attribute for hyphenation was not set, but
    // it's always set in SwTxtFrm::Hyphenate, because we want
    // to set breakpoints.
    SwLineLayout *pOldCurr = pCurr;

    InitCntHyph();

    // 5298: IsParaLine() (ex.IsFirstLine) fragt auf GetParaPortion() ab.
    // wir muessen gleiche Bedingungen schaffen: in der ersten
    // Zeile formatieren wir SwParaPortions...
    if( pOldCurr->IsParaPortion() )
    {
        SwParaPortion *pPara = new SwParaPortion();
        SetParaPortion( &rInf, pPara );
        pCurr = pPara;
        OSL_ENSURE( IsParaLine(), "SwTxtFormatter::Hyphenate: not the first" );
    }
    else
        pCurr = new SwLineLayout();

    nWrdStart = FormatLine( nWrdStart );

    // Man muss immer im Hinterkopf behalten, dass es z.B.
    // Felder gibt, die aufgetrennt werden koennen ...
    if( pCurr->PrtWidth() && pCurr->GetLen() )
    {
        // Wir muessen uns darauf einstellen, dass in der Zeile
        // FlyFrms haengen, an denen auch umgebrochen werden darf.
        // Wir suchen also die erste HyphPortion in dem angegebenen
        // Bereich.

        SwLinePortion *pPos = pCurr->GetPortion();
        const sal_Int32 nPamStart = rHyphInf.nStart;
        nWrdStart = nStart;
        const sal_Int32 nEnd = rHyphInf.GetEnd();
        while( pPos )
        {
            // Entweder wir liegen drueber oder wir laufen gerade auf eine
            // Hyphportion die am Ende der Zeile oder vor einem Flys steht.
            if( nWrdStart >= nEnd )
            {
                nWrdStart = 0;
                break;
            }

            if( nWrdStart >= nPamStart && pPos->InHyphGrp()
                && ( !pPos->IsSoftHyphPortion()
                     || static_cast<SwSoftHyphPortion*>(pPos)->IsExpand() ) )
            {
                nWrdStart = nWrdStart + pPos->GetLen();
                break;
            }

            nWrdStart = nWrdStart + pPos->GetLen();
            pPos = pPos->GetPortion();
        }
        // Wenn pPos 0 ist, wurde keine Trennstelle ermittelt.
        if( !pPos )
            nWrdStart = 0;
    }

    // Das alte LineLayout wird wieder eingestellt ...
    delete pCurr;
    pCurr = pOldCurr;

    if( pOldCurr->IsParaPortion() )
    {
        SetParaPortion( &rInf, static_cast<SwParaPortion*>(pOldCurr) );
        OSL_ENSURE( IsParaLine(), "SwTxtFormatter::Hyphenate: even not the first" );
    }

    if( nWrdStart==0 )
        return false;

    // nWrdStart bezeichnet nun die Position im String, der
    // fuer eine Trennung zur Debatte steht.
    // Start() hangelt sich zum End()
    rHyphInf.nWordStart = nWrdStart;

    sal_Int32 nLen = 0;
    const sal_Int32 nEnd = nWrdStart;

    // Wir suchen vorwaerts
    Reference< XHyphenatedWord > xHyphWord;

    Boundary aBound =
        g_pBreakIt->GetBreakIter()->getWordBoundary( rInf.GetTxt(), nWrdStart,
        g_pBreakIt->GetLocale( rInf.GetFont()->GetLanguage() ), WordType::DICTIONARY_WORD, true );
    nWrdStart = aBound.startPos;
    nLen = aBound.endPos - nWrdStart;
    if ( nLen == 0 )
        return false;

    OUString aSelTxt( rInf.GetTxt().copy(nWrdStart, nLen) );
    const sal_Int32 nMinTrail = ( nWrdStart + nLen > nEnd ) ? nWrdStart + nLen - nEnd - 1 : 0;

    //!! rHyphInf.SetHyphWord( ... ) mu??? hier geschehen
    xHyphWord = rInf.HyphWord( aSelTxt, nMinTrail );
    if ( xHyphWord.is() )
    {
        rHyphInf.SetHyphWord( xHyphWord );
        rHyphInf.nWordStart = nWrdStart;
        rHyphInf.nWordLen = nLen;
        rHyphInf.SetNoLang( false );
        rHyphInf.SetCheck( true );
        return true;
    }

    if ( !rHyphInf.IsCheck() )
        rHyphInf.SetNoLang( true );

    return false;
}

bool SwTxtPortion::CreateHyphen( SwTxtFormatInfo &rInf, SwTxtGuess &rGuess )
{
    Reference< XHyphenatedWord >  xHyphWord = rGuess.HyphWord();

    OSL_ENSURE( !pPortion, "SwTxtPortion::CreateHyphen(): another portion, another planet..." );
    OSL_ENSURE( xHyphWord.is(), "SwTxtPortion::CreateHyphen(): You are lucky! The code is robust here." );

    if( rInf.IsHyphForbud() ||
        pPortion || // robust
        !xHyphWord.is() || // more robust
        // Mehrzeilige Felder duerfen nicht interaktiv getrennt werden.
        ( rInf.IsInterHyph() && InFldGrp() ) )
        return false;

    SwHyphPortion *pHyphPor;
    sal_Int32 nPorEnd;
    SwTxtSizeInfo aInf( rInf );

    // first case: hyphenated word has alternative spelling
    if ( xHyphWord->isAlternativeSpelling() )
    {
        SvxAlternativeSpelling aAltSpell;
        aAltSpell = SvxGetAltSpelling( xHyphWord );
        OSL_ENSURE( aAltSpell.bIsAltSpelling, "no alternatve spelling" );

        OUString aAltTxt = aAltSpell.aReplacement;
        nPorEnd = aAltSpell.nChangedPos + rGuess.BreakStart() - rGuess.FieldDiff();
        sal_Int32 nTmpLen = 0;

        // soft hyphen at alternative spelling position?
        if( rInf.GetTxt()[ rInf.GetSoftHyphPos() ] == CHAR_SOFTHYPHEN )
        {
            pHyphPor = new SwSoftHyphStrPortion( aAltTxt );
            nTmpLen = 1;
        }
        else {
            pHyphPor = new SwHyphStrPortion( aAltTxt );
        }

        // length of pHyphPor is adjusted
        pHyphPor->SetLen( aAltTxt.getLength() + 1 );
        (SwPosSize&)(*pHyphPor) = pHyphPor->GetTxtSize( rInf );
        pHyphPor->SetLen( aAltSpell.nChangedLength + nTmpLen );
    }
    else
    {
        // second case: no alternative spelling
        SwHyphPortion aHyphPor;
        aHyphPor.SetLen( 1 );

        static const void* pLastMagicNo = 0;
        static sal_uInt16 aMiniCacheH = 0, aMiniCacheW = 0;
        const void* pTmpMagic;
        sal_uInt16 nFntIdx;
        rInf.GetFont()->GetMagic( pTmpMagic, nFntIdx, rInf.GetFont()->GetActual() );
        if( !pLastMagicNo || pLastMagicNo != pTmpMagic ) {
            pLastMagicNo = pTmpMagic;
            (SwPosSize&)aHyphPor = aHyphPor.GetTxtSize( rInf );
            aMiniCacheH = aHyphPor.Height(), aMiniCacheW = aHyphPor.Width();
        } else {
            aHyphPor.Height( aMiniCacheH ), aHyphPor.Width( aMiniCacheW );
        }
        aHyphPor.SetLen( 0 );
        pHyphPor = new SwHyphPortion( aHyphPor );

        pHyphPor->SetWhichPor( POR_HYPH );

        // values required for this
        nPorEnd = xHyphWord->getHyphenPos() + 1 + rGuess.BreakStart()
                - rGuess.FieldDiff();
    }

    // portion end must be in front of us
    // we do not put hyphens at start of line
    if ( nPorEnd > rInf.GetIdx() ||
         ( nPorEnd == rInf.GetIdx() && rInf.GetLineStart() != rInf.GetIdx() ) )
    {
        aInf.SetLen( nPorEnd - rInf.GetIdx() );
        pHyphPor->SetAscent( GetAscent() );
        SetLen( aInf.GetLen() );
        CalcTxtSize( aInf );

        Insert( pHyphPor );

        short nKern = rInf.GetFont()->CheckKerning();
        if( nKern )
            new SwKernPortion( *this, nKern );

        return true;
    }

    // last exit for the lost
    delete pHyphPor;
    BreakCut( rInf, rGuess );
    return false;
}

bool SwHyphPortion::GetExpTxt( const SwTxtSizeInfo &/*rInf*/, OUString &rTxt ) const
{
    rTxt = "-";
    return true;
}

void SwHyphPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Special( GetLen(), OUString('-'), GetWhichPor() );
}

bool SwHyphPortion::Format( SwTxtFormatInfo &rInf )
{
    const SwLinePortion *pLast = rInf.GetLast();
    Height( pLast->Height() );
    SetAscent( pLast->GetAscent() );
    OUString aTxt;

    if( !GetExpTxt( rInf, aTxt ) )
        return false;

    PrtWidth( rInf.GetTxtSize( aTxt ).Width() );
    const bool bFull = rInf.Width() <= rInf.X() + PrtWidth();
    if( bFull && !rInf.IsUnderflow() ) {
        Truncate();
        rInf.SetUnderflow( this );
    }

    return bFull;
}

bool SwHyphStrPortion::GetExpTxt( const SwTxtSizeInfo &, OUString &rTxt ) const
{
    rTxt = aExpand;
    return true;
}

void SwHyphStrPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Special( GetLen(), aExpand, GetWhichPor() );
}

SwLinePortion *SwSoftHyphPortion::Compress() { return this; }

SwSoftHyphPortion::SwSoftHyphPortion() :
    bExpand(false), nViewWidth(0), nHyphWidth(0)
{
    SetLen(1);
    SetWhichPor( POR_SOFTHYPH );
}

sal_uInt16 SwSoftHyphPortion::GetViewWidth( const SwTxtSizeInfo &rInf ) const
{
    // Although we're in the const, nViewWidth should be calculated at
    // the last possible moment
    if( !Width() && rInf.OnWin() && rInf.GetOpt().IsSoftHyph() && !IsExpand() )
    {
        if( !nViewWidth )
            const_cast<SwSoftHyphPortion*>(this)->nViewWidth
                = rInf.GetTxtSize(OUString('-')).Width();
    }
    else
        const_cast<SwSoftHyphPortion*>(this)->nViewWidth = 0;
    return nViewWidth;
}

/**
 * Cases:
 *
 *  1) SoftHyph is in the line, ViewOpt off
 *     -> invisible, neighbors unchanged
 *  2) SoftHyph is in the line, ViewOpt on
 *     -> visible, neighbors unchanged
 *  3) SoftHyph is at the end of the line, ViewOpt or or off
 *     -> always visible, neighbors unchanged
 */
void SwSoftHyphPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( Width() )
    {
        rInf.DrawViewOpt( *this, POR_SOFTHYPH );
        SwExpandPortion::Paint( rInf );
    }
}

/**
 * We get the final width from the FormatEOL()
 *
 * During the underflow-phase we determine, whether or not
 * there's an alternative spelling at all ...
 *
 * Case 1: "Au-to"
 * 1) {Au}{-}{to}, {to} does not fit anymore => underflow
 * 2) {-} calls hyphenate => no alternative
 * 3) FormatEOL() and bFull = true
 *
 * Case 2: "Zuc-ker"
 * 1) {Zuc}{-}{ker}, {ker} does not fit anymore => underflow
 * 2) {-} calls hyphenate => alternative!
 * 3) Underflow() and bFull = true
 * 4) {Zuc} calls hyphenate => {Zuk}{-}{ker}
 */
bool SwSoftHyphPortion::Format( SwTxtFormatInfo &rInf )
{
    bool bFull = true;

    // special case for old german spelling
    if( rInf.IsUnderflow()  )
    {
        if( rInf.GetSoftHyphPos() )
            return true;

        const bool bHyph = rInf.ChgHyph( true );
        if( rInf.IsHyphenate() )
        {
            rInf.SetSoftHyphPos( rInf.GetIdx() );
            Width(0);
            // if the soft hyphend word has an alternative spelling
            // when hyphenated (old german spelling), the soft hyphen
            // portion has to trigger an underflow
            SwTxtGuess aGuess;
            bFull = rInf.IsInterHyph() ||
                    !aGuess.AlternativeSpelling( rInf, rInf.GetIdx() - 1 );
        }
        rInf.ChgHyph( bHyph );

        if( bFull && !rInf.IsHyphForbud() )
        {
            rInf.SetSoftHyphPos(0);
            FormatEOL( rInf );
            if ( rInf.GetFly() )
                rInf.GetRoot()->SetMidHyph( true );
            else
                rInf.GetRoot()->SetEndHyph( true );
        }
        else
        {
            rInf.SetSoftHyphPos( rInf.GetIdx() );
            Truncate();
            rInf.SetUnderflow( this );
        }
        return true;
    }

    rInf.SetSoftHyphPos(0);
    SetExpand( true );
    bFull = SwHyphPortion::Format( rInf );
    SetExpand( false );
    if( !bFull )
    {
        // By default, we do not have a width, but we do have a height
        nHyphWidth = Width();
        Width(0);
    }
    return bFull;
}

/**
 * Format End of Line
 */
void SwSoftHyphPortion::FormatEOL( SwTxtFormatInfo &rInf )
{
    if( !IsExpand() )
    {
        SetExpand( true );
        if( rInf.GetLast() == this )
            rInf.SetLast( FindPrevPortion( rInf.GetRoot() ) );

        // We need to reset the old values
        const SwTwips nOldX  = rInf.X();
        const sal_Int32 nOldIdx = rInf.GetIdx();
        rInf.X( rInf.X() - PrtWidth() );
        rInf.SetIdx( rInf.GetIdx() - GetLen() );
        const bool bFull = SwHyphPortion::Format( rInf );
        nHyphWidth = Width();

        // Shady business: We're allowed to get wider, but a Fly is also
        // being processed, which needs a correct X position
        if( bFull || !rInf.GetFly() )
            rInf.X( nOldX );
        else
            rInf.X( nOldX + Width() );
        rInf.SetIdx( nOldIdx );
    }
}

/**
 * We're expanding:
 * - if the special characters should be visible
 * - if we're at the end of the line
 * - if we're before a (real/emulated) line break
 */
bool SwSoftHyphPortion::GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const
{
    if( IsExpand() || ( rInf.OnWin() && rInf.GetOpt().IsSoftHyph() ) ||
        ( GetPortion() && ( GetPortion()->InFixGrp() ||
          GetPortion()->IsDropPortion() || GetPortion()->IsLayPortion() ||
          GetPortion()->IsParaPortion() || GetPortion()->IsBreakPortion() ) ) )
    {
        return SwHyphPortion::GetExpTxt( rInf, rTxt );
    }
    return false;
}

void SwSoftHyphPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    const sal_uInt16 nWhich = ! Width() ?
                          POR_SOFTHYPH_COMP :
                          GetWhichPor();
    rPH.Special( GetLen(), OUString('-'), nWhich );
}

void SwSoftHyphStrPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    // Bug or feature?:
    // {Zu}{k-}{ker}, {k-} will be gray instead of {-}
    rInf.DrawViewOpt( *this, POR_SOFTHYPH );
    SwHyphStrPortion::Paint( rInf );
}

SwSoftHyphStrPortion::SwSoftHyphStrPortion( const OUString &rStr )
    : SwHyphStrPortion( rStr )
{
    SetLen( 1 );
    SetWhichPor( POR_SOFTHYPHSTR );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
