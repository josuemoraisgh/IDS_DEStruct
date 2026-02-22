#include <qlabel.h>
#include <QtGui>
#include "designerworkaround.h"
//////////////////////////////////////////////////////////////////////////////////
namespace
{
  QString tr(const char* val)
  {
    return QObject::tr(val);
  }
  //////////////////////////////////////////////////////////////////////////////////
  void setCheckable(QActionGroup* ag) 
  {
    QList<QAction*> tmplist = ag->actions();
    for (QList<QAction*>::iterator it=tmplist.begin(); it!=tmplist.end(); ++it)
      (*it)->setCheckable(true);
  }
}
//////////////////////////////////////////////////////////////////////////////////
void DesignerWorkaround::setupWorkaround(QMainWindow* mw)
{
  // actions
  ActionIni = new QAction(tr("Iniciar"), mw);
  ActionIni->setShortcut( QKeySequence(tr("CTRL+I")));
  ActionParar = new QAction(tr("&Parar"), mw);
  ActionParar->setShortcut( QKeySequence(tr("CTRL+P")));
  ActionCarDados = new QAction(tr("Carre&gar"), mw);
  ActionCarDados->setShortcut( QKeySequence(tr("CTRL+G")));
  ActionSalvarDados = new QAction(tr("&Salvar Dados"), mw);
  ActionSalvarDados->setShortcut( QKeySequence(tr("CTRL+S")));
  //ActionSalvarConfig = new QAction(tr("Salvar Confi&g."), mw);
  //ActionSalvarConfig->setShortcut( QKeySequence(tr("CTRL+G")));

  Exit = new QAction(tr("&Exit"), mw);
  Exit->setShortcut( QKeySequence(tr("CTRL+Q")));

  dump = new QAction(QIcon(":/images/savecontent.png"), "", mw);

  coord = new QActionGroup(mw);
    Box = new QAction(QIcon(":/images/box.png"), "", coord);
    Frame = new QAction(QIcon(":/images/frame.png"), "", coord);
    None = new QAction(QIcon(":/images/grid.png"), "", coord);
  setCheckable(coord);

  grids = new QActionGroup(mw);
    front = new QAction(QIcon(":/images/gridfr.png"), "", grids);
    back = new QAction(QIcon(":/images/gridb.png"), "", grids);
    right = new QAction(QIcon(":/images/gridr.png"), "", grids);
    left = new QAction(QIcon(":/images/gridl.png"), "", grids);
    ceil = new QAction(QIcon(":/images/gridc.png"), "", grids);
    floor = new QAction(QIcon(":/images/gridf.png"), "", grids);
  grids->setExclusive(false);
  setCheckable(grids);

  plotstyle = new QActionGroup(mw);
    pointstyle = new QAction(QIcon(":/images/scattered.png"), "", plotstyle);
    wireframe = new QAction(QIcon(":/images/wireframe.png"), "", plotstyle);
    hiddenline = new QAction(QIcon(":/images/hiddenline.png"), "", plotstyle);
    polygon = new QAction(QIcon(":/images/polygon.png"), "", plotstyle);
    filledmesh = new QAction(QIcon(":/images/filledmesh.png"), "", plotstyle);
    nodata = new QAction(QIcon(":/images/nodata.png"), "", plotstyle);
  setCheckable(plotstyle);
  
  floorstyle = new QActionGroup(mw);
    floordata = new QAction(QIcon(":/images/floordata.png"), "", floorstyle);
    flooriso = new QAction(QIcon(":/images/flooriso.png"), "", floorstyle);
    floornone = new QAction(QIcon(":/images/floorempty.png"), "", floorstyle);
  setCheckable(floorstyle);  

  normals = new QAction(QIcon(":/images/normals.png"), "", mw);
  normals->setCheckable(true);

  color = new QActionGroup(mw);
    axescolor = new QAction(tr("&Axes"), color);
    backgroundcolor = new QAction(tr("&Background"), color);
    meshcolor = new QAction(tr("&Mesh"), color);
    numbercolor = new QAction(tr("&Numbers"), color);
    labelcolor = new QAction(tr("&Label"), color);
    titlecolor = new QAction(tr("Caption"), color);
    datacolor = new QAction(tr("Data color"), color);
    resetcolor = new QAction(tr("&Reset"), color);

  fontAC = new QActionGroup(mw);
    numberfont = new QAction(tr("&Scale numbering"), fontAC);
    labelfont = new QAction(tr("&Axes label"), fontAC);
    titlefont = new QAction(tr("&Caption"), fontAC);
    resetfont = new QAction(tr("&Reset"), fontAC);
  ////////////////////////////////////////////////////////////////////////////
  // toolbars
  mainToolbar = qFindChild<QToolBar*>(mw, "toolBar");
  mainToolbar->setObjectName(QString::fromUtf8("mainToolbar"));
  mainToolbar->setMovable(true);
  //CBSelecao = new QComboBox;
  styleComboBox = new QComboBox;

  mainToolbar->addAction(Exit);
  mainToolbar->addSeparator();
  mainToolbar->addAction(ActionIni);
  mainToolbar->addAction(ActionParar);
  mainToolbar->addAction(ActionCarDados);
  mainToolbar->addAction(ActionSalvarDados);
  //mainToolbar->addWidget(CBSelecao);

  mainToolbar->addSeparator();
  LN = new QLabel(" N:=",mw);
  QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Fixed);
  sizePolicy3.setHeightForWidth(LN->sizePolicy().hasHeightForWidth());
  sizePolicy3.setHorizontalStretch(0);
  sizePolicy3.setVerticalStretch(0);
  QFont font;
  font.setFamily(QString::fromUtf8("Arial"));
  LN->setSizePolicy(sizePolicy3);
  mainToolbar->addWidget(LN);

            LEN = new QLineEdit("100",mw);
            LEN->setObjectName(QString::fromUtf8("LEN"));
            LEN->setSizePolicy(sizePolicy3);
            LEN->setMinimumSize(QSize(1, 10));
            LEN->setMaximumSize(QSize(30, 16777215));
            LEN->setMaxLength(50);
            LEN->setCursorPosition(2);

  mainToolbar->addWidget(LEN);
