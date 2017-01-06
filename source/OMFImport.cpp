// This part is originally created and released into the public
// domain by Gunnar Selke <gselke@physnet.uni-hamburg.de>.
// Converted to Qt libs and custom QVector based matrix class
// by Graham Rowlands <grahamrow@gmail.com>

#include <QSharedPointer>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QDataStream>
#include <QDebug>
#include <stdlib.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>

#include "matrix.h"
#include "OMFImport.h"
#include "OMFEndian.h"

QSharedPointer<OMFReader> readOMF(QString &path)
{
    bool success;
    QFile file(path);
    OMFReader *reader = new OMFReader(file);
    success = reader->read();
    if (!success) {
        return QSharedPointer<OMFReader>();
    } else {
        return QSharedPointer<OMFReader>(reader);
    }
    return QSharedPointer<OMFReader>();
}

OMFReader::OMFReader(QFile &fileref) :
    file(fileref),
    Title("<title>"),
    meshunit("<meshunit>"),
    valueunit("<valueunit>"),
    valuemultiplier(0.0),
    xmin(0.0), ymin(0.0), zmin(0.0),
    xmax(0.0), ymax(0.0), zmax(0.0),
    ValueRangeMaxMag(0.0), ValueRangeMinMag(0.0),
    meshtype("rectangular"),
    xbase(0.0), ybase(0.0), zbase(0.0),
    xstepsize(0.0), ystepsize(0.0), zstepsize(0.0),
    xnodes(0), ynodes(0), znodes(0)
{

}

bool OMFReader::read()
{
    bool ok;
    QString key, value;

    if (!file.open(QIODevice::ReadOnly))
    {
        return QSharedPointer<matrix>();
    }
    // Read first line
    acceptLine();
    ok = parseFirstLine(key, value, version);
    if (ok && key == "oommf") {
        acceptLine();
    } else {
        qDebug() << "Expected valid 'OOMMF' header at line 1";
        return false;
    }

    ok = parseCommentLine(key, value);
    if (ok && key == "segment count") {
        acceptLine();
    } else {
        qDebug() << "Expected 'Segment count' at line 2";
        return false;
    }

    ok = parseCommentLine(key, value);
    if (ok && key == "begin" && value == "segment") {
        parseSegment();
    } else {
        qDebug() << "Expected begin of segment";
        return false;
    }

    return true;
}

bool OMFReader::parseFirstLine(QString &key, QString &value, int &version)
{
    if (line=="# OOMMF: OVF 2.0") {
        key="oommf";
        value="ovf 2.0";
        version=2;
        return true;
    } else if (line=="# OOMMF: rectangular mesh v1.0") {
        key="oommf";
        value="rectangular mesh v1.0";
        version=1;
        return true;
    } else if (line=="# OOMMF OVF 2.0") {
        key="oommf";
        value="ovf 2.0";
        version=2;
        return true;
    } else {
        return false;
    }
    return false;
}

bool OMFReader::parseCommentLine(QString &key, QString &value)
{
    if (line.at(0) == '#') {
        int sep = line.indexOf(':');
        key   = line.mid(2,sep-2).toLower().simplified();
        value = line.mid(sep+1).toLower().simplified();
        return true;
    } else {
        return false;
    }
}

void OMFReader::acceptLine()
{
    bool reallydone = false;
    while(!reallydone && !file.atEnd())
    {
        line = file.readLine();
        if( line.endsWith("\n") ) line.truncate( line.length() - 1 );
        if (line=="#") {
            reallydone=false;
        } else if (line=="") {
            reallydone=true;
        } else if (line.isNull()) {
            reallydone=true;
        } else {
            reallydone=true;
        }
    }
}

bool OMFReader::parseSegment()
{
    bool ok;
    QString key, value;

    ok = parseCommentLine(key, value);
    if (!ok || key != "begin" || value != "segment") {
        qDebug() << "Parse error. Expected 'Begin Segment'";
        return false;
    }
    acceptLine();

    parseHeader();
    ok = parseCommentLine(key, value);
    if (!ok || key != "begin") {
        qDebug() << "Parse error. Expected 'Begin Data <type>'";
        return false;
    }
    if (value == "data text") {
        ok = parseDataAscii();
    } else if (value == "data binary 4") {
        ok = parseDataBinary4();
    } else if (value == "data binary 8") {
        ok = parseDataBinary8();
    } else {
        qDebug() << "Expected either 'Text', 'Binary 4' or 'Binary 8' chunk type";
        return false;
    }

    if (!ok) {
        qDebug() << "Parsing failed. May load anyway!";
        return false;
    }

    return true;
}

