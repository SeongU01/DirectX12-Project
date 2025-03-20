#pragma once
#include "Base.h"

class Scene abstract : public Base
{
protected:
	explicit Scene() = default;
	virtual ~Scene() = default;

public:
	virtual void Start() = 0;
	virtual int Update(const float& dt) = 0;
	virtual int LateUpdate(const float& dt) = 0;
	virtual bool Initialize() = 0;
	virtual void Render() = 0;

};

