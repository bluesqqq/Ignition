#pragma once

#include <vector>

class EnvelopeFollower
{
public:
	EnvelopeFollower(float attackTime = 0.01f, float releaseTime = 0.5f, float sampleRate = 44100.0f);

	void setAttackTime(float attack);
	void setReleaseTime(float release);
	void setSampleRate(float rate);

	float process(float input);

	float getEnvelope() const;

	std::vector<float>& getEnvelopeHistory();

private:
	void updateCoefficients();

	float attackTime, releaseTime;
	float sampleRate;
	float attackCoef, releaseCoef;
	float envelope;

	std::vector<float> envelopeHistory;
	const int historySize = 512;
	int sampleCounter = 0;
};