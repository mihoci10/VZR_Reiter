#pragma once

#include "ReiterSim.h"

class ReiterSequential : public ReiterSimulation{
    public:
        ReiterSequential(int width, int height) : ReiterSimulation(width, height) {};

        virtual double RunSimulation(float alpha, float beta, float gamma) override;
};