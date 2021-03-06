/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITENUMS_H
#define INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITENUMS_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef LOK_USE_UNSTABLE_API
typedef enum
{
  LOK_DOCTYPE_TEXT,
  LOK_DOCTYPE_SPREADSHEET,
  LOK_DOCTYPE_PRESENTATION,
  LOK_DOCTYPE_DRAWING,
  LOK_DOCTYPE_OTHER
}
LibreOfficeKitDocumentType;

typedef enum
{
    LOK_PARTMODE_DEFAULT,
    LOK_PARTMODE_SLIDE,
    LOK_PARTMODE_NOTES,
    LOK_PARTMODE_SLIDENOTES,
    LOK_PARTMODE_EMBEDDEDOBJ
}
LibreOfficeKitPartMode;

typedef enum
{
    /**
     * Any tiles which are over the rectangle described in the payload are no
     * longer valid.
     *
     * Rectangle format: "width, height, x, y", where all numbers are document
     * coordinates, in twips. When all tiles are supposed to be dropped, the
     * format is the "EMPTY" string.
     */
    LOK_CALLBACK_INVALIDATE_TILES,
    /**
     * The size and/or the position of the visible cursor changed.
     *
     * Rectangle format is the same as LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR,
    /**
     * The list of rectangles representing the current text selection changed.
     *
     * List format is "rectangle1[; rectangle2[; ...]]" (without quotes and
     * brackets), where rectangleN has the same format as
     * LOK_CALLBACK_INVALIDATE_TILES. When there is no selection, an empty
     * string is provided.
     */
    LOK_CALLBACK_TEXT_SELECTION,
    /**
     * The position and size of the cursor rectangle at the text
     * selection start. It is used to draw the selection handles.
     *
     * This callback must be called prior to LOK_CALLBACK_TEXT_SELECTION every
     * time the selection is updated.
     *
     * Rectangle format is the same as LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_TEXT_SELECTION_START,
    /**
     * The position and size of the cursor rectangle at the text
     * selection end. It is used to draw the selection handles.
     *
     * This callback must be called prior to LOK_CALLBACK_TEXT_SELECTION every
     * time the selection is updated.
     *
     * Rectangle format is the same as LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_TEXT_SELECTION_END,
    /**
     * The blinking text cursor is now visible or not.
     *
     * Clients should assume that this is true initially and are expected to
     * hide the blinking cursor at the rectangle described by
     * LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR once it becomes false. Payload is
     * either the "true" or the "false" string.
     */
    LOK_CALLBACK_CURSOR_VISIBLE,
    /**
     * The size and/or the position of the graphic selection changed.
     *
     * Rectangle format is the same as LOK_CALLBACK_INVALIDATE_TILES.
     */
    LOK_CALLBACK_GRAPHIC_SELECTION,

    /**
     * User clicked on an hyperlink that should be handled by other
     * applications accordingly.
     */
    LOK_CALLBACK_HYPERLINK_CLICKED,

    /**
     * Emit state update to the client.
     * For example, when cursor is on bold text, this callback is triggered
     * with payload: ".uno:Bold=true"
     */
    LOK_CALLBACK_STATE_CHANGED,

    /**
     * Start a "status indicator" (here restricted to a progress bar type
     * indicator). The payload is the descriptive text (or empty). Even if
     * there is no documentation that would promise so, we assume that de facto
     * for a document being viewed or edited, there will be at most one status
     * indicator, and its descriptive text will not change.
     *
     * Note that for the case of the progress indication during loading of a
     * document, the status indicator callbacks will arrive to the callback
     * registered for the LibreOfficeKit (singleton) object, not a
     * LibreOfficeKitDocument one, because we are in the very progress of
     * loading a docuemnt and then constructing a LibreOfficeKitDocument
     * object.
     */
    LOK_CALLBACK_STATUS_INDICATOR_START,

    /**
     * Sets the numeric value of the status indicator.
     * The payload should be a percentage, an integer between 0 and 100.
     */
    LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE,

    /**
     * Ends the status indicator.
     *
     * Not necessarily ever emitted.
     */
    LOK_CALLBACK_STATUS_INDICATOR_FINISH
}
LibreOfficeKitCallbackType;

typedef enum
{
    /// A key on the keyboard is pressed.
    LOK_KEYEVENT_KEYINPUT,
    /// A key on the keyboard is released.
    LOK_KEYEVENT_KEYUP
}
LibreOfficeKitKeyEventType;

typedef enum
{
    /// A pressed gesture has started.
    LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
    /// A pressed gesture has finished.
    LOK_MOUSEEVENT_MOUSEBUTTONUP,
    /// A change has happened during a press gesture.
    LOK_MOUSEEVENT_MOUSEMOVE
}
LibreOfficeKitMouseEventType;

typedef enum
{
    /// The start of selection is to be adjusted.
    LOK_SETTEXTSELECTION_START,
    /// The end of selection is to be adjusted.
    LOK_SETTEXTSELECTION_END,
    /// Both the start and the end of selection is to be adjusted.
    LOK_SETTEXTSELECTION_RESET
}
LibreOfficeKitSetTextSelectionType;

typedef enum
{
    /**
     * A move or a resize action starts. It is assumed that there is a valid
     * graphic selection (see LOK_CALLBACK_GRAPHIC_SELECTION) and the supplied
     * coordinates are the ones the user tapped on.
     *
     * The type of the action is move by default, unless the coordinates are
     * the position of a handle (see below), in which case it's a resize.
     *
     * There are 8 handles for a graphic selection:
     * - top-left, top-center, top-right
     * - middle-left, middle-right
     * - bottom-left, bottom-center, bottom-right
     */
    LOK_SETGRAPHICSELECTION_START,
    /**
     * A move or resize action stops. It is assumed that this is always used
     * only after a LOK_SETTEXTSELECTION_START. The supplied coordinates are
     * the ones where the user released the screen.
     */
    LOK_SETGRAPHICSELECTION_END
}
LibreOfficeKitSetGraphicSelectionType;

#endif // LOK_USE_UNSTABLE_API

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITENUMS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
