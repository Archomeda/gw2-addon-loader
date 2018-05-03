#pragma once
#include "../stdafx.h"

namespace loader {
    namespace hooks {

        struct MumbleLinkMem {
            uint32_t uiVersion;
            uint32_t uiTick;
            float fAvatarPosition[3];
            float fAvatarFront[3];
            float fAvatarTop[3];
            wchar_t name[256];
            float fCameraPosition[3];
            float fCameraFront[3];
            float fCameraTop[3];
            wchar_t identity[256];
            uint32_t context_len;
            unsigned char context[256];
            wchar_t description[2048];
        };

        struct GuildWars2Context {
            uint8_t serverAddress[28];
            uint32_t mapId;
            uint32_t mapType;
            uint32_t shardId;
            uint32_t instance;
            uint32_t buildId;
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

        const std::string MapTypeToString(MapType type);

        class MumbleLink {
        public:
            void Start();
            void Stop();

            uint32_t GetMapId() const { return this->mapId; }
            MapType GetMapType() const { return this->mapType; }
            bool IsTypeCompetitive() const;

        private:
            void Loop();

            bool active = false;
            HANDLE hMap = NULL;
            std::thread loopThread;
            std::unique_ptr<MumbleLinkMem> linkMemory;
            unsigned int lastTick;

            uint32_t mapId;
            MapType mapType;
        };

        extern MumbleLink Gw2MumbleLink;

    }
}