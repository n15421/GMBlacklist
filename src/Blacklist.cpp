#include "Global.h"
#include <GMLIB/Server/UserCache.h>

nlohmann::json mBanList;
nlohmann::json mBanIpList;

std::mutex mtx;

void saveBanFile() {
    std::string path = "./banned-players.json";
    GMLIB::Files::JsonFile::writeFile(path, mBanList);
}

void saveBanIpFile() {
    std::string path = "./banned-ips.json";
    GMLIB::Files::JsonFile::writeFile(path, mBanIpList);
}

void initDataFile() {
    auto emptyFile = nlohmann::json::array();
    mBanList       = GMLIB::Files::JsonFile::initJson("./banned-players.json", emptyFile);
    mBanIpList     = GMLIB::Files::JsonFile::initJson("./banned-ips.json", emptyFile);
}

std::string getIP(std::string ipAndPort) {
    auto pos = ipAndPort.find(":");
    return ipAndPort.substr(0, pos);
}

std::time_t convertStringToTime(const std::string& timeString) {
    std::tm            tm = {};
    std::istringstream iss(timeString);
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return std::mktime(&tm);
}

bool isExpired(std::string targetTimeStr) {
    if (targetTimeStr == "forever") {
        return false;
    }
    std::time_t                           targetTime   = convertStringToTime(targetTimeStr);
    std::chrono::system_clock::time_point currentTime  = std::chrono::system_clock::now();
    std::time_t                           currentTimeT = std::chrono::system_clock::to_time_t(currentTime);
    if (targetTime > currentTimeT) {
        return false;
    }
    return true;
}

std::string getExpiredTime(int minutes) {
    std::chrono::system_clock::time_point now        = std::chrono::system_clock::now();
    std::time_t                           timestamp  = std::chrono::system_clock::to_time_t(now);
    timestamp                                       += minutes * 60;
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S %z");
    return ss.str();
}

bool isBanned(std::string& info) {
    for (auto& key : mBanList) {
        if (key.contains("uuid")) {
            if (key["uuid"] == info) {
                return true;
            }
        } else {
            if (key["name"] == info) {
                auto data = GMLIB::Server::UserCache::tryFindCahceInfoFromName(info);
                if (data.has_value()) {
                    key["uuid"] = data.value()["uuid"];
                    saveBanFile();
                }
                return true;
            }
        }
    }
    return false;
}

bool isIpBanned(std::string& ip) {
    for (auto& key : mBanIpList) {
        if (key["ip"] == ip) {
            return true;
        }
    }
    return false;
}

std::pair<std::string, std::string> getBannedInfo(std::string& uuid) {
    for (auto& key : mBanList) {
        if (key.contains("uuid")) {
            if (key["uuid"] == uuid) {
                auto reason  = key["reason"].get<std::string>();
                auto expires = key["expires"].get<std::string>();
                return {reason, expires};
            }
        }
    }
    return {"", ""};
}

std::pair<std::string, std::string> getBannedIpInfo(std::string& ip) {
    for (auto& key : mBanIpList) {
        if (key["ip"] == ip) {
            auto reason  = key["reason"].get<std::string>();
            auto expires = key["expires"].get<std::string>();
            return {reason, expires};
        }
    }
    return {"", ""};
}

bool banPlayer(std::string& name, std::string& opSource, int time, std::string& reason) {
    if (isBanned(name)) {
        return false;
    }
    auto info = GMLIB::Server::UserCache::tryFindCahceInfoFromName(name);
    auto key  = nlohmann::json::object();
    if (info.has_value()) {
        auto cacheUuid = info.value()["uuid"].get<std::string>();
        if (isBanned(cacheUuid)) {
            return false;
        }
        key["uuid"] = cacheUuid;
    }
    auto endTime   = time < 0 ? "forever" : getExpiredTime(time);
    key["name"]    = name;
    key["reason"]  = reason;
    key["source"]  = opSource;
    key["expires"] = endTime;
    key["created"] = getExpiredTime();
    mBanList.push_back(key);
    saveBanFile();
    return true;
}

bool banIP(std::string& ip, std::string& opSource, int time, std::string& reason) {
    if (!isIpBanned(ip)) {
        auto key       = nlohmann::json::object();
        auto endTime   = time < 0 ? "forever" : getExpiredTime(time);
        key["ip"]      = ip;
        key["reason"]  = reason;
        key["source"]  = opSource;
        key["expires"] = endTime;
        key["created"] = getExpiredTime();
        mBanIpList.push_back(key);
        saveBanIpFile();
        auto lastTime = endTime == "forever" ? tr("disconnect.forever") : endTime;
        auto msg      = tr("disconnect.ipIsBanned", {reason, lastTime});
        ll::service::getLevel()->forEachPlayer([&](Player& player) -> bool {
            auto ipAddress = getIP(player.getIPAndPort());
            if (ipAddress == ip) {
                player.disconnect(msg);
            }
            return true;
        });
        return true;
    }
    return false;
}

