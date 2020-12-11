
#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

#include <SimConnect.h>

#include "util.h"
#include "simconnect-udp.h"

HANDLE hSimConnect = NULL;
Aircraft aircraft;

sockaddr_in RecvAddr;
SOCKET SendSocket = INVALID_SOCKET;

const int UPDATES_PER_SECOND = 20;
const size_t BUFFER_SIZE = 77;

// MARK: -

void openUDPBroadcast(char* destAddress, int destPort) {
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != NO_ERROR) {
        printf("WSAStartup failed with error: %d\n", result);
        exit(1);
    }
    // Create a socket for sending data
    SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (SendSocket == INVALID_SOCKET) {
        printf("Socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        exit(1);
    }
    // Set up the RecvAddr
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(destPort);
    RecvAddr.sin_addr.s_addr = inet_addr(destAddress);
}

void broadcast(char buffer[BUFFER_SIZE]) {
    int result = sendto(SendSocket,
        buffer, BUFFER_SIZE, 0, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
    if (result == SOCKET_ERROR) {
        printf("sendto failed with error: %d\n", WSAGetLastError());
        closesocket(SendSocket);
        WSACleanup();
        exit(1);
    }
}

void closeUDPBrodcast() {
    int result = closesocket(SendSocket);
    if (result == SOCKET_ERROR) {
        printf("closesocket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
    }
    WSACleanup();
}

// MARK: -

void setupDataDefinitions() {
    ASSERT_SC_SUCCESS(
        SimConnect_AddToDataDefinition(hSimConnect,
            DEFINITION_AIRCRAFT_POSITION,
            "PLANE HEADING DEGREES MAGNETIC",
            "Radians")
    );
    ASSERT_SC_SUCCESS(
        SimConnect_AddToDataDefinition(hSimConnect,
            DEFINITION_AIRCRAFT_POSITION,
            "PLANE LATITUDE", 
            "Radians")
    );
    ASSERT_SC_SUCCESS(
        SimConnect_AddToDataDefinition(hSimConnect,
            DEFINITION_AIRCRAFT_POSITION,
            "PLANE LONGITUDE",
            "Radians")
    );
}

void setupDataRequests() {
    ASSERT_SC_SUCCESS(
        SimConnect_RequestDataOnSimObject(hSimConnect,
            REQUEST_AIRCRAFT_POSITION,
            DEFINITION_AIRCRAFT_POSITION,
            SIMCONNECT_OBJECT_ID_USER, 
            SIMCONNECT_PERIOD_SIM_FRAME)
    );
}

void valueCopy(char *destination, float value) {
    memcpy(destination, (char *)&value, sizeof(float));
}

// https://docs.microsoft.com/en-us/cpp/preprocessor/pack?view=msvc-160
#pragma pack(1)
struct AircraftStruct {
    char prologue[5] = { 68, 65, 84, 65, 42 };
    char row19[4] = { 19, 0, 0, 0 };
    float heading;
    float h2 = -999.0;
    float h3 = -999.0;
    float h4 = -999.0;
    float h5 = -999.0;
    float h6 = -999.0;
    float h7 = -999.0;
    float h8 = -999.0;
    char row20[4] = { 20, 0, 0, 0 };
    float latitude;
    float longitude;
    float l3 = -999.0;
    float l4 = -999.0;
    float l5 = -999.0;
    float l6 = -999.0;
    float l7 = -999.0;
    float l8 = -999.0;
};

void didReceiveObjectData(SIMCONNECT_RECV* pData, DWORD cbData) {
    SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData = reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA *>(pData);
    if (pObjData->dwRequestID == REQUEST_AIRCRAFT_POSITION) {
        aircraft = *reinterpret_cast<Aircraft*>(&pObjData->dwData);

        //printf("Aircraft: %lf %lf %lf\n", degrees(aircraft.heading), degrees(aircraft.latitude), degrees(aircraft.longitude));
        AircraftStruct a;
        a.heading = degrees(aircraft.heading);
        a.latitude = degrees(aircraft.latitude);
        a.longitude = degrees(aircraft.longitude);
        if (BUFFER_SIZE != sizeof(a)) {
            printf("BUFFER_SIZE != sizeof(a)");
            exit(1);
        }
        char buffer[BUFFER_SIZE];
        memcpy(buffer, (char*)&a, BUFFER_SIZE);
        //for (int i = 0; i < 77; i++) { printf("%d, ", buffer[i]); }

        broadcast(buffer);
    }
}

void didReceiveEvent(SIMCONNECT_RECV_EVENT *e) {
    switch (e->uEventID) {
    default:
        break;
    }
}

void didReceivedOpen() {
    printf("SimConnect Received Open\n");
}

void didReceivedQuit() {
    printf("SimConnect Received Quit\n");
    exit(0);
}

void CALLBACK SC_Dispatch_Handler(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext) {
    switch (pData->dwID) {
    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA: {
        didReceiveObjectData(pData, cbData);
        break;
    }
    case SIMCONNECT_RECV_ID_EVENT: {
        SIMCONNECT_RECV_EVENT *e = (SIMCONNECT_RECV_EVENT*)pData;
        didReceiveEvent(e);
        break;
    }
    case SIMCONNECT_RECV_ID_OPEN: {
        didReceivedOpen();
        break;
    }
    case SIMCONNECT_RECV_ID_QUIT: {
        didReceivedQuit();
        break;
    }
    default:
        printf("Unhandled: %d", pData->dwID);
        break;
    }
}

void runLoop() {
    printf("Waiting for MSFS...\n");
    while (SimConnect_Open(&hSimConnect, "SimConnect UDP Broadcast", NULL, 0, 0, 0) != S_OK);
    printf("Connected\n");

    setupDataDefinitions();
    setupDataRequests();

    while (true) {
        Sleep(1000 / UPDATES_PER_SECOND);
        SimConnect_CallDispatch(hSimConnect, SC_Dispatch_Handler, NULL);
    }
    SimConnect_Close(hSimConnect);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage:\n");
        printf("\tsimconnect-udp <IP_ADDRESS> <PORT>\n");
        return 1;
    }
    char *destAddress = argv[1];
    char *destPort = argv[2];
    printf("IP: %s Port: %d\n", destAddress, atoi(destPort));

    openUDPBroadcast(destAddress, atoi(destPort));
    runLoop();
    return 0;
}
