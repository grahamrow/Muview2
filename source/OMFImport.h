// This part is originally created and released into the public
// domain by Gunnar Selke <gselke@physnet.uni-hamburg.de>.
// Modified by Graham Rowlands <grahamrow@gmail.com>

#ifndef OMF_IMPORT_H
#define OMF_IMPORT_H

#include <QSharedPointer>
#include <string>
#include <istream>
#include "matrix.h"
#include "OMFHeader.h"

// Now we return pointers to a custom matrix class.
// Arrays should be garbage collected when
// we dereference the pointers everywhere

QSharedPointer<matrix> readOMF(const std::string  &path, OMFHeader &header);
QSharedPointer<matrix> readOMF(      std::istream   &in, OMFHeader &header);

#endif

