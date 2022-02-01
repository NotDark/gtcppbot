#pragma once

//    Grotopia Bot
//    Copyright (C) 2018  Growtopia Noobs
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Affero General Public License as published
//    by the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "enet/include/enet.h"
#include <string>
#include <conio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include "proton/variant.hpp"
#include "packet.h"

using namespace std;

string SERVER_HOST = "213.179.209.168";
int SERVER_PORT = 17196;

/*********  hidden vars  ********/
class GrowtopiaBot
{
public:
	ENetPeer *peer;
	ENetHost *client;

	int login_user = 0;
	int login_token = 0;

	string currentWorld;
	int timeFromWorldEnter = 0; // in 10mss...

	string gameVersion = "3.011";
	/*********  hidden vars  ********/

	/********** user sutff ***************/
	int owner = -1;
	string ownerUsername;
	
	int localx=-1;
	int localy=-1;
	int localnetid=-1;

	struct ObjectData
	{
		int netId = -1;	 // used to interact with stuff in world
		int userId = -1; // growtopia id
		string name = "";
		string country = "";
		string objectType = ""; // "avatar" is player creature
		float x = -1;
		float y = -1;
		bool isGone = false; // set true if character left
		int rectX;			 // collision stuff
		int rectY;			 // collision stuff
		int rectWidth;		 // collision stuff
		int rectHeight;		 // collision stuff
		bool isMod = false;
		bool isLocal = false;
	};

	vector<ObjectData> objects;

	string uname;
	string upass;
	string worldName; // excepted world name

	bool isFollowing = false;
	bool backwardWalk = false;

	int respawnX;
	int respawnY;
	/********** user sutff ***************/

	/*********** structs declaration *********/
	struct PlayerMoving
	{
		int netID;
		float x;
		float y;
		int characterState;
		int plantingTree;
		float XSpeed;
		float YSpeed;
		int punchX;
		int punchY;
	};
	/*********** structs declaration *********/

	/********* user funcs  *********/
	void userRender();
	void userRender2();
	void userLoop();
	void userInit();
	void onLoginRequested();
	void packet_type3(string text);
	void packet_type6(string text);
	void packet_unknown(ENetPacket *packet);
	void OnSendToServer(string address, int port, int userId, int token);
	void OnConsoleMessage(string message);
	void OnPlayPositioned(string sound);
	void OnSetFreezeState(int state);
	void OnRemove(string data);
	void OnSpawn(string data);
	void OnAction(string command);
	void SetHasGrowID(int state, string name, string password);
	void SetHasAccountSecured(int state);
	void OnTalkBubble(int netID, string bubbleText, int type);
	void SetRespawnPos(int respawnPos);
	void OnEmoticonDataChanged(int val1, string emoticons);
	void OnSetPos(float x, float y);
	void OnAddNotification(string image, string message, string audio, int val1);
	void AtApplyTileDamage(int x, int y, int damge);
	void AtApplyLock(int x, int y, int itemId);
	void AtPlayerMoving(PlayerMoving *data);
	void AtAvatarSetIconState(int netID, int state);
	void WhenConnected();
	void WhenDisconnected();

	void respawn();
	/********* user funcs  *********/

	/***************    debug stuff   ***************/
	void dbgPrint(string text)
	{
		cout << "[DBG] " + text << endl;
	}
	/***************    debug stuff   ***************/

	GrowtopiaBot(string username, string password)
	{
		this->uname = username;
		this->upass = password;
	}

	/******************* enet core *********************/
	void SendPacket(int a1, string a2, ENetPeer *enetPeer)
	{
		if (enetPeer)
		{
			ENetPacket *v3 = enet_packet_create(0, a2.length() + 5, 1);
			memcpy(v3->data, &a1, 4);
			//*(v3->data) = (DWORD)a1;
			memcpy((v3->data) + 4, a2.c_str(), a2.length());

			//cout << std::hex << (int)(char)v3->data[3] << endl;
			enet_peer_send(enetPeer, 0, v3);
		}
	}
	
	void SendPacketNew(int type, uint8_t* data, int len)
	{
		auto packet = enet_packet_create(0, len + 5, ENET_PACKET_FLAG_RELIABLE);
    auto game_packet = (gametextpacket_t*)packet->data;
    game_packet->m_type = type;
    if (data)
        memcpy(&game_packet->m_data, data, len);

    memset(&game_packet->m_data + len, 0, 1);
    int code = enet_peer_send(peer, 0, packet);
    if (code != 0)
        printf("Error sending packet! code: %d\n", code);
    enet_host_flush(client);
		
	}