bool banOnlinePlayer(Player* pl, std::string& opSource, int time, std::string& reason) {
    auto uuid = pl->getUuid().asString();
    if (!isBanned(uuid)) {
        auto info       = nlohmann::json::object();
        auto endTime    = time < 0 ? "forever" : getExpiredTime(time);
        info["uuid"]    = uuid;
        info["name"]    = pl->getRealName();
        info["source"]  = opSource;
        info["reason"]  = reason;
        info["expires"] = endTime;
        info["created"] = getExpiredTime();
        mBanList.push_back(info);
        saveBanFile();
        auto lastTime = endTime == "forever" ? tr("disconnect.forever") : endTime;
        auto msg      = tr("disconnect.isBanned", {reason, lastTime});
        pl->disconnect(msg);
        return true;
    }
    return false;
}

bool unbanPlayer(std::string& name) {
    for (auto it = mBanList.begin(); it != mBanList.end(); ++it) {
        if (it.value()["name"] == name) {
            mBanList.erase(it);
            --it;
            saveBanFile();
            return true;
        }
    }
    return false;
}

bool unbanIP(std::string& ip) {
    for (auto it = mBanIpList.begin(); it != mBanIpList.end(); ++it) {
        if (it.value()["ip"] == ip) {
            mBanIpList.erase(it);
            --it;
            saveBanIpFile();
            return true;
        }
    }
    return false;
}

void handleBanPlayer(NetworkIdentifier const& source, std::string& uuid, std::string& realName) {
    if (isBanned(uuid) || isBanned(realName)) {
        auto info    = getBannedInfo(uuid);
        auto endtime = info.second == "forever" ? tr("disconnect.forever") : info.second;
        if (isExpired(info.second)) {
            unbanPlayer(realName);
            return;
        }
        auto msg = tr("disconnect.isBanned", {info.first, endtime});
        ll::service::getServerNetworkHandler()
            ->disconnectClient(source, Connection::DisconnectFailReason::Kicked, msg, false);
        return;
    }
}

void handleBanIP(NetworkIdentifier const& source, std::string& ip) {
    if (isIpBanned(ip)) {
        auto info    = getBannedIpInfo(ip);
        auto endtime = info.second == "forever" ? tr("disconnect.forever") : info.second;
        if (isExpired(info.second)) {
            unbanIP(ip);
            return;
        }
        auto msg = tr("disconnect.ipIsBanned", {info.first, endtime});
        ll::service::getServerNetworkHandler()
            ->disconnectClient(source, Connection::DisconnectFailReason::Kicked, msg, false);
        return;
    }
}

void checkBanTime() {
    mtx.lock();
    for (auto it = mBanList.begin(); it != mBanList.end(); ++it) {
        auto endTime = it.value().at("expires").get<std::string>();
        if (isExpired(endTime)) {
            mBanList.erase(it);
            --it;
        }
    }
    saveBanFile();
    for (auto it = mBanIpList.begin(); it != mBanIpList.end(); ++it) {
        auto endTime = it.value().at("expires").get<std::string>();
        if (isExpired(endTime)) {
            mBanIpList.erase(it);
            --it;
        }
    }
    saveBanIpFile();
    mtx.unlock();
}

void checkBanTimeTask() {
    std::thread([&] {
        while (true) {
            std::this_thread::sleep_for(std::chrono::minutes(1));
            checkBanTime();
        }
    }).detach();
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    PlayerLoginHook,
    ll::memory::HookPriority::High,
    ServerNetworkHandler,
    "?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@AEBVLoginPacket@@@Z",
    void,
    class NetworkIdentifier const& source,
    class LoginPacket const&       packet
) {
    origin(source, packet);
    auto cert       = packet.mConnectionRequest->getCertificate();
    auto uuid       = ExtendedCertificate::getIdentity(*cert);
    auto clientXuid = ExtendedCertificate::getXuid(*cert, true);
    auto serverXuid = ExtendedCertificate::getXuid(*cert, false);
    auto realName   = ExtendedCertificate::getIdentityName(*cert);
    auto ipAddress  = getIP(source.getIPAndPort());
    if (clientXuid.empty()) {
        std::string msg = tr("disconnect.clientNotAuth");
        ll::service::getServerNetworkHandler()
            ->disconnectClient(source, Connection::DisconnectFailReason::Kicked, msg, false);
    }
    auto strUuid = uuid.asString();
    handleBanPlayer(source, strUuid, realName);
    handleBanIP(source, ipAddress);
}