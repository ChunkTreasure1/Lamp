#pragma once

#include "fmod/fmod_studio.hpp"
#include "fmod/fmod.hpp"
#include <string>
#include <map>
#include <vector>
#include <math.h>
#include "Lamp/Core/Log.h"
#include "Lamp/Core/Core.h"

#include <glm/glm.hpp>

namespace Lamp
{
	struct AudioImplementation
	{
		using SoundMap = std::map<std::string, FMOD::Sound*>;
		using ChannelMap = std::map<int, FMOD::Channel*>;

		using EventMap = std::map<std::string, FMOD::Studio::EventInstance*>;
		using BankMap = std::map<std::string, FMOD::Studio::Bank*>;

		AudioImplementation();
		~AudioImplementation();

		void Update();

		FMOD::Studio::System* StudioSystem;
		FMOD::System* System;

		int NextChannelID;
		int NumberOfListeners = 0;

		SoundMap Sounds;
		ChannelMap Channels;
		EventMap Events;
		BankMap Banks;
	};

	struct Listener
	{
		Listener(uint32_t id)
			: ListenerID(id)
		{}

		uint32_t ListenerID;
	};

	struct ListenerAttributes
	{
		ListenerAttributes(const glm::vec3& pos, const glm::vec3& vel, const glm::vec3& forward, const glm::vec3& up)
			: Position(pos), Velocity(vel), Forward(forward), Up(up)
		{}

		glm::vec3 Position;
		glm::vec3 Velocity;
		glm::vec3 Forward;
		glm::vec3 Up;
	};

	class AudioEngine
	{
	public:
		void LoadBank(const std::string& name, FMOD_STUDIO_LOAD_BANK_FLAGS flags);
		void LoadEvent(const std::string& name);
		void LoadSound(const std::string& name, bool threeD = false, bool looping = false, bool stream = false);

		void UnloadSound(const std::string& name);
		void Set3DListenerAndOrientation(const glm::vec3& pos = { 0.f, 0.f, 0.f }, float volume = 0.f);
		int PlaySound(const std::string& name, const glm::vec3& pos = { 0.f, 0.f, 0.f }, float volume = 0.f);

		void PlayEvent(const std::string& name, const glm::vec3& pos);
		void StopChannel(int channel);
		void StopEvent(const std::string& name, bool immediate = false);

		void GetEventParameter(const std::string& name, const std::string& eventParams, float* parameter);
		void SetEventParameter(const std::string& name, const std::string& paramName, float value);
		void StopAllChannels();

		void SetChannel3DPosition(int channel, const glm::vec3& position);
		void SetChannelVolume(int channel, float volume);

		void SetListenerAttributes(const Listener& listener, const ListenerAttributes& attr);
		
		Listener AddListener();
		void RemoveListener(const Listener& listener);

		bool IsPlaying(int channel) const;
		bool IsEventPlaying(const std::string& name) const;

		float DBToVolume(float db);
		float VolumeToDB(float volume);

		FMOD_VECTOR VectorToFmod(const glm::vec3& pos);

	public:
		static void Initialize();
		static void Update();
		static void Shutdown();
		static int ErrorCheck(FMOD_RESULT result);

		static Ref<AudioEngine>& Get() { return s_Instance; }

	private:
		static Ref<AudioEngine> s_Instance;

	private:
		void LoadBanksFromDefault();
	};
}