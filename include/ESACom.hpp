#include <Arduino.h>


struct ESAStatus
{
    uint16_t speed; //in m/h
    bool light;
    bool eco;
    uint8_t soc; //in %
    uint8_t errorCode;
    bool shutdown;
    bool buttonPressed;
};

class ESACom
{
public:
    ESACom(HardwareSerial *conSerial);
    ~ESACom();

    bool sendPacket(const uint8_t *data);
    bool setSpeed(uint32_t speed); //in km/h
    bool rxHandler();
    const ESAStatus &getStatus() { return status; }

    uint16_t createChecksum(const uint8_t *);


private:
    uint8_t readByte();

    ESAStatus status;
    HardwareSerial *conSerial;
};

ESACom::ESACom(HardwareSerial *conSerial) : conSerial(conSerial)
{
    //conSerial->begin(115200);
}

ESACom::~ESACom()
{
    
}

bool ESACom::setSpeed(uint32_t speed)
{
    uint8_t data[] = {	0x04, 0x22, 0x01, 0xF2,
                        0, 0, //rpm
                        };

		//*(uint16_t *)&data[4] = (speed * 252) / 10;
		*(uint16_t *)(data + 4) = (speed * 252) / 10;


		this->sendPacket(data);

        return true;
}


uint16_t ESACom::createChecksum(const uint8_t *packet)
{
	uint16_t sum = 0;
	for(int i = 0; i < packet[0] + 2; i++)
		sum += packet[i];

	return sum ^ 0xFFFF;
}


bool ESACom::sendPacket(const uint8_t *data) //data[0] must be the length-2
{
    uint8_t length = data[0] + 2;


    Serial.println(length);


    uint16_t checksum = this->createChecksum(data);

/*
    Serial.println(0x55);
    Serial.println(0xAA);
    for(int i = 0; i < length; i++)
        Serial.println(data[i]);
    Serial.println((int)(checksum & 0x00FF));
    Serial.println((int)(checksum >> 8));
    */
    
    conSerial->write(0x55);
    conSerial->write(0xAA);
    conSerial->write(data, length);
    conSerial->write(checksum & 0x00FF);
    conSerial->write(checksum >> 8);

    return true;
}

uint8_t ESACom::readByte()
{
	while(!conSerial->available()) delay(1);

	return conSerial->read();
}

bool ESACom::rxHandler()
{
	uint8_t data[100];

    //Searching for start
    if(this->readByte() != 0x55)
		return false;
	if(this->readByte() != 0xAA)
		return false;

	uint8_t length = data[0] = this->readByte();

	if(length >= 100 - 4)
		return false;

	data[1] = this->readByte(); //addr

	if(data[1] != 0x28) //0x28 status update
		return false;

    //Payload
	for(int i = 0; i < length; i++)
		data[i + 2] = this->readByte();


	uint16_t checksumRX = (uint16_t)this->readByte() | (uint16_t)this->readByte() << 8;
    uint16_t checksum = this->createChecksum(data);
	if(checksumRX != checksum)    
    {
        Serial.print("Wrong checksum:  ");
        Serial.print(checksum);
        Serial.print(" != ");
        Serial.println(checksumRX);

        return false;
    }

	this->status.eco = data[4] == 0x02;
	this->status.shutdown = data[5] == 0x08;
	this->status.light = data[6] == 0x01;
	this->status.speed = *(uint16_t *)(data + 8);
	this->status.buttonPressed = data[10] == 0x01;
	this->status.errorCode = data[11];
	this->status.soc = data[12];

	return true;
}