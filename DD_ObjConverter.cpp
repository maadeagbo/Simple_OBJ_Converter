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
	std::vector<vec3_u>		indices;
	std::vector<unsigned>	mesh_offset;
	std::map<cbuff<16>, unsigned> meshbin;
	cbuff<16> 				vertex_id;
	cbuff<32>				obj_id;

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
	tang.clear();
	unique_v = 0;
	copied_v = 0;
	obj_id.set("static_mesh");

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
		out.x() = std::strtoul(str, &nxt, 10) - 1; nxt++;
		out.y() = std::strtoul(nxt, &nxt, 10) - 1; nxt++;
		out.z() = std::strtoul(nxt, nullptr, 10) - 1;

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
		vec3_u info_idx = faceToVec3(buff);
		vertex_id.set(buff);

		if (meshbin.count(vertex_id)) {
			copied_v += 1;
			//printf("Bang!!!\t");
			return meshbin[vertex_id];
		}
		else {
			//printf("Miss: %s\n", vertex_id._str());
			meshbin[vertex_id] = vertices.size();

			//printf("%u/%u/%u\t", info_idx.x(), info_idx.y(), info_idx.z());
			// position
			output.position[0] = vert[info_idx.x()].x();
			output.position[1] = vert[info_idx.x()].y();
			output.position[2] = vert[info_idx.x()].z();
			// texture coords
			output.texCoords[0] = uv[info_idx.y()].x();
			output.texCoords[1] = uv[info_idx.y()].y();
			// normal
			output.normal[0] = norm[info_idx.z()].x();
			output.normal[1] = norm[info_idx.z()].y();
			output.normal[2] = norm[info_idx.z()].z();

			// printf("v->\t %.3f %.3f %.3f\n", 
			// 		output.position[0], output.position[1], output.position[2]);
			// printf("n->\t %.3f %.3f %.3f\n", 
			// 		output.normal[0], output.normal[1], output.normal[2]);
			// printf("uv->\t %.3f %.3f", 
			// 		output.texCoords[0], output.texCoords[1]);

			vertices.push_back(output);
			unique_v += 1;
			return (unsigned)vertices.size() - 1;
		}
	};

	/// \brief Lambda to calculate tangent space for face 
	auto getTanSpaceVector = [&](const vec3_u idxs)
	{
		vec3_f out;
		float factor = 0.f;

		// get uv and position info
		Vertex &v1 = vertices[idxs.x()];
		Vertex &v2 = vertices[idxs.y()];
		Vertex &v3 = vertices[idxs.z()];
		vec3_f vert_1 = vec3_f(v1.position);
		vec3_f vert_2 = vec3_f(v2.position);
		vec3_f vert_3 = vec3_f(v3.position);
		vec3_f uv_1 = vec3_f(v1.texCoords[0], v1.texCoords[1]);
		vec3_f uv_2 = vec3_f(v2.texCoords[0], v2.texCoords[1]);
		vec3_f uv_3 = vec3_f(v3.texCoords[0], v3.texCoords[1]);

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
		// printf("tan -> %.3f %.3f %.3f\n", out.x(), out.y(), out.z());
		// printf("-----------------------------------------\n");
		for (int i = 0; i < 3; i++) {
			v1.tangent[i] = out.data[i];
			v2.tangent[i] = out.data[i];
			v3.tangent[i] = out.data[i];
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
			if(strcmp(lineId, "us") == 0) {
				mesh_offset.push_back(indices.size());
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
						idxs.data[count] = getVertex(str);
						// calc tangent on third vertex
						if (count == 2) {
							indices.push_back(idxs);
							getTanSpaceVector(idxs);
						}
					}
					else {
						// triengle fan
						idxs.x() = indices[start_idx].x();
						idxs.y() = indices[indices.size() - 1].z();
						idxs.z() = getVertex(str);
						indices.push_back(idxs);
						// calc tangent after every extra face
						getTanSpaceVector(idxs);
					}
					count += 1;
				}
			}
		}
		mesh_offset.push_back(indices.size());
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
	printf("\tPositions read:  %lu\n", vert.size());
	printf("\tNormals read:    %lu\n", norm.size());
	printf("\tUVs read:        %lu\n", uv.size());
	printf("\n");
	printf("\tVertices\n");
	printf("\t  total:         %u\n", unique_v);
	printf("\t  re-referenced: %u\n", copied_v);
	printf("\n");
	printf("\tTriangles\n");
	printf("\t  total:         %lu\n", indices.size());
	printf("\n");
	printf("\tMesh offsets\n");
	for (unsigned i = 0; i < mesh_offset.size() - 1; i++) {
		printf("\t  mesh #%u:\t%u\n", i, mesh_offset[i]);
	}
}

