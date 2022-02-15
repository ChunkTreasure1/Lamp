#include "SandboxLayer.h"
#include "Windows/MeshImporterPanel.h"

#include <Lamp/Utility/SerializeMacros.h>
#include <Lamp/Utility/YAMLSerializationHelpers.h>

#include <yaml-cpp/yaml.h>
#include <rapidxml/rapidxml_print.hpp>

namespace Sandbox
{
	static std::filesystem::path s_editorIni = "editor.ini";

	bool SandboxLayer::OnWindowClose(Lamp::WindowCloseEvent& e)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "editorConfig" << YAML::Value;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "windowsOpen" << YAML::Value;
			{
				out << YAML::BeginMap;

				LP_SERIALIZE_PROPERTY(perspective, m_perspectiveOpen, out);
				LP_SERIALIZE_PROPERTY(log, m_logToolOpen, out);

				for (const auto window : m_pWindows)
				{
					LP_SERIALIZE_PROPERTY_STRING(window->GetName(), window->GetIsOpen(), out);
				}

				out << YAML::EndMap;
			}
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream file;
		file.open(s_editorIni);
		file << out.c_str();
		file.close();

		return true;
	}

	void SandboxLayer::SetupFromConfig()
	{
		if (!std::filesystem::exists(s_editorIni))
		{
			LP_WARN("[Sandbox]: File {0} not found!", s_editorIni.string());
			return;
		}

		std::ifstream stream(s_editorIni);
		if (!stream.is_open())
		{
			LP_WARN("[Sandbox]: File {0} could not be opened! File could be corrupt, try removing it and restarting.", s_editorIni.string());
			return;
		}

		std::stringstream strStream;
		strStream << stream.rdbuf();

		stream.close();

		YAML::Node root = YAML::Load(strStream.str());
		YAML::Node configNode = root["editorConfig"];

		if (configNode["windowsOpen"])
		{
			YAML::Node openNode = configNode["windowsOpen"];
			LP_DESERIALIZE_PROPERTY(perspective, m_perspectiveOpen, openNode, true);
			LP_DESERIALIZE_PROPERTY(log, m_logToolOpen, openNode, false);

			for (auto window : m_pWindows)
			{
				LP_DESERIALIZE_PROPERTY_STRING(window->GetName(), window->GetIsOpen(), openNode, false);
			}
		}
	}
}