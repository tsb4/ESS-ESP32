/* ---------------------------------------------------
        Radioenge Equipamentos de Telecomunicações
   ---------------------------------------------------
    This library contains a set of functions to configure
    and operate the EndDevice LoRaMESH Radioenge
  
  Date: 13/12/18
*/
#include "LoRaMESH.h"

/* Frame type */
typedef struct
{
  uint8_t buffer[MAX_BUFFER_SIZE];
  uint8_t size;
  bool command;
} Frame_Typedef;

/* ----- Private Variables ----- */
/* not used in ESP32
static SoftwareSerial* hSerialCommand = NULL;
static SoftwareSerial* hSerialTransp = NULL; */
static Frame_Typedef frame;
static uint16_t deviceId = -1;
static uint16_t deviceNet = -1;
static uint32_t deviceUniqueId = -1;


/* ----- Public Variables ----- */

/* ----- Private Functions ----- */

void SerialFlush()
{
  while(Serial1.available() > 0 || Serial2.available() > 0)
  {
    Serial1.read();
    Serial2.read();
  }
}



/**
  * @brief Private function - Gets the ID, NET and UNIQUE ID info from the local or remote device
  * @param idIn[in]: Remote device's id
  * @param idOut[out]: Local device's id
  * @param net[out]: Configured NET on local device
  * @param uniqueId[out]: Device Unique ID 
  * @retval MESH_OK or MESH_ERROR
  */
MeshStatus_Typedef LocalRemoteRead(uint16_t idIn, uint16_t* idOut, uint16_t* net, uint32_t* uniqueId)
{
  uint8_t crc = 0;
  uint8_t bufferPayload[31];
  uint8_t payloadSize;
  uint8_t i = 0;
  uint8_t command;
  uint16_t id = 0;
  
  /* Asserts parameters */
  if(net == NULL) return MESH_ERROR;
  if(uniqueId == NULL) return MESH_ERROR;
  
  /* Loads dummy bytes */
  for(i=0; i<3; i++)
    bufferPayload[i] = 0x00;
  
  /* Prepares frame for transmission */
  if((idOut == NULL) && (idIn < 1023)) /* Remote Read */
  {
    PrepareFrameCommand(idIn, CMD_REMOTEREAD, &bufferPayload[0], i);
  }
  else if(idOut != NULL) /* Local read */
  {
    PrepareFrameCommand(0, CMD_LOCALREAD, &bufferPayload[0], i);
  }
  else return MESH_ERROR;
  
  /* Sends packet */
  SendPacket();
  
  /* Flush serial input buffer */
  SerialFlush();

  /* Waits for response */
  if( ReceivePacketCommand(&id, &command, &bufferPayload[0], &payloadSize, 5000) != MESH_OK)
    return MESH_ERROR;
  
  /* Checks command */
  if((command != CMD_REMOTEREAD) && (command != CMD_LOCALREAD))
    return MESH_ERROR;
  
  /* Stores the received data */
  if(idOut != NULL) /* Local read */
  {
    deviceId = id;
    *idOut = id;
  }
  *net = (uint16_t)bufferPayload[0] | ((uint16_t)(bufferPayload[1]) << 8);
  *uniqueId =  (uint32_t)bufferPayload[2] | 
              ((uint32_t)(bufferPayload[3]) << 8) |
              ((uint32_t)(bufferPayload[4]) << 16) |
              ((uint32_t)(bufferPayload[5]) << 24);
  
  return MESH_OK; 
}




/* ----- Public Function Definitions ----- */

