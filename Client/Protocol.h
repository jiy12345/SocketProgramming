#pragma once
#define PACKET_HEADER_SIZE 4
#pragma pack(push, 1)
typedef struct
{
	short len;
	short type;
}PACKET_HEADER;

typedef struct
{
	PACKET_HEADER ph;
	char   msg[2048];
}PACKET;

typedef struct {
	char    szName[9];
}USERNAME_REQ;

typedef struct {
	int		iResult;
}USERNAME_ACK;
#pragma pack(pop)

#define PACKET_CHAR_MSG   1000      // client ->
#define PACKET_CHATNAME_REQ   1001  // server -> client
#define PACKET_NAME_REQ   2000		// client -> server
#define PACKET_NAME_ACK   3000		// server -> client
#define PACKET_JOIN_USER  4000		// server -> client

