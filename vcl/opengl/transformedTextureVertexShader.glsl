/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

attribute vec4 position;
attribute vec2 tex_coord_in;
uniform vec2 viewport;
uniform mat4 transform;
varying vec2 tex_coord;

void main() {
    vec4 pos = transform * position;
    pos.x = (2.0 * pos.x) / viewport.x - 1.0;
    pos.y = 1.0 - (2.0 * pos.y / viewport.y);
    gl_Position = pos;
    tex_coord = tex_coord_in;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