MeshStatus_Typedef Remote_RSSI(uint16_t id_intermediario, uint16_t id_final, uint8_t *buffer){
  uint8_t remote_payload[6];
  
  uint16_t remote_id;
  uint8_t remote_cmd;
  uint8_t buffer_payload[11];
  uint8_t payload_size;

  remote_payload[0] = id_final&0xFF;
  remote_payload[1] = (id_final>>8)&0x03;
  remote_payload[2] = CMD_READRSSI;
  remote_payload[3] = 0;
  remote_payload[4] = 0;
  remote_payload[5] = 0;
  PrepareFrameCommand(id_intermediario, CMD_VIA, remote_payload, sizeof(remote_payload));
  SendPacket();
  MeshStatus_Typedef ret = ReceivePacketCommand(&remote_id, &remote_cmd, buffer_payload, &payload_size, 5000);
  buffer[0] = buffer_payload[5];
  buffer[1] = buffer_payload[6];
  return ret;
}



void SerialCommandsInit(uint32_t baudRate)
{
  /* filter not used baudrates */
  
  Serial1.begin(baudRate, SERIAL_8N1, 2, 4);

  /* Run local read */
  LocalRead(&deviceId, &deviceNet, &deviceUniqueId);

}



void SerialTranspInit(uint32_t baudRate)
{
  /* filter not used baudrates */
  
  Serial2.begin(baudRate, SERIAL_8N1, 16, 17);

}




MeshStatus_Typedef PrepareFrameCommand(uint16_t id, uint8_t command, uint8_t* payload, uint8_t payloadSize)
{
  if(payload == NULL) return MESH_ERROR;
  if(id > 1023) return MESH_ERROR;

  uint16_t crc = 0;

  frame.size = payloadSize + 5;
  
  /* Loads the target's ID */
  frame.buffer[0] = id&0xFF;
  frame.buffer[1] = (id>>8)&0x03;
  
  /* Loads the command */
  frame.buffer[2] = command;
  
  if((payloadSize >= 0) && (payloadSize < MAX_PAYLOAD_SIZE))
  {
    /* Loads the payload */
    memcpy(&(frame.buffer[3]), payload, payloadSize);
  
    /* Computes CRC */
    crc = ComputeCRC((&frame.buffer[0]), payloadSize+3);
    frame.buffer[payloadSize+3] = crc&0xFF;
    frame.buffer[payloadSize+4] = (crc>>8)&0xFF;
  }
  else
  {
    /* Invalid payload size */
    memset(&frame.buffer[0], 0, MAX_BUFFER_SIZE);
    return MESH_ERROR;
  }

  frame.command = true;

  return MESH_OK;
}




MeshStatus_Typedef PrepareFrameTransp(uint16_t id, uint8_t* payload, uint8_t payloadSize)
{
  uint8_t i = 0;

  if(payload == NULL) return MESH_ERROR;
  if(id > 1023) return MESH_ERROR;
  if(deviceId == -1) return MESH_ERROR;
  
  if((id != 0) && (deviceId == 0))  /* Is master */
  {
    frame.size = payloadSize + 2;
    /* Loads the target's ID */
    frame.buffer[i++] = id&0xFF;
    frame.buffer[i++] = (id>>8)&0x03;
  }
  else
  {
    frame.size = payloadSize;
  }
  
  if((payloadSize >= 0) && (payloadSize < MAX_PAYLOAD_SIZE))
  {
    /* Loads the payload */
    memcpy(&frame.buffer[i], payload, payloadSize);
  }
  else
  {
    /* Invalid payload size */
    memset(&frame.buffer[0], 0, MAX_BUFFER_SIZE);
    return MESH_ERROR;
  }

  frame.command = false;

  return MESH_OK;
}




MeshStatus_Typedef SendPacket()
{
  if(frame.size == 0) return MESH_ERROR;

  if(frame.command)
  {
    Serial1.write(frame.buffer, frame.size);
  }
  else
  {
    Serial2.write(frame.buffer, frame.size);
  }

  return MESH_OK;
}




