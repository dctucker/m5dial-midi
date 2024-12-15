//#include <M5Stack.h>
#include <M5Dial.h>
//#include <M5_SAM2695.h>

#define MIDI_BAUD 31250
#define MidiIn  Serial1
//#define MidiIn Serial2

long encoder_pos = 0;
byte status, channel, cc, data_pos;
byte shown_channel;
typedef struct {
	byte bank_msb, bank_lsb, pgm;
} ChannelProgram;
ChannelProgram programs[16];
byte frames;

void setup() {
	auto cfg = M5.config();
	MidiIn.begin(MIDI_BAUD); //, SERIAL_8N1);
							 //MidiOut.begin(MIDI_BAUD); // , SERIAL_8N1);

	M5Dial.begin(cfg, true, false);
	M5Dial.Lcd.setTextColor(YELLOW);
	shown_channel = 0;
	frames = 0;
	M5Dial.Lcd.clear();
	display();
}

const int chwid = 3;
const int textw = 9;
const int texth = 12;

void display() {
	int x, y, w, h, left;
	auto p = programs[shown_channel];

	M5Dial.Lcd.setTextDatum(middle_center);
	M5Dial.Lcd.setTextSize(1);

	// ch
	y = M5Dial.Lcd.height()/2;
	x = 0;
	M5Dial.Lcd.setColor(DARKGREY);
	M5Dial.Lcd.fillRect(0, y - 16, chwid*textw+8, 32);

	M5Dial.Lcd.setTextFont(&fonts::FreeMonoBold12pt7b);
	M5Dial.Lcd.setTextColor(0x000000);
	M5Dial.Lcd.drawString(String(shown_channel + 1, DEC), 4+chwid*textw/2, M5Dial.Lcd.height()/2);
	left = 4+chwid*textw;

	// msb
	w = M5Dial.Lcd.width() / 4;
	x = left + (M5Dial.Lcd.width() - left) / 4;
	h = 24;
	M5Dial.Lcd.setColor(0x000000);
	M5Dial.Lcd.fillRect(x-w/2, y-h/2, w, h);

	M5Dial.Lcd.setTextFont(&fonts::FreeMonoBold12pt7b);
	M5Dial.Lcd.setTextColor(ORANGE);
	M5Dial.Lcd.drawString(String(p.bank_msb + 1, DEC), x, y);

	M5Dial.Lcd.setTextFont(&fonts::Font0);
	M5Dial.Lcd.setTextColor(DARKGREY);
	M5Dial.Lcd.drawString("MSB", x, y-h);

	// lsb
	x = left + 2 * (M5Dial.Lcd.width() - left) / 4;
	M5Dial.Lcd.setColor(0x000000);
	M5Dial.Lcd.fillRect(x-w/2, y-h/2, w, h);

	M5Dial.Lcd.setTextFont(&fonts::FreeMonoBold12pt7b);
	M5Dial.Lcd.setTextColor(ORANGE);
	M5Dial.Lcd.drawString(String(p.bank_lsb + 1, DEC), x, y);

	M5Dial.Lcd.setTextFont(&fonts::Font0);
	M5Dial.Lcd.setTextColor(DARKGREY);
	M5Dial.Lcd.drawString("LSB", x, y-h);

	// pgm
	x = left + 3 * (M5Dial.Lcd.width() - left) / 4;
	M5Dial.Lcd.setColor(0x000000);
	M5Dial.Lcd.fillRect(x-w/2, y-h/2, w, h);

	M5Dial.Lcd.setTextFont(&fonts::FreeMonoBold12pt7b);
	M5Dial.Lcd.setTextColor(ORANGE);
	M5Dial.Lcd.drawString(String(p.pgm + 1, DEC), x, y);

	M5Dial.Lcd.setTextFont(&fonts::Font0);
	M5Dial.Lcd.setTextColor(DARKGREY);
	M5Dial.Lcd.drawString("PGM", x, y-h);
}

void loop() {
	M5Dial.update();

	bool dirty;
	byte input;
	dirty = false;

	while (MidiIn.available()) {
		input = MidiIn.read();
		//M5Dial.Speaker.tone(2000, 20);
		///*
		if (input & 0x80) { // status
			status = input & 0xf0;
			channel = input & 0x0f;
			data_pos = 0;
		} else { // data
			switch(status) {
				case 0x80: // note off
				case 0x90: // note on
				case 0xa0: // key pressure
					break;
				case 0xb0: // controller
						   //dirty = true;
					switch(data_pos) {
						case 0:
							cc = input;
							break;
						case 1:
							switch(cc){
								case 0:
									dirty = true;
									programs[channel].bank_msb = input;
									break;
								case 32:
									dirty = true;
									programs[channel].bank_lsb = input;
									break;
								default:;
							}
							break;
						default:;
					}
					data_pos++;
					break;
				case 0xc0: // program
					dirty = true;
					programs[channel].pgm = input;
					break;
				case 0xd0: // channel pressure
					break;
				case 0xe0: // pitch bend
					break;
				case 0xf0: // system
					break;
				default:;
			}
		} //*/
	}

	long newpos = M5Dial.Encoder.read();
	if (newpos != encoder_pos) {
		if (newpos % 4 == 0) {
			M5Dial.Speaker.tone(8000, 20);
			shown_channel += 16 + (newpos - encoder_pos);
			shown_channel %= 16;
			dirty = true;
		}
		encoder_pos = newpos;
		//M5Dial.Lcd.drawString(String(encoder_pos), M5Dial.Lcd.width()/2, M5Dial.Lcd.height()/2);
	}
	if (M5Dial.BtnA.wasPressed()) {
		M5Dial.Encoder.readAndReset();
		shown_channel = 0;
	}
	if (M5Dial.BtnA.pressedFor(5000)) {
		M5Dial.Encoder.write(100);
		M5Dial.Speaker.tone(2000, 25);
	}

	if (dirty) {
		display();
	}
}
