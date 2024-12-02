#define CARRIER_INTERVAL 4 // for DigiSpark
//#define CARRIER_INTERVAL 6 // for Arduino Pro Mini / Nano
#define MLT_SPACE_INTERVAL 600
#define MLT_HEADER_CYCLES 128 // adjusted experimentally
#define MLT_BIT0_CYCLES 32 // adjusted experimentally
#define MLT_BIT1_CYCLES 64 // adjusted experimentally

#define HEADING_RECEIVED 2

#ifndef LaserWar_h
#define LaserWar_h
#include <Arduino.h>
#include <LWCommand.h>

class LaserWar {
	private:
		uint8_t pin;
		unsigned long lostSignalTime, getSignalTime, buffer;
		unsigned int thresholod, headerLength, oneLength, calmValue;
		bool receivingSignal;

		void sendPulse(int cycles) {
			for (int i=0; i<cycles; i++ ) {
				digitalWrite(this->pin, HIGH);
				delayMicroseconds(CARRIER_INTERVAL);
				digitalWrite(this->pin, LOW);
				delayMicroseconds(CARRIER_INTERVAL);
			}
		}
		
		void sendHeader() {
			this->sendPulse(MLT_HEADER_CYCLES);
			delayMicroseconds(MLT_SPACE_INTERVAL);
		}

		bool hasSignal(unsigned int result){
			return result < this->calmValue;
		}

		short decodeSignal(unsigned long duration){
			if (duration < this->thresholod){
				return -1;
			}

			this->getSignalTime = 0;

			if (duration >= this->headerLength) {
				return HEADING_RECEIVED;
			}

			if (duration >= this->oneLength) {
				return 1;
			}

			return 0;
		}

		void sendByte(byte x) {
			for(int i=0; i<8; i++) {
				if (x & 0x80) {
				this->sendPulse(MLT_BIT1_CYCLES);
				} else {
				this->sendPulse(MLT_BIT0_CYCLES);
				}
				x = x << 1;
				delayMicroseconds(MLT_SPACE_INTERVAL);
			}
		}
	public:
		void setPin(uint8_t pin){ this->pin = pin; }
		void setThreshold(unsigned int threshold){ this->thresholod = threshold; }
		void setDurationOfHeader(unsigned int headerLength){ this->headerLength = headerLength; }
		void setDurationOfOne(unsigned int oneLength){ this->oneLength = oneLength; }
		void setReceiverCalmValue(unsigned int calmValue){ this->calmValue = calmValue; }

		LaserWar(uint8_t pin) {
			this->setPin(pin);
			this->buffer = 0;
			this->getSignalTime = 0;
			this->lostSignalTime = 0;
			this->receivingSignal = false;
			this->thresholod = 50;
			this->headerLength = 2000;
			this->oneLength = 1000;
			this->calmValue = 995;
		}

		void send(LWCommand cmd){
			sendHeader();
			sendByte(cmd.getGroup());
			sendByte(cmd.getData());
			sendByte(LW_CMD_END);
		}

		unsigned long read(){
			int result = analogRead(this->pin);
			unsigned long time = micros();
			
			if (this->hasSignal(result)){
				if (!this->receivingSignal){
				if (time - this->lostSignalTime > this->thresholod){
					this->receivingSignal = true;
					this->getSignalTime = time;
					this->lostSignalTime = 0;
				}
				} else {
					this->lostSignalTime = 0;
				}
			} else {
				if (this->receivingSignal){
					if (time - this->lostSignalTime > this->thresholod && this->lostSignalTime > 0){
						this->receivingSignal = false;
						short signal = decodeSignal(this->lostSignalTime - this->getSignalTime);
						if (signal >= 0){
							if (signal == HEADING_RECEIVED){
								this->buffer = 0;
							} else {
								this->buffer = (this->buffer << 1) + signal;
							}
						}
					}
					this->lostSignalTime = time;
				} else {
					if (this->lostSignalTime > 0 && time - this->lostSignalTime > this->headerLength){
						this->lostSignalTime = 0;
						return this->buffer;
					}
				}
			}

			return 0;
		}
};

#endif