bool OMFReader::parseHeader()
{
    bool ok;
    QString key, value;

    ok = parseCommentLine(key, value);
    if (!ok || key != "begin" || value != "header") {
        qDebug() << "Expected 'Begin Header'";
    }
    acceptLine();

    bool done = false;
    while (!done) {
        ok = parseCommentLine(key, value);
        if (!ok) {
            qDebug() << "Skipped erroneous line in header.";
            continue;
        }

        if (key == "end" && value == "header") {
            done = true;
            break;
        } else if (key == "title") {
            Title = value;
        } else if (key == "desc") {
            Desc.push_back(value);
        } else if (key == "meshunit") {
            meshunit = value;
        } else if (key == "valueunit") {
            valueunit = value;
        } else if (key == "valuemultiplier") {
            valuemultiplier = value.toDouble();
        } else if (key == "xmin") {
            xmin = value.toDouble();
        } else if (key == "ymin") {
            ymin = value.toDouble();
        } else if (key == "zmin") {
            zmin = value.toDouble();
        } else if (key == "xmax") {
            xmax = value.toDouble();
        } else if (key == "ymax") {
            ymax = value.toDouble();
        } else if (key == "zmax") {
            zmax = value.toDouble();
        } else if (key == "valuedim") {   // OVF 2.0
            valuedim = value.toInt();
        } else if (key == "valueunits") { // OVF 2.0
            valueunits.push_back(value);
        } else if (key == "valuelabels") { // OVF 2.0
            valuelabels.push_back(value);
        } else if (key == "valuerangeminmag") {
            ValueRangeMinMag = value.toDouble();
        } else if (key == "valuerangemaxmag") {
            ValueRangeMaxMag = value.toDouble();
        } else if (key == "meshtype") {
            meshtype = value;
        } else if (key == "xbase") {
            xbase = value.toDouble();
        } else if (key == "ybase") {
            ybase = value.toDouble();
        } else if (key == "zbase") {
            zbase = value.toDouble();
        } else if (key == "xstepsize") {
            xstepsize = value.toDouble();
        } else if (key == "ystepsize") {
            ystepsize = value.toDouble();
        } else if (key == "zstepsize") {
            zstepsize = value.toDouble();
        } else if (key == "xnodes") {
            xnodes = value.toInt();
        } else if (key == "ynodes") {
            ynodes = value.toInt();
        } else if (key == "znodes") {
            znodes = value.toInt();
        } else {
            qDebug() << "OMFReader::parseHeader: Unknown key: " << key << "---" << value;
        }
        acceptLine();
    }

    if (version == 1) {
        valuedim = 3;
    }

    ok = parseCommentLine( key, value);
    if (!ok || key != "end" || value != "header") {
        qDebug() << "Expected 'End Header'";
        return false;
    }
    acceptLine();
    return true;
}

bool OMFReader::parseDataAscii()
{
    bool ok;
    QString key, value;

    ok = parseCommentLine(key, value);
    if (!ok || key != "begin" || value != "data text") {
        qDebug() << "Expected 'Begin DataText'";
        return false;
    }
    acceptLine();

    // Create field matrix object
    field = QSharedPointer<matrix>(new matrix(xnodes, ynodes, znodes));

    for (int z=0; z<znodes; ++z)
        for (int y=0; y<ynodes; ++y)
            for (int x=0; x<xnodes; ++x) {
                QTextStream ss(&line);

                double v1, v2, v3;
                QVector3D val;
                if (valuedim == 1) {
                    ss >> v1;
                    val = QVector3D(v1,v1,v1);
                } else {
                    ss >> v1 >> v2 >> v3;
                    val = QVector3D(v1,v2,v3);
                }

                if (version==1) {
                    val = val*valuemultiplier;
                }

                field->set(x,y,z,val);

                acceptLine();
            }

    return true;
}

