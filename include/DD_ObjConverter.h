/*
* Copyright (c) 2017, Moses Adeagbo
* All rights reserved.
*/
#pragma once

#include "DD_Container.h"
#include "DD_Strings.h"
#include "DD_MeshUtility.h"

enum ObjImportStatus
{
	GOOD,
	FILE_NOT_FOUND,
	V_VT_VN_MISSING
};

struct DD_ObjConverter
{
	ObjImportStatus importOBJ(const char* filename);
	void exportMesh();
	void printStats();
};