#include "Sandbox3D.h"
#include <rapidxml/rapidxml_print.hpp>

namespace Sandbox3D
{
	static std::string ToString(const bool& var)
	{
		std::string str;
		if (var)
		{
			str = "1";
		}
		else
		{
			str = "0";
		}

		return str;
	}
	static std::string ToString(const glm::vec2& var)
	{
		std::string str(std::to_string(var.x) + "," + std::to_string(var.y));
		return str;
	}

	static bool GetValue(char* val, glm::vec2& var)
	{
		if (val)
		{
			float x, y;
			if (sscanf(val, "%f,%f", &x, &y) == 2)
			{
				var = glm::vec2(x, y);
				return true;
			}
		}
		return false;
	}
	static bool GetValue(char* val, bool& var)
	{
		if (val)
		{
			var = atoi(val) != 0;
			return true;
		}

		return false;
	}

	bool Sandbox3D::OnWindowClose(Lamp::WindowCloseEvent& e)
	{
		using namespace rapidxml;

		std::ofstream file;
		xml_document<> doc;
		file.open("config.cfg");

		xml_node<>* pRoot = doc.allocate_node(rapidxml::node_element, "Sandbox");

		xml_node<>* pWindowSize = doc.allocate_node(node_element, "WindowSize");
		char* pSize = doc.allocate_string(ToString(glm::vec2(Lamp::Application::Get().GetWindow().GetWidth(), Lamp::Application::Get().GetWindow().GetHeight())).c_str());
		pWindowSize->append_attribute(doc.allocate_attribute("size", pSize));

		xml_node<>* pWindowsOpen = doc.allocate_node(node_element, "WindowsOpen");
		char* pPers = doc.allocate_string(ToString(m_PerspectiveOpen).c_str());
		pWindowsOpen->append_attribute(doc.allocate_attribute("Perspective", pPers));

		char* pAssetB = doc.allocate_string(ToString(m_AssetBrowserOpen).c_str());
		pWindowsOpen->append_attribute(doc.allocate_attribute("AssetBrowser", pAssetB));

		char* pProps = doc.allocate_string(ToString(m_InspectiorOpen).c_str());
		pWindowsOpen->append_attribute(doc.allocate_attribute("Inspector", pProps));

		char* pModelImp = doc.allocate_string(ToString(m_ModelImporterOpen).c_str());
		pWindowsOpen->append_attribute(doc.allocate_attribute("ModelImporter", pModelImp));

		char* pLayers = doc.allocate_string(ToString(m_LayerViewOpen).c_str());
		pWindowsOpen->append_attribute(doc.allocate_attribute("LayerView", pLayers));

		char* pCreate = doc.allocate_string(ToString(m_CreateToolOpen).c_str());
		pWindowsOpen->append_attribute(doc.allocate_attribute("CreateTool", pCreate));

		char* pLog = doc.allocate_string(ToString(m_LogToolOpen).c_str());
		pWindowsOpen->append_attribute(doc.allocate_attribute("LogTool", pLog));

		pRoot->append_node(pWindowSize);
		pRoot->append_node(pWindowsOpen);

		doc.append_node(pRoot);
		file << doc;
		file.close();

		return true;
	}

	void Sandbox3D::SetupFromConfig()
	{
		using namespace rapidxml;

		xml_document<> file;
		xml_node<>* pRootNode;

		std::ifstream levelFile("config.cfg");
		std::vector<char> buffer((std::istreambuf_iterator<char>(levelFile)), std::istreambuf_iterator<char>());
		buffer.push_back('\0');

		file.parse<0>(&buffer[0]);
		pRootNode = file.first_node("Sandbox");

		if (xml_node<>* pWindowSize = pRootNode->first_node("WindowSize"))
		{
			glm::vec2 wS;
			GetValue(pWindowSize->first_attribute("size")->value(), wS);
			Lamp::Application::Get().GetWindow().SetSize(wS);
		}

		if (xml_node<>* pWindowsOpen = pRootNode->first_node("WindowsOpen"))
		{
			GetValue(pWindowsOpen->first_attribute("Perspective")->value(), m_PerspectiveOpen);
			GetValue(pWindowsOpen->first_attribute("AssetBrowser")->value(), m_AssetBrowserOpen);
			GetValue(pWindowsOpen->first_attribute("Inspector")->value(), m_InspectiorOpen);
			GetValue(pWindowsOpen->first_attribute("ModelImporter")->value(), m_ModelImporterOpen);
			GetValue(pWindowsOpen->first_attribute("LayerView")->value(), m_LayerViewOpen);
			GetValue(pWindowsOpen->first_attribute("CreateTool")->value(), m_CreateToolOpen);
			GetValue(pWindowsOpen->first_attribute("LogTool")->value(), m_LogToolOpen);
		}
	}
}