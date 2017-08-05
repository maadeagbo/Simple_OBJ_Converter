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
	std::vector<vec3_f>		tang;
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

	/// \brief Lambda to calculate tangent space for face 
	auto getTanSpaceVector = [&](const vec3_u idxs)
	{
		vec3_f out;
		float factor = 0.f;

		// get uv and position info
		vec3_f vert_1 = vert[idxs.x()];
		vec3_f vert_2 = vert[idxs.y()];
		vec3_f vert_3 = vert[idxs.z()];
		vec3_f uv_1 = uv[idxs.x()];
		vec3_f uv_2 = uv[idxs.y()];
		vec3_f uv_3 = uv[idxs.z()];

		// get edge and uv (direction) info
		vec3_f edge_1 = vert_2 - vert_1;
		vec3_f edge_2 = vert_3 - vert_1;
		vec3_f del_uv1 = uv_2 - uv_1;
		vec3_f del_uv2 = uv_3 - uv_1;

		// calculate the inverse of the UV matrix and multiply by edge 1 & 2
		factor = 1.f / (del_uv1.x() * del_uv2.y() - del_uv2.x() * del_uv1.y());
		out.x() = factor * (del_uv2.y() * edge_1.x() - del_uv1.y() * edge_2.x());
		out.y() = factor * (del_uv2.y() * edge_1.y() - del_uv1.y() * edge_2.y());
		out.z() = factor * (del_uv2.y() * edge_1.z() - del_uv1.y() * edge_2.z());
		// normalize
		float mag = std::sqrt(out.x() * out.x() + out.y() * out.y() + out.z() * 
							  out.z());
		out = vec3_f( out.x()/mag, out.y()/mag, out.z()/mag );
		printf("tan -> %.3f %.3f %.3f\n", out.x(), out.y(), out.z());
		return out;
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
				vec3_u idxs;
				unsigned count = 0;
				while(*str) {		// null terminate evaluates to false
					if (count < 3) {
						indices.push_back(getVertex(str));
						// calc tangent on third vertex
						if (count == 2) {
							idxs.x() = indices[start_idx];
							idxs.y() = indices[start_idx + 1];
							idxs.z() = indices[start_idx + 2];
							getTanSpaceVector(idxs);
						}
					}
					else {
						idxs.x() = indices[start_idx];
						idxs.y() = indices[start_idx + (count - 1)];
						idxs.z() = indices[start_idx + count];
						// triengle fan
						indices.push_back(idxs.x());
						indices.push_back(idxs.y());
						indices.push_back(getVertex(str));
						// calc tangent after every face
						getTanSpaceVector(idxs);
					}
					count += 1;
				}
				printf("\n");
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