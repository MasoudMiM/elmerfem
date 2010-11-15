/*****************************************************************************
 *                                                                           *
 *  Elmer, A Finite Element Software for Multiphysical Problems              *
 *                                                                           *
 *  Copyright 1st April 1995 - , CSC - IT Center for Science Ltd., Finland   *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or            *
 *  modify it under the terms of the GNU General Public License              *
 *  as published by the Free Software Foundation; either version 2           *
 *  of the License, or (at your option) any later version.                   *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with this program (in file fem/GPL-2); if not, write to the        *
 *  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,         *
 *  Boston, MA 02110-1301, USA.                                              *
 *                                                                           *
 *****************************************************************************/

/*****************************************************************************
 *                                                                           *
 *  ElmerClips                                                               *
 *                                                                           *
 *****************************************************************************
 *                                                                           *
 *  Authors: Mikko Lyly                                                      *
 *  Email:   Juha.Ruokolainen@csc.fi                                         *
 *  Web:     http://www.csc.fi/elmer                                         *
 *  Address: CSC - IT Center for Science Ltd.                                *
 *           Keilaranta 14                                                   *
 *           02101 Espoo, Finland                                            *
 *                                                                           *
 *  Original Date: 14 Nov 2010                                               *
 *                                                                           *
 *****************************************************************************/
#include "preview.h"

Preview::Preview(QWidget *parent) : QLabel(parent)
{
  setWindowIcon(QIcon(":/img/ElmerClips.ico"));

  setAlignment(Qt::AlignCenter);

  setMinimumSize(400, 400);

  showInfo();

  connect(&encoder, SIGNAL(drawThumbnail(const QString &)),
	  this, SLOT(drawThumbnail(const QString &)),
	  Qt::BlockingQueuedConnection);

  smallAction = new QAction(QIcon(""), "Small (width 640 pixels)", this);
  smallAction->setCheckable(true);
  smallAction->setChecked(true);

  mediumAction = new QAction(QIcon(""), "Medium (width 720 pixels)", this);
  mediumAction->setCheckable(true);
  mediumAction->setChecked(true);

  bigAction = new QAction(QIcon(""), "Big (width 1280 pixels)", this);
  bigAction->setCheckable(true);
  bigAction->setChecked(true);

  hugeAction = new QAction(QIcon(""), "Huge (width 1920 pixels)", this);
  hugeAction->setCheckable(true);
  hugeAction->setChecked(true);

  quitAction = new QAction(QIcon(""), "Quit", this);

  connect(quitAction, SIGNAL(triggered()),
	  this, SLOT(quitSlot()));

  resolutionMenu = new QMenu("Resolution", this);
  resolutionMenu->addAction(smallAction);
  resolutionMenu->addAction(mediumAction);
  resolutionMenu->addAction(bigAction);
  resolutionMenu->addAction(hugeAction);

  contextMenu = new QMenu(this);
  contextMenu->addMenu(resolutionMenu);
  contextMenu->addAction(quitAction);
}

void Preview::checkCommandLine()
{
  QList<QUrl> urls;

  if(qApp->arguments().count() > 1) {
    setAcceptDrops(false);

    foreach(const QString &arg, qApp->arguments())
      urls << QUrl(arg);

    encoder.setUrls(urls);
    encoder.setResolutions(getResolutions());
    encoder.start();
  }
}

void Preview::dragEnterEvent(QDragEnterEvent *event)
{
  if(event->mimeData()->hasUrls())
    event->acceptProposedAction();
}

void Preview::dropEvent(QDropEvent *event)
{
  if(!encoder.isRunning()) {
    setAcceptDrops(false);

    encoder.setUrls(event->mimeData()->urls());
    encoder.setResolutions(getResolutions());
    encoder.start();
  }  

  event->acceptProposedAction();
}

void Preview::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event);

  quitSlot();
}

void Preview::contextMenuEvent(QContextMenuEvent *event)
{
  contextMenu->popup(event->globalPos());
}

void Preview::drawThumbnail(const QString &fileName)
{
  if(fileName.startsWith("FILE")) {
    setWindowTitle(fileName);
    return;
  }

  if(fileName.startsWith("DONE")) {
    showInfo();

    if(qApp->arguments().count() > 1)
      quitSlot();

    return;
  }

  QPixmap pixmap(fileName);

  setPixmap(pixmap.scaledToWidth(width(), Qt::SmoothTransformation));
}

void Preview::showInfo()
{
  clear();

  setWindowTitle("ElmerClips");

  QPixmap video(":/img/500px-Crystal_Clear_mimetype_video.svg.png");

  int videoHeight = 250;

  video = video.scaledToHeight(videoHeight, Qt::SmoothTransformation);

  int videoWidth = video.width();

  QPixmap splash(400, 400);

  splash.fill(Qt::transparent);

  QPainter painter(&splash);

  QRectF target((400-videoWidth)/2, 25, videoWidth, videoHeight);

  QRectF source(0, 0, videoWidth, videoHeight);

  painter.drawPixmap(target, video, source);

  QFont defaultFont = painter.font();

  QFont boldFont = defaultFont;

  boldFont.setBold(true);

  painter.setFont(boldFont);

  painter.drawText(QRect(0, videoHeight+40, 400, 20), Qt::AlignCenter,
		   "Drag and drop image files/folders here");

  painter.setFont(defaultFont);

  painter.drawText(QRect(0, videoHeight+70, 400, 20), Qt::AlignCenter,
		   "Supported formats: png, jpg (jpeg), tiff, gif");

  painter.drawText(QRect(0, videoHeight+90, 400, 20), Qt::AlignCenter,
		   "Automatic ordering: first integer in file name");


  painter.drawText(QRect(0, videoHeight+110, 400, 20), Qt::AlignCenter,
		   "Right-click for more details");

  setPixmap(splash);

  setAcceptDrops(true);
}

void Preview::quitSlot()
{
  exit(0);
}

QList<int> Preview::getResolutions() const
{
  QList<int> resolutions;

  if(smallAction->isChecked())
    resolutions << 640;

  if(mediumAction->isChecked())
    resolutions << 720;

  if(bigAction->isChecked())
    resolutions << 1280;

  if(hugeAction->isChecked())
    resolutions << 1920;

  return resolutions;
}
