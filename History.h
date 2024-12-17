template <typename T> class History {
	public:
		T value;
		bool m_changed;

	History() : value(~0) { forget(); this->m_changed = false; }
	History(T value) { forget(); this->value = value; }

	void set(T value) {
		this->m_changed = (value != this->value);
		this->value = value;
	}
	bool unknown() const {
		return !this->m_changed && this->value == (T)~0;
	}
	bool operator!() const {
		return unknown();
	}

	void forget() {
		this->m_changed = true;
	}
	bool changed() {
		//if (this->unknown()) return true;
		if (this->m_changed) {
			this->m_changed = false;
			return true;
		}
		return false;

	}
	void inc(T max) {
		set((this->value + 1) % max);
	}
	void dec(T max) {
		set((this->value + max - 1) % max);
	}

	History<T>& operator=(T rhs) {
		set(rhs);
		return *this;
	}
	template <class T2>
	History<T>& operator=( const History<T2>& rhs ) {
		set( static_cast< T >( rhs ) );
		return *this;
	}
	History<T>& operator+=(T rhs) {
		set(this->value + rhs);
		return *this;
	}
	History<T>& operator%=(T rhs) {
		set(this->value % rhs);
		return *this;
	}
	operator T() const { return this->value; }
};