	void SendPacketRaw(int a1, void *packetData, size_t packetDataSize, void *a4, ENetPeer *peer, int packetFlag)
	{
		ENetPacket *p;

		if (peer) // check if we have it setup
		{
			if (a1 == 4 && *((BYTE *)packetData + 12) & 8)
			{
				p = enet_packet_create(0, packetDataSize + *((DWORD *)packetData + 13) + 5, packetFlag);
				int four = 4;
				memcpy(p->data, &four, 4);
				memcpy((char *)p->data + 4, packetData, packetDataSize);
				memcpy((char *)p->data + packetDataSize + 4, a4, *((DWORD *)packetData + 13));
				enet_peer_send(peer, 0, p);
			}
			else
			{
				p = enet_packet_create(0, packetDataSize + 5, packetFlag);
				memcpy(p->data, &a1, 4);
				memcpy((char *)p->data + 4, packetData, packetDataSize);
				enet_peer_send(peer, 0, p);
			}
		}
	}

	// Connect with default value
	void connectClient()
	{
		connectClient(SERVER_HOST, SERVER_PORT);
	}

	void connectClient(string hostName, int port)
	{
		cout << "Connecting bot to " << hostName << ":" << port << endl;
		client = enet_host_create(NULL /* create a client host */,
								  1 /* only allow 1 outgoing connection */,
								  2 /* allow up 2 channels to be used, 0 and 1 */,
								  0 /* 56K modem with 56 Kbps downstream bandwidth */,
								  0 /* 56K modem with 14 Kbps upstream bandwidth */);
		if (client == NULL)
		{
			cout << "An error occurred while trying to create an ENet client host.\n";
			_getch();
			exit(EXIT_FAILURE);
		}
		ENetAddress address;
		client->usingNewPacket = true;
		client->checksum = enet_crc32;
		enet_host_compress_with_range_coder(client);
		enet_address_set_host(&address, hostName.c_str());
		address.port = port;

		/* Initiate the connection, allocating the two channels 0 and 1. */
		peer = enet_host_connect(client, &address, 2, 0);
		if (peer == NULL)
		{
			cout << "No available peers for initiating an ENet connection.\n";
			_getch();
			exit(EXIT_FAILURE);
		}
		enet_host_flush(client);
	}
	/******************* enet core *********************/

	/*************** sender sutff **************/

	void RequestItemActivate(unsigned int item)
	{
		BYTE *data = new BYTE[56];
		for (int i = 0; i < 56; i++)
		{
			data[i] = 0;
		}
		BYTE ten = 10;
		memcpy(data + 0, &ten, 1);
		memcpy(data + 20, &item, 1);
		SendPacketRaw(4, data, 0x38u, 0, peer, 1);
		free(data);
	}

	void SetAndBroadcastIconState(int netID, int state)
	{
		BYTE *data = new BYTE[56];
		for (int i = 0; i < 56; i++)
		{
			data[i] = 0;
		}
		BYTE eighteen = 18;
		memcpy(data + 0, &eighteen, 1);
		memcpy(data + 4, &netID, 4); // (a1+40)
		memcpy(data + 44, &state, 4);
		SendPacketRaw(4, data, 0x38u, 0, peer, 1);
		free(data);
	}

	void SendPing()
	{
		BYTE *data = new BYTE[56];
		for (int i = 0; i < 56; i++)
		{
			data[i] = 0;
		}
		BYTE twentytwo = 22;
		memcpy(data + 0, &twentytwo, 1);
		SendPacketRaw(4, data, 56, 0, peer, 1);
		free(data);
	}

	/*************** sender sutff **************/

	int GetMessageTypeFromPacket(ENetPacket *packet)
	{
		int result;

		if (packet->dataLength > 3u)
		{
			result = *(packet->data);
		}
		else
		{
			cout << "Bad packet length, ignoring message" << endl;
			result = 0;
		}
		return result;
	}

	char *GetTextPointerFromPacket(ENetPacket *packet)
	{
		char zero = 0;
		memcpy(packet->data + packet->dataLength - 1, &zero, 1);
		return (char *)(packet->data + 4);
	}

