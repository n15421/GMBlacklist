#pragma once
#include "Global.h"
#include <GMLIB/Files/JsonConfig.h>
#include <GMLIB/Files/JsonLanguage.h>

std::string defaultConfig = R"({
    "language": "zh_CN"
})";

std::string defaultLanguage = R"({
    "disconnect.isBanned": "你已被服务器封禁！\n\n原因： %1$s \n解封时间： %2$s",
    "disconnect.ipIsBanned": "你的IP地址已被服务器封禁！\n\n原因： %1$s \n解封时间： %2$s",
    "disconnect.defaultReason": "你已被管理员封禁",
    "disconnect.forever": "永久封禁",
    "disconnect.clientNotAuth": "客户端未登录！\n\n如需进入服务器，请登录Xbox账户！",
    "command.ban.desc": "封禁一名玩家",
    "command.ban.success": "已成功封禁玩家 %1$s ， 解封日期： %2$s",
    "command.ban.isBanned": "玩家 %1$s 已经被封禁",
    "command.unban.desc": "解封一名玩家",
    "command.unban.success": "已成功解除玩家 %1$s 的封禁",
    "command.unban.notBanned": "无法解除封禁，玩家 %1$s 未被服务器封禁！",
    "command.banip.desc": "封禁一个IP地址",
    "command.banip.success": "已成功IP地址 %1$s ，解封日期： %2$s",
    "command.banip.isBanned": "IP地址 %1$s 已经被封禁",
    "command.unbanip.desc": "解封一个IP地址",
    "command.unbanip.success": "已成功IP地址 %1$s 的封禁",
    "command.unbanip.notBanned": "无法解除封禁，IP地址 %1$s 未被服务器封禁！",
    "command.error.invalidTime": "封禁时长必须大于等于1分钟！",
    "command.error.invalidCommandOrigin": "该命令只能由玩家或控制台执行！"
})";