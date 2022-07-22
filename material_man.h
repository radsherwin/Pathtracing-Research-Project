#ifndef YSL_RT_MATERIAL_MAN_H
#define YSL_RT_MATERIAL_MAN_H

#include "material.h"

#include <vector>
#include <memory>

class material_man
{
public:
	material_man() : mats{}, m_idx(0) {mats.reserve(10);}
	material_man(const material_man &) = delete;
	material_man &operator=(const material_man &) = delete;
	~material_man()
	{
		for (auto &it : mats)
		{
			delete it;
		}
		m_idx = 0;
	}

	i32 add(material *m)
	{
		mats.emplace_back(std::move(m));
		return m_idx++;
	}

	/*i32 add(material *m)
	{
		mats.emplace_back(std::move(m));
		return m_idx++;
	}*/

	material *get(const i32 idx) const
	{
		return mats[idx];
	}

	void reset()
	{
		mats.clear();
		m_idx = 0;
	}

private:
	std::vector<material *> mats;
	i32 m_idx;
};

#endif