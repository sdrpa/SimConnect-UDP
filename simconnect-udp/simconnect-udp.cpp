
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

void setupDataDefinitions() {
    ASSERT_SC_SUCCESS(
        SimConnect_AddToDataDefinition(hSimConnect,
            DEFINITION_AIRCRAFT_POSITION,
            "PLANE HEADING DEGREES MAGNETIC", 
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

void didReceiveObjectData(SIMCONNECT_RECV* pData, DWORD cbData) {
    SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData = reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA *>(pData);
    if (pObjData->dwRequestID == REQUEST_AIRCRAFT_POSITION) {
        aircraft = *reinterpret_cast<Aircraft *>(&pObjData->dwData);

        //int numVars = dataDefinitions[pObjData->dwDefineID].num_values;
        //std::vector<SIMCONNECT_DATATYPE> valTypes = dataDefinitions[pObjData->dwDefineID].datum_types;
        //std::vector<std::string> valIds = dataDefinitions[pObjData->dwDefineID].datum_names;

        printf("HEADING: %lf\n", degrees(aircraft.heading));
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
    while (SimConnect_Open(&hSimConnect, "SimConnect UDP Broadcast", NULL, 0, 0, 0) != S_OK);
    printf("Connected\n");

    setupDataDefinitions();
    setupDataRequests();

    while (true) {
        SimConnect_CallDispatch(hSimConnect, SC_Dispatch_Handler, NULL);
    }
    SimConnect_Close(hSimConnect);
}

int main() {
    runLoop();
    return 0;
}
