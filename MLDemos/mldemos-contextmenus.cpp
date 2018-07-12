/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#include "mldemos.h"

void MLDemos::ShowContextMenuSpray(const QPoint &point)
{
    HideContextMenus();
    QPoint pt = QPoint(30, 0);
    drawContext1Widget->move(drawToolbar->sprayButton->mapToGlobal(pt));
    drawContext1Widget->show();
    drawContext1Widget->setFocus();
    drawContext1Widget->repaint();
    update();
}
void MLDemos::ShowContextMenuLine(const QPoint &point)
{
    HideContextMenus();
    QPoint pt = QPoint(30, 0);
    drawContext2Widget->move(drawToolbar->lineButton->mapToGlobal(pt));
    drawContext2Widget->show();
    drawContext2Widget->setFocus();
    drawContext2Widget->repaint();
    update();
}
void MLDemos::ShowContextMenuEllipse(const QPoint &point)
{
    HideContextMenus();
    QPoint pt = QPoint(30, 0);
    drawContext2Widget->move(drawToolbar->ellipseButton->mapToGlobal(pt));
    drawContext2Widget->show();
    drawContext2Widget->setFocus();
    drawContext2Widget->repaint();
    update();
}
void MLDemos::ShowContextMenuErase(const QPoint &point)
{
    HideContextMenus();
    QPoint pt = QPoint(30, 0);
    drawContext1Widget->move(drawToolbar->eraseButton->mapToGlobal(pt));
    drawContext1Widget->show();
    drawContext1Widget->setFocus();
    drawContext1Widget->repaint();
    update();
}
void MLDemos::ShowContextMenuObstacle(const QPoint &point)
{
    HideContextMenus();
    QPoint pt = QPoint(30, 0);
    drawContext3Widget->move(drawToolbar->obstacleButton->mapToGlobal(pt));
    drawContext3Widget->show();
    drawContext3Widget->setFocus();
    drawContext3Widget->repaint();
    update();
}
void MLDemos::ShowContextMenuReward(const QPoint &point)
{
    HideContextMenus();
    QPoint pt = QPoint(30, 0);
    drawContext4Widget->move(drawToolbar->paintButton->mapToGlobal(pt));
    drawContext4Widget->show();
    drawContext4Widget->setFocus();
    drawContext4Widget->repaint();
    update();
}
inline bool IsChildOf(QObject *child, QObject *parent)
{
    if (!parent || !child) return false;
    if (child == parent) return true;
    QList<QObject*> list = parent->children();
    if (list.isEmpty()) return false;
    QList<QObject*>::iterator i;
    for (i = list.begin(); i<list.end(); ++i) {
        if (IsChildOf(child, *i)) return true;
    }
    return false;
}
void MLDemos::FocusChanged(QWidget *old, QWidget *now)
{
    if (drawContext1Widget && drawContext1Widget->isVisible()) {
        if (!IsChildOf(now, drawContext1Widget)) HideContextMenus();
    }
    if (drawContext2Widget && drawContext2Widget->isVisible()) {
        if (!IsChildOf(now, drawContext2Widget)) HideContextMenus();
    }
    if (drawContext3Widget && drawContext3Widget->isVisible()) {
        if (!IsChildOf(now, drawContext3Widget)) HideContextMenus();
    }
    if (drawContext4Widget && drawContext4Widget->isVisible()) {
        if (!IsChildOf(now, drawContext4Widget)) HideContextMenus();
    }
}
void MLDemos::HideContextMenus()
{
    if(drawContext1Widget) drawContext1Widget->hide();
    if(drawContext2Widget) drawContext2Widget->hide();
    if(drawContext3Widget) drawContext3Widget->hide();
    if(drawContext4Widget) drawContext4Widget->hide();
}
