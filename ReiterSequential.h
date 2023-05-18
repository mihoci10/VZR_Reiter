#pragma once

#include "ReiterSim.h"

class ReiterSequential : public ReiterSimulation{
    public:
        ReiterSequential(size_t width, size_t height) : ReiterSimulation(width, height) {};

        virtual double RunSimulation(float alpha, float beta, float gamma) override;
};