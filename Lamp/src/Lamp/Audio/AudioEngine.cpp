#include "lppch.h"
#include "AudioEngine.h"
#include "Lamp/Input/FileSystem.h"

#include <thread>

namespace Lamp
{
Ref<AudioEngine> AudioEngine::s_Instance;

AudioImplementation::AudioImplementation()
{
    StudioSystem = nullptr;
    AudioEngine::ErrorCheck(FMOD::Studio::System::create(&StudioSystem));
    AudioEngine::ErrorCheck(StudioSystem->getCoreSystem(&System));


    //Set the settings
    int sampleRate = 0;
    int numRawSpeakers = 0;
    FMOD_SPEAKERMODE speakerMode = FMOD_SPEAKERMODE_DEFAULT;

    AudioEngine::ErrorCheck(System->getSoftwareFormat(&sampleRate, &speakerMode, &numRawSpeakers));
    AudioEngine::ErrorCheck(System->setSoftwareFormat(sampleRate, speakerMode, numRawSpeakers));
    AudioEngine::ErrorCheck(System->set3DSettings(1.f, 1.f, 1.f));

    //Initialize the studio system
    AudioEngine::ErrorCheck(StudioSystem->initialize(512, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_PROFILE_ENABLE, nullptr));

    System = nullptr;
}

AudioImplementation::~AudioImplementation()
{
    AudioEngine::ErrorCheck(StudioSystem->unloadAll());
    AudioEngine::ErrorCheck(StudioSystem->release());
}

void AudioImplementation::Update()
{
    std::vector<ChannelMap::iterator> stoppedChannels;
    for (auto it = Channels.begin(); it != Channels.end(); ++it)
    {
        bool isPlaying = false;
        it->second->isPlaying(&isPlaying);

        if (!isPlaying)
        {
            stoppedChannels.push_back(it);
        }
    }

    for (auto& it : stoppedChannels)
    {
        Channels.erase(it);
    }

    AudioEngine::ErrorCheck(StudioSystem->update());
}

AudioImplementation* g_AudioImplementation;

void AudioEngine::Initialize()
{
    s_Instance = CreateRef<AudioEngine>();
    g_AudioImplementation = new AudioImplementation();

    s_Instance->LoadBanksFromDefault();
}

void AudioEngine::Update()
{
    g_AudioImplementation->Update();
}

void AudioEngine::Shutdown()
{
    delete g_AudioImplementation;
}

int AudioEngine::ErrorCheck(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        LP_CORE_WARN("FMOD: " + result);
        return 1;
    }

    return 0;
}

void AudioEngine::LoadBanksFromDefault()
{
    std::vector<std::string> files = FileSystem::GetFiles(std::string(DEFAULT_BANK_PATH));
    if (files.size() > 0)
    {
        for (auto& file : files)
        {
            std::replace(file.begin(), file.end(), '\\', '/');

            LoadBank(file, FMOD_STUDIO_LOAD_BANK_NORMAL);
        }
    }
}

void AudioEngine::LoadDefaults()
{
}

void AudioEngine::LoadBank(const std::string& name, FMOD_STUDIO_LOAD_BANK_FLAGS flags)
{
    if (auto it = g_AudioImplementation->Banks.find(name); it != g_AudioImplementation->Banks.end())
    {
        return;
    }

    FMOD::Studio::Bank* bank;
    AudioEngine::ErrorCheck(g_AudioImplementation->StudioSystem->loadBankFile(name.c_str(), flags, &bank));
    if (bank)
    {
        g_AudioImplementation->Banks[name] = bank;
    }
}

void AudioEngine::LoadEvent(const std::string& name)
{
    if (auto it = g_AudioImplementation->Events.find(name); it != g_AudioImplementation->Events.end())
    {
        return;
    }

    FMOD::Studio::EventDescription* eventDesc = nullptr;
    AudioEngine::ErrorCheck(g_AudioImplementation->StudioSystem->getEvent(name.c_str(), &eventDesc));

    if (eventDesc)
    {
        FMOD::Studio::EventInstance* eventInstance = nullptr;
        AudioEngine::ErrorCheck(eventDesc->createInstance(&eventInstance));
        if (eventInstance)
        {
            g_AudioImplementation->Events[name] = eventInstance;
        }
    }
}

