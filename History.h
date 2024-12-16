template <typename T> class History {
	public:
		T current;
		T previous;

	History() : current(~0) { forget(); previous = ~0; }
	History(T value) { forget(); current = value; }
	void set(T value) {
		previous = current;
		current = value;
	}
	void forget() {
		current = ~0;
		previous = ~0;
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
	History<T>& operator+=(T rhs) {
		previous = current;
		current += rhs;
		return *this;
	}
	History<T>& operator%=(T rhs) {
		previous = current;
		current %= rhs;
		return *this;
	}
	operator T() const { return current; }
	bool operator!() const { return current == previous && current == (T)~0; }
};