MeshStatus_Typedef ReceivePacketCommand(uint16_t* id, uint8_t* command, uint8_t* payload, uint8_t* payloadSize, uint32_t timeout)
{
  uint16_t waitNextByte = 500;
  uint8_t i = 0;
  uint16_t crc = 0;
  
  /* Assert parameters */
  if(id == NULL) return MESH_ERROR;
  if(command == NULL) return MESH_ERROR;
  if(payload == NULL) return MESH_ERROR;
  if(payloadSize == NULL) return MESH_ERROR;
  
  /* Waits for reception */
  while( ((timeout > 0 ) || (i > 0)) && (waitNextByte > 0) )
  {
    if(Serial1.available() > 0)
    {
      frame.buffer[i++] = Serial1.read();
	    waitNextByte = 500;
    }
    
	  if(i > 0)
    {
      waitNextByte--;
    }
    timeout--;
    delay(1);
  }

  /* In case it didn't get any data */
  if((timeout == 0) && (i == 0)) return MESH_ERROR;
  
  /* Checks CRC16 */
  crc = (uint16_t)frame.buffer[i-2] | ((uint16_t)frame.buffer[i-1] << 8);
  if(ComputeCRC(&frame.buffer[0], i-2) != crc) return MESH_ERROR;

  /* Copies ID */
  *id = (uint16_t)frame.buffer[0] | ((uint16_t)frame.buffer[1] << 8);
  /* Copies command */
  *command = frame.buffer[2];
  /* Copies payload size */
  *payloadSize = i-5;
  /* Copies payload */
  memcpy(payload, &frame.buffer[3], i-5);
  
  return MESH_OK;
}




MeshStatus_Typedef ReceivePacketTransp(uint16_t* id, uint8_t* payload, uint8_t* payloadSize, uint32_t timeout)
{
  uint16_t waitNextByte = 500;
  uint8_t i = 0;
  
  /* Assert parameters */
  if((id == NULL) && (deviceId == 0)) return MESH_ERROR;
  if(payload == NULL) return MESH_ERROR;
  if(payloadSize == NULL) return MESH_ERROR;
  if(deviceId == -1) return MESH_ERROR;
  
  /* Waits for reception */
  while( ((timeout > 0 ) || (i > 0)) && (waitNextByte > 0) )
  {
    if(Serial2.available() > 0)
    {
      frame.buffer[i++] = Serial2.read();
	    waitNextByte = 500;
    }
    
	  if(i > 0)
    {
      waitNextByte--;
    }
    timeout--;
    delay(1);
  }

  /* In case it didn't get any data */
  if((timeout == 0) && (i == 0)) return MESH_ERROR;

  if(deviceId == 0)
  {
    /* Copies ID */
    *id = (uint16_t)frame.buffer[0] | ((uint16_t)frame.buffer[1] << 8);
    /* Copies payload size */
    *payloadSize = i-2;
    /* Copies payload */
    memcpy(payload, &frame.buffer[3], i-2);
  }
  else
  {
    /* Copies payload size */
    *payloadSize = i;
    /* Copies payload */
    memcpy(payload, &frame.buffer[0], i);
  }
  
  return MESH_OK;
}

// REMINDER: removing gio config|write|read for now...


MeshStatus_Typedef LocalRead(uint16_t* id, uint16_t* net, uint32_t* uniqueId)
{
  return LocalRemoteRead(0xFFFF, id, net, uniqueId);
}



MeshStatus_Typedef RemoteRead(uint16_t id, uint16_t* net, uint32_t* uniqueId)
{
  return LocalRemoteRead(id, NULL, net, uniqueId);
}




uint16_t ComputeCRC(uint8_t* data_in, uint16_t length)
{
  uint16_t i;
  uint8_t bitbang, j;
  uint16_t crc_calc;

  crc_calc = 0xC181;
  for(i=0; i<length; i++)
  {
    crc_calc ^= (((uint16_t)data_in[i]) & 0x00FF);
    
    for(j=0; j<8; j++)
    {
      bitbang = crc_calc;
      crc_calc >>= 1;
      
      if(bitbang & 1)
      {
        crc_calc ^= 0xA001;
      }
    }
  }
  return (crc_calc&0xFFFF);
}