/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND 
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, SEMTECH SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 * 
 * Original Copyright (C) SEMTECH S.A.
 * Modified Copyright (C) 2020 AIIT XUOS Lab
 */
/*! 
 * \file       sx1276-LoRa.c
 * \brief      SX1276 RF chip driver mode LoRa
 *
 * \version    2.0.0 
 * \date       Nov 21 2012
 * \author     Miguel Luis
 *
 * Last modified by Miguel Luis on Jun 19 2013
 */
/*************************************************
File name: sx1276-LoRa.c
Description: support aiit board configure and register function
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. replace original macro and basic date type with AIIT XUOS Lab's own defination
*************************************************/

#include <string.h>

#include "platform.h"

#if defined( USE_SX1276_RADIO )
#include "radio.h"
#include "sx1276-Hal.h"
#include "sx1276.h"
#include "sx1276-LoRaMisc.h"
#include "sx1276-LoRa.h"

#define LoRa_FREQENCY                              433000000

#define RSSI_OFFSET_LF                              -155.0
#define RSSI_OFFSET_HF                              -150.0

#define NOISE_ABSOLUTE_ZERO                         -174.0
#define NOISE_FIGURE_LF                                4.0
#define NOISE_FIGURE_HF                                6.0 

volatile uint32 TickCounter = 0; 

uint32 Tx_Time_Start,Tx_Time_End;   
uint32 Rx_Time_Start,Rx_Time_End;
//Signal bandwidth, used to calculate RSSI
const double SignalBwLog[] =
{
    3.8927900303521316335038277369285,  // 7.8 kHz
    4.0177301567005500940384239336392,  // 10.4 kHz
    4.193820026016112828717566631653,   // 15.6 kHz
    4.31875866931372901183597627752391, // 20.8 kHz
    4.4948500216800940239313055263775,  // 31.2 kHz
    4.6197891057238405255051280399961,  // 41.6 kHz
    4.795880017344075219145044421102,   // 62.5 kHz
    5.0969100130080564143587833158265,  // 125 kHz
    5.397940008672037609572522210551,   // 250 kHz
    5.6989700043360188047862611052755   // 500 kHz
};

//These values need testing
const double RssiOffsetLF[] =
{   
    -155.0,
    -155.0,
    -155.0,
    -155.0,
    -155.0,
    -155.0,
    -155.0,
    -155.0,
    -155.0,
    -155.0,
};

//These values need testing
const double RssiOffsetHF[] =
{  
    -150.0,
    -150.0,
    -150.0,
    -150.0,
    -150.0,
    -150.0,
    -150.0,
    -150.0,
    -150.0,
    -150.0,
};

/*!
 * Frequency hopping frequencies table
 */
const int32_t HoppingFrequencies[] =
{
    916500000,
    923500000,
    906500000,
    917500000,
    917500000,
    909000000,
    903000000,
    916000000,
    912500000,
    926000000,
    925000000,
    909500000,
    913000000,
    918500000,
    918500000,
    902500000,
    911500000,
    926500000,
    902500000,
    922000000,
    924000000,
    903500000,
    913000000,
    922000000,
    926000000,
    910000000,
    920000000,
    922500000,
    911000000,
    922000000,
    909500000,
    926000000,
    922000000,
    918000000,
    925500000,
    908000000,
    917500000,
    926500000,
    908500000,
    916000000,
    905500000,
    916000000,
    903000000,
    905000000,
    915000000,
    913000000,
    907000000,
    910000000,
    926500000,
    925500000,
    911000000,
};

// Default settings
tLoRaSettings LoRaSettings =
{
    LoRa_FREQENCY ,   // RFFrequency
    20,               // Power
    9,                // SignalBw [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved] 
    12,                // SpreadingFactor [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096  chips]
    2,                // ErrorCoding [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
    true,             // CrcOn [0: OFF, 1: ON]
    false,            // ImplicitHeaderOn [0: OFF, 1: ON]
    0,                // RxSingleOn [0: Continuous, 1 Single]
    0,                // FreqHopOn [0: OFF, 1: ON]
    4,                // HopPeriod Hops every frequency hopping period symbols
    1000,             // TxPacketTimeout
    1000,             // RxPacketTimeout
    128,              // PayloadLength (used for implicit header mode)
};

/*!
 * SX1276 LoRa registers variable
 */
tSX1276LR* SX1276LR;

/*!
 * Local RF buffer for communication support
 */
