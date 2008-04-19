/*****************************************************************************
 *                                                                           *
 *  Elmer, A Finite Element Software for Multiphysical Problems              *
 *                                                                           *
 *  Copyright 1st April 1995 - , CSC - Scientific Computing Ltd., Finland    *
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
 *  ELMER/Mesh3D convergenceview                                             *
 *                                                                           *
 *****************************************************************************
 *                                                                           *
 *  Authors: Mikko Lyly, Juha Ruokolainen and Peter R�back                   *
 *  Email:   Juha.Ruokolainen@csc.fi                                         *
 *  Web:     http://www.csc.fi/elmer                                         *
 *  Address: CSC - Scientific Computing Ltd.                                 *
 *           Keilaranta 14                                                   *
 *           02101 Espoo, Finland                                            *
 *                                                                           *
 *  Original Date: 15 Mar 2008                                               *
 *                                                                           *
 *****************************************************************************/

#include <QtGui>
#include <iostream>
#include "convergenceview.h"

using namespace std;

//-----------------------------------------------------------------------------
CurveData::CurveData()
  : d_count(0)
{
}

void CurveData::append(double *x, double *y, int count)
{
  int newSize = ((d_count + count) / 1000 + 1) * 1000;
  if(newSize > size()) {
    d_x.resize(newSize);
    d_y.resize(newSize);
  }
  
  for(register int i = 0; i < count; i++) {
    d_x[d_count + i] = x[i];
    d_y[d_count + i] = y[i];
  }
  
  d_count += count;
}

int CurveData::count() const
{
  return d_count;
}

int CurveData::size() const
{
  return d_x.size();
}

const double *CurveData::x() const
{
  return d_x.data();
}

const double *CurveData::y() const
{
  return d_y.data();
}

//-----------------------------------------------------------------------------
ConvergenceView::ConvergenceView(QWidget *parent)
  : QMainWindow(parent)
{
  plot = new QwtPlot(this);
  plot->resize(200, 200);

  plot->setAutoReplot(true);
  plot->setCanvasBackground(QColor(Qt::white));
  title = "Nonlinear system convergence";
  plot->setTitle(title);

  // legend
  legend = new QwtLegend;
  legend->setFrameStyle(QFrame::Box|QFrame::Sunken);
  plot->insertLegend(legend, QwtPlot::RightLegend);
  
  // grid
  grid = new QwtPlotGrid;
  grid->enableXMin(true);
  grid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
  grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
  grid->attach(plot);

  // axis
  plot->setAxisTitle(QwtPlot::xBottom, "Iteration step");
  plot->setAxisTitle(QwtPlot::yLeft, "Relative change");
  plot->setAxisMaxMajor(QwtPlot::xBottom, 20);
  plot->setAxisMaxMinor(QwtPlot::xBottom, 1);
  plot->setAxisMaxMajor(QwtPlot::yLeft, 10);
  plot->setAxisMaxMinor(QwtPlot::yLeft, 10);

  // scale engine
  scaleEngine = new QwtLog10ScaleEngine;
  plot->setAxisScaleEngine(QwtPlot::yLeft, scaleEngine);

  // pens
  pen[0] = QPen(Qt::red);
  pen[1] = QPen(Qt::green);
  pen[2] = QPen(Qt::blue);
  pen[3] = QPen(Qt::black);
  pen[4] = QPen(Qt::cyan);
  pen[5] = QPen(Qt::yellow);

  createActions();
  createMenus();
  createToolBars();
  createStatusBar();

  setCentralWidget(plot);
  setWindowTitle("Convergence monitor");

  showLegend = true;
  showGrid = true;
  showNSHistory = true;
  showSSHistory = false;
}

ConvergenceView::~ConvergenceView()
{
  curveList.clear();
  delete plot;
}


void ConvergenceView::appendData(double y, QString name)
{
  appendData(&y, 1, name);
}

void ConvergenceView::appendData(double *y, int size, QString name)
{
  QStringList nameSplitted = name.trimmed().split("/");

  bool NS = false;
  if(nameSplitted.at(0) == "NS")
    NS = true;

  bool SS = false;
  if(nameSplitted.at(0) == "SS")
    SS = true;

  Curve *curve = curveList.value(name, NULL);
  
  if(curve == NULL) {
    curve = new Curve;
    curve->d_data = new CurveData;    
    curve->d_curve = new QwtPlotCurve(name);
//    curve->d_curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    QPen currentPen = pen[curveList.count()];
    currentPen.setWidth(2);
    curve->d_curve->setPen(currentPen);

    if(NS && showNSHistory)
      curve->d_curve->attach(plot);

    if(SS && showSSHistory)
      curve->d_curve->attach(plot);

    curveList.insert(name, curve);
  }

  double x = (double)(curve->d_data->count());
  curve->d_data->append(&x, y, size);
  curve->d_curve->setRawData(curve->d_data->x(), 
			     curve->d_data->y(), 
			     curve->d_data->count());
  plot->setTitle(title);
}

