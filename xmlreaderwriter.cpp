#include <QtWidgets>
#include <xtipodados.h>
#include <QXmlStreamReader>
#include "xmlreaderwriter.h"


XmlReaderWriter::XmlReaderWriter(Config *Ajts)
{
    XWR_Ajts  = Ajts;
    xmlWriter.setAutoFormatting(true);
}

bool XmlReaderWriter::writeFile(QIODevice *device)
{
    qint32 i=0, j=0, k=0, l=0, m=0;
    QString str,strTermo,strCoefic,strExpoente;
    xmlWriter.setDevice(device);

    xmlWriter.writeStartDocument();
    xmlWriter.writeDTD("<!DOCTYPE xbel>");
    xmlWriter.writeStartElement("xbel");
    xmlWriter.writeAttribute("version", "1.0");

    if(XWR_Ajts!=NULL)
    {
        xmlWriter.writeStartElement("Ajustes");
        xmlWriter.writeAttribute("numeroCiclos",QString::number(XWR_Ajts->numeroCiclos));
        xmlWriter.writeAttribute("jnrr",QString::number(XWR_Ajts->jnrr));
        xmlWriter.writeAttribute("serr",QString::number(XWR_Ajts->serr));
        xmlWriter.writeAttribute("isExpoReal",QString::number(XWR_Ajts->isTipoExpo));
        xmlWriter.writeAttribute("isRacional",QString::number(XWR_Ajts->isRacional));

        xmlWriter.writeAttribute("isIniciaEnabled", QString::number(XWR_Ajts->isIniciaEnabled));
        xmlWriter.writeAttribute("salvarAutomati", QString::number(XWR_Ajts->salvarAutomati));        

        str.clear();
        for(i=0;i<XWR_Ajts->decimacao.size();i++) str += QString::number(XWR_Ajts->decimacao.at(i))+" ";
        xmlWriter.writeAttribute("decimacao",str);
        str.clear();
        for(i=0;i<XWR_Ajts->talDecim.size();i++) str += QString::number(XWR_Ajts->talDecim.at(i))+" ";
        xmlWriter.writeAttribute("talDecim",str);

        xmlWriter.writeAttribute("iteracoes", QString::number(XWR_Ajts->iteracoes));
        xmlWriter.writeAttribute("isPararContinuarEnabled", QString::number(XWR_Ajts->isPararContinuarEnabled));
        xmlWriter.writeAttribute("qdadeCrSalvos", QString::number(XWR_Ajts->qdadeCrSalvos));
        xmlWriter.writeAttribute("salvDadosCarre", QString::number(XWR_Ajts->salvDadosCarre));
        xmlWriter.writeAttribute("nomeArqConfTxT", XWR_Ajts->nomeArqConfTxT);
        xmlWriter.writeEndElement();

    //XWR_Ajts->Dados

        xmlWriter.writeStartElement("PSim");
        xmlWriter.writeAttribute("qtSaidas", QString::number(XWR_Ajts->Dados.variaveis.qtSaidas));
        str.clear();
        for(i=0;i<XWR_Ajts->Dados.variaveis.nome.size();i++) str +=  XWR_Ajts->Dados.variaveis.nome.at(i)+" ";
        xmlWriter.writeAttribute("nome",str);
        str.clear();
        for(i=0;i<XWR_Ajts->Dados.variaveis.Vmaior.size();i++) str +=  QString::number(XWR_Ajts->Dados.variaveis.Vmaior.at(i))+" ";
        xmlWriter.writeAttribute("Vmaior",str);
        str.clear();
        for(i=0;i<XWR_Ajts->Dados.variaveis.Vmenor.size();i++) str +=  QString::number(XWR_Ajts->Dados.variaveis.Vmenor.at(i))+" ";
        xmlWriter.writeAttribute("Vmenor",str);
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("valores");
        str.clear();
        for(i=0;i<XWR_Ajts->Dados.variaveis.valores.size();i++)
            str += QString::number(XWR_Ajts->Dados.variaveis.valores.at(i))+" ";
        xmlWriter.writeAttribute("valores",str);
        xmlWriter.writeAttribute("linha",QString::number(XWR_Ajts->Dados.variaveis.valores.numLinhas()));
        xmlWriter.writeAttribute("coluna",QString::number(XWR_Ajts->Dados.variaveis.valores.numColunas()));
        xmlWriter.writeAttribute("iElitismo", QString::number(XWR_Ajts->Dados.iElitismo));
        xmlWriter.writeAttribute("qtdadeVarAnte", QString::number(XWR_Ajts->Dados.qtdadeVarAnte));
        //xmlWriter.writeAttribute("timeAmost", QString::number(XWR_Ajts->Dados.timeAmost));
        xmlWriter.writeAttribute("timeInicial", QString::number(XWR_Ajts->Dados.timeInicial));
        xmlWriter.writeAttribute("timeFinal", QString::number(XWR_Ajts->Dados.timeFinal));
        xmlWriter.writeAttribute("isElitismo", QString::number(XWR_Ajts->Dados.isElitismo));
        xmlWriter.writeEndElement();

    //XWR_Ajts->Pop
        for(j=0;j<XWR_Ajts->Dados.variaveis.qtSaidas;j++)
        {
            xmlWriter.writeStartElement("Saida"+QString::number(j));
            for(k=0;k<XWR_Ajts->Pop.at(j).size();k++)
            {
                xmlWriter.writeStartElement("Cromossomo"+QString::number(k));
                xmlWriter.writeAttribute("aptidao", QString::number(XWR_Ajts->Pop.at(j).at(k).aptidao));
                xmlWriter.writeAttribute("erro", QString::number(XWR_Ajts->Pop.at(j).at(k).erro));
                xmlWriter.writeAttribute("idSaida", QString::number(XWR_Ajts->Pop.at(j).at(k).idSaida));
                xmlWriter.writeAttribute("maiorAtraso", QString::number(XWR_Ajts->Pop.at(j).at(k).maiorAtraso));
                strTermo.clear();strCoefic.clear();strExpoente.clear();
                for(l=0;l<XWR_Ajts->Pop.at(j).at(k).regress.size();l++)
                {
                    xmlWriter.writeStartElement("Regress"+QString::number(l));
                    xmlWriter.writeAttribute("Err", QString::number(XWR_Ajts->Pop.at(j).at(k).err.at(l)));
                    xmlWriter.writeAttribute("Coefic", QString::number(XWR_Ajts->Pop.at(j).at(k).vlrsCoefic.at(l)));
                    for(m=0;m<XWR_Ajts->Pop.at(j).at(k).regress.at(l).size();m++)
                    {
                        xmlWriter.writeStartElement("Termo"+QString::number(m));
                        strTermo =  QString::number(XWR_Ajts->Pop.at(j).at(k).regress.at(l).at(m).vTermo.tTermo1.var);
                        xmlWriter.writeAttribute("var",strTermo);
                        strTermo =  QString::number(XWR_Ajts->Pop.at(j).at(k).regress.at(l).at(m).vTermo.tTermo1.atraso);
                        xmlWriter.writeAttribute("atraso",strTermo);
                        strTermo =  QString::number(XWR_Ajts->Pop.at(j).at(k).regress.at(l).at(m).vTermo.tTermo1.reg);
                        xmlWriter.writeAttribute("reg",strTermo);
                        strTermo =  QString::number(XWR_Ajts->Pop.at(j).at(k).regress.at(l).at(m).vTermo.tTermo1.nd);
                        xmlWriter.writeAttribute("nd",strTermo);
                        strTermo =  QString::number(XWR_Ajts->Pop.at(j).at(k).regress.at(l).at(m).expoente);
                        xmlWriter.writeAttribute("expoente",strTermo);
                        strTermo =  QString::number(XWR_Ajts->Pop.at(j).at(k).regress.at(l).at(m).basisType);
                        xmlWriter.writeAttribute("basisType",strTermo);
                        xmlWriter.writeEndElement();
                    }
                    xmlWriter.writeEndElement();                    
                }
                xmlWriter.writeEndElement();
            }
            xmlWriter.writeEndElement();
        }
    }
    xmlWriter.writeEndDocument();
    return true;
}

