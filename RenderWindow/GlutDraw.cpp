#include "GlutDraw.h"

void GlutDraw::drawLine(float x1, float y1, float z1,
        float x2, float y2, float z2)
{
    glBegin(GL_LINES);
        glVertex3d(x1, y1, z1);
        glVertex3d(x2, y2, z2);
    glEnd();
}

//TODO PRIO: drawRectangle so that we can do buttons.
void GlutDraw::drawRectangle(float x, float y,
        float width, float height)
{
    glBegin(GL_QUADS);
        glVertex2d(x, y);
        glVertex2d(x + width, y);
        glVertex2d(x + width, y + height);
        glVertex2d(x, y + height);
    glEnd();
}

void GlutDraw::drawSphere(float r,GLint n, GLint m/*, float* outR, float* outG, float* outB*/)
{
    glutSolidSphere(r, n, m);
}
