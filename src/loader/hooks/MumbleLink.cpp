#include "MumbleLink.h"
#include "../log.h"
#include "../utils/encoding.h"

using namespace std;
using json = nlohmann::json;
using namespace loader::utils;

namespace loader::hooks {

    const string MapTypeToString(MapType type) {
        switch (type)
        {
        case MapType::Redirect:
            return "Redirect";
        case MapType::CharacterCreate:
            return "CharacterCreate";
        case MapType::CompetitivePvp:
            return "CompetitivePvp";
        case MapType::Gvg:
            return "Gvg";
        case MapType::Instance:
            return "Instance";
        case MapType::Public:
            return "Public";
        case MapType::Tournament:
            return "Tournament";
        case MapType::Tutorial:
            return "Tutorial";
        case MapType::UserTournament:
            return "UserTournament";
        case MapType::EternalBattlegrounds:
            return "EternalBattlegrounds";
        case MapType::BlueBorderlands:
            return "BlueBorderlands";
        case MapType::GreenBorderlands:
            return "GreenBorderlands";
        case MapType::RedBorderlands:
            return "RedBorderlands";
        case MapType::FortunesVale:
            return "FortunesVale";
        case MapType::ObsidianSanctum:
            return "ObsidianSanctum";
        case MapType::EdgeOfTheMists:
            return "EdgeOfTheMists";
        case MapType::PublicMini:
            return "PublicMini";
        default:
            return "Unknown";
        }
    }

    const string RaceToString(Race race) {
        switch (race) {
        case Race::Asura:
            return "Asura";
        case Race::Charr:
            return "Charr";
        case Race::Human:
            return "Human";
        case Race::Norn:
            return "Norn";
        case Race::Sylvari:
            return "Sylvari";
        default:
            return "Unknown";
        }
    }

    const string ProfessionToString(Profession profession) {
        switch (profession)
        {
        case Profession::Guardian:
            return "Guardian";
        case Profession::Warrior:
            return "Warrior";
        case Profession::Engineer:
            return "Engineer";
        case Profession::Ranger:
            return "Ranger";
        case Profession::Thief:
            return "Thief";
        case Profession::Elementalist:
            return "Elementalist";
        case Profession::Mesmer:
            return "Mesmer";
        case Profession::Necromancer:
            return "Necromancer";
        case Profession::Revenant:
            return "Revenant";
        default:
            return "Unknown";
        }
    }

    const string UiSizeToString(UiSize size) {
        switch (size)
        {
        case UiSize::Small:
            return "Small";
        case UiSize::Normal:
            return "Normal";
        case UiSize::Large:
            return "Large";
        case UiSize::Larger:
            return "Larger";
        default:
            return "Unknown";
        }
    }

    void from_json(const json& j, GuildWars2Identity& i) {
        i.characterName = j.at("name").get<std::string>();
        i.profession = j.at("profession");
        i.race = j.at("race");
        i.mapId = j.at("map_id");
        i.worldId = j.at("world_id");
        i.teamColorId = j.at("team_color_id");
        i.commander = j.at("commander");
        i.fov = j.at("fov");
        i.uiSize = j.at("uisz");
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
        this->SignalFrame();
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

    void MumbleLink::SignalFrame() {
        this->loopProceed = true;
        this->loopCv.notify_all();
    }

    bool MumbleLink::IsTypeCompetitive() const {
        switch (this->GetMapType()) {
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
            unique_lock<mutex> lock(this->loopMutex);
            this->loopCv.wait(lock, [=] { return this->loopProceed; });
            this->loopProceed = false;
            if (!this->active) {
                // End early if we are stopping
                return;
            }

            try {
                if (this->lastTick != this->linkMemory->uiTick) {
                    this->lastTick = this->linkMemory->uiTick;
                    wstring name = wstring(this->linkMemory->name);
                    if (name == L"Guild Wars 2") {
                        GuildWars2Context* context = reinterpret_cast<GuildWars2Context*>(this->linkMemory->context);

                        this->buildId = context->buildId;
                        wstring identity = this->linkMemory->identity;
                        if (this->identity != identity) {
                            // Identity has changed
                            this->identity = identity;
                            GuildWars2Identity gw2Identity = json::parse(u8(identity));
                            if (this->characterName != gw2Identity.characterName ||
                                this->profession != gw2Identity.profession ||
                                this->race != gw2Identity.race ||
                                this->commander != gw2Identity.commander) {
                                // Character info has updated
                                this->characterName = gw2Identity.characterName;
                                this->profession = gw2Identity.profession;
                                this->race = gw2Identity.race;
                                this->commander = gw2Identity.commander;
                            }
                            if (this->teamColorId != gw2Identity.teamColorId) {
                                // Team has updated
                                this->teamColorId = gw2Identity.teamColorId;
                            }
                            if (this->fov != gw2Identity.fov) {
                                // FoV has updated
                                this->fov = gw2Identity.fov;
                            }
                            if (this->uiSize != gw2Identity.uiSize) {
                                // UI has updated
                                this->uiSize = gw2Identity.uiSize;
                            }
                        }

                        if (this->mapId != context->mapId ||
                            this->mapType != context->mapType ||
                            this->shardId != context->shardId) {
                            // We don't really need to check if the server address is different, we assume that the shard id will be different
                            // Map has updated
                            this->mapId = context->mapId;
                            this->mapType = static_cast<MapType>(context->mapType);
                            this->shardId = context->shardId;
                            this->serverAddress = context->serverAddress;
                        }

                        if (this->characterPosition.x != this->linkMemory->fAvatarPosition.x ||
                            this->characterPosition.y != this->linkMemory->fAvatarPosition.y ||
                            this->characterPosition.z != this->linkMemory->fAvatarPosition.z ||
                            this->characterFront.x != this->linkMemory->fAvatarFront.x ||
                            this->characterFront.y != this->linkMemory->fAvatarFront.y ||
                            this->characterFront.z != this->linkMemory->fAvatarFront.z ||
                            this->characterTop.x != this->linkMemory->fAvatarTop.x ||
                            this->characterTop.y != this->linkMemory->fAvatarTop.y ||
                            this->characterTop.z != this->linkMemory->fAvatarTop.z) {
                            // Character position has updated
                            this->characterPosition = this->linkMemory->fAvatarPosition;
                            this->characterFront = this->linkMemory->fAvatarFront;
                            this->characterTop = this->linkMemory->fAvatarTop;
                        }

                        if (this->cameraPosition.x != this->linkMemory->fCameraPosition.x ||
                            this->cameraPosition.y != this->linkMemory->fCameraPosition.y ||
                            this->cameraPosition.z != this->linkMemory->fCameraPosition.z ||
                            this->cameraFront.x != this->linkMemory->fCameraFront.x ||
                            this->cameraFront.y != this->linkMemory->fCameraFront.y ||
                            this->cameraFront.z != this->linkMemory->fCameraFront.z ||
                            this->cameraTop.x != this->linkMemory->fCameraTop.x ||
                            this->cameraTop.y != this->linkMemory->fCameraTop.y ||
                            this->cameraTop.z != this->linkMemory->fCameraTop.z) {
                            // Camera position has updated
                            this->cameraPosition = this->linkMemory->fCameraPosition;
                            this->cameraFront = this->linkMemory->fCameraFront;
                            this->cameraTop = this->linkMemory->fCameraTop;
                        }
                    }
                }
            }
            catch (...) {
                // Skip logging for now, this might hit with 50 times a second worst case
            }
        }
    }

}
