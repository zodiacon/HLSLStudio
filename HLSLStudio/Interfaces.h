#pragma once

enum class ShaderType {
	Vertex = 0,
	Pixel,
	Geometry,
	Hull,
	Compute,
	Domain,
	_Count
};

struct IMainFrame {
	virtual BOOL UIAddToolBar(HWND tb) = 0;
};

