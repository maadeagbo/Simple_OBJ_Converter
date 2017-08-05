/*
* Copyright (c) 2017, Moses Adeagbo
* All rights reserved.
*/
#include "DD_ObjConverter.h"
#include <fstream>
#include <vector>
#include <map>

namespace 
{
	std::vector<vec3_f>		vert;
	std::vector<vec3_f>		norm;
	std::vector<vec3_f>		uv;
	std::vector<Vertex>		vertices;
	std::vector<unsigned>	indices;
	std::map<cbuff<16>, unsigned> meshbin;
	cbuff<16> 				vertex_id;

	bool v_vt_vn[] = { false, false, false };

	unsigned unique_v = 0;
	unsigned copied_v = 0;
}

/// \brief Read in obj file and parse to get MeshContainer
ObjImportStatus DD_ObjConverter::importOBJ(const char* filename)
{
	vert.clear();
	norm.clear();
	uv.clear();
	vertices.clear();
	indices.clear();
	meshbin.clear();

	/// \brief Lambda to move c string past next space
	auto skipPastDelim = [&](char *&str, const char delim = ' ') 
	{
		while (*str != delim && *str) { str++; } str++;
	};
	
	/// \brief Lambda to get line id from obj file
	auto getLineId = [&](char *str, char *buff, const unsigned count)
	{
		for(unsigned i = 0; i < count; ++i) {
			buff[i] = str[i];
		}
		buff[count] = '\0';
	};
	
	/// \brief Lambda to get vec3_f from c string
	auto getVec3 = [&](char *str, const unsigned count)
	{
		vec3_f output;
		char buff[32];
		uint8_t idx = 0;

		skipPastDelim(str); // skip identifier
		for(unsigned i = 0; i < count && i < 4; i++) {
			idx = 0;
		    while (*str != ' ' && *str) {
				buff[idx] = *str;
				idx++; str++; 
			}
			buff[idx] = '\0';
			skipPastDelim(str);
			output.data[i] = std::strtod(buff, nullptr);
		}
		return output;
	};

	/// \brief Lambda to translate face index string to vec3_u
	auto faceToVec3 = [&](char* str)
	{
		vec3_u out;
		unsigned start = 0;
		// replace all '/' with ' '
		while (*str) { *str = (*str == '/') ? ' ' : *str; str++; start++; }
		str -= start; // set string back to [0] index
		char *nxt;
		out.x() = std::strtoul(str, &nxt, 10); nxt++;
		out.y() = std::strtoul(nxt, &nxt, 10); nxt++;
		out.z() = std::strtoul(nxt, nullptr, 10);
		printf("%u %u %u ", out.x(), out.y(), out.z());

		return out;
	};

	/// \brief Lambda to get Vertex object from c string
	auto getVertex = [&](char *&str)
	{
		Vertex output;
		char buff[64];
		uint8_t idx = 0;
		if (*str == ' ') { str++; } // remove space at head

		while (*str != ' ' && *str) {
			buff[idx] = *str;
			idx++; str++; 
		}
		buff[idx] = '\0';
		faceToVec3(buff);
		vertex_id.set(buff);

		if (meshbin.count(vertex_id)) {
			copied_v += 1;
			return meshbin[vertex_id];
		}
		else {
			//printf("Miss: %s\n", vertex_id._str());
			meshbin[vertex_id] = vertices.size();
			vertices.push_back(output);
			unique_v += 1;
			return vertices.size() - 1;
		}
	};

	// get file contents
	std::ifstream file(filename);
	if (file.good()) {
		char line[256];
		char lineId[4];

		while (file.getline(line, sizeof(line))) {
			getLineId(line, lineId, 2);
			if(strcmp(lineId, "v ") == 0) {
				if (!v_vt_vn[0]) { v_vt_vn[0] = true; }
				vert.push_back(getVec3(line, 3));
			}
			if(strcmp(lineId, "vn") == 0) {
				if (!v_vt_vn[2]) { v_vt_vn[2] = true; }
				norm.push_back(getVec3(line, 3));	
			}
			if(strcmp(lineId, "vt") == 0) {
				if (!v_vt_vn[1]) { v_vt_vn[1] = true; }
				uv.push_back(getVec3(line, 2));
			}
			if(strcmp(lineId, "f ") == 0) {
				if (!v_vt_vn[0] || !v_vt_vn[1] || !v_vt_vn[2]) { 
					return ObjImportStatus::V_VT_VN_MISSING;
				}

				char* str = line;
				skipPastDelim(str); // skip identifier
				const unsigned start_idx = indices.size();
				unsigned count = 0;
				while(*str) {		// null terminate evaluates to false
					if (count < 3) {
						indices.push_back(getVertex(str));
					}
					else {
						// triengle fan
						indices.push_back(indices[start_idx]);
						indices.push_back(indices[start_idx + (count - 1)]);
						indices.push_back(getVertex(str));
					}
					count += 1;
					printf("\t");
				}
				printf("\t\n");
			}
		}
		return ObjImportStatus::GOOD;
	}
	else {
		printf("Cannot open %s\n", filename);
		return ObjImportStatus::FILE_NOT_FOUND;
	}
}

void DD_ObjConverter::printStats()
{
	printf("OBJ Stats \n");
	printf("\tPositions read:  %d\n", vert.size());
	printf("\tNormals read:    %d\n", norm.size());
	printf("\tUVs read:        %d\n", uv.size());
	printf("\n");
	printf("\tVertices\n");
	printf("\t  total:         %u\n", unique_v);
	printf("\t  re-referenced: %u\n", copied_v);
	printf("\n");
	printf("\tTriangles\n");
	printf("\t  indices:       %u\n", indices.size());
	printf("\t  total:         %u\n", indices.size()/3);
}