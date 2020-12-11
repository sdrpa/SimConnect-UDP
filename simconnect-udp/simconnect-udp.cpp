
#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

#include <SimConnect.h>

#include "util.h"
#include "simconnect-udp.h"

int quit = 0;
HANDLE hSimConnect = NULL;

AircraftPosition ac_position;

void setupDataDefinitions() {
    ASSERT_SC_SUCCESS(SimConnect_AddToDataDefinition(hSimConnect,
        DEFINITION_AIRCRAFT_POSITION,
        "PLANE HEADING DEGREES TRUE", "Radians"));
}

void setupDataRequests() {
    // Get position information for every frame
    ASSERT_SC_SUCCESS(
        SimConnect_RequestDataOnSimObject(hSimConnect,
            REQUEST_AIRCRAFT_POSITION,
            DEFINITION_AIRCRAFT_POSITION,
            SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SIM_FRAME, 0));
}

void broadcast() {
    printf("HEADING: %lf\n", degrees(ac_position.heading));
}


void CALLBACK SC_Dispatch_Handler(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext) {
    switch (pData->dwID) {

    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA: {
        SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData = reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA *>(pData);
        if (pObjData->dwRequestID == REQUEST_AIRCRAFT_POSITION) {
            //ac_position = *reinterpret_cast<AircraftPosition *>(&pObjData->dwData);

            //int numVars = dataDefinitions[pObjData->dwDefineID].num_values;
            //std::vector<SIMCONNECT_DATATYPE> valTypes = dataDefinitions[pObjData->dwDefineID].datum_types;
            //std::vector<std::string> valIds = dataDefinitions[pObjData->dwDefineID].datum_names;


            broadcast();
        }
        break;
    }

    case SIMCONNECT_RECV_ID_EVENT: {
        SIMCONNECT_RECV_EVENT* evt = (SIMCONNECT_RECV_EVENT*)pData;
        switch (evt->uEventID) {
        case EVENT_SIM_START: {
        }
                            break;

        default:
            break;
        }
        break;
    }

    case SIMCONNECT_RECV_ID_QUIT: {
        quit = 1;
        break;
    }

    default:
        printf("Received: %d", pData->dwID);
        break;
    }
}

void runLoop() {
    while (SimConnect_Open(&hSimConnect, "SimConnect UDP Broadcast", NULL, 0, 0, 0) != S_OK);
    printf("Connected\n");

    setupDataDefinitions();
    setupDataRequests();

    while (0 == quit) {
        SimConnect_CallDispatch(hSimConnect, SC_Dispatch_Handler, NULL);
    }
    SimConnect_Close(hSimConnect);
}

int main() {
    runLoop();
    return 0;
}
