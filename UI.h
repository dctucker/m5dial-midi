const int chwid = 3;
const int textw = 9;
const int texth = 12;

enum CursorState {
	Unselected,
	Selected,
	Editing
};

void drawMidiValue(History<byte> value, String desc, int state, int x, int y, int w, int h);
class Widget {
	public:
		History<byte> *history;
		byte m_max;
		int x, y, w, h;
		String m_label;
	Widget() : history(NULL) {}
	template <typename T>
	Widget &track(History<T> &h) {
		this->history = &h;
		this->history->forget();
		return *this;
	}
	Widget &pos(int x, int y) {
		this->x = x; this->y = y;
		return *this;
	}
	Widget &size(int w, int h) {
		this->w = w; this->h = h;
		return *this;
	}
	Widget &label(String s) {
		this->m_label = s;
		return *this;
	}
	Widget &max(byte m) {
		this->m_max = m;
		return *this;
	}
	void display(CursorState state) {
		if (history == NULL) return;
		if (!history->changed()) return;

		drawMidiValue(*history, m_label, state, x, y, w, h);
	}
	void inc() {
		if (history == NULL) return;

		history->inc(m_max);
	}
	void dec() {
		if (history == NULL) return;

		history->dec(m_max);
	}
	void forget() {
		history->forget();
	}
};

class Page {
	public:
		CursorState state;
		size_t n_widgets;
		Widget *widgets[32];
		History<size_t> selected;

	Page() : n_widgets(0), selected(0) {}

	void add(Widget &w) {
		this->widgets[n_widgets] = &w;
		n_widgets++;
	}
	void display() {
		for(int i=0; i < n_widgets; i++) {
			CursorState st = (i == selected) ? state : Unselected;
			widgets[i]->display(st);
		}
	}
	void enter() {
		state = Editing;
	}
	void leave() {
		state = Selected;
	}
	void toggle() {
		currentWidget().forget();
		if (state == Editing)
			leave();
		else
			enter();
	}
	Widget &currentWidget() {
		return *(this->widgets[selected]);
	}
	void inc() {
		if (state == Editing) {
			currentWidget().inc();
		}
		else if (state == Selected) {
			selected.inc(n_widgets);
		}
	}
	void dec() {
		if (state == Editing) {
			currentWidget().dec();
		}
		else if (state == Selected) {
			selected.dec(n_widgets);
		}
	}
};
