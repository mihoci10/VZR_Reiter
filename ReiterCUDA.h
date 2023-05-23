#pragma once

#include "ReiterSim.h"

class ReiterCUDA : public ReiterSimulation{
    public:
        ReiterCUDA(int width, int height) : ReiterSimulation(width, height) {};

        virtual double RunSimulation(float alpha, float beta, float gamma) override;
};