#pragma once

enum class ShaderType {
	Vertex,
	Pixel,
	Geometry,
	Hull,
	Compute,
	Domain,
};

struct IMainFrame {
	virtual BOOL UIAddToolBar(HWND tb) = 0;
};