static uint8_t RFBuffer[RF_BUFFER_SIZE];

/*!
 * RF state machine variable
 */
static uint8_t RFLRState = RFLR_STATE_IDLE;

/*!
 * Rx management support variables
 */
static uint16_t RxPacketSize = 0;     
static int8_t RxPacketSnrEstimate;    
static double RxPacketRssiValue;      
static uint8_t RxGain = 1;            
static uint32_t RxTimeoutTimer = 0;  

/*!
 * PacketTimeout Stores the Rx window time value for packet reception
 */ 
static uint32_t PacketTimeout;  

/*!
 * Tx management support variables
 */
static uint16_t TxPacketSize = 0;     


void SX1276LoRaInit( void )
{
	RFLRState = RFLR_STATE_IDLE;                                     

	SX1276LoRaSetDefaults();                                         
	
    
	SX1276ReadBuffer( REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1 );     
    
    
    //SX1276LoRaSetOpMode( RFLR_OPMODE_SLEEP );
	
    SX1276LR->RegLna = RFLR_LNA_GAIN_G1;                             
    SX1276WriteBuffer( REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1 );    
	
    // set the RF settings 
    SX1276LoRaSetRFFrequency( LoRaSettings.RFFrequency );            
    SX1276LoRaSetSpreadingFactor( LoRaSettings.SpreadingFactor );    
    SX1276LoRaSetErrorCoding( LoRaSettings.ErrorCoding );            
    SX1276LoRaSetPacketCrcOn( LoRaSettings.CrcOn );                  
    SX1276LoRaSetSignalBandwidth( LoRaSettings.SignalBw );           
    SX1276LoRaSetImplicitHeaderOn( LoRaSettings.ImplicitHeaderOn );  
	
    SX1276LoRaSetSymbTimeout(0x3FF);                                 
    SX1276LoRaSetPayloadLength( LoRaSettings.PayloadLength );        
    SX1276LoRaSetLowDatarateOptimize( true );                        

    #if( ( MODULE_SX1276RF1IAS == 1 ) || ( MODULE_SX1276RF1KAS == 1 ) )
        if( LoRaSettings.RFFrequency > 860000000 )                   
        {
            SX1276LoRaSetPAOutput( RFLR_PACONFIG_PASELECT_RFO );     
            SX1276LoRaSetPa20dBm( false );                           
            LoRaSettings.Power = 14;                                 
            SX1276LoRaSetRFPower( LoRaSettings.Power );              
        }
        else                                                         
        {
			//SX1276Write( REG_LR_OCP, 0x3f );
            SX1276LoRaSetPAOutput( RFLR_PACONFIG_PASELECT_PABOOST ); 
            SX1276LoRaSetPa20dBm( true );                            
            LoRaSettings.Power = 20;                                 
            SX1276LoRaSetRFPower( LoRaSettings.Power );                     
        } 
    #elif( MODULE_SX1276RF1JAS == 1 )	
		if( LoRaSettings.RFFrequency > 380000000 )                   
		{
			SX1276LoRaSetPAOutput( RFLR_PACONFIG_PASELECT_PABOOST ); 
			SX1276LoRaSetPa20dBm( true );                            
			LoRaSettings.Power = 20;                                 
			SX1276LoRaSetRFPower( LoRaSettings.Power );                
		}
		else
		{
			SX1276LoRaSetPAOutput( RFLR_PACONFIG_PASELECT_RFO );
			SX1276LoRaSetPa20dBm( false );
			LoRaSettings.Power = 14;
			SX1276LoRaSetRFPower( LoRaSettings.Power );
		} 
	#endif
    SX1276LoRaSetOpMode( RFLR_OPMODE_STANDBY );	                      
}


void SX1276LoRaSetDefaults( void )
{
    // REMARK: See SX1276 datasheet for modified default values.

    // Sets IF frequency selection manual
    SX1276Read( REG_LR_VERSION, &SX1276LR->RegVersion );
}



void SX1276LoRaReset( void )
{
    uint32_t startTick;
	
	SX1276SetReset( RADIO_RESET_ON );
    
	// Wait 1ms
    startTick = GET_TICK_COUNT( );                                    
    while( ( GET_TICK_COUNT( ) - startTick ) < TICK_RATE_MS( 1 ) );    

    SX1276SetReset( RADIO_RESET_OFF );
    
	// Wait 6ms
    startTick = GET_TICK_COUNT( );                                    
    while( ( GET_TICK_COUNT( ) - startTick ) < TICK_RATE_MS( 6 ) );    
}

