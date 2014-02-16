// This part is originally created and released into the public
// domain by Gunnar Selke <gselke@physnet.uni-hamburg.de>.
// Modified by Graham Rowlands <grahamrow@gmail.com>

#include <QSharedPointer>
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

struct OMFImport
{
  void read(std::istream &input);

  void parse();
  void parseSegment();
  void parseHeader();
  void parseDataAscii();
  void parseDataBinary4();
  void parseDataBinary8();

  OMFHeader header;
  std::istream *input;
  int lineno;
  std::string line;
  bool eof;
  char next_char;

  QSharedPointer<matrix> field;

  void acceptLine();
};


QSharedPointer<matrix> readOMF(const std::string &path, OMFHeader &header)
{
  std::ifstream in(path.c_str());
  if (!in.good()) {
    throw std::runtime_error(std::string("Could not open file: ") + path);
  }

  OMFImport omf;
  omf.read(in);
  header = omf.header;
  return QSharedPointer<matrix>(omf.field);
}

QSharedPointer<matrix> readOMF(std::istream &in, OMFHeader &header)
{
	OMFImport omf;
	omf.read(in);
	header = omf.header;
    return QSharedPointer<matrix>(omf.field);
}

////////////////////////////////////////////////////////////////////////////////

static int str2int(const std::string &value)
{
	return atoi(value.c_str());
}

static double str2dbl(const std::string &value)
{
	return strtod(value.c_str(), 0);
}

static bool parseFirstLine(const std::string &line, std::string &key, std::string &value,
			   int &version)
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

static bool parseCommentLine(const std::string &line, std::string &key, std::string &value)
{
  if (line[0] == '#') {
      const int sep = line.find(':');
      key = std::string(line.begin()+2, line.begin()+sep);
      std::transform(key.begin(), key.end(), key.begin(), ::tolower);
      value = std::string(line.begin()+sep+2, line.end());
      std::transform(value.begin(), value.end(), value.begin(), ::tolower);
      return true;
      //}
  } else {
    return false;
  }
}

void OMFImport::read(std::istream &in)
{
	OMFHeader header = OMFHeader();
	input = &in;
	lineno = 0;
	eof = false;
	input->read(&next_char, sizeof(char));

	acceptLine(); // read in first line
	parse();      // Parse file
}

void OMFImport::acceptLine()
{
	static const char LF = 0x0A;
	static const char CR = 0x0D;

	// Accept LF (Unix), CR, CR+LF (Dos) and LF+CR as line terminators.
	bool reallydone = false;
	while(!reallydone) {
	  line = "";
	  bool done = false;
	  while (!done) {
	    if (next_char == LF) {
	      done = true;
	      input->read(&next_char, sizeof(char));
	      if (next_char == CR) input->read(&next_char, sizeof(char));
	    } else if (next_char == CR) {
	      done = true;
	      input->read(&next_char, sizeof(char));
	      if (next_char == LF) input->read(&next_char, sizeof(char));
	    } else {
	      line += next_char;
	      input->read(&next_char, sizeof(char));
	    }
	  }
	  if (line=="#") {
	    reallydone=false;
	  } else if (line=="") {
	    reallydone=true;
	  } else {
	    reallydone=true;
	  }
	}
	
}

// OMF file parser /////////////////////////////////////////////////////////////////////////////

void OMFImport::parse()
{
	bool ok;
	std::string key, value;

	ok = parseFirstLine(line, key, value, header.version);
	if (ok && key == "oommf") {
	  acceptLine();
	} else {
		throw std::runtime_error("Expected 'OOMMF' at line 1");
	}

	ok = parseCommentLine(line, key, value);
	if (ok && key == "segment count") {
		acceptLine();
	} else {
		throw std::runtime_error("Expected 'Segment count' at line 2");
	}

	ok = parseCommentLine(line, key, value);
	if (ok && key == "begin" && value == "segment") {
		parseSegment();
	} else {
		throw std::runtime_error("Expected begin of segment");
	}
}