void ConvergenceView::removeData()
{
  for( int i = 0; i < curveList.count(); i++) {
    Curve *curve = curveList.values().at(i);
    delete curve->d_data;
    curve->d_data = NULL;
    delete curve->d_curve;
    curve->d_curve = NULL;
  }
  curveList.clear();  
  title = "";
  plot->clear();
  plot->replot();
}

QSize ConvergenceView::minimumSizeHint() const
{
  return QSize(64, 64);
}


QSize ConvergenceView::sizeHint() const
{
  return QSize(480, 640);
}

void ConvergenceView::createActions()
{
  exitAct = new QAction(QIcon(":/icons/application-exit.png"), tr("&Quit"), this);
  exitAct->setShortcut(tr("Ctrl+Q"));
  exitAct->setStatusTip("Quit convergence monitor");
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

  showGridAct = new QAction(QIcon(":/icons/dialog-ok-apply.png"), tr("&Grid"), this);
  showGridAct->setStatusTip("Show grid");
  connect(showGridAct, SIGNAL(triggered()), this, SLOT(showGridSlot()));  

  showLegendAct = new QAction(QIcon(":/icons/dialog-ok-apply.png"), tr("&Legend"), this);
  showLegendAct->setStatusTip("Show legend");
  connect(showLegendAct, SIGNAL(triggered()), this, SLOT(showLegendSlot())); 

  showNSHistoryAct = new QAction(QIcon(":/icons/dialog-ok-apply.png"), 
				 tr("&Nonlinear system"), this);
  showNSHistoryAct->setStatusTip("Show nonlinear system convergence history");
  connect(showNSHistoryAct, SIGNAL(triggered()), this, SLOT(showNSHistorySlot())); 

  showSSHistoryAct = new QAction(QIcon(""), tr("&Steady state"), this);
  showSSHistoryAct->setStatusTip("Show steady state convergence history");
  connect(showSSHistoryAct, SIGNAL(triggered()), this, SLOT(showSSHistorySlot())); 
}

void ConvergenceView::createMenus()
{
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(exitAct);
  
  viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(showGridAct);
  viewMenu->addAction(showLegendAct);
  viewMenu->addSeparator();
  viewMenu->addAction(showNSHistoryAct);
  viewMenu->addAction(showSSHistoryAct);
}

void ConvergenceView::createToolBars()
{
#if 0
  viewToolBar = addToolBar(tr("&View"));
  viewToolBar->addAction(showGridAct);
  viewToolBar->addAction(showLegendAct);
#endif
}

void ConvergenceView::createStatusBar()
{
  statusBar()->showMessage("Ready");
}

void ConvergenceView::showGridSlot()
{
  showGrid = !showGrid;
  
  if(showGrid) {
    grid->attach(plot);
    showGridAct->setIcon(QIcon(":/icons/dialog-ok-apply.png"));
  } else {
    grid->detach();
    showGridAct->setIcon(QIcon(""));
  }

  plot->replot();
}

void ConvergenceView::showLegendSlot()
{
  showLegend = !showLegend;
  
  if(showLegend) {
    legend = new QwtLegend;
    legend->setFrameStyle(QFrame::Box|QFrame::Sunken);
    plot->insertLegend(legend, QwtPlot::RightLegend);
    showLegendAct->setIcon(QIcon(":/icons/dialog-ok-apply.png"));
  } else {
    delete legend;
    showLegendAct->setIcon(QIcon(""));
  }

  plot->replot();
}

void ConvergenceView::showNSHistorySlot()
{
  showNSHistory = !showNSHistory;
  
  if(showNSHistory) {
    showNSHistoryAct->setIcon(QIcon(":/icons/dialog-ok-apply.png"));
  } else {
    showNSHistoryAct->setIcon(QIcon(""));
  }

  // attach/detach curves:
  for(int i = 0; i < curveList.count(); i++) {
    QString key = curveList.keys().at(i);
    Curve *curve = curveList.values().at(i);
    QStringList keySplitted = key.trimmed().split("/");
    if(keySplitted.at(0) == "NS") {
      if(showNSHistory)
	curve->d_curve->attach(plot);
      else
	curve->d_curve->detach();
    }
  }

  plot->replot();
}

void ConvergenceView::showSSHistorySlot()
{
  showSSHistory = !showSSHistory;
  
  if(showSSHistory) {
    showSSHistoryAct->setIcon(QIcon(":/icons/dialog-ok-apply.png"));
  } else {
    showSSHistoryAct->setIcon(QIcon(""));
  }

  // attach/detach curves:
  for(int i = 0; i < curveList.count(); i++) {
    QString key = curveList.keys().at(i);
    Curve *curve = curveList.values().at(i);
    QStringList keySplitted = key.trimmed().split("/");
    if(keySplitted.at(0) == "SS") {
      if(showSSHistory)
	curve->d_curve->attach(plot);
      else
	curve->d_curve->detach();
    }
  }

  plot->replot();
}