void SX1276LoRaSetOpMode( uint8_t opMode )
{
    static uint8_t opModePrev = RFLR_OPMODE_STANDBY;
    static bool antennaSwitchTxOnPrev = true;
    bool antennaSwitchTxOn = false;                                  
    opModePrev = SX1276LR->RegOpMode & ~RFLR_OPMODE_MASK;
    if( opMode != opModePrev )                                       
    {
        if( opMode == RFLR_OPMODE_TRANSMITTER )                      
		{
			antennaSwitchTxOn = true;
		}
		else
		{
			antennaSwitchTxOn = false;
		}
		if( antennaSwitchTxOn != antennaSwitchTxOnPrev )
		{
			antennaSwitchTxOnPrev = antennaSwitchTxOn;	// Antenna switch control
			RXTX( antennaSwitchTxOn );                                
		}
		SX1276LR->RegOpMode = ( SX1276LR->RegOpMode & RFLR_OPMODE_MASK ) | opMode;
		
		SX1276Write( REG_LR_OPMODE, SX1276LR->RegOpMode );        
	}
}


uint8_t SX1276LoRaGetOpMode( void )                       
{
    SX1276Read( REG_LR_OPMODE, &SX1276LR->RegOpMode );
    
    return SX1276LR->RegOpMode & ~RFLR_OPMODE_MASK;
}


uint8_t SX1276LoRaReadRxGain( void )                    
{
	
	SX1276Read( REG_LR_LNA, &SX1276LR->RegLna );
	return( SX1276LR->RegLna >> 5 ) & 0x07;
}


double SX1276LoRaReadRssi( void )
{  
	// Reads the RSSI value
    SX1276Read( REG_LR_RSSIVALUE, &SX1276LR->RegRssiValue );

    if( LoRaSettings.RFFrequency < 860000000 )  
    {
        return RssiOffsetLF[LoRaSettings.SignalBw] + ( double )SX1276LR->RegRssiValue;
    }
    else
    {
        return RssiOffsetHF[LoRaSettings.SignalBw] + ( double )SX1276LR->RegRssiValue;
    }
}


uint8_t SX1276LoRaGetPacketRxGain( void )
{
    return RxGain;
}


int8_t SX1276LoRaGetPacketSnr( void )
{
    return RxPacketSnrEstimate;
}


double SX1276LoRaGetPacketRssi( void )
{
    return RxPacketRssiValue;
}


void SX1276LoRaStartRx( void )
{
    SX1276LoRaSetRFState( RFLR_STATE_RX_INIT );
}


void SX1276LoRaGetRxPacket( void *buffer, uint16_t *size )
{
	*size = RxPacketSize;
	RxPacketSize = 0;
	memcpy( (void*)buffer, (void*)RFBuffer, (size_t)*size );
}


void SX1276LoRaSetTxPacket( const void *buffer, uint16_t size )
{
    if( LoRaSettings.FreqHopOn == false )    
    {
        TxPacketSize = size;
    }
    else
    {
        TxPacketSize = 255;
    }
    memcpy( ( void * )RFBuffer, buffer, ( size_t )TxPacketSize );  

    RFLRState = RFLR_STATE_TX_INIT;                                
}


uint8_t SX1276LoRaGetRFState( void )
{
    return RFLRState;
}


void SX1276LoRaSetRFState( uint8_t state )
{
    RFLRState = state;
}

/*!
 * \brief Process the LoRa modem Rx and Tx state machines depending on the
 *        SX1276 operating mode.
 *
 * \retval rfState Current RF state [RF_IDLE, RF_BUSY, 
 *                                   RF_RX_DONE, RF_RX_TIMEOUT,
 *                                   RF_TX_DONE, RF_TX_TIMEOUT]
 */