	void handlevarlist(uint8_t *data)
	{
		variantlist_t varlist{};
		varlist.serialize_from_mem(data+56);
		printf("varlist: %s\n", varlist.print().c_str());

		 auto action = varlist[0].get_string();
		 if (action == "OnSendToServer")
		{
			OnSendToServer(varlist[4].get_string(), varlist[1].get_int32(), varlist[3].get_int32(), varlist[2].get_int32());
		}
		else if (action == "OnConsoleMessage")
		{
			OnConsoleMessage(varlist[1].get_string());
		}else if (action == "OnRemove")
		{
			OnRemove(varlist[1].get_string());
		}
		else if (action == "OnSpawn")
		{
			OnSpawn(varlist[1].get_string());
		}else if (action == "OnTalkBubble")
		{
			OnTalkBubble(varlist[1].get_int32(), varlist[2].get_string(), 0);
		}
	}
 
	BYTE *GetExtendedDataPointerFromTankPacket(BYTE *a1)
	{
		return (BYTE *)(/*(*(int *)(a1 + 12) << 28 >> 31) & */ (int)(a1 + 56));
	}

	void HandlePacketTileChangeRequest(gameupdatepacket_t*data)
	{
		
	}

	struct WorldThingStruct
	{
	};

	struct WorldStruct
	{
		int XSize;
		int YSize;
		int tileCount;
		string name;
		__int16 *foreground;
		__int16 *background;
		WorldThingStruct *specials;
	};

	BYTE *packPlayerMoving(PlayerMoving *dataStruct)
	{
		BYTE *data = new BYTE[56];
		for (int i = 0; i < 56; i++)
		{
			data[i] = 0;
		}
		memcpy(data + 4, &dataStruct->netID, 4);
		memcpy(data + 12, &dataStruct->characterState, 4);
		memcpy(data + 20, &dataStruct->plantingTree, 4);
		memcpy(data + 24, &dataStruct->x, 4);
		memcpy(data + 28, &dataStruct->y, 4);
		memcpy(data + 32, &dataStruct->XSpeed, 4);
		memcpy(data + 36, &dataStruct->YSpeed, 4);
		memcpy(data + 44, &dataStruct->punchX, 4);
		memcpy(data + 48, &dataStruct->punchY, 4);
		return data;
	}

	PlayerMoving *unpackPlayerMoving(BYTE *data)
	{
		PlayerMoving *dataStruct = new PlayerMoving;
		memcpy(&dataStruct->netID, data + 4, 4);
		memcpy(&dataStruct->characterState, data + 12, 4);
		memcpy(&dataStruct->plantingTree, data + 20, 4);
		memcpy(&dataStruct->x, data + 24, 4);
		memcpy(&dataStruct->y, data + 28, 4);
		memcpy(&dataStruct->XSpeed, data + 32, 4);
		memcpy(&dataStruct->YSpeed, data + 36, 4);
		memcpy(&dataStruct->punchX, data + 44, 4);
		memcpy(&dataStruct->punchY, data + 48, 4);
		return dataStruct;
	}

	WorldStruct *world = NULL;

	//ProcessTankUpdatePacket(0, NULL, tankUpdatePacket);
	void ProcessTankUpdatePacket(float someVal, BYTE *structPointer)
	{
		
		auto gamepacket = reinterpret_cast<gameupdatepacket_t*>(structPointer);
		
		
		
		switch (gamepacket->m_type)
		{
				
                case 0: // AvatarPacketReceiver::LerpState
		{
			if(gamepacket->m_player_flags != owner) return;
			 SendPacketNew(4,(uint8_t *)&gamepacket, sizeof(gameupdatepacket_t));
			break;
		}
				
		case 1:
		{
			handlevarlist(structPointer);
			break;
		}
				
					 
		case 3:
			dbgPrint("Destroyed/placed tile might be tree also!!! TODO!!!"); 
			break;
				
		 case 5:
		{
			BYTE *tileData = GetExtendedDataPointerFromTankPacket(structPointer);
			dbgPrint("Update of blocks visuals..");
		}
				
		case 8:
		{
			HandlePacketTileChangeRequest(gamepacket);
			//cout << *(int*)(structPointer + 4) << endl;
			break;
		}
		case 9:
		{
			BYTE *playerItems = GetExtendedDataPointerFromTankPacket(structPointer);
			dbgPrint("Update player inventory..");
			break;
		}

		 case 12:
		{
			string x = std::to_string(*(int *)(structPointer + 44));
			string y = std::to_string(*(int *)(structPointer + 48));
			cout << "Some tile change at X:" + x + " Y: " + y << endl;
			break;
		}
				
				 
				
		case 14:
		{
			dbgPrint("Object change request! TODO!!!!!!"); 
		}
			 
		case 16:
		{
			BYTE *itemsData = GetExtendedDataPointerFromTankPacket(structPointer);
			__int16 val1 = *(__int16 *)itemsData;
			int itemsCount = *(int *)(itemsData + 2);
			break;
		}
		 
		case 18:
		{
			AtAvatarSetIconState(*(int *)(structPointer + 4), *(int *)(structPointer + 44));
			break;
		}
		case 20:
		{
		 
			dbgPrint("Set character state"); 
			break;
		}
		 
		 case 22:
		{
			dbgPrint("Ping reply found!");
			break;
		}
	 
		case 4:
		{
			//BYTE *worldPtr = GetExtendedDataPointerFromTankPacket(structPointer); // World::LoadFromMem
			BYTE *worldPtr = structPointer + 60; // 4 + 56
			world = new WorldStruct; 
			worldPtr += 6;
			__int16 strLen = *(__int16 *)worldPtr;
			worldPtr += 2;
			world->name = "";
			for (int i = 0; i < strLen; i++)
			{
				world->name += worldPtr[0];
				worldPtr++;
			}
			world->XSize = *(int *)worldPtr;
			worldPtr += 4;
			world->YSize = *(int *)worldPtr;
			worldPtr += 4;
			world->tileCount = *(int *)worldPtr;
			worldPtr += 4;
			 
			cout << "World " + std::to_string(world->XSize) + "x" + std::to_string(world->YSize) + " with name " + world->name << endl;
			currentWorld = world->name;
			break;
		}
		 
		default:
			dbgPrint("Unknown tank update packet type " + std::to_string(*(char *)structPointer));
			break;
		}
	}

