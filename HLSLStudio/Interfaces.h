#pragma once

enum class ShaderType {
	Vertex = 0,
	Pixel,
	Geometry,
	Hull,
	Compute,
	Domain,
	Amplification,
	Mesh,
	Library,
	_Count
};

struct IMainFrame {
	virtual BOOL UIAddToolBar(HWND tb) = 0;
	virtual BOOL UIEnable(UINT id, bool enable) = 0;
};