void AudioEngine::LoadSound(const std::string& name, bool threeD, bool looping, bool stream)
{
    if (auto it = g_AudioImplementation->Sounds.find(name); it != g_AudioImplementation->Sounds.end())
    {
        return;
    }

    FMOD_MODE mode = FMOD_DEFAULT;
    mode |= threeD ? FMOD_3D : FMOD_2D;
    mode |= looping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
    mode |= stream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;

    FMOD::Sound* sound = nullptr;
    AudioEngine::ErrorCheck(g_AudioImplementation->System->createSound(name.c_str(), mode, nullptr, &sound));
    if (sound)
    {
        g_AudioImplementation->Sounds[name] = sound;
    }
}

void AudioEngine::UnloadSound(const std::string& name)
{
    if (auto it = g_AudioImplementation->Sounds.find(name); it != g_AudioImplementation->Sounds.end())
    {
        AudioEngine::ErrorCheck(it->second->release());
        g_AudioImplementation->Sounds.erase(it);
    }
}

void AudioEngine::Set3DListenerAndOrientation(const glm::vec3& pos, float volume)
{
}

int AudioEngine::PlaySound(const std::string& name, const glm::vec3& pos, float volume)
{
    int channelId = g_AudioImplementation->NextChannelID++;
    auto it = g_AudioImplementation->Sounds.find(name);

    if (it == g_AudioImplementation->Sounds.end())
    {
        LoadSound(name);
        it = g_AudioImplementation->Sounds.find(name);
        if (it == g_AudioImplementation->Sounds.end())
        {
            return channelId;
        }
    }

    FMOD::Channel* channel = nullptr;
    AudioEngine::ErrorCheck(g_AudioImplementation->System->playSound(it->second, nullptr, true, &channel));
    if (channel)
    {
        FMOD_MODE currMode;
        it->second->getMode(&currMode);
        if (currMode & FMOD_3D)
        {
            FMOD_VECTOR position = VectorToFmod(pos);
            AudioEngine::ErrorCheck(channel->set3DAttributes(&position, nullptr));
        }

        AudioEngine::ErrorCheck(channel->setVolume(DBToVolume(volume)));
        AudioEngine::ErrorCheck(channel->setPaused(false));

        g_AudioImplementation->Channels[channelId] = channel;
    }

    return channelId;
}

void AudioEngine::PlayEvent(const std::string& name, const glm::vec3& pos)
{
    auto it = g_AudioImplementation->Events.find(name);
    if (it == g_AudioImplementation->Events.end())
    {
        LoadEvent(name);
        it = g_AudioImplementation->Events.find(name);
    }

    FMOD_3D_ATTRIBUTES attributes;
    attributes.position = VectorToFmod(pos);
    attributes.forward = VectorToFmod(glm::vec3(0.f, 0.f, 1.f));
    attributes.up = VectorToFmod(glm::vec3(0.f, 1.f, 0.f));
    attributes.velocity = VectorToFmod(glm::vec3(0.f, 0.f, 0.f));

    FMOD::Studio::EventDescription* eventDesc;
    AudioEngine::ErrorCheck(it->second->getDescription(&eventDesc));

    bool is3D;
    AudioEngine::ErrorCheck(eventDesc->is3D(&is3D));
    if (is3D)
    {
        AudioEngine::ErrorCheck(it->second->set3DAttributes(&attributes));
    }
    AudioEngine::ErrorCheck(it->second->start());
}

void AudioEngine::StopChannel(int channel)
{
    if (auto it = g_AudioImplementation->Channels.find(channel); it != g_AudioImplementation->Channels.end())
    {
        AudioEngine::ErrorCheck(it->second->stop());
    }
}

