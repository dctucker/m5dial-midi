// M5Dial MIDI application
// Author: Casey Tucker <dctucker@hotmail.com>
//

//#include <M5Stack.h>
#include <M5Dial.h>
//#include <M5_SAM2695.h>

#include "MidiState.h"
#include "History.h"
#include "UI.h"

#define MIDI_BAUD 31250
#define MidiIn  Serial1
//#define MidiIn Serial2

typedef struct {
	History<byte> bank_msb, bank_lsb, pgm;
} ChannelProgram;

ChannelProgram programs[16];

long encoder_pos = 0;
History<byte> shown_channel;

uint32_t frames;

const uint16_t primary_color = M5Dial.Lcd.color565(255, 96, 0);

MidiState midi_state;

Page patchPage;
Widget w_ch, w_msb, w_lsb, w_pgm;

void setup() {
	auto cfg = M5.config();
	MidiIn.begin(MIDI_BAUD);
	//MidiOut.begin(MIDI_BAUD);

	M5Dial.begin(cfg, true, false);
	M5Dial.Lcd.setTextColor(YELLOW);
	shown_channel = 0;
	frames = 0;

	patchPage.add( w_ch );
	patchPage.add( w_msb );
	patchPage.add( w_lsb );
	patchPage.add( w_pgm );
	patchPage.edit();

	int x, y, w, h, left;
	x = 4+chwid*textw/2;
	y = M5Dial.Lcd.height()/2;
	w = 32;
	h = 24;
	w_ch.track(shown_channel).pos(x,y).size(w,h);

	left = 4+chwid*textw;

	x = left + (M5Dial.Lcd.width() - left) / 4 + 1;
	w = M5Dial.Lcd.width() / 5;
	w_msb.track(programs[0].bank_msb).pos(x,y).size(w,h);

	x = left + 2 * (M5Dial.Lcd.width() - left) / 4 + 1;
	w_lsb.track(programs[0].bank_lsb).pos(x,y).size(w,h);

	x = left + 3 * (M5Dial.Lcd.width() - left) / 4 + 1;
	w_pgm.track(programs[0].pgm).pos(x,y).size(w,h);


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

String midiString(History<byte> b) {
	if (b.unknown()) {
		return "-";
	}
	return String((int)b + 1, DEC);
}

void drawMidiValue(History<byte> value, String desc, int state, int x, int y, int w, int h) {
	if (state != Editing) {
		M5Dial.Lcd.setColor(BLACK);
		M5Dial.Lcd.fillRect(x-w/2, y-h/2-1, w, h);
	}
	if (state == Selected) {
		M5Dial.Lcd.setColor(LIGHTGREY);
		M5Dial.Lcd.drawRect(x-w/2, y-h/2-1, w, h);
	} else if (state == Editing) {
		M5Dial.Lcd.setColor(DARKGREY);
		M5Dial.Lcd.fillRect(x-w/2, y-h/2-1, w, h);
	}

	M5Dial.Lcd.setTextFont(&fonts::FreeMonoBold12pt7b);
	if (state == Editing) {
		M5Dial.Lcd.setTextColor(BLACK);
	} else {
		M5Dial.Lcd.setTextColor(primary_color);
	}
	M5Dial.Lcd.drawString(midiString(value), x, y);

	M5Dial.Lcd.setTextFont(&fonts::Font0);
	M5Dial.Lcd.setTextColor(DARKGREY);
	M5Dial.Lcd.drawString(desc, x, y-h);
}

void display() {
	int x, y, w, h, left;
	auto p = programs[shown_channel];

	M5Dial.Lcd.setTextDatum(middle_center);
	M5Dial.Lcd.setTextSize(1);

	patchPage.display();
}

void updateChannel() {
	w_msb.track( programs[shown_channel].bank_msb );
	w_lsb.track( programs[shown_channel].bank_lsb );
	w_pgm.track( programs[shown_channel].pgm );
}

void cursorDec() {
	shown_channel.dec(16);
	updateChannel();
}

void cursorInc() {
	shown_channel.inc(16);
	updateChannel();
}

void illuminate() {
	frames = 1;
	M5Dial.Lcd.setBrightness(127);
}

void dim() {
	M5Dial.Lcd.setBrightness(20);
}

static m5::touch_state_t prev_state;
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
	if (M5Dial.BtnA.pressedFor(5000)) {
		M5Dial.Encoder.write(100);
		M5Dial.Speaker.tone(2000, 25);
	}
	if (M5Dial.BtnA.wasPressed()) {
		//M5Dial.Encoder.readAndReset();
		shown_channel = 0;
		dirty = true;
	}
	if (newpos != encoder_pos) {
		if (newpos % 4 == 0) {
			M5Dial.Speaker.tone(8000, 20);
			auto d = newpos - encoder_pos;
			if (d > 0) {
				cursorInc();
			} else if (d < 0) {
				cursorDec();
			}
			dirty = true;
		}
		encoder_pos = newpos;
		//M5Dial.Lcd.drawString(String(encoder_pos), M5Dial.Lcd.width()/2, M5Dial.Lcd.height()/2);
	}

	auto t = M5Dial.Touch.getDetail();
	if (prev_state != t.state) {
		prev_state = t.state;
		static constexpr const char* state_name[16] = {
			"none", "touch", "touch_end", "touch_begin",
			"___",  "hold",  "hold_end",  "hold_begin",
			"___",  "flick", "flick_end", "flick_begin",
			"___",  "drag",  "drag_end",  "drag_begin"};
		illuminate();
	}

	if (dirty) {
		display();
		illuminate();
	}
	if (frames % (1<<20) == 0) {
		dim();
	}
}
