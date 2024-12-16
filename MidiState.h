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
			return *this;
		}
		switch(status) {
			case 0x80: // note off
			case 0x90: // note on
			case 0xa0: // key pressure
				break;
			case 0xb0: // controller
				if(data_pos % 2 == 0) {
					cc = input;
				} else {
					dirty = cc_handler(channel, cc, input);
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
		return *this;
	}
};