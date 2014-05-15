// This part is originally created and released into the public
// domain by Gunnar Selke <gselke@physnet.uni-hamburg.de>.
// Modified by Graham Rowlands <grahamrow@gmail.com>

#ifndef OMF_IMPORT_H
#define OMF_IMPORT_H

#include <QFile>
#include <QSharedPointer>
#include <QString>
#include <QTextStream>
//#include <string>
//#include <istream>
#include "matrix.h"
#include "OMFHeader.h"

// Now we return pointers to a custom matrix class.
// Arrays should be garbage collected when
// we dereference the pointers everywhere

class OMFReader
{
public:
    OMFReader(QFile &fileref);
    bool read();
    OMFHeader getHeader();
    QSharedPointer<matrix> getField();
private:
    bool parse();
    bool parseFirstLine(QString &key, QString &value, int &version);
    bool parseSegment();
    bool parseHeader();
    bool parseCommentLine(QString &key, QString &value);
    bool parseDataAscii();
    bool parseDataBinary4();
    bool parseDataBinary8();
    void acceptLine();

    OMFHeader header;
    QSharedPointer<matrix> field;
    QTextStream in;
    QString line;
    QString filename;
    QFile &file;
    int lineno;
};

//QSharedPointer<matrix> readOMF(const std::string  &path, OMFHeader &header);
//QSharedPointer<matrix> readOMF(      std::istream   &in, OMFHeader &header);
QSharedPointer<matrix> readOMF(QString &path, OMFHeader &header);
//QSharedPointer<matrix> readOMF(QTextStream &in, OMFHeader &header);

#endif

