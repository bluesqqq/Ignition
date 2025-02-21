#include "EnvelopeFollower.h"
#include <cmath>

EnvelopeFollower::EnvelopeFollower(float attackTime, float releaseTime, float sampleRate)
    : attackTime(attackTime), releaseTime(releaseTime), sampleRate(sampleRate), envelope(0.0f)
{
    updateCoefficients();
}

void EnvelopeFollower::setAttackTime(float attack)
{
    attackTime = attack;
    updateCoefficients();
}

void EnvelopeFollower::setReleaseTime(float release)
{
    releaseTime = release;
    updateCoefficients();
}

void EnvelopeFollower::setSampleRate(float rate)
{
    sampleRate = rate;
    updateCoefficients();
}

float EnvelopeFollower::process(float input)
{
    float absInput = std::abs(input);

    if (absInput > envelope)
        envelope = attackCoef * envelope + (1.0f - attackCoef) * absInput;  // Attack phase
    else
        envelope = releaseCoef * envelope + (1.0f - releaseCoef) * absInput; // Release phase

    if (++sampleCounter >= 255)
    {
        envelopeHistory.push_back(envelope);
        sampleCounter = 0;  // Reset the counter
    }

    if (envelopeHistory.size() > historySize) envelopeHistory.erase(envelopeHistory.begin());

    return envelope;
}

float EnvelopeFollower::getEnvelope() const
{
    return envelope;
}

std::vector<float>& EnvelopeFollower::getEnvelopeHistory()
{
    return envelopeHistory;
}

void EnvelopeFollower::updateCoefficients()
{
    attackCoef = std::exp(-std::log(9.0f) / (attackTime * sampleRate));
    releaseCoef = std::exp(-std::log(9.0f) / (releaseTime * sampleRate));
}