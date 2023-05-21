#pragma once

#include "ReiterSim.h"

class ReiterOpenMP : public ReiterSimulation{
    public:
        ReiterOpenMP(int width, int height) : ReiterSimulation(width, height) {};

        virtual double RunSimulation(float alpha, float beta, float gamma) override;
};