bool OMFReader::parseDataBinary4()
{
    Q_ASSERT(sizeof(float) == 4);

    bool ok;
    QString key, value;

    // Parse "Begin: Data Binary 4"
    ok = parseCommentLine(key, value);
    if (!ok || key != "begin" || value != "data binary 4") {
        qDebug() << "Expected 'Begin Binary 4'";
        return false;
    }

    // Create field matrix object
    field = QSharedPointer<matrix>(new matrix(xnodes, ynodes, znodes));
    const int num_cells = field->num_elements();

    // Read magic value and field contents from file
    double magic;
    QByteArray magicArray = file.read(sizeof(float));
    QDataStream magicStream(magicArray);
    magicStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    if (version==1) {
        magicStream.setByteOrder(QDataStream::BigEndian);
        magicStream >> magic;
    } else if (version==2) {
        magicStream.setByteOrder(QDataStream::LittleEndian);
        magicStream >> magic;
    } else {
        qDebug() << "Wrong version number detected.";
        return false;
    }

    if (magic != 1234567.0) qDebug() << "Wrong magic number (binary 4 format)";

    QByteArray dataArray;
    if (valuedim == 1) {
        dataArray = file.read(num_cells*sizeof(float));
    } else {
        dataArray = file.read(3*num_cells*sizeof(float));
    }
    QDataStream dataStream(dataArray);
    dataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    if (version==1) {
        dataStream.setByteOrder(QDataStream::BigEndian);
    } else {
        dataStream.setByteOrder(QDataStream::LittleEndian);
    }

    QVector3D val;
    double v1, v2, v3;
    for (int i=0; i<num_cells; ++i) {
        if (valuedim == 1) {
            dataStream >> v1;
            if (version==1) {
                v1 = v1 * valuemultiplier;
            }
            val = QVector3D(v1,v1,v1);
        } else {
            dataStream >> v1 >> v2 >> v3;
            val = QVector3D(v1, v2, v3);
            if (version==1) {
                val = val * valuemultiplier;
            }
        }
        field->set(i,val);
    }
    return true;
}

bool OMFReader::parseDataBinary8()
{
    Q_ASSERT(sizeof(double) == 8);

    bool ok;
    QString key, value;

    // Parse "Begin: Data Binary 8"
    ok = parseCommentLine(key, value);
    if (!ok || key != "begin" || value != "data binary 8") {
        qDebug() << "Expected 'Begin Binary 8'";
        return false;
    }

    // Create field matrix object
    field = QSharedPointer<matrix>(new matrix(xnodes, ynodes, znodes));
    const int num_cells = field->num_elements();

    // Read magic value and field contents from file
    double magic;
    QByteArray magicArray = file.read(sizeof(double));
    QDataStream magicStream(magicArray);
    magicStream.setFloatingPointPrecision(QDataStream::DoublePrecision);
    if (version==1) {
        magicStream.setByteOrder(QDataStream::BigEndian);
        magicStream >> magic;
    } else {
        magicStream.setByteOrder(QDataStream::LittleEndian);
        magicStream >> magic;
    }

    if (magic != 123456789012345.0) qDebug() << "Wrong magic number (binary 8 format)";

    QByteArray dataArray;
    if (valuedim == 1) {
        dataArray = file.read(num_cells*sizeof(double));
    } else {
        dataArray = file.read(3*num_cells*sizeof(double));
    }
    QDataStream dataStream(dataArray);
    dataStream.setFloatingPointPrecision(QDataStream::DoublePrecision);
    if (version==1) {
        dataStream.setByteOrder(QDataStream::BigEndian);
    } else {
        dataStream.setByteOrder(QDataStream::LittleEndian);
    }

    QVector3D val;
    double v1, v2, v3;
    for (int i=0; i<num_cells; ++i) {
        if (valuedim == 1) {
            dataStream >> v1;
            if (version==1) {
                v1 = v1 * valuemultiplier;
            }
            val = QVector3D(v1,v1,v1);
        } else {
            dataStream >> v1 >> v2 >> v3;
            val = QVector3D(v1, v2, v3);
            if (version==1) {
                val = val * valuemultiplier;
            }
        }
        field->set(i,val);
    }
    return true;
}