void OMFImport::parseSegment()
{
	bool ok;
	std::string key, value;

	ok = parseCommentLine(line, key, value);
	if (!ok || key != "begin" || value != "segment") {
		throw std::runtime_error("Parse error. Expected 'Begin Segment'");
	}
	acceptLine();

	parseHeader();
	ok = parseCommentLine(line, key, value);
	if (!ok || key != "begin") {
		throw std::runtime_error("Parse error. Expected 'Begin Data <type>'");
	}
	if (value == "data text") {
	  parseDataAscii();
	} else if (value == "data binary 4") {
	  parseDataBinary4();
	} else if (value == "data binary 8") {
	  parseDataBinary8();
	} else {
		throw std::runtime_error("Expected either 'Text', 'Binary 4' or 'Binary 8' chunk type");
	}

	ok = parseCommentLine(line, key, value);
	if (!ok || key != "end" || value != "segment") {
		throw std::runtime_error("Expected 'End Segment'");
	}
	acceptLine();
}

void OMFImport::parseHeader()
{
	bool ok;
	std::string key, value;

	ok = parseCommentLine(line, key, value);
	if (!ok || key != "begin" || value != "header") {
		throw std::runtime_error("Expected 'Begin Header'");
	}
	acceptLine();
	
	bool done = false;
	while (!done) {
		ok = parseCommentLine(line, key, value);
		if (!ok) {
		  std::cout << "Skipped line." << std::endl;
		  continue;
		}

		if (key == "end" && value == "header") {
			done = true;
			break;
		} else if (key == "title") {
			header.Title = value;
		} else if (key == "desc") {
			header.Desc.push_back(value);
		} else if (key == "meshunit") {
			header.meshunit = value;
		} else if (key == "valueunit") {
			header.valueunit = value;
		} else if (key == "valuemultiplier") {
			header.valuemultiplier = str2dbl(value);
		} else if (key == "xmin") {
			header.xmin = str2dbl(value);
		} else if (key == "ymin") {
			header.ymin = str2dbl(value);
		} else if (key == "zmin") {
			header.zmin = str2dbl(value);
		} else if (key == "xmax") {
			header.xmax = str2dbl(value);
		} else if (key == "ymax") {
			header.ymax = str2dbl(value);
		} else if (key == "zmax") {
			header.zmax = str2dbl(value);
		} else if (key == "valuedim") {   // OVF 2.0
		  header.valuedim = str2int(value);
		} else if (key == "valueunits") { // OVF 2.0
		  header.valueunits.push_back(value);
		} else if (key == "valuelabels") { // OVF 2.0
		  header.valuelabels.push_back(value);
		} else if (key == "valuerangeminmag") {
			header.ValueRangeMinMag = str2dbl(value);
		} else if (key == "valuerangemaxmag") {
			header.ValueRangeMaxMag = str2dbl(value);
		} else if (key == "meshtype") {
			header.meshtype = value;
		} else if (key == "xbase") {
			header.xbase = str2dbl(value);
		} else if (key == "ybase") {
			header.ybase = str2dbl(value);
		} else if (key == "zbase") {
			header.zbase = str2dbl(value);
		} else if (key == "xstepsize") {
			header.xstepsize = str2dbl(value);
		} else if (key == "ystepsize") {
			header.ystepsize = str2dbl(value);
		} else if (key == "zstepsize") {
			header.zstepsize = str2dbl(value);
		} else if (key == "xnodes") {
			header.xnodes = str2int(value);
		} else if (key == "ynodes") {
			header.ynodes = str2int(value);
		} else if (key == "znodes") {
			header.znodes = str2int(value);
		} else {
		  std::clog << "OMFImport::parseHeader: Unknown key: " << key << "/" << value << std::endl;
		}
		acceptLine();
	}

	ok = parseCommentLine(line, key, value);
	if (!ok || key != "end" || value != "header") {
		throw std::runtime_error("Expected 'End Header'");
	}
	acceptLine();
}

