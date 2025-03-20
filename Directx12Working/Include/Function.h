#pragma once


	template <typename T>
	void SafeRelease(T& pointer)
	{
		if (pointer)
		{
			pointer->Release();
			pointer = nullptr;
		}
	}

	template <typename T>
	class SingleTon
	{
	protected:
		SingleTon() = default;
		~SingleTon() = default;

	public:
		static T* GetInstance()
		{
			if (nullptr == _pInstance)
				_pInstance = new T;

			return _pInstance;
		}
		static void ResetInstance() { _pInstance = nullptr; }
	private:
		static T* _pInstance;
	};
	template <typename T>
	T* SingleTon<T>::_pInstance = nullptr;


class com_exception : public std::exception
{
public:
	com_exception(HRESULT hr) : result(hr) {}

	const char* what() const noexcept override
	{
		static char s_str[64] = {};
		sprintf_s(s_str, "Failure with HRESULT of %08X",
			static_cast<unsigned int>(result));
		return s_str;
	}

private:
	HRESULT result;
};

inline void HR_T(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw com_exception(hr);
	}
}

inline std::string WStringToString(const std::wstring& wstr)
{
	size_t convertedSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	std::vector<char> buffer(convertedSize, 0);

	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buffer.data(), static_cast<int>(buffer.size()), nullptr, nullptr);

	return std::string(buffer.data());
}