	BYTE *GetStructPointerFromTankPacket(ENetPacket *packet)
	{
		unsigned int packetLenght = packet->dataLength;
		BYTE *result = NULL;
		if (packetLenght >= 0x3C)
		{
			BYTE *packetData = packet->data;
			result = packetData + 4;

			if (*(BYTE *)(packetData + 16) & 8)
			{
				cout << "Is zero!" << *(int *)(packetData + 56) << endl;
				if (packetLenght < *(int *)(packetData + 56) + 60)
				{
					cout << "Packet too small for extended packet to be valid" << endl;
					cout << "Sizeof float is 4.  TankUpdatePacket size: 56" << endl;
					result = 0;
				}
			}
			else
			{
				int zero = 0;
				memcpy(packetData + 56, &zero, 4);
			}
		}
		return result;
	}

	void ProcessPacket(ENetEvent *event, ENetPeer *peer)
	{
		int messageType = GetMessageTypeFromPacket(event->packet);
		//cout << "Packet type is " << messageType << endl;
		//cout << (event->packet->data+4) << endl;
		switch (messageType)
		{
		case 1:
			onLoginRequested();
			break;
		default:
			packet_unknown(event->packet);
			break;
		case 3:
			packet_type3(GetTextPointerFromPacket(event->packet));
			break;
		case 4:
		{
			BYTE *tankUpdatePacket = GetStructPointerFromTankPacket(event->packet);
			if (tankUpdatePacket)
			{
				//cout << std::hex << (char*)(tankUpdatePacket+4) << endl;
				/*v12 = (EntityComponent *)GetGameLogic();
				GameLogicComponent::ProcessTankUpdatePacket(a1, v12, tankUpdatePacket);*/
				//cout << "Size: " << event->packet->dataLength << endl;
				ProcessTankUpdatePacket(0, tankUpdatePacket);
			}

			else
			{
				cout << "Got bad tank packet";
			}
			/*char buffer[2048];
			for (int i = 0; i < event->packet->dataLength; i++)
			{
			sprintf(&buffer[2 * i], "%02X", event->packet->data[i]);
			}
			cout << buffer;*/
		}
		break;
		case 5:

			break;
		case 6:
			packet_type6(GetTextPointerFromPacket(event->packet));
			break;
		}
	}

	void eventLoop()
	{
		ENetEvent event;
		while (enet_host_service(client, &event, 0) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_NONE:
				cout << "No event???" << endl;
				break;
			case ENET_EVENT_TYPE_CONNECT:
				WhenConnected();
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				WhenDisconnected();
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				ProcessPacket(&event, peer);
				enet_packet_destroy(event.packet);
				break;
			default:
				cout << "WTF???" << endl;
				break;
			}
		}
		userLoop();
	}
};

void init()
{
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		exit(0);
	}
	atexit(enet_deinitialize);
	srand(time(NULL));
}
