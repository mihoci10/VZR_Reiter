#pragma once

#include "ReiterSim.h"

class ReiterMPI : public ReiterSimulation{
    public:
        ReiterMPI(int width, int height) : ReiterSimulation(width, height) {};

        virtual double RunSimulation(float alpha, float beta, float gamma) override;
    
        void Simulation(float alpha, float beta, float gamma);
};