/// \brief Export mesh to format specified by dd_entity_map.txt
void DD_ObjConverter::exportMesh()
{
	char lineBuff[256];
	snprintf(lineBuff, sizeof(lineBuff), "%s.ddm", obj_id._str());
	std::fstream outfile;
	outfile.open(lineBuff, std::ofstream::out);

	// check file is open
	if (outfile.bad()) {
		printf("Could not open mesh output file\n" );
		return;
	}

	// name
	snprintf(lineBuff, sizeof(lineBuff), "%s", obj_id._str());
	outfile << "<name>\n" << lineBuff << "\n</name>\n";
	// buffer sizes
	outfile << "<buffer>\n";
	snprintf(lineBuff, sizeof(lineBuff), "v %lu", vertices.size());
	outfile << lineBuff << "\n";
	snprintf(lineBuff, sizeof(lineBuff), "e %lu", mesh_offset.size() - 1);
	outfile << lineBuff << "\n";
	snprintf(lineBuff, sizeof(lineBuff), "m 1");
	outfile << lineBuff << "\n";
	outfile << "</buffer>\n";

	// material data
	outfile << "<material>\n";
	snprintf(lineBuff, sizeof(lineBuff), "n %s", "default");
	outfile << lineBuff << "\n";
	// vector properties
	snprintf(lineBuff, sizeof(lineBuff), "d 0.500 0.500 0.500");
	outfile << lineBuff << "\n";
	snprintf(lineBuff, sizeof(lineBuff), "s 0.500 0.500 0.500");
	outfile << lineBuff << "\n";

	outfile << "</material>\n";

	// vertex data
	outfile << "<vertex>\n";
	for (size_t i = 0; i < vertices.size(); i++) {
		snprintf(lineBuff, sizeof(lineBuff), "v %.3f %.3f %.3f",
				 vertices[i].position[0],
				 vertices[i].position[1],
				 vertices[i].position[2]);
		outfile << lineBuff << "\n";
		snprintf(lineBuff, sizeof(lineBuff), "n %.3f %.3f %.3f",
				 vertices[i].normal[0],
				 vertices[i].normal[1],
				 vertices[i].normal[2]);
		outfile << lineBuff << "\n";
		snprintf(lineBuff, sizeof(lineBuff), "t %.3f %.3f %.3f",
				 vertices[i].tangent[0],
				 vertices[i].tangent[1],
				 vertices[i].tangent[2]);
		outfile << lineBuff << "\n";
		snprintf(lineBuff, sizeof(lineBuff), "u %.3f %.3f",
				 vertices[i].texCoords[0],
				 vertices[i].texCoords[1]);
		outfile << lineBuff << "\n";
		snprintf(lineBuff, sizeof(lineBuff), "j 0 0 0 0");
		outfile << lineBuff << "\n";
		snprintf(lineBuff, sizeof(lineBuff), "b 0.000 0.000 0.000 0.000");
		outfile << lineBuff << "\n";
	}
	outfile << "</vertex>\n";

	// ebo data
	for (size_t i = 0; i < mesh_offset.size() - 1; i++) {
		outfile << "<ebo>\n";
		unsigned e_size = mesh_offset[i + 1] - mesh_offset[i];
		snprintf(lineBuff, sizeof(lineBuff), "s %u", e_size * 3);
		outfile << lineBuff << "\n";
		outfile << "m " << 0 << "\n"; // material index

		// printf("\nStart idx:\t%u\nEnd idx:\t%u\nSize:\t\t%u\n", 
		// 	   mesh_offset[i],
		// 	   mesh_offset[i] + e_size,
		// 	   e_size);
		for (size_t j = 0; j < e_size; j++) {
			snprintf(lineBuff, sizeof(lineBuff), "- %u %u %u",
					 indices[j].data[0], indices[j].data[1], indices[j].data[2]);
			outfile << lineBuff << "\n";
		}
		outfile << "</ebo>\n";
	}

	outfile.close();
}