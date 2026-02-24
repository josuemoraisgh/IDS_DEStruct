#include <QProgressBar>
#include <QtCore>
#include <QTimer>
#include <QRegularExpression>
#include <math.h>
#include <algorithm>
#include "icarregar.h"
#include "destruct.h"
#include "xtipodados.h"
////////////////////////////////////////////////////////////////////////////
////////////////////////////////Classe /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
ICarregar::ICarregar(QWidget* parent)
    : DICarregar(parent)
{
    setupUi(this);
    UL_IndexVar = 0;
    //////////////////////////////////////////////////////////////////////////////
    //Inicializa o dialog Max Min
    dialogMaxMin = new QDialog(this);
    //dialogDecimacao = new QDialog(this);
    (new Ui::DialogMaxMin())->setupUi(dialogMaxMin);
    //(new Ui::DialogDecimacao())->setupUi(dialogDecimacao);
    dmmComboBox     = dialogMaxMin->findChild<QComboBox *>( "comboBox"   );
    dmmLineEditMax  = dialogMaxMin->findChild<QLineEdit *>( "lineEditMax");
    dmmLineEditMin  = dialogMaxMin->findChild<QLineEdit *>( "lineEditMin");
    dmmLineEditDECI = dialogMaxMin->findChild<QLineEdit *>( "lineEditDelta");
    //////////////////////////////////////////////////////////////////////////////
    //dmmComboBox1    = qFindChild<QComboBox *>(dialogMaxMin, "comboBox"   );
    dmmTal10        = dialogMaxMin->findChild<QLabel *>( "labelTal10");
    dmmTal20        = dialogMaxMin->findChild<QLabel *>( "labelTal20");
    dmmgb           = dialogMaxMin->findChild<QGroupBox *>( "groupBox");
    //////////////////////////////////////////////////////////////////////////////
    //Conex�es
    connect( dmmComboBox,  SIGNAL( activated(int)), this, SLOT(slot_UL_ChangeCombo(int)));
    connect( dialogMaxMin, SIGNAL( finished(int )), this, SLOT(slot_UL_ChangeFim(int)));
    //////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
ICarregar::~ICarregar()
{

}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void ICarregar::slot_UL_ChangeCombo(int indexVar)
{
    if((indexVar< DEStruct::DES_Adj.Dados.variaveis.qtSaidas)&&(DEStruct::DES_Adj.decimacao.size()>indexVar))
        DEStruct::DES_Adj.decimacao[indexVar] = dmmLineEditDECI->text().toDouble();
    DEStruct::DES_Adj.Dados.variaveis.Vmaior.replace(UL_IndexVar,dmmLineEditMax->text().toDouble());
    DEStruct::DES_Adj.Dados.variaveis.Vmenor.replace(UL_IndexVar,dmmLineEditMin->text().toDouble());
    UL_IndexVar=indexVar;
    dmmLineEditMax->setText(QString::number(DEStruct::DES_Adj.Dados.variaveis.Vmaior.at(UL_IndexVar)));
    dmmLineEditMin->setText(QString::number(DEStruct::DES_Adj.Dados.variaveis.Vmenor.at(UL_IndexVar)));
    if((UL_IndexVar< DEStruct::DES_Adj.Dados.variaveis.qtSaidas)&&(DEStruct::DES_Adj.decimacao.size()>UL_IndexVar))
    {
        dmmgb->setEnabled(true);
        dmmLineEditDECI->setText(QString::number(DEStruct::DES_Adj.decimacao.at(UL_IndexVar)));
        dmmTal10->setText(" < "+QString::number(((qreal) DEStruct::DES_Adj.talDecim.at(UL_IndexVar))/10.));
        dmmTal20->setText(QString::number(((qreal) DEStruct::DES_Adj.talDecim.at(UL_IndexVar))/20.)+" < ");
    }
    else dmmgb->setDisabled(true);
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void ICarregar::slot_UL_Finalizado()
{
    if(this->isActiveWindow()) BCarregar->setEnabled(true);
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void ICarregar::slot_UL_Parado()
{
    if(this->isActiveWindow()) BCarregar->setEnabled(true);
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void ICarregar::slot_UL_ChangeFim(int)
{
    if((UL_IndexVar<DEStruct::DES_Adj.Dados.variaveis.qtSaidas)&&(DEStruct::DES_Adj.decimacao.size()>UL_IndexVar))
        DEStruct::DES_Adj.decimacao[UL_IndexVar] = dmmLineEditDECI->text().toDouble();
    DEStruct::DES_Adj.Dados.variaveis.Vmaior.replace(UL_IndexVar,dmmLineEditMax->text().toDouble());
    DEStruct::DES_Adj.Dados.variaveis.Vmenor.replace(UL_IndexVar,dmmLineEditMin->text().toDouble());
    UL_IndexVar=0;
    //Manda normalizar os dados
    emit signal_UL_Estado(5);//Manda o comando multi-thread para normalizar
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void ICarregar::slot_UL_ModelClicked()
{
    if(LVEntradas->selectionModel()->selectedIndexes().size() && LVSaida->selectionModel()->selectedIndexes().size())
    {
        BCarregar->setEnabled(true);
        BConcatenar->setEnabled(true);
    }
    else
    {
        BCarregar->setEnabled(false);
        BConcatenar->setEnabled(false);
    }
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void ICarregar::ativar(const bool &atv)
{
    qint32 i;
    BCancel->setEnabled(false);
    BConcatenar->setEnabled(false);
    if(BCarregar->text() == "Parar")
    {
        BCarregar->setText("Continuar");
        emit signal_UL_Estado(2);//Para a thread onde estiver
    }
    else
    {
        if(BCarregar->text() == "Continuar")
        {
           BCarregar->setText("Parar");
           emit signal_UL_Estado(4);//Continua de onde parou
        }
        else //Inicializa
        {
            UL_cVariaveis.clear();
            BCarregar->setText("Parar");
            BCancel->setText("Finalizar");
            DIC_pb->setRange(0,5);
            //Seleciona as pertuba��es que ser�o lidas.
            for(i=0;i < LVEntradas->selectionModel()->selectedIndexes().size();i++)
                UL_cVariaveis.append(LVEntradas->selectionModel()->selectedIndexes().at(i).row());
            //Como a sele��o pode ter sido feita fora da ordem ent�o � ordenado a lista.
            std::sort(UL_cVariaveis.begin(),UL_cVariaveis.end(),std::less<int>());           
            //Seleciona a variavel dependente (saida).
            DEStruct::DES_Adj.Dados.variaveis.qtSaidas = LVSaida->selectionModel()->selectedIndexes().size();
            for(i=0;i < LVSaida->selectionModel()->selectedIndexes().size();i++)
                UL_cVariaveis.insert(i,LVSaida->selectionModel()->selectedIndexes().at(i).row());
            DEStruct::DES_Adj.Dados.timeInicial = LETIni->text().toDouble()*pow(10.0,CBTIni->currentIndex()*(-3));
            DEStruct::DES_Adj.Dados.timeFinal = LETFim->text().toDouble()*pow(10.0,CBTFim->currentIndex()*(-3));
            //Inicializando selecionando o arquivo
            DEStruct::DES_cVariaveis = UL_cVariaveis;
            DEStruct::DES_isCarregar = atv;
            emit signal_UL_Estado(4);
        }
    }
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void ICarregar::slot_UL_Carregar()
{  
    ativar(true);
}
/////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void ICarregar::slot_UL_Concatenar()
{
    ativar(false);
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void ICarregar::slot_UL_Status(const quint16 &std)
{
    switch (std)
    {
        case 0:
            //Carrega o arquivo de banco de dados.
            //Captura os dados selecionandos e registrar os maiores e menores valores para cada variavel, os quais ser�o usados para a normaliza��o.
            LVStBar->setText(QObject::tr("Carregando arquivo de banco de dados..."));
            DIC_pb->setValue(1);
            LVStBar->repaint();
            break;

        case 1:
            //Cada thread roda por vez em ordem concatenando os dados
            LVStBar->setText(QObject::tr("Concatenando dados das threads do arquivo..."));
            DIC_pb->setValue(2);
            LVStBar->repaint();
            break;

        case 2:
            //As threads rodam para desnormalizar os dados antigos se for concatenar.
            LVStBar->setText(QObject::tr("Desnormalizar dados antigos..."));
            DIC_pb->setValue(3);
            LVStBar->repaint();
            break;

        case 3:
            //Fazendo a decima��o
            if(DEStruct::DES_Adj.decimacao.size())
            {
                DEStruct::DES_Adj.decimacao[0] = (DEStruct::DES_Adj.talDecim.at(0)/20) + ((DEStruct::DES_Adj.talDecim.at(0)%20)?1:0);
                dmmLineEditDECI->setText(QString::number(DEStruct::DES_Adj.decimacao.at(0)));
                dmmTal10->setText(" < "+QString::number(((qreal) DEStruct::DES_Adj.talDecim.at(0))/10.));
                dmmTal20->setText(QString::number(((qreal) DEStruct::DES_Adj.talDecim.at(0))/20.)+" < ");
                //dialogDecimacao->show();
                LVStBar->setText(QObject::tr("Decimacao..."));
            }
            //Normalizar os dados (0 � 1) e trunca-os para duas casas decimais (considerando erro de 1%).
            for(qint32 i=0;i<DEStruct::DES_Adj.Dados.variaveis.nome.size();i++)
            {
                dmmComboBox->addItem(DEStruct::DES_Adj.Dados.variaveis.nome.at(i));
                //dmmComboBox1->addItem(DEStruct::DES_Adj.Dados.variaveis.nome.at(i));
            }
            dmmLineEditMax->setText(QString::number(DEStruct::DES_Adj.Dados.variaveis.Vmaior.at(0)));
            dmmLineEditMin->setText(QString::number(DEStruct::DES_Adj.Dados.variaveis.Vmenor.at(0)));
            dialogMaxMin->show();
            LVStBar->setText(QObject::tr("Normalizando e truncando os Dados..."));
            DIC_pb->setValue(4);
            LVStBar->repaint();
            break;
        case 4:
            //Salvando o dados em arquivo
            if(CBSalvar->isChecked())
            {
                LVStBar->setText(QObject::tr("Salvando os dados em arquivo..."));
                DIC_pb->setValue(5);
                LVStBar->repaint();
                emit signal_UL_SalvarArquivo();
            }
            slot_UL_Status(5);
            break;

        case 5:
            //Fechando a tela
            dmmComboBox->clear();
            LVStBar->setText(QObject::tr("Fechando a tela..."));
            LVStBar->repaint();            
            BConcatenar->setEnabled(true);
            BCancel->setEnabled(true);
            BCarregar->setText("Carregar");
            BCancel->setText("Fechar");
            DIC_pb->setValue(0);
            break;
    }
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void ICarregar::slot_UL_Tam()
{
    this->LVStBar2->setText(QString("Linhas: %1, Colunas: %2").arg(DEStruct::DES_Adj.Dados.variaveis.valores.numLinhas()).arg(DEStruct::DES_Adj.Dados.variaveis.valores.numColunas()));
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void ICarregar::slot_UL_Finalizar()
{
    BCarregar->setText("Carregar");
    BCancel->setText("Fechar");
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void ICarregar::slot_UL_Caminho()
{
    //Existe um conex�o nesta line edit que so dela mudar seu valor a fun��o slot_UL_Indicar � chamada.
    LECaminho->setText(QFileDialog::getOpenFileName(this,tr("Carregar Dados"), QDir::currentPath() , tr("Arquivo de Dados (*.txt)")));
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void ICarregar::slot_UL_Indicar(const QString &fileName)
{
    quint64 tam;
    bool isok= false,confere = false;
    QString line;
    QStringList strList;
    if (!fileName.isEmpty())
    {
        emit signal_UL_FName(fileName);
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream stream(&file);
            line = stream.readLine();
            if (!line.isNull())
            {
                UL_CabecalhoList.clear();
                strList = line.split(QRegularExpression("(\\s+)"));
                if(strList.size()?strList.first().isEmpty():false) strList.removeFirst();
                UL_CabecalhoList.append(strList);
                UL_slm = new QStringListModel(UL_CabecalhoList);
                LVEntradas->setSelectionMode(QAbstractItemView::MultiSelection);
                LVSaida->setSelectionMode(QAbstractItemView::MultiSelection);
                LVEntradas->setModel(UL_slm);
                LVSaida->setModel(UL_slm);
                isok = true;
            }
            stream.setRealNumberNotation(QTextStream::ScientificNotation);
            line = stream.readLine();
            if (!line.isNull())
            {
                LETIni->setText(line.split(QString(QChar(32)),Qt::SkipEmptyParts).at(0));
                CBTIni->setCurrentIndex(0);
            }
            tam = file.size();
            do
            {
                tam-=50;
                confere=stream.seek(tam);
                stream.readLine();
                line = stream.readLine();
            }
            while(stream.atEnd()||line.size()<=7||!confere);
            if(!line.isNull()&&line.size()>7)
            {
                line.split(QString(QChar(32)),Qt::SkipEmptyParts).at(0).toFloat(&confere);
                if(confere) LETFim->setText(line.split(QString(QChar(32)),Qt::SkipEmptyParts).at(0));
            }
            do
            {
                line = stream.readLine();
                if(!line.isNull()&&line.size()>7)
                {
                    line.split(QString(QChar(32)),Qt::SkipEmptyParts).at(0).toFloat(&confere);
                    if(confere) LETFim->setText(line.split(QString(QChar(32)),Qt::SkipEmptyParts).at(0));
                }
            }
            while(!stream.atEnd());
            file.close();
            CBTFim->setCurrentIndex(0);
        }
    }
    if(!isok)
    {
        UL_slm = new QStringListModel();
        LVEntradas->setModel(UL_slm);
        LVSaida->setModel(UL_slm);
    }
}