uint32_t SX1276LoRaProcess( void )
{
    uint32_t result = RF_BUSY;
    uint8_t regValue=0;
    switch( RFLRState )
    {
		case RFLR_STATE_IDLE:                                                 
             break;
        case RFLR_STATE_RX_INIT:                                               
			 SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);                         
		     
		     SX1276LR->RegIrqFlagsMask = RFLR_IRQFLAGS_RXTIMEOUT          |    
									   
									     RFLR_IRQFLAGS_PAYLOADCRCERROR    |    
									     RFLR_IRQFLAGS_VALIDHEADER        |
									     RFLR_IRQFLAGS_TXDONE             |
									     RFLR_IRQFLAGS_CADDONE            |
                                         RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |    
									     RFLR_IRQFLAGS_CADDETECTED;
		     SX1276Write( REG_LR_IRQFLAGSMASK, SX1276LR->RegIrqFlagsMask );    

		     if(LoRaSettings.FreqHopOn == true )                               
		     {
		         SX1276LR->RegHopPeriod = LoRaSettings.HopPeriod;              
		       
                 SX1276Read( REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel );    
			     SX1276LoRaSetRFFrequency( HoppingFrequencies[SX1276LR->RegHopChannel & RFLR_HOPCHANNEL_CHANNEL_MASK] );
		     }
		     else    
		     { 
                 SX1276LR->RegHopPeriod = 255;
		     } 
		     
		     SX1276Write( REG_LR_HOPPERIOD, SX1276LR->RegHopPeriod );  
			
		     
                                    // RxDone     
		     SX1276LR->RegDioMapping1 = RFLR_DIOMAPPING1_DIO0_00 | RFLR_DIOMAPPING1_DIO1_00 | RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_00;
                                    // CadDetected   
		     SX1276LR->RegDioMapping2 = RFLR_DIOMAPPING2_DIO4_10 | RFLR_DIOMAPPING2_DIO5_00;
		
		     SX1276WriteBuffer( REG_LR_DIOMAPPING1, &SX1276LR->RegDioMapping1, 2 );
			
		     if( LoRaSettings.RxSingleOn == true )        // Rx single mode                       
		     {
			     SX1276LoRaSetOpMode( RFLR_OPMODE_RECEIVER_SINGLE );
		     }
	         else                        // Rx continuous mode                                        
		     {
			     SX1276LR->RegFifoAddrPtr = SX1276LR->RegFifoRxBaseAddr;
			     
			     SX1276Write( REG_LR_FIFOADDRPTR, SX1276LR->RegFifoAddrPtr );    
			     SX1276LoRaSetOpMode( RFLR_OPMODE_RECEIVER );
				 
		     }   
		     memset( RFBuffer, 0, ( size_t )RF_BUFFER_SIZE );                    
             Rx_Time_Start=TickCounter;
		     PacketTimeout = LoRaSettings.RxPacketTimeout;                       
		     RxTimeoutTimer = GET_TICK_COUNT( );
		     RFLRState = RFLR_STATE_RX_RUNNING;
		     break;
		case RFLR_STATE_RX_RUNNING:                                              
			 SX1276Read(0x12,&regValue);                                         
             //if( DIO0 == 1 )         // RxDone
			 if(regValue & (1<<6))                                               
             {
				 
		         RxTimeoutTimer = GET_TICK_COUNT( );
                 if( LoRaSettings.FreqHopOn == true )                             
                 { 
				     SX1276Read( REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel );
					 SX1276LoRaSetRFFrequency( HoppingFrequencies[SX1276LR->RegHopChannel & RFLR_HOPCHANNEL_CHANNEL_MASK] );
				 }
				 // Clear Irq
				 SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_RXDONE  );           
				 RFLRState = RFLR_STATE_RX_DONE;
			  }
			  //if( DIO2 == 1 )         // FHSS Changed Channel
			  if(regValue & (1<<1))                                               
			  {
			      RxTimeoutTimer = GET_TICK_COUNT( );
			      if( LoRaSettings.FreqHopOn == true )
				  {
					  SX1276Read( REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel );
					  SX1276LoRaSetRFFrequency( HoppingFrequencies[SX1276LR->RegHopChannel & RFLR_HOPCHANNEL_CHANNEL_MASK] );
				  }
					// Clear Irq
				  SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL ); 
				  //RxGain = SX1276LoRaReadRxGain( );
			  }
			  if( LoRaSettings.RxSingleOn == true )    // Rx single mode                             
			  {
			     if( ( GET_TICK_COUNT( ) - RxTimeoutTimer ) > PacketTimeout )       
				 {
				      RFLRState = RFLR_STATE_RX_TIMEOUT;                            
				 }
			 }
             break;
		case RFLR_STATE_RX_DONE:                                                    
			                                                                        
			 																		
		     SX1276Read( REG_LR_IRQFLAGS, &SX1276LR->RegIrqFlags );
			 if( ( SX1276LR->RegIrqFlags & RFLR_IRQFLAGS_PAYLOADCRCERROR ) == RFLR_IRQFLAGS_PAYLOADCRCERROR )   
			 {      
				 // Clear Irq
				 SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_PAYLOADCRCERROR );     
				 if( LoRaSettings.RxSingleOn == true ) // Rx single mode                             
				 {
					RFLRState = RFLR_STATE_RX_INIT;                                
				 }
				 else
				 { 
					RFLRState = RFLR_STATE_RX_RUNNING;
				 }
				 break;
		     }
		     
