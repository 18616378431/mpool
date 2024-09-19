#ifndef TYPES_H
#define TYPES_H

#include <iostream>

#pragma pack(push, 1)

typedef std::int64_t int64;
typedef std::int32_t int32;
typedef std::int16_t int16;
typedef std::int8_t int8;
typedef std::uint64_t uint64;
typedef std::uint32_t uint32;
typedef std::uint16_t uint16;
typedef std::uint8_t uint8;

enum Cmd
{
    AUTH_LOGON_CHALLENGE = 0x00,
    AUTH_LOGON_PROOF = 0x01,
    AUTH_RECONNECT_CHALLENGE = 0x02,
    AUTH_RECONNECT_PROOF = 0x03,
    REALM_LIST = 0x10,
    XFER_INITIATE = 0x30,
    XFER_DATA = 0x31,
    XFER_ACCEPT = 0x32,
    XFER_RESUME = 0x33,
    XFER_CANCEL = 0x34
};

enum ChallengeResult
{
    WOW_SUCCESS                                  = 0x00,
    WOW_FAIL_BANNED                              = 0x03,
    WOW_FAIL_UNKNOWN_ACCOUNT                     = 0x04,
    WOW_FAIL_INCORRECT_PASSWORD                  = 0x05,
    WOW_FAIL_ALREADY_ONLINE                      = 0x06,
    WOW_FAIL_NO_TIME                             = 0x07,
    WOW_FAIL_DB_BUSY                             = 0x08,
    WOW_FAIL_VERSION_INVALID                     = 0x09,
    WOW_FAIL_VERSION_UPDATE                      = 0x0A,
    WOW_FAIL_INVALID_SERVER                      = 0x0B,
    WOW_FAIL_SUSPENDED                           = 0x0C,
    WOW_FAIL_FAIL_NOACCESS                       = 0x0D,
    WOW_SUCCESS_SURVEY                           = 0x0E,
    WOW_FAIL_PARENTCONTROL                       = 0x0F,
    WOW_FAIL_LOCKED_ENFORCED                     = 0x10,
    WOW_FAIL_TRIAL_ENDED                         = 0x11,
    WOW_FAIL_USE_BATTLENET                       = 0x12,
    WOW_FAIL_ANTI_INDULGENCE                     = 0x13,
    WOW_FAIL_EXPIRED                             = 0x14,
    WOW_FAIL_NO_GAME_ACCOUNT                     = 0x15,
    WOW_FAIL_CHARGEBACK                          = 0x16,
    WOW_FAIL_INTERNET_GAME_ROOM_WITHOUT_BNET     = 0x17,
    WOW_FAIL_GAME_ACCOUNT_LOCKED                 = 0x18,
    WOW_FAIL_UNLOCKABLE_LOCK                     = 0x19,
    WOW_FAIL_CONVERSION_REQUIRED                 = 0x20,
    WOW_FAIL_DISCONNECTED                        = 0xFF
};

#define AUTH_LOGON_CHALLENGE_INITIAL_SIZE 4

//1. 第一个包
typedef struct AUTH_LOGON_CHALLENGE_C //35byte 4字节头 size为body字节大小
{
    //header
    uint8   cmd;//AUTH_LOGON_CHALLENGE 0
    uint8   error;
    uint16  size;//body length
    //body
    uint8   gamename[4];//default wow any
    uint8   version1;//3 any
    uint8   version2;//3 any
    uint8   version3;//5 any
    uint16  build;//12340
    uint8   platform[4];//x86
    uint8   os[4];//Win 关联versionchallenge
    uint8   country[4];//zhCN
    uint32  timezone_bias;//参考tentacli
    uint32  ip;//客户端实际IP
    uint8   I_len;//用户名实际长度 length
    uint8   I[1];//用户名 仅存储实际用户名不包含结尾控制字符
} sAuthLogonChallenge_C;


//2.第一个回包
typedef struct AUTH_LOGON_CHALLENGE_S//118字节 完全长度
{
    uint8 cmd;//CMD_AUTH_LOGON_CHALLENGE
    uint8 unk2;
    uint8 error;
    uint8 B[32];
    uint8 g_len;
    uint8 g[1];
    uint8 N_len;
    uint8 N[32];
    uint8 s[32];
    uint8 unk3[16];
    uint8 securityFlags;
} sAuthLogonChallenge_S;

//3. 第二个发包
typedef struct AUTH_LOGON_PROOF_C//75字节
{
    uint8 cmd;//CMD_AUTH_LOGON_PROOF
    uint8 A[32];
    uint8 M1[20];
    uint8 crc_hash[20];
    uint8 num_of_keys;
    uint8 security_Falgs;//0x00-0x04
} sAuthLogonProof_C;

//4. 第二个回包
typedef struct AUTH_LOGON_PROOF_S//32字节 完全长度 仅首包变长
{
    uint8 cmd;//CMD_AUTH_LOGON_PROOF
    uint8 error;
    uint8 M2[20];//客户端校验值 K为session_key
    uint32 accountFlags;
    uint32 surveyId;//0
    uint16 unkFlags;
} sAuthLogonProof_S;

//5. 获取服务器列表及角色数量
// 发送CMD_REALM_LIST

#pragma pack(pop)

#endif // TYPES_H
