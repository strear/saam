#pragma once

template <typename T>
class Array {
public:
	static const size_t max_dimensions = 16;

	Array() = default;

	template <typename... Dummy_initializer_list_T>
	Array(size_t firstArg, Dummy_initializer_list_T... args) : dataOffset(0) {
		reshape(firstArg, args...);

		dataBase.data = new T[measures[0]];
		dataBase.refCount = new size_t(1);
	}

	Array(T* source, size_t len) : dataOffset(0) {
		axisCount = 1;
		measures[0] = len;

		dataBase.data = new T[measures[0]];
		dataBase.refCount = new size_t(1);

		memcpy(dataBase.data, source, len * sizeof(T));
	}

	template <typename... Dummy_initializer_list_T>
	Array(const Array& ref, size_t firstArg, Dummy_initializer_list_T... args) {
		dataOffset = ref.dataOffset;

		dummyInitRef(ref, 1, firstArg, args...);

		axisCount = ref.axisCount - 1;
		dataBase = ref.dataBase;
		(*dataBase.refCount)++;

		for (size_t i = 0; i + 1 < ref.axisCount; i++) {
			measures[i] = ref.measures[i + 1];
		}
	}

	Array(const Array& ref) {
		dataOffset = ref.dataOffset;

		axisCount = ref.axisCount;
		dataBase = ref.dataBase;
		(*dataBase.refCount)++;

		for (size_t i = 0; i < ref.axisCount; i++) {
			measures[i] = ref.measures[i];
		}
	}

	~Array() {
		if (dataBase.refCount == nullptr) return;

		(*dataBase.refCount)--;

		if (*(dataBase.refCount) == 0) {
			delete[] dataBase.data;
			delete dataBase.refCount;
		}
	}

	size_t measureOf(size_t dimension) const {
		return measures[dimension];
	}

	size_t sizeOf(size_t dimension) const {
		if (dimension + 1 < axisCount) {
			return size_t(1.f * measures[dimension] / measures[dimension + 1]);
		} else {
			return measures[dimension];
		}
	}

	size_t getAxisCount() const {
		return axisCount;
	}

	template <typename ... Dummy_Σίσυφος_T>
	Array at(size_t firstArg, Dummy_Σίσυφος_T... args) const {
		return Array(*this, firstArg, args...);
	}

	template <typename... Dummy_initializer_list_T>
	Array copy() const {
		Array ans(dataBase.data, measures[0] * sizeof(T));

		ans.axisCount = this->axisCount;
		for (size_t i = 0; i < axisCount; i++) {
			ans.measures[i] = measures[i];
		}

		return ans;
	}

	template <typename... Dummy_initializer_list_T>
	void reshape(size_t firstArg, Dummy_initializer_list_T... args) {
		axisCount = 0;
		dummyInit(firstArg, args...);

		// Calculate size of each dimension to make it easier to get the capacity
		// For example, store { 18, 9, 3 } measures for a 2 x 3 x 3 size
		for (intptr_t r = axisCount - 2; r >= 0; r--) {
			measures[r] *= measures[r + 1];
		}
	}

	template <typename... Dummy_initializer_list_T>
	Array resample(size_t firstArg, Dummy_initializer_list_T... args) const {
		Array result;
		result.dataOffset = this->dataOffset;

		result.reshape(firstArg, args...);
		result.dataBase.data = new T[result.measures[0]];
		result.dataBase.refCount = new size_t(1);

		T* newData = result.dataBase.data;
		resampleInternal(newData, this->dataOffset, 0, firstArg, args...);

		return result;
	}

	template <typename... Dummy_initializer_list_T>
	void paste(const Array& ref, Dummy_initializer_list_T... pos) {
		size_t pos_list[] = { pos... };
		size_t initial_offset = dataOffset;

		for (size_t i = 0; i < sizeof...(pos); i++) {
			initial_offset += (i < axisCount - 1 ? measures[i + 1] : 1) * pos_list[i];
		}

		pasteInternal(ref, initial_offset);
	}

	T* cptr() {
		return dataBase.data + dataOffset;
	}

	const T* cptr() const {
		return dataBase.data + dataOffset;
	}

	T get() const {
		return dataBase.data[dataOffset];
	}

	operator T() const {
		return dataBase.data[dataOffset];
	}

	void put(T value) {
		dataBase.data[dataOffset] = value;
	}

	Array& operator=(T value) {
		this->put(value);
		return *this;
	}

	Array operator[](int index) {
		return at(index);
	}

	const Array operator[](int index) const {
		return at(index);
	}

	Array& operator=(const Array& ref) {
		if (dataBase.refCount != nullptr) {
			(*dataBase.refCount)--;

			if (*(dataBase.refCount) == 0) {
				delete[] dataBase.data;
				delete dataBase.refCount;
			}
		}

		dataOffset = ref.dataOffset;

		axisCount = ref.axisCount;
		dataBase = ref.dataBase;
		if (dataBase.refCount != nullptr) (*dataBase.refCount)++;

		for (size_t i = 0; i < ref.axisCount; i++) {
			measures[i] = ref.measures[i];
		}
		return *this;
	}

private:
	struct counted_ptr {
		T* data = nullptr;
		size_t* refCount = nullptr;
	} dataBase;
	size_t dataOffset = 0;

	size_t axisCount = 0;
	size_t measures[max_dimensions]{};

	void dummyInit() {}

	template <typename... Dummy_initializer_list_T>
	void dummyInit(size_t firstArg, Dummy_initializer_list_T... args) {
		measures[axisCount++] = firstArg;
		dummyInit(args...);
	}

	void dummyInitRef(const Array&, size_t) {}

	template <typename... Dummy_initializer_list_T>
	void dummyInitRef(const Array& ref, size_t depth,
		size_t firstArg, Dummy_initializer_list_T... args) {

		if (depth < ref.axisCount) {
			dataOffset += firstArg * ref.measures[depth];
		} else {
			dataOffset += firstArg;
		}

		dummyInitRef(ref, depth + 1, args...);
	}

	void resampleInternal(T*& dstPtr, size_t srcOffset, size_t depth) const {
		*dstPtr = dataBase.data[srcOffset];
		dstPtr++;
	}

	template <typename... Dummy_initializer_list_T>
	void resampleInternal(T*& dstPtr, size_t srcOffset, size_t depth,
		const size_t firstArg, Dummy_initializer_list_T... args) const {

		size_t stepwidth = sizeof...(args) > 0 ? measures[depth + 1] : 1;

		for (size_t c = 0; c < firstArg; c++) {
			resampleInternal(dstPtr,
				measures[depth] / stepwidth * c / firstArg * stepwidth + srcOffset,
				depth + 1, args...);
		}
	}

	void pasteInternal(const Array& ref, size_t offset) {
		if (ref.axisCount == 0) {
			dataBase.data[offset] = ref.get();
		} else if (ref.axisCount == 1) {
			memcpy(dataBase.data + offset, ref.cptr(), ref.measureOf(0) * sizeof(T));
		} else {
			for (size_t i = 0; i < ref.sizeOf(0); i++) {
				pasteInternal(ref[i], offset + i * measures[1]);
			}
		}
	}
};