/*
            LPC = new QLabel(" Pc:=",mw);
            LPC->setSizePolicy(sizePolicy3);
  mainToolbar->addWidget(LPC);

            LEPC = new QLineEdit("0.5",mw);
            LEPC->setObjectName(QString::fromUtf8("LEPC"));
            LEPC->setSizePolicy(sizePolicy3);
            LEPC->setMaximumSize(QSize(30, 16777215));
            LEPC->setMaxLength(50);
            LEPC->setCursorPosition(4);
  mainToolbar->addWidget(LEPC);

            LPM = new QLabel(" Pm:=",mw);
            LPM->setSizePolicy(sizePolicy3);
 mainToolbar->addWidget(LPM);

            LEPM = new QLineEdit("0.01",mw);
            LEPM->setObjectName(QString::fromUtf8("LEPM"));
            LEPM->setSizePolicy(sizePolicy3);
            LEPM->setMaximumSize(QSize(40, 16777215));
            LEPM->setContextMenuPolicy(Qt::NoContextMenu);
            LEPM->setMaxLength(50);
            LEPM->setCursorPosition(5);

  mainToolbar->addWidget(LEPM);
*/
  mainToolbar->addSeparator();

            LEL = new QLabel(" E:=",mw);
            LEL->setSizePolicy(sizePolicy3);
  mainToolbar->addWidget(LEL);

            LEEL = new QLineEdit("5",mw);
            LEEL->setObjectName(QString::fromUtf8("LEEL"));
            LEEL->setSizePolicy(sizePolicy3);
            LEEL->setMaximumSize(QSize(20, 16777215));
            LEEL->setContextMenuPolicy(Qt::NoContextMenu);
  mainToolbar->addWidget(LEEL);

            LEM = new QLabel("    Erro Médio[máximo]:=  ",mw);
            LEM->setSizePolicy(sizePolicy3);
  mainToolbar->addWidget(LEM);

            LEEM = new QLineEdit("0.01",mw);
            LEEM->setObjectName(QString::fromUtf8("LEEM"));
            LEEM->setSizePolicy(sizePolicy3);
            LEEM->setMaximumSize(QSize(40, 16777215));
            LEEM->setContextMenuPolicy(Qt::NoContextMenu);
  mainToolbar->addWidget(LEEM);
  mainToolbar->addSeparator();
/*
            LDI = new QLabel(" Qtdade de entradas:=",mw);
            LDI->setSizePolicy(sizePolicy3);
  mainToolbar->addWidget(LDI);

            LEDI = new QLineEdit("0",mw);
            LEDI->setObjectName(QString::fromUtf8("LEDI"));
            LEDI->setSizePolicy(sizePolicy3);
            LEDI->setMinimumSize(QSize(0, 0));
            LEDI->setMaximumSize(QSize(20, 16777215));
            LEDI->setFont(font);
  mainToolbar->addWidget(LEDI);
  mainToolbar->addSeparator();

            LPI = new QLabel(" Ordem máxima:=",mw);
            LPI->setSizePolicy(sizePolicy3);
  mainToolbar->addWidget(LPI);
            LEPI = new QLineEdit("5",mw);
            LEPI->setObjectName(QString::fromUtf8("LEPI"));
            LEPI->setSizePolicy(sizePolicy3);
            LEPI->setMinimumSize(QSize(0, 0));
            LEPI->setMaximumSize(QSize(20, 16777215));
            LEPI->setFont(font);
  mainToolbar->addWidget(LEPI);
  */
  mainToolbar->addSeparator();
  mainToolbar->addWidget(styleComboBox);
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // menubar
  /*
  menubar = mw->menuBar();//new QMenuBar();
  ExecutMenu = menubar->addMenu("&Organizer");
  ExecutMenu->addAction(ActionIni);
  ExecutMenu->addAction(ActionParar);
  ExecutMenu->addAction(ActionCarDados);
  ExecutMenu->addAction(Exit);

  StylesMenu = menubar->addMenu("E&stilos");

  colormenu = menubar->addMenu(tr("&Color"));
  colormenu->addActions(color->actions());
  fontmenu = menubar->addMenu(tr("&Fonts"));
  fontmenu->addActions(fontAC->actions());
  */
  styleComboBox->addItem("NorwegianWood");
  styleComboBox->addItems(QStyleFactory::keys());
  styleComboBox->setCurrentIndex(6);
  mw->addToolBar(Qt::TopToolBarArea, mainToolbar);
  mw->insertToolBarBreak( mainToolbar);

  //CBSelecao->clear();
  //CBSelecao->addItem( tr( "Rank" ) );
  //CBSelecao->addItem( tr( "Roleta" ) );
  //CBSelecao->addItem( tr( "Torneio" ) );
  //CBSelecao->setToolTip(tr( "Metodo de Seleção" ) );
  QObject::connect(Exit, SIGNAL(triggered()), mw, SLOT(close()));
}
//////////////////////////////////////////////////////////////////////////////////
