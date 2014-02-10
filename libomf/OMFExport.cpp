// This part is originally created and released into the public
// domain by Gunnar Selke <gselke@physnet.uni-hamburg.de>.

#include "OMFExport.h"

#include "matrix/Vector3d.h"
#include "matrix/VectorMatrix.h"

#include "endian.h"

#include <string.h>
#include <stdio.h>
#include <cfloat>
#include <fstream>
#include <stdexcept>

#include "OMFHeader.h"

#include <iostream>
using namespace std;

static void writeAsciiValues(std::ostream &out, const VectorMatrix &field);
static void writeBinary4Values(std::ostream &out, const VectorMatrix &field);
static void writeBinary8Values(std::ostream &out, const VectorMatrix &field);
static void getMinMaxValueRange(const VectorMatrix &field, double &min, double &max);

static std::string hdr(std::string key, std::string val)
{
	return std::string("# ") + key + ": " + val;
}

static std::string hdr(std::string key, int val)
{
	char tmp[1024];
	sprintf(tmp, "%i", val);	
	return std::string("# ") + key + ": " + std::string(tmp);
}

static std::string hdr(std::string key, double val)
{
	char tmp[1024];
	sprintf(tmp, "%g", val);	
	return std::string("# ") + key + ": " + std::string(tmp);
}

void writeOMF(const std::string &path, OMFHeader &header, const VectorMatrix &field, OMFFormat format)
{
	std::ofstream out(path.c_str());
	if (!out.good()) {
		throw std::runtime_error(std::string("writeOMF: Could not open ") + path + " for omf file writing");
	}
	writeOMF(out, header, field, format);
}

void writeOMF(std::ostream &out, OMFHeader &header, const VectorMatrix &field, OMFFormat format)
{
	double minLen, maxLen;
	getMinMaxValueRange(field, minLen, maxLen);

	out << hdr("OOMMF", "rectangular mesh v1.0") << endl;
	out << hdr("Segment count", 1) << endl;
	out << hdr("Begin", "Segment") << endl;
	out << hdr("Begin", "Header") << endl;
	out << hdr("Title", header.Title) << endl;
	for (size_t i=0; i<header.Desc.size(); ++i) out << hdr("Desc", header.Desc[i]) << endl;
	out << hdr("meshunit", header.meshunit) << endl;
	out << hdr("valueunit", header.valueunit) << endl;
	out << hdr("valuemultiplier", header.valuemultiplier) << endl;
	out << hdr("xmin", header.xmin) << endl;
	out << hdr("ymin", header.ymin) << endl;
	out << hdr("zmin", header.zmin) << endl;
	out << hdr("xmax", header.xmax) << endl;
	out << hdr("ymax", header.ymax) << endl;
	out << hdr("zmax", header.zmax) << endl;
	out << hdr("ValueRangeMaxMag", /*header.ValueRangeMaxMag*/ maxLen) << endl;
	out << hdr("ValueRangeMinMag", /*header.ValueRangeMinMag*/ minLen) << endl;
	out << hdr("meshtype", header.meshtype) << endl;
	out << hdr("xbase", header.xbase) << endl;
	out << hdr("ybase", header.ybase) << endl;
	out << hdr("zbase", header.zbase) << endl;
	out << hdr("xstepsize", header.xstepsize) << endl;
	out << hdr("ystepsize", header.ystepsize) << endl;
	out << hdr("zstepsize", header.zstepsize) << endl;
	out << hdr("xnodes", header.xnodes) << endl;
	out << hdr("ynodes", header.ynodes) << endl;
	out << hdr("znodes", header.znodes) << endl;
	out << hdr("End", "Header") << endl;

	switch (format) {
		case OMF_FORMAT_ASCII:
			out << hdr("Begin", "Data Text") << endl;
			writeAsciiValues(out, field);
			out << hdr("End", "Data Text") << endl;
			break;

		case OMF_FORMAT_BINARY_4:
			out << hdr("Begin", "Data Binary 4") << endl;
			writeBinary4Values(out, field);
			out << hdr("End", "Data Binary 4") << endl;
			break;

		case OMF_FORMAT_BINARY_8:
			out << hdr("Begin", "Data Binary 8") << endl;
			writeBinary8Values(out, field);
			out << hdr("End", "Data Binary 8") << endl;
			break;
	}

	out << hdr("End", "Segment") << endl;
}

static void writeAsciiValues(std::ostream &out, const VectorMatrix &field)
{
	out.unsetf(ios_base::scientific);
	out.unsetf(ios_base::fixed);
	out.precision(16); // print the significant 52 digital digits (which is ~16 decimal digits)

	VectorMatrix::const_accessor field_acc(field);
	for (size_t i=0; i<field.numElements(); ++i) {
		const Vector3d vec = vector_get(field_acc, i);
		out << vec.x << " " << vec.y << " " << vec.z << '\n'; // <-- no std::endl because endl implies flushing (= slowdown)
	}
}

static void writeBinary4Values(std::ostream &out, const VectorMatrix &field)
{
	assert(sizeof(float) == 4);

	VectorMatrix::const_accessor field_acc(field);

	const int num_cells = field.numElements();
	float *buffer = new float [3*num_cells];
	for (int i=0; i<num_cells; ++i) {
		for (int j=0; j<3; ++j)
			buffer[i*3+j] = toBigEndian(static_cast<float>(field_acc.linearGet(i,j)));
	}

	const float magic = toBigEndian<float>(1234567.0f);
	out.write((const char*)&magic, sizeof(float));
	out.write((const char*)buffer, num_cells * 3 * sizeof(float));
	out << endl;

	delete [] buffer;
}

static void writeBinary8Values(std::ostream &out, const VectorMatrix &field)
{
	assert(sizeof(double) == 8);

	VectorMatrix::const_accessor field_acc(field);

	const int num_cells = field.numElements();
	double *buffer = new double [3*num_cells];
	for (int i=0; i<num_cells; ++i) {
		for (int j=0; j<3; ++j)
			buffer[i*3+j] = toBigEndian(field_acc.linearGet(i,j));
	}

	const double magic = toBigEndian<double>(123456789012345.0);
	out.write((const char*)&magic, sizeof(double));
	out.write((const char*)buffer, num_cells * 3 * sizeof(double));
	out << endl;

	delete [] buffer;
}

static void getMinMaxValueRange(const VectorMatrix &field, double &min, double &max)
{
	max = DBL_MIN;
	min = DBL_MAX;

	VectorMatrix::const_accessor field_acc(field);

	const size_t total_nodes = field.numElements();
	for (size_t i=0; i<total_nodes; ++i) {
		const double len = vector_get(field_acc, i).abs();
		if (len < min) min = len;
		if (len > max) max = len;
	}
}