void OMFImport::parseDataAscii()
{
	bool ok;
	std::string key, value;
	
	ok = parseCommentLine(line, key, value);
	if (!ok || key != "begin" || value != "data text") {
		throw std::runtime_error("Expected 'Begin DataText'");
	}
	acceptLine();

	// Create field matrix object
//	field = array_ptr(new array_type(boost::extents[header.xnodes][header.ynodes][header.znodes][3]));
    field = QSharedPointer<matrix>(new matrix(header.xnodes, header.ynodes, header.znodes));

	//std::cout << "Loading!" << std::endl;

	for (int z=0; z<header.znodes; ++z)
	  for (int y=0; y<header.ynodes; ++y)
	    for (int x=0; x<header.xnodes; ++x) {
	      std::stringstream ss;
	      ss << line;
	      
	      double v1, v2, v3;
          QVector3D val;
	      if (header.valuedim == 1) {
	      	ss >> v1;
            val = QVector3D(v1,v1,v1);
	      	// std::cout << v1 << std::endl;
	      } else {
			ss >> v1 >> v2 >> v3;
            val = QVector3D(v1,v2,v3);
			// std::cout << v1 << std::endl;
	      }

	      if (header.version==1) {
              val = val*header.valuemultiplier;
	      }

          field->set(x,y,z,val);

	      acceptLine();
	    }

	ok = parseCommentLine(line, key, value);
	if (!ok || key != "end" || value != "data text") {
		throw std::runtime_error("Expected 'End Data Text'");
	}
	acceptLine();
}

void OMFImport::parseDataBinary4()
{
  assert(sizeof(float) == 4);

  bool ok;
  std::string key, value;

  // Parse "Begin: Data Binary 4"
  ok = parseCommentLine(line, key, value);
  if (!ok || key != "begin" || value != "data binary 4") {
    throw std::runtime_error("Expected 'Begin Binary 4'");
  }

  // Create field matrix object
  field = QSharedPointer<matrix>(new matrix(header.xnodes, header.ynodes, header.znodes));

  //const int num_cells = field->numElements();
  int num_cells = field->num_elements();

  // Read magic value and field contents from file
  float magic; 
  ((char*)&magic)[0] = next_char; next_char = -1;
  input->read((char*)&magic+1, sizeof(char)); 
  input->read((char*)&magic+2, sizeof(char)); 
  input->read((char*)&magic+3, sizeof(char)); 
  if (header.version==1) {
    magic = fromBigEndian(magic);
  } else if (header.version==2) {
    magic = fromLittleEndian(magic);
  } else {
    magic = 0;
    throw std::runtime_error("Wrong version number detected.");
  }
  
  if (magic != 1234567.0f) throw std::runtime_error("Wrong magic number (binary 4 format)");

  float *buffer;
  if (header.valuedim == 1) {
	  buffer = new float [num_cells];
	  input->read((char*)buffer, num_cells*sizeof(float));
  } else {
	  buffer = new float [3*num_cells];
	  input->read((char*)buffer, 3*num_cells*sizeof(float));
  }

  QVector3D val;
  for (int i=0; i<num_cells; ++i) {
    if (header.valuedim == 1) {
        if (header.version==1) {
            float v1 = fromBigEndian(buffer[i]) * header.valuemultiplier;
            val = QVector3D(v1,v1,v1);
        } else {
            float v1 = fromLittleEndian(buffer[i]);
            val = QVector3D(v1,v1,v1);
        }
    } else {
        if (header.version==1) {
            val.setX(fromBigEndian(buffer[i*3+0]));
            val.setY(fromBigEndian(buffer[i*3+1]));
            val.setZ(fromBigEndian(buffer[i*3+2]));
            val = val * header.valuemultiplier;
        } else {
            val.setX(fromLittleEndian(buffer[i*3+0]));
            val.setY(fromLittleEndian(buffer[i*3+1]));
            val.setZ(fromLittleEndian(buffer[i*3+2]));
        }
     }
    field->set(i,val);
  }

  delete [] buffer;

  acceptLine(); // read next line...

  // Seems that we must remove the first erroneous char
  line = line.substr(1,line.size()-1);
  
// Parse "End: Data Binary 4"
  if (header.version==1) {
    ok = parseCommentLine(line, key, value);
    if (!ok || key != "end" || value != "data binary 4") {
      throw std::runtime_error("Expected 'End Data Binary 4'");
    }
    acceptLine();
  } else {
    acceptLine();
  }
}

