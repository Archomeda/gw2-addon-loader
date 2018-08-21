#pragma once
#include "../stdafx.h"

namespace loader::hooks {

    struct vector3 {
        float x;
        float y;
        float z;
    };

    enum MapType {
        Redirect = 0,
        CharacterCreate = 1,
        CompetitivePvp = 2,
        Gvg = 3,
        Instance = 4,
        Public = 5,
        Tournament = 6,
        Tutorial = 7,
        UserTournament = 8,
        EternalBattlegrounds = 9,
        BlueBorderlands = 10,
        GreenBorderlands = 11,
        RedBorderlands = 12,
        FortunesVale = 13,
        ObsidianSanctum = 14,
        EdgeOfTheMists = 15,
        PublicMini = 16
    };

    enum Race {
        Asura = 0,
        Charr = 1,
        Human = 2,
        Norn = 3,
        Sylvari = 4
    };

    enum Profession {
        Guardian = 1,
        Warrior = 2,
        Engineer = 3,
        Ranger = 4,
        Thief = 5,
        Elementalist = 6,
        Mesmer = 7,
        Necromancer = 8,
        Revenant = 9
    };

    enum UiSize {
        Small = 0,
        Normal = 1,
        Large = 2,
        Larger = 3
    };

    struct MumbleLinkMem {
        uint32_t uiVersion;
        uint32_t uiTick;
        vector3 fAvatarPosition;
        vector3 fAvatarFront;
        vector3 fAvatarTop;
        wchar_t name[256];
        vector3 fCameraPosition;
        vector3 fCameraFront;
        vector3 fCameraTop;
        wchar_t identity[256];
        uint32_t context_len;
        unsigned char context[256];
        wchar_t description[2048];
    };

    struct GuildWars2Context {
        union {
            uint8_t _serverAddress[28];
            sockaddr_in serverAddress;
        };
        uint32_t mapId;
        uint32_t mapType;
        uint32_t shardId;
        uint32_t instance;
        uint32_t buildId;
    };

    struct GuildWars2Identity {
        std::string characterName;
        Profession profession;
        Race race;
        int mapId;
        int worldId;
        int teamColorId;
        bool commander;
        float fov;
        UiSize uiSize;
    };

    const std::string MapTypeToString(MapType type);
    const std::string RaceToString(Race race);
    const std::string ProfessionToString(Profession profession);
    const std::string UiSizeToString(UiSize size);
    void from_json(const nlohmann::json& j, GuildWars2Identity& i);


    class MumbleLink {
    public:
        MumbleLink(MumbleLink const&) = delete;
        void operator=(MumbleLink const&) = delete;

        static MumbleLink& GetInstance() {
            static MumbleLink instance;
            return instance;
        }

        void Start();
        void Stop();
        void SignalFrame();

        uint32_t GetBuildId() const { return this->buildId; }
        std::string GetCharacterName() const { return this->characterName; }
        Profession GetProfession() const { return this->profession; }
        Race GetRace() const { return this->race; }
        bool IsCommander() const { return this->commander; }
        uint32_t GetMapId() const { return this->mapId; }
        MapType GetMapType() const { return this->mapType; }
        bool IsTypeCompetitive() const;
        uint32_t GetShardId() const { return this->shardId; }
        sockaddr_in GetServerAddress() const { return this->serverAddress; }
        uint32_t GetTeamColorId() const { return this->teamColorId; }
        float GetFov() const { return this->fov; }
        UiSize GetUiSize() const { return this->uiSize; }
        vector3 GetCharacterPosition() const { return this->characterPosition; }
        vector3 GetCharacterTop() const { return this->characterTop; }
        vector3 GetCharacterFront() const { return this->characterFront; }
        vector3 GetCameraPosition() const { return this->cameraPosition; }
        vector3 GetCameraTop() const { return this->cameraTop; }
        vector3 GetCameraFront() const { return this->cameraFront; }

    private:
        MumbleLink() { };

        void Loop();

        HANDLE hMap = NULL;
        std::unique_ptr<MumbleLinkMem> linkMemory;
        unsigned int lastTick;

        uint32_t buildId;
        std::wstring identity;
        std::string characterName;
        Profession profession;
        Race race;
        bool commander;
        uint32_t mapId;
        MapType mapType;
        uint32_t shardId;
        sockaddr_in serverAddress;
        uint32_t teamColorId;
        float fov;
        UiSize uiSize;
        vector3 characterPosition;
        vector3 characterFront;
        vector3 characterTop;
        vector3 cameraPosition;
        vector3 cameraFront;
        vector3 cameraTop;

#pragma region Loop varables
        std::atomic_bool active = false;
        std::thread loopThread;
        std::mutex loopMutex;
        std::condition_variable loopCv;
        bool loopProceed = false;
#pragma endregion
    };

}
