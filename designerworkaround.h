#ifndef designerworkaround_h__2005_07_10_10_46_begin_guarded_code
#define designerworkaround_h__2005_07_10_10_46_begin_guarded_code

#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QMainWindow>
#include <QComboBox>
#include <QMenuBar>
#include <QLabel>
#include <QLineEdit>

class DesignerWorkaround
{
public:
  QMenuBar *menubar;
  QMenu *ExecutMenu;
  QMenu *StylesMenu;
  QMenu *colormenu;
  QMenu *fontmenu;
  QToolBar *mainToolbar;
  QAction* ActionIni;
  QAction* ActionParar;
  QAction* ActionCarDados;
  QAction* ActionSalvarDados;
  QAction* dump;
  QAction* normals;
  QAction* Exit;
  
  QActionGroup* coord;
  QAction* Box;
  QAction* Frame;
  QAction* None;
  
  QActionGroup* plotstyle;
  QAction* wireframe;
  QAction* hiddenline;
  QAction* polygon;
  QAction* filledmesh;
  QAction* nodata;
  QAction* pointstyle;
  
  QActionGroup* color;
  QAction* axescolor;
  QAction* backgroundcolor;
  QAction* meshcolor;
  QAction* numbercolor;
  QAction* labelcolor;
  QAction* titlecolor;
  QAction* datacolor;
  QAction* resetcolor;
  
  QActionGroup* fontAC;
  QAction* numberfont;
  QAction* labelfont;
  QAction* titlefont;
  QAction* resetfont;
  
  QActionGroup* floorstyle;
  QAction* floordata;
  QAction* flooriso;
  QAction* floornone;
  
  QActionGroup* grids;
  QAction* front;
  QAction* back;
  QAction* right;
  QAction* left;
  QAction* ceil;
  QAction* floor;

  QLabel *LMESH;
  QLabel *LMSX;
  QLineEdit *LEMSX;
  QLabel *LMSY;
  QLineEdit *LEMSY;

  QLabel *LESCALA;
  QLabel *LESX;
  QLineEdit *LEESX;
  QLabel *LESY;
  QLineEdit *LEESY;
  QLabel *LESZ;
  QLineEdit *LEESZ;

  QLabel *LN;
  QLineEdit *LEN;
  //QLabel *LPC;
  //QLineEdit *LEPC;
  //QLabel *LPM;
  //QLineEdit *LEPM;

  QLabel *LEL;
  QLineEdit *LEEL;
  QLabel *LEM;
  QLineEdit *LEEM;

  QLabel *LDI;
  QLineEdit *LEDI;

  QLabel *LPI;
  QLineEdit *LEPI;

  QLabel *LMX;
  QLabel *LMY;
  QLabel *LFMXY;

  //QComboBox *CBSelecao;
  QComboBox *styleComboBox;

  void setupWorkaround(QMainWindow* mw);
};

#endif