bool XmlReaderWriter::readFile(QIODevice *device)
{
    QString aux1,aux2;
    int termo=0,regress=0,cromo=0,saidas=0;

    xmlRead.setDevice(device);

    if (xmlRead.readNextStartElement()) {
        if (xmlRead.name() == "xbel" && xmlRead.attributes().value("version") == "1.0");
        else
            xmlRead.raiseError(QObject::tr("The file is not an XBEL version 1.0 file."));
    }

    while (xmlRead.readNextStartElement())
    {
        if (xmlRead.name() == "Ajustes")
        {
            XWR_Ajts->numeroCiclos = xmlRead.attributes().value("numeroCiclos").toString().toInt();
            XWR_Ajts->jnrr = xmlRead.attributes().value("jnrr").toString().toDouble();
            XWR_Ajts->serr = xmlRead.attributes().value("serr").toString().toDouble();
            XWR_Ajts->isTipoExpo = xmlRead.attributes().value("isExpoReal").toString().toInt();
            XWR_Ajts->isRacional = xmlRead.attributes().value("isRacional").toString().toInt();
            XWR_Ajts->isIniciaEnabled = xmlRead.attributes().value("isIniciaEnabled").toString().toInt();
            XWR_Ajts->salvarAutomati = xmlRead.attributes().value("salvarAutomati").toString().toInt();
            aux1 = xmlRead.attributes().value("decimacao").toString();
            for(int i=0; i<aux1.length(); i++)
            {
                if(aux1.at(i) != ' ') aux2.append(aux1.at(i));
                else
                {
                    XWR_Ajts->decimacao.push_back(aux2.toInt());
                    aux2.clear();
                }
            }
            aux1 = xmlRead.attributes().value("talDecim").toString();
            for(int i=0; i<aux1.length(); i++)
            {
                if(aux1.at(i) != ' ') aux2.append(aux1.at(i));
                else
                {
                    XWR_Ajts->talDecim.push_back(aux2.toInt());
                    aux2.clear();
                }
            }
            XWR_Ajts->iteracoes = xmlRead.attributes().value("iteracoes").toString().toDouble();
            XWR_Ajts->isPararContinuarEnabled = xmlRead.attributes().value("isPararContinuarEnabled").toString().toDouble();
            XWR_Ajts->qdadeCrSalvos = xmlRead.attributes().value("qdadeCrSalvos").toString().toDouble();
            XWR_Ajts->salvDadosCarre = xmlRead.attributes().value("salvDadosCarre").toString().toDouble();
            XWR_Ajts->nomeArqConfTxT = xmlRead.attributes().value("nomeArqConfTxT").toString();
        }
    }

    while (xmlRead.readNextStartElement())
    {
        if (xmlRead.name() == "PSim")
        {
            XWR_Ajts->Dados.variaveis.qtSaidas = xmlRead.attributes().value("qtSaidas").toString().toDouble();
            aux1 = xmlRead.attributes().value("nome").toString();
            for(int i=0; i<aux1.length(); i++)
            {
                if(aux1.at(i) != ' '){
                    aux2.append(aux1.at(i));
                }
                else{
                    XWR_Ajts->Dados.variaveis.nome.push_back(aux2);
                    aux2.clear();
                }
            }

            aux1 = xmlRead.attributes().value("Vmaior").toString();
            for(int i=0; i<aux1.length(); i++) {
                if(aux1.at(i) != ' '){
                    aux2.append(aux1.at(i));
                }
                else{
                    XWR_Ajts->Dados.variaveis.Vmaior.push_back(aux2.toDouble());
                    aux2.clear();
                }
            }

            aux1 = xmlRead.attributes().value("Vmenor").toString();
            for(int i=0; i<aux1.length(); i++) {
                if(aux1.at(i) != ' '){
                    aux2.append(aux1.at(i));
                }
                else{
                    XWR_Ajts->Dados.variaveis.Vmenor.push_back(aux2.toDouble());
                    aux2.clear();
                }
            }
        }
        else
            xmlRead.skipCurrentElement();
    }
    XWR_Ajts->Dados.variaveis.valores.clear();
    XWR_Ajts->Dados.variaveis.valores.remove('C',0);
    while (xmlRead.readNextStartElement())
    {
        if (xmlRead.name() == "valores")
        {
            aux1 = xmlRead.attributes().value("valores").toString();
            for(int i=0; i<aux1.length(); i++) {
                if(aux1.at(i) != ' '){
                    aux2.append(aux1.at(i));
                }
                else{
                    XWR_Ajts->Dados.variaveis.valores.push_back(aux2.toDouble());
                    aux2.clear();
                }
            }
            XWR_Ajts->Dados.variaveis.valores.setNumLinhas( xmlRead.attributes().value("linha").toString().toInt());
            XWR_Ajts->Dados.variaveis.valores.setNumColunas( xmlRead.attributes().value("coluna").toString().toInt());
            XWR_Ajts->Dados.iElitismo = xmlRead.attributes().value("iElitismo").toString().toInt();
            XWR_Ajts->Dados.qtdadeVarAnte = xmlRead.attributes().value("qtdadeVarAnte").toString().toInt();
            XWR_Ajts->Dados.timeInicial = xmlRead.attributes().value("timeInicial").toString().toDouble();
            XWR_Ajts->Dados.timeFinal = xmlRead.attributes().value("timeFinal").toString().toDouble();
            XWR_Ajts->Dados.isElitismo = xmlRead.attributes().value("isElitismo").toString().toInt();
        }
        else
            xmlRead.skipCurrentElement();
    }
    saidas=-1;
    cromo=-1;
    regress=-1;
    termo=-1;
    aux1 = "Inicio";
    while(aux1.size())
    {
        xmlRead.readNextStartElement();
        aux1 = xmlRead.name().toString();
        if(aux1==("Termo"+QString::number(termo+1)))
        {
            termo++;
            XWR_Ajts->Pop[saidas][cromo].regress[regress].append(compTermo());
            XWR_Ajts->Pop[saidas][cromo].regress[regress][termo].vTermo.tTermo1.var = xmlRead.attributes().value("var").toString().toInt();
            XWR_Ajts->Pop[saidas][cromo].regress[regress][termo].vTermo.tTermo1.atraso = xmlRead.attributes().value("atraso").toString().toInt();
            XWR_Ajts->Pop[saidas][cromo].regress[regress][termo].vTermo.tTermo1.reg = xmlRead.attributes().value("reg").toString().toInt();
            XWR_Ajts->Pop[saidas][cromo].regress[regress][termo].vTermo.tTermo1.nd = xmlRead.attributes().value("nd").toString().toInt();
            XWR_Ajts->Pop[saidas][cromo].regress[regress][termo].expoente = xmlRead.attributes().value("expoente").toString().toDouble();
            // Le basisType se existir (compatibilidade com arquivos antigos que nao tem)
            if(xmlRead.attributes().hasAttribute("basisType"))
                XWR_Ajts->Pop[saidas][cromo].regress[regress][termo].basisType = xmlRead.attributes().value("basisType").toString().toInt();
            else
                XWR_Ajts->Pop[saidas][cromo].regress[regress][termo].basisType = BASIS_POW;
        }
        else if(aux1 == ("Regress"+QString::number(regress+1)))
        {
            termo=-1;
            regress++;
            XWR_Ajts->Pop[saidas][cromo].err.append('C',xmlRead.attributes().value("Err").toString().toDouble());
            XWR_Ajts->Pop[saidas][cromo].vlrsCoefic.append('C',xmlRead.attributes().value("Coefic").toString().toDouble());
            XWR_Ajts->Pop[saidas][cromo].regress.append(QVector<compTermo>());
        }
        else if(aux1 == ("Cromossomo"+QString::number(cromo+1)))
        {
            termo=-1;
            regress=-1;
            cromo++;
            XWR_Ajts->Pop[saidas].append(Cromossomo());
            XWR_Ajts->Pop[saidas][cromo].aptidao = xmlRead.attributes().value("aptidao").toString().toDouble();
            XWR_Ajts->Pop[saidas][cromo].erro = xmlRead.attributes().value("erro").toString().toDouble();
            XWR_Ajts->Pop[saidas][cromo].idSaida = xmlRead.attributes().value("idSaida").toString().toInt();
            XWR_Ajts->Pop[saidas][cromo].maiorAtraso = xmlRead.attributes().value("maiorAtraso").toString().toInt();
            XWR_Ajts->Pop[saidas][cromo].err.remove('C',0);
            XWR_Ajts->Pop[saidas][cromo].vlrsCoefic.remove('C',0);
        }
        else if(aux1 == ("Saida"+QString::number(saidas+1)))
        {
            termo=-1;
            regress=-1;
            cromo=-1;
            saidas++;
            XWR_Ajts->Pop.append(QVector<Cromossomo >());
        }
    }
    return(true);
    //return(!xmlRead.error());
}

QString XmlReaderWriter::errorString() const
{
    return QObject::tr("%1\nLine %2, column %3")
            .arg(xmlRead.errorString())
            .arg(xmlRead.lineNumber())
            .arg(xmlRead.columnNumber());
}
