/*
    Client/Server Testbed

    Copyright © 2016, The Network Protocol Company, Inc.
    
    All rights reserved.
*/

#define CLIENT 1

#include "shared.h"
#include <signal.h>

static volatile int quit = 0;

void interrupt_handler( int /*dummy*/ )
{
    quit = 1;
}

int ClientMain()
{
    GamePacketFactory packetFactory;

//    Address serverAddress( "::1", ServerPort );

    Address clientAddress = GetFirstNetworkAddress_IPV4();

    if ( !clientAddress.IsValid() )
    {
        printf( "error: no valid IPV4 address\n" );
        return 1;
    }

    GameNetworkInterface clientInterface( packetFactory, clientAddress );

    if ( clientInterface.GetError() != SOCKET_ERROR_NONE )
    {
        printf( "error: failed to initialize client socket\n" );
        return 1;
    }
    
    char clientAddressString[64];
    clientInterface.GetAddress().ToString( clientAddressString, sizeof( clientAddressString ) );
    printf( "client started on %s\n", clientAddressString );

    clientInterface.SetFlags( NETWORK_INTERFACE_FLAG_INSECURE_MODE );

    GameClient client( clientInterface );

    Address serverAddress( "173.255.195.190", ServerPort );
    //Address serverAddress( "::1", ServerPort );

    client.InsecureConnect( serverAddress );

    double time = 0.0;

    const double deltaTime = 0.1;

    signal( SIGINT, interrupt_handler );    

    while ( !quit )
    {
        client.SendPackets();

        clientInterface.WritePackets();

        clientInterface.ReadPackets();

        client.ReceivePackets();

        client.CheckForTimeOut();

        if ( client.IsDisconnected() )
            break;

        time += deltaTime;

        client.AdvanceTime( time );

        clientInterface.AdvanceTime( time );

        if ( client.ConnectionFailed() )
            break;

        platform_sleep( deltaTime );
    }

    if ( quit )
        printf( "\nclient stopped\n" );

    if ( client.IsConnected() )
        client.Disconnect();

    return 0;
}

int main()
{
    printf( "\n" );

    if ( !InitializeYojimbo() )
    {
        printf( "error: failed to initialize Yojimbo!\n" );
        return 1;
    }

    srand( (unsigned int) time( NULL ) );

    int result = ClientMain();

    ShutdownYojimbo();

    printf( "\n" );

    return result;
}
