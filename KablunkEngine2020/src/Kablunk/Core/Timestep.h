#ifndef KABLUNK_CORE_TIMESTEP_H
#define KABLUNK_CORE_TIMESTEP_H

namespace Kablunk
{
	class Timestep
	{
	public:
		Timestep(float time = 0.0f)
			: m_Time(time)
		{

		}

		operator float() const { return m_Time; }

		float GetSeconds() const { return m_Time; }
		float GetMiliseconds() const { return m_Time * 1000.0f; }
	private:
		float m_Time;

	};
}

#endif