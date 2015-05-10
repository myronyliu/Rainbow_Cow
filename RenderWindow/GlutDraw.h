#ifndef GLUTDRAW_H
#define GLUTDRAW_H

#include "stdafx.h"

namespace GlutDraw
{
void drawLine(float x1, float y1, float z1,
        float x2, float y2, float z2);

//TODO PRIO: drawRectangle so that we can do buttons.
void drawRectangle(float x, float y,
        float width, float height);

//Draw sphere at center of panel
void drawSphere(float r,GLint n, GLint m);
};
#endif /* GLUTDRAW_H */