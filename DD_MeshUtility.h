/*
* Copyright (c) 2017, Moses Adeagbo
* All rights reserved.
*/
#pragma once

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include "DD_Strings.h"

template<typename T>
struct dd_vec4
{
	T data[4];
	dd_vec4(T x = 0, T y = 0, T z = 0, T w = 0)
	{
		data[0] = x;
		data[1] = y;
		data[2] = z;
		data[3] = w;
	}

	dd_vec4(T _data[4])
	{
		data[0] = _data[0];
		data[1] = _data[1];
		data[2] = _data[2];
		data[3] = _data[3];
	}

	void operator=(const dd_vec4 other)
	{
		data[0] = other.data[0];
		data[1] = other.data[1];
		data[2] = other.data[2];
		data[3] = other.data[3];
	}

	dd_vec4 operator-(const dd_vec4 other) const
	{
		return dd_vec4(data[0] - other.data[0],
					   data[1] - other.data[1],
					   data[2] - other.data[2],
					   data[3] - other.data[3]);
	}

	T& x() { return data[0]; }
	T const& x() const { return data[0]; }
	T& y() { return data[1]; }
	T const& y() const { return data[1]; }
	T& z() { return data[2]; }
	T const& z() const { return data[2]; }
	T& w() { return data[3]; }
	T const& w() const { return data[3]; }
};

struct vec3_f : public dd_vec4<float> {};
struct vec3_u : public dd_vec4<unsigned> {};

struct Vertex
{
	float position[3] = {0, 0, 0};
	float normal[3] = {0, 0, 0};
	float texCoords[2] = {0, 0};
	float tangent[3] = {0, 0, 0};
};

struct MeshContainer
{
	dd_array<Vertex>	data;
	vec3_f				bbox_min;
	vec3_f				bbox_max;
	dd_array<unsigned>	indices;
	dd_2Darray<unsigned> mesh_idx;
};