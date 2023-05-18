#pragma once

#include <string>
#include <memory>

#define MAX_ITER 1000

class ReiterSimulation {
    public:
        ReiterSimulation(int width, int height) : m_Width(width), m_Height(height) {};

        virtual double RunSimulation(float alpha, float beta, float gamma) = 0;

    protected:
        std::shared_ptr<float> CreateGrid(float beta);
        void GetNeighbourCellIds(size_t cellId, const std::shared_ptr<size_t> &outIdArray);
        bool CheckReceptiveCell(const std::shared_ptr<float> &data, size_t cellId);

        int m_Width, m_Height;

    private:
        std::string m_OutImgName; 
};