void AudioEngine::StopEvent(const std::string& name, bool immediate)
{
    if (auto it = g_AudioImplementation->Events.find(name); it != g_AudioImplementation->Events.end())
    {
        FMOD_STUDIO_STOP_MODE mode;
        mode = immediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
        AudioEngine::ErrorCheck(it->second->stop(mode));
    }
}

void AudioEngine::GetEventParameter(const std::string& name, const std::string& eventParams, float* parameter)
{
    if (auto it = g_AudioImplementation->Events.find(name); it != g_AudioImplementation->Events.end())
    {
        AudioEngine::ErrorCheck(it->second->getParameterByName(eventParams.c_str(), parameter));
    }
}

void AudioEngine::SetEventParameter(const std::string& name, const std::string& paramName, float value)
{
    if (auto it = g_AudioImplementation->Events.find(name); it != g_AudioImplementation->Events.end())
    {
        AudioEngine::ErrorCheck(it->second->setParameterByName(paramName.c_str(), value));
    }
}

void AudioEngine::StopAllChannels()
{
    for (auto channel : g_AudioImplementation->Channels)
    {
        AudioEngine::ErrorCheck(channel.second->stop());
    }
}

void AudioEngine::SetChannel3DPosition(int channel, const glm::vec3& position)
{
    if (auto it = g_AudioImplementation->Channels.find(channel); it != g_AudioImplementation->Channels.end())
    {
        FMOD_VECTOR pos = VectorToFmod(position);
        AudioEngine::ErrorCheck(it->second->set3DAttributes(&pos, NULL));
    }
}

void AudioEngine::SetChannelVolume(int channel, float volume)
{
    if (auto it = g_AudioImplementation->Channels.find(channel); it != g_AudioImplementation->Channels.end())
    {
        AudioEngine::ErrorCheck(it->second->setVolume(DBToVolume(volume)));
    }
}

void AudioEngine::SetListenerAttributes(const Listener& listener, const ListenerAttributes& attr)
{
    FMOD_3D_ATTRIBUTES attributes;
    attributes.forward = VectorToFmod(attr.Forward);
    attributes.position = VectorToFmod(attr.Position);
    attributes.up = VectorToFmod(attr.Up);
    attributes.velocity = VectorToFmod(attr.Velocity);

    AudioEngine::ErrorCheck(g_AudioImplementation->StudioSystem->setListenerAttributes(listener.ListenerID, &attributes, NULL));
}

Listener AudioEngine::AddListener()
{
    int curr;
    g_AudioImplementation->StudioSystem->getNumListeners(&curr);
    curr++;

    g_AudioImplementation->StudioSystem->setNumListeners(curr);

    return Listener(curr);
}

void AudioEngine::RemoveListener(const Listener& listener)
{

}

bool AudioEngine::IsPlaying(int channel) const
{
    if (auto it = g_AudioImplementation->Channels.find(channel); it != g_AudioImplementation->Channels.end())
    {
        bool playing;
        AudioEngine::ErrorCheck(it->second->isPlaying(&playing));

        return playing;
    }

    return false;
}

bool AudioEngine::IsEventPlaying(const std::string& name) const
{
    if (auto it = g_AudioImplementation->Events.find(name); it != g_AudioImplementation->Events.end())
    {
        FMOD_STUDIO_PLAYBACK_STATE* state = nullptr;
        if (it->second->getPlaybackState(state) == FMOD_STUDIO_PLAYBACK_PLAYING)
        {
            return true;
        }
    }

    return false;
}

float AudioEngine::DBToVolume(float db)
{
    return std::powf(10.f, 0.05f * db);
}

float AudioEngine::VolumeToDB(float volume)
{
    return 20.f * std::log10f(volume);
}

FMOD_VECTOR AudioEngine::VectorToFmod(const glm::vec3& pos)
{
    FMOD_VECTOR vec;
    vec.x = pos.x;
    vec.y = pos.y;
    vec.z = pos.z;

    return vec;
}
}