/*			 {
				 uint8_t rxSnrEstimate;
				 
				 SX1276Read( REG_LR_PKTSNRVALUE, &rxSnrEstimate );                     
				 if( rxSnrEstimate & 0x80 )  
				 {

					 RxPacketSnrEstimate = ( ( ~rxSnrEstimate + 1 ) & 0xFF ) >> 2;     
					 RxPacketSnrEstimate = -RxPacketSnrEstimate;
				 }
				 else
				 {
					 RxPacketSnrEstimate = ( rxSnrEstimate & 0xFF ) >> 2;
				 }
			}        
			if( LoRaSettings.RFFrequency < 860000000 ) 
			{    
				if( RxPacketSnrEstimate < 0 )          
				{
					
					RxPacketRssiValue = NOISE_ABSOLUTE_ZERO + 10.0 * SignalBwLog[LoRaSettings.SignalBw] + NOISE_FIGURE_LF + ( double )RxPacketSnrEstimate;
				}
				else
				{    
					SX1276Read( REG_LR_PKTRSSIVALUE, &SX1276LR->RegPktRssiValue );
					RxPacketRssiValue = RssiOffsetLF[LoRaSettings.SignalBw] + ( double )SX1276LR->RegPktRssiValue;
				}
			}
			else                                        
			{    
				if( RxPacketSnrEstimate < 0 )
				{
					RxPacketRssiValue = NOISE_ABSOLUTE_ZERO + 10.0 * SignalBwLog[LoRaSettings.SignalBw] + NOISE_FIGURE_HF + ( double )RxPacketSnrEstimate;
				}
				else
				{    
					SX1276Read( REG_LR_PKTRSSIVALUE, &SX1276LR->RegPktRssiValue );
					RxPacketRssiValue = RssiOffsetHF[LoRaSettings.SignalBw] + ( double )SX1276LR->RegPktRssiValue;
				}
			}*/
			if( LoRaSettings.RxSingleOn == true )     // Rx single mode                                
			{
			    SX1276LR->RegFifoAddrPtr = SX1276LR->RegFifoRxBaseAddr;       
				SX1276Write( REG_LR_FIFOADDRPTR, SX1276LR->RegFifoAddrPtr );          
			    if( LoRaSettings.ImplicitHeaderOn == true )                           
				{
				    RxPacketSize = SX1276LR->RegPayloadLength;
				    SX1276ReadFifo( RFBuffer, SX1276LR->RegPayloadLength );           
				}
				else	
				{
					
				    SX1276Read( REG_LR_NBRXBYTES, &SX1276LR->RegNbRxBytes );
					RxPacketSize = SX1276LR->RegNbRxBytes;
				    SX1276ReadFifo( RFBuffer, SX1276LR->RegNbRxBytes );
			     }
			 }
			 else     // Rx continuous mode                                                                
			 {
			     
				 SX1276Read( REG_LR_FIFORXCURRENTADDR, &SX1276LR->RegFifoRxCurrentAddr );
				 if( LoRaSettings.ImplicitHeaderOn == true )
				 {
					 RxPacketSize = SX1276LR->RegPayloadLength;
					 SX1276LR->RegFifoAddrPtr = SX1276LR->RegFifoRxCurrentAddr;
					 SX1276Write( REG_LR_FIFOADDRPTR, SX1276LR->RegFifoAddrPtr );
				     SX1276ReadFifo( RFBuffer, SX1276LR->RegPayloadLength );
				 }
				 else
				 {
					 SX1276Read( REG_LR_NBRXBYTES, &SX1276LR->RegNbRxBytes );
					 RxPacketSize = SX1276LR->RegNbRxBytes;
					 SX1276LR->RegFifoAddrPtr = SX1276LR->RegFifoRxCurrentAddr;
					 SX1276Write( REG_LR_FIFOADDRPTR, SX1276LR->RegFifoAddrPtr );
					 SX1276ReadFifo( RFBuffer, SX1276LR->RegNbRxBytes );
				 }
			 }  
			 if( LoRaSettings.RxSingleOn == true )   // Rx single mode                                  
			 { 
				 RFLRState = RFLR_STATE_RX_INIT;
			 }
			 else  // Rx continuous mode                                                                    
			 { 
				 RFLRState = RFLR_STATE_RX_RUNNING;
			 }
			 Rx_Time_End=TickCounter;
			 result = RF_RX_DONE;
			 break;
		case RFLR_STATE_RX_TIMEOUT:                                                   
		     RFLRState = RFLR_STATE_RX_INIT;                                          
		     result = RF_RX_TIMEOUT;
		     break;
		case RFLR_STATE_TX_INIT:                                                      
																					  
		     Tx_Time_Start=TickCounter;
			 SX1276LoRaSetOpMode( RFLR_OPMODE_STANDBY );                              
			 if( LoRaSettings.FreqHopOn == true )                                     
			 {
				 SX1276LR->RegIrqFlagsMask = RFLR_IRQFLAGS_RXTIMEOUT          |
											 RFLR_IRQFLAGS_RXDONE             |
											 RFLR_IRQFLAGS_PAYLOADCRCERROR    |
											 RFLR_IRQFLAGS_VALIDHEADER        |
										   //RFLR_IRQFLAGS_TXDONE             |
											 RFLR_IRQFLAGS_CADDONE            |
										   //RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
											 RFLR_IRQFLAGS_CADDETECTED;
				 SX1276LR->RegHopPeriod = LoRaSettings.HopPeriod;
				 SX1276Read( REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel );
				 SX1276LoRaSetRFFrequency( HoppingFrequencies[SX1276LR->RegHopChannel & RFLR_HOPCHANNEL_CHANNEL_MASK] );
			 }
			 else
			 {
				 
				 SX1276LR->RegIrqFlagsMask = RFLR_IRQFLAGS_RXTIMEOUT          |
									         RFLR_IRQFLAGS_RXDONE             |
											 RFLR_IRQFLAGS_PAYLOADCRCERROR    |
											 RFLR_IRQFLAGS_VALIDHEADER        |
										   //RFLR_IRQFLAGS_TXDONE             |
											 RFLR_IRQFLAGS_CADDONE            |
											 RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
											 RFLR_IRQFLAGS_CADDETECTED;             
				 SX1276LR->RegHopPeriod = 0;
			 }
			 SX1276Write( REG_LR_HOPPERIOD, SX1276LR->RegHopPeriod );                
			 SX1276Write( REG_LR_IRQFLAGSMASK, SX1276LR->RegIrqFlagsMask );      
			 // Initializes the payload size
			 SX1276LR->RegPayloadLength = TxPacketSize;                              
			 SX1276Write( REG_LR_PAYLOADLENGTH, SX1276LR->RegPayloadLength );
			
			 SX1276LR->RegFifoTxBaseAddr = 0x00;  // Full buffer used for Tx                                   
			 SX1276Write( REG_LR_FIFOTXBASEADDR, SX1276LR->RegFifoTxBaseAddr );

			 SX1276LR->RegFifoAddrPtr = SX1276LR->RegFifoTxBaseAddr;
			 SX1276Write( REG_LR_FIFOADDRPTR, SX1276LR->RegFifoAddrPtr );
			  // Write payload buffer to LORA modem
			 SX1276WriteFifo( RFBuffer, SX1276LR->RegPayloadLength );                
			 								// TxDone               RxTimeout                   FhssChangeChannel          ValidHeader         
			 SX1276LR->RegDioMapping1 = RFLR_DIOMAPPING1_DIO0_01 | RFLR_DIOMAPPING1_DIO1_00 | RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_01;
											// PllLock              Mode Ready
			 SX1276LR->RegDioMapping2 = RFLR_DIOMAPPING2_DIO4_01 | RFLR_DIOMAPPING2_DIO5_00;
			
			 SX1276WriteBuffer( REG_LR_DIOMAPPING1, &SX1276LR->RegDioMapping1, 2 );

			 SX1276LoRaSetOpMode( RFLR_OPMODE_TRANSMITTER );

			 RFLRState = RFLR_STATE_TX_RUNNING;
			 break;
		case RFLR_STATE_TX_RUNNING:                                            
			 SX1276Read(0x12,&regValue);
		     //if( DIO0 == 1 )    // TxDone
			 if(regValue & (1<<3))
			 {
		         // Clear Irq
				 SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_TXDONE );          
				 RFLRState = RFLR_STATE_TX_DONE;   
			 }	
			 //if( DIO2 == 1 )   // FHSS Changed Channel
			 if(regValue & (1<<3))
			 {
				 if( LoRaSettings.FreqHopOn == true )
				 {
					 SX1276Read( REG_LR_HOPCHANNEL, &SX1276LR->RegHopChannel );
					 SX1276LoRaSetRFFrequency( HoppingFrequencies[SX1276LR->RegHopChannel & RFLR_HOPCHANNEL_CHANNEL_MASK] );
				 }		
				 // Clear Irq		
				 SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL ); 
			 }
			 break;
		case RFLR_STATE_TX_DONE:                                                
             Tx_Time_End=TickCounter;			
			 SX1276LoRaSetOpMode( RFLR_OPMODE_STANDBY );                         
			 RFLRState = RFLR_STATE_IDLE;
			 result = RF_TX_DONE;
			 break;
		case RFLR_STATE_CAD_INIT:
		// optimize the power consumption by switching off the transmitter as soon as the packet has been sent                                               
			 SX1276LoRaSetOpMode( RFLR_OPMODE_STANDBY );
			 SX1276LR->RegIrqFlagsMask = RFLR_IRQFLAGS_RXTIMEOUT           |
										 RFLR_IRQFLAGS_RXDONE              |
										 RFLR_IRQFLAGS_PAYLOADCRCERROR     |
										 RFLR_IRQFLAGS_VALIDHEADER         |
										 RFLR_IRQFLAGS_TXDONE              |
									     //RFLR_IRQFLAGS_CADDONE             |
										 RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL;
									     //RFLR_IRQFLAGS_CADDETECTED;
			 SX1276Write( REG_LR_IRQFLAGSMASK, SX1276LR->RegIrqFlagsMask );

										 // RxDone                   RxTimeout                   FhssChangeChannel           CadDone
			 SX1276LR->RegDioMapping1 = RFLR_DIOMAPPING1_DIO0_00 | RFLR_DIOMAPPING1_DIO1_00 | RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_00;
										 // CAD Detected              ModeReady
			 SX1276LR->RegDioMapping2 = RFLR_DIOMAPPING2_DIO4_00 | RFLR_DIOMAPPING2_DIO5_00;
			 SX1276WriteBuffer( REG_LR_DIOMAPPING1, &SX1276LR->RegDioMapping1, 2 );


			 SX1276LoRaSetOpMode( RFLR_OPMODE_CAD );
			 RFLRState = RFLR_STATE_CAD_RUNNING;

			 break;
		case RFLR_STATE_CAD_RUNNING:
			 SX1276Read(0x12,&regValue);                                         
			 int cad_done = regValue & (1<<2);
			 int cad_detected = regValue & (1<<0);

			 if( cad_done  ) //CAD Done interrupt
			 { 
				SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_CADDONE  );
				if( cad_detected  ) // CAD Detected interrupt
				{
					SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_CADDETECTED  );
					//CAD detected, we have a LoRa preamble
					RFLRState = RFLR_STATE_RX_INIT;
					result = RF_CHANNEL_ACTIVITY_DETECTED;
                } 
				else
				{    
					// The device goes in Standby Mode automatically    
					RFLRState = RFLR_STATE_IDLE;
					result = RF_CHANNEL_EMPTY;
				}
			 }   
			 break;			
        default:
             break;
    } 
    return result;
}

uint32_t SX1276LoraChannelEmpty( void )
{
	uint32_t result = 0;
	RFLRState = RFLR_STATE_CAD_INIT;
	SX1276LoRaProcess();
    while(RFLRState == RFLR_STATE_CAD_RUNNING)
	{
		//KPrintf("\nLora--SX1276LoRaProcess()");
		result = SX1276LoRaProcess();
	}

	if(result == RF_CHANNEL_EMPTY)
	{
		KPrintf("\nLora--信道可用（RF_CHANNEL_EMPTY）\n");
		return 0;
	}
	else if(result == RF_CHANNEL_ACTIVITY_DETECTED)
	{
		KPrintf("\nLora--信道正被占用（RF_CHANNEL_ACTIVITY_DETECTED）\n");
		return 1;
	}
	else
	{
		return 2;
	}
}

#endif // USE_SX1276_RADIO