void OMFImport::parseDataBinary8()
{
  assert(sizeof(double) == 8);

  bool ok;
  std::string key, value;

  // Parse "Begin: Data Binary 8"
  ok = parseCommentLine(line, key, value);
  if (!ok || key != "begin" || value != "data binary 8") {
    throw std::runtime_error("Expected 'Begin Binary 8'");
  }

  // Create field matrix object
  //field.reset(new VectorMatrix(IntVector3d(header.xnodes, header.ynodes, header.znodes), Vector3d(0.0, 0.0, 0.0)));
  field = QSharedPointer<matrix>(new matrix(header.xnodes, header.ynodes, header.znodes));

  //const int num_cells = field->numElements();
  const int num_cells = field->num_elements();

  // Read magic value and field contents from file
  double magic;
  ((char*)&magic)[0] = next_char; next_char = -1;
  input->read((char*)&magic+1, sizeof(char)); 
  input->read((char*)&magic+2, sizeof(char)); 
  input->read((char*)&magic+3, sizeof(char)); 
  input->read((char*)&magic+4, sizeof(char)); 
  input->read((char*)&magic+5, sizeof(char)); 
  input->read((char*)&magic+6, sizeof(char)); 
  input->read((char*)&magic+7, sizeof(char)); 
  if (header.version==1) {
    magic = fromBigEndian(magic);
  } else if (header.version==2) {
    magic = fromLittleEndian(magic);
  } else {
    magic = 0;
    throw std::runtime_error("Wrong version number detected.");
  }

  if (magic != 123456789012345.0) throw std::runtime_error("Wrong magic number (binary 8 format)");

  double *buffer = new double [3*num_cells];
  input->read((char*)buffer, 3*num_cells*sizeof(double));

  QVector3D val;
  for (int i=0; i<num_cells; ++i) {
    if (header.valuedim == 1) {
        if (header.version==1) {
            float v1 = fromBigEndian(buffer[i]) * header.valuemultiplier;
            val = QVector3D(v1,v1,v1);
        } else {
            float v1 = fromLittleEndian(buffer[i]);
            val = QVector3D(v1,v1,v1);
        }
    } else {
        if (header.version==1) {
            val.setX(fromBigEndian(buffer[i*3+0]));
            val.setY(fromBigEndian(buffer[i*3+1]));
            val.setZ(fromBigEndian(buffer[i*3+2]));
            val = val * header.valuemultiplier;
        } else {
            val.setX(fromLittleEndian(buffer[i*3+0]));
            val.setY(fromLittleEndian(buffer[i*3+1]));
            val.setZ(fromLittleEndian(buffer[i*3+2]));
        }
     }
    field->set(i,val);
  }

  delete [] buffer;

  acceptLine(); // read next line...

  // Seems that we must remove the first erroneous char
  line = line.substr(1,line.size()-1);

  // Parse "End: Data Binary 8"
  ok = parseCommentLine(line, key, value);
  if (!ok || key != "end" || value != "data binary 8") {
    throw std::runtime_error("Expected 'End Data Binary 8'");
  }
  acceptLine();
}
