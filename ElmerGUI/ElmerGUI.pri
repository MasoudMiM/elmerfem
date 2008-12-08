#==============================================================================
#
#      ElmerGUI: libraries and headers for the qmake project file
#
#==============================================================================

#------------------------------------------------------------------------------
# Installation directory:
#------------------------------------------------------------------------------
unix: ELMER_HOME = /usr/local
win32: ELMER_HOME = c:\Elmer5.4
macx: ELMER_HOME = /usr/local

#------------------------------------------------------------------------------
# Python library:
#------------------------------------------------------------------------------
unix {
   PY_INCLUDEPATH = /usr/include/python2.5
   PY_LIBPATH = /usr/lib
   PY_LIBS = -lpython2.5
}

win32 {
   PY_INCLUDEPATH = c:\PYTHON\Python-2.6.1\Include
   PY_LIBPATH = c:\PYTHON\Python-2.6.1\PCbuild
   PY_LIBS = -lpython26
}

macx {
}

#------------------------------------------------------------------------------
# QWT library:
#------------------------------------------------------------------------------
unix {
  QWT_INCLUDEPATH = /usr/include/qwt-qt4
  QWT_LIBPATH = /usr/lib
  QWT_LIBS = -lqwt-qt4
}

win32 {
  QWT_INCLUDEPATH = c:\qwt\5.1.1\vc\include
  QWT_LIBPATH = c:\qwt\5.1.1\vc\lib
  QWT_LIBS = -lqwt5
}

macx {
  QWT_INCLUDEPATH = /usr/local/qwt-5.0.2/include
  QWT_LIBPATH = /usr/local/qwt-5.0.2/lib
  QWT_LIBS =  -lqwt5
}

#------------------------------------------------------------------------------
# VTK library:
#------------------------------------------------------------------------------
unix {
   VTK_INCLUDEPATH = /usr/include/vtk-5.0
   VTK_LIBPATH = /usr/lib
   VTK_LIBS = -lvtkHybrid \
              -lvtkWidgets \
    	      -lQVTK
}

win32 {
   VTK_INCLUDEPATH = c:\VTK\VC\include\vtk-5.2
   VTK_LIBPATH = c:\VTK\VC\lib\vtk-5.2
   VTK_LIBS = -lQVTK \
              -lvtkCommon \
              -lvtkDICOMParser \
              -lvtkFiltering \
              -lvtkGenericFiltering \
              -lvtkGraphics \
              -lvtkHybrid \
              -lvtkIO \
              -lvtkImaging \
              -lvtkInfovis \
              -lvtkNetCDF \
              -lvtkRendering \
              -lvtkViews \
              -lvtkVolumeRendering \
              -lvtkWidgets \
              -lvtkexoIIc \
              -lvtkexpat \
              -lvtkfreetype \
              -lvtkftgl \
              -lvtkjpeg \
              -lvtklibxml2 \
              -lvtkmetaio \
              -lvtkpng \
              -lvtksys \
              -lvtktiff \
              -lvtkverdict \
              -lvtkzlib \
              -ladvapi32
}

macx {
   VTK_INCLUDEPATH = /usr/local/include/vtk-5.0
   VTK_LIBPATH = /usr/lib
   VTK_LIBS = -lvtkHybrid \
              -lvtkWidgets \
	      -lQVTK
}

#------------------------------------------------------------------------------
# OpenCASCADE library:
#------------------------------------------------------------------------------
unix {
   OCC_INCLUDEPATH = /usr/local/OpenCASCADE/inc
   OCC_LIBPATH = /usr/local/OpenCASCADE/lib
   OCC_LIBS = -lTKBRep -lTKSTL -lTKSTEP -lTKIGES
}

win32 {
   OCC_INCLUDEPATH = $(CASROOT)/inc
   OCC_LIBPATH = $(CASROOT)/win32/lib
   OCC_LIBS = $(CASROOT)/win32/lib/TKBRep.lib \
              $(CASROOT)/win32/lib/TKernel.lib \
              $(CASROOT)/win32/lib/TKG2d.lib \
              $(CASROOT)/win32/lib/TKG3d.lib \
              $(CASROOT)/win32/lib/TKGeomAlgo.lib \
              $(CASROOT)/win32/lib/TKGeomBase.lib \
              $(CASROOT)/win32/lib/TKMath.lib \
              $(CASROOT)/win32/lib/TKMesh.lib \
              $(CASROOT)/win32/lib/TKShHealing.lib \
              $(CASROOT)/win32/lib/TKSTEP.lib \
              $(CASROOT)/win32/lib/TKSTEP209.lib \
              $(CASROOT)/win32/lib/TKSTEPAttr.lib \
              $(CASROOT)/win32/lib/TKSTEPBase.lib \
              $(CASROOT)/win32/lib/TKIGES.lib \
              $(CASROOT)/win32/lib/TKTopAlgo.lib \
              $(CASROOT)/win32/lib/TKXSBase.lib
}

macx {
   OCC_INCLUDEPATH = 
   OCC_LIBPATH = 
   OCC_LIBS = 
}
