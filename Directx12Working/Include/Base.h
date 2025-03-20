#pragma once
#include "Define.h"
class Base abstract
{
protected:
	explicit Base() = default;
	virtual ~Base() = default;
public:
	void Release()
	{
		Free();
		delete this;
	}
	inline void SetActive(bool isActive) { _isActive = false; }
	inline void SetName(std::wstring name) { _name = name; }
	inline std::wstring GetName()const { return _name; }
	inline bool IsActive()const { return _isActive; }
	inline bool operator==(std::wstring str) { return !lstrcmp(_name.c_str(), str.c_str()); }
	inline bool operator!=(std::wstring str) { return lstrcmp(_name.c_str(), str.c_str()); }

protected:
	virtual void Free() = 0;
private:
	std::wstring _name;
	bool _isActive = true;
};
