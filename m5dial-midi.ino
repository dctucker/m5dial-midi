// M5Dial MIDI application
// Author: Casey Tucker <dctucker@hotmail.com>
//

//#include <M5Stack.h>
#include <M5Dial.h>
//#include <M5_SAM2695.h>

#define MIDI_BAUD 31250
#define MidiIn  Serial1
//#define MidiIn Serial2

template <typename T> class History {
	public:
		T current;
		T previous;
		History() : current(0) {}
		History(T value): current(value) {}
		void set(T value) {
			previous = current;
			current = value;
		}
		History<T>& operator=(T rhs) {
			set(rhs);
			return *this;
		}
		template <class T2>
		History<T>& operator=( const History<T2>& rhs )
		{
			previous = current;
			current = static_cast< T >( rhs );
			return *this;
		}
		operator T() const { return current; }
};

typedef struct {
	History<byte> bank_msb, bank_lsb, pgm;
} ChannelProgram;

ChannelProgram programs[16];

class MidiState {
	private:
		bool (*cc_handler)(byte, byte, byte);
		bool (*pc_handler)(byte, byte);
	public:
		byte status, channel, cc, data_pos;
		bool dirty;

	void onControl( bool (*handler)(byte ch, byte cc, byte val) ) {
		cc_handler = handler;
	}

	void onProgram( bool (*handler)(byte ch, byte pc) ) {
		pc_handler = handler;
	}

	MidiState operator<<(byte const& input) {
		dirty = false;
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
					switch(data_pos % 2) {
						case 0:
							cc = input;
							break;
						case 1:
							dirty = cc_handler(channel, cc, input);
							break;
						default:;
					}
					data_pos++;
					break;
				case 0xc0: // program
					dirty = pc_handler(channel, input);
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
		return *this;
	}
};

long encoder_pos = 0;
byte shown_channel;

uint32_t frames;

const uint16_t primary_color = M5Dial.Lcd.color565(255, 96, 0);

MidiState midi_state;

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

	midi_state.onControl( [](byte ch, byte cc, byte val) -> bool {
		switch(cc){
			case  0: programs[ch].bank_msb = val; break;
			case 32: programs[ch].bank_lsb = val; break;
			default: return false;
		}
		return true;
	});
	midi_state.onProgram( [](byte ch, byte val) -> bool {
		programs[ch].pgm = val;
		return true;
	});
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
	M5Dial.Lcd.setTextColor(primary_color);
	M5Dial.Lcd.drawString(String(p.bank_msb + 1, DEC), x, y);

	M5Dial.Lcd.setTextFont(&fonts::Font0);
	M5Dial.Lcd.setTextColor(DARKGREY);
	M5Dial.Lcd.drawString("MSB", x, y-h);

	// lsb
	x = left + 2 * (M5Dial.Lcd.width() - left) / 4;
	M5Dial.Lcd.setColor(0x000000);
	M5Dial.Lcd.fillRect(x-w/2, y-h/2, w, h);

	M5Dial.Lcd.setTextFont(&fonts::FreeMonoBold12pt7b);
	M5Dial.Lcd.setTextColor(primary_color);
	M5Dial.Lcd.drawString(String(p.bank_lsb + 1, DEC), x, y);

	M5Dial.Lcd.setTextFont(&fonts::Font0);
	M5Dial.Lcd.setTextColor(DARKGREY);
	M5Dial.Lcd.drawString("LSB", x, y-h);

	// pgm
	x = left + 3 * (M5Dial.Lcd.width() - left) / 4;
	M5Dial.Lcd.setColor(0x000000);
	M5Dial.Lcd.fillRect(x-w/2, y-h/2, w, h);

	M5Dial.Lcd.setTextFont(&fonts::FreeMonoBold12pt7b);
	M5Dial.Lcd.setTextColor(primary_color);
	M5Dial.Lcd.drawString(String(p.pgm.current + 1, DEC), x, y);

	M5Dial.Lcd.setTextFont(&fonts::Font0);
	M5Dial.Lcd.setTextColor(DARKGREY);
	M5Dial.Lcd.drawString("PGM", x, y-h);
}

void loop() {
	M5Dial.update();

	bool dirty;
	byte input;
	dirty = false;
	frames++;

	while (MidiIn.available()) {
		input = MidiIn.read();
		midi_state << input;
		dirty = dirty || midi_state.dirty;
		//M5Dial.Speaker.tone(2000, 20);
		///*
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
		frames = 1;
		M5Dial.Lcd.setBrightness(127);
	}
	if (frames % (1<<20) == 0) {
		M5Dial.Lcd.setBrightness(20);
	}
}
