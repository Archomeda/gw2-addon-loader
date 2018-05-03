#include "MumbleLink.h"
#include "../log.h"

using namespace std;

namespace loader {
    namespace hooks {

        MumbleLink Gw2MumbleLink;

        const string MapTypeToString(MapType type) {
            switch (type)
            {
            case MapType::Redirect:
                return "Redirect (0)";
            case MapType::CharacterCreate:
                return "CharacterCreate (1)";
            case MapType::CompetitivePvp:
                return "CompetitivePvp (2)";
            case MapType::Gvg:
                return "Gvg (3)";
            case MapType::Instance:
                return "Instance (4)";
            case MapType::Public:
                return "Public (5)";
            case MapType::Tournament:
                return "Tournament (6)";
            case MapType::Tutorial:
                return "Tutorial (7)";
            case MapType::UserTournament:
                return "UserTournament (8)";
            case MapType::EternalBattlegrounds:
                return "EternalBattlegrounds (9)";
            case MapType::BlueBorderlands:
                return "BlueBorderlands (10)";
            case MapType::GreenBorderlands:
                return "GreenBorderlands (11)";
            case MapType::RedBorderlands:
                return "RedBorderlands (12)";
            case MapType::FortunesVale:
                return "FortunesVale (13)";
            case MapType::ObsidianSanctum:
                return "ObsidianSanctum (14)";
            case MapType::EdgeOfTheMists:
                return "EdgeOfTheMists (15)";
            case MapType::PublicMini:
                return "PublicMini (16)";
            default:
                return "Unknown (" + to_string(type) + ")";
            }
        }

        void MumbleLink::Start() {
            if (this->active) {
                return;
            }

            MUMBLE_LOG()->info("Starting MumbleLink loop");

            this->active = true;

            this->hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(MumbleLinkMem), L"MumbleLink");
            if (this->hMap == NULL) {
                this->Stop();
                return;
            }

            this->linkMemory = unique_ptr<MumbleLinkMem>((MumbleLinkMem*)MapViewOfFile(this->hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(MumbleLinkMem)));
            if (!this->linkMemory) {
                this->Stop();
                return;
            }

            this->loopThread = thread(&MumbleLink::Loop, this);
        }

        void MumbleLink::Stop() {
            if (!this->active) {
                return;
            }

            MUMBLE_LOG()->info("Stopping MumbleLink loop");

            this->active = false;
            this->loopThread.join();

            if (this->linkMemory) {
                UnmapViewOfFile(this->linkMemory.get());
                this->linkMemory.release();
            }

            if (this->hMap != NULL) {
                CloseHandle(this->hMap);
                this->hMap = NULL;
            }
        }

        bool MumbleLink::IsTypeCompetitive() const {
            switch (Gw2MumbleLink.GetMapType()) {
            case MapType::Redirect:
            case MapType::CharacterCreate:
            case MapType::Instance:
            case MapType::Public:
            case MapType::Tutorial:
            case MapType::PublicMini:
                return false;
            case MapType::CompetitivePvp:
            case MapType::Gvg:
            case MapType::Tournament:
            case MapType::UserTournament:
            case MapType::EternalBattlegrounds:
            case MapType::BlueBorderlands:
            case MapType::GreenBorderlands:
            case MapType::RedBorderlands:
            case MapType::FortunesVale:
            case MapType::ObsidianSanctum:
            case MapType::EdgeOfTheMists:
                return true;
            }
            // Default to true for auto-blacklisting
            return true;
        }

        void MumbleLink::Loop() {
            while (this->active) {
                try {
                    if (this->lastTick != this->linkMemory->uiTick) {
                        this->lastTick = this->linkMemory->uiTick;
                        wstring name = wstring(this->linkMemory->name);
                        if (name == L"Guild Wars 2") {
                            GuildWars2Context* context = reinterpret_cast<GuildWars2Context*>(this->linkMemory->context);
                            if (this->mapId != context->mapId) {
                                this->mapId = context->mapId;
                                MUMBLE_LOG()->info("Map changed to {0}", this->mapId);
                            }
                            if (this->mapType != context->mapType) {
                                this->mapType = static_cast<MapType>(context->mapType);
                                MUMBLE_LOG()->info("Map type changed to {0}", MapTypeToString(this->mapType));
                            }
                        }
                    }

                    this_thread::sleep_for(chrono::milliseconds(20)); // Update rate: 50/s
                }
                catch (...) {
                    // Skip logging for now, this might hit with 50 times a second worst case
                }
            }
        }

    }
}
