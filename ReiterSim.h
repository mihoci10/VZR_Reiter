#pragma once

#include <string>
#include <memory>

#define MAX_ITER 1000
#define PIX_PER_CELL 2

class ReiterSimulation {

    enum class DebugType{
        None, Txt, Img, All
    };

    public:
        ReiterSimulation(int width, int height) : m_Width(width), m_Height(height) {};

        virtual double RunSimulation(float alpha, float beta, float gamma) = 0;

    protected:
        std::shared_ptr<float> CreateGrid(float beta);
        void GetNeighbourCellIds(size_t cellId, size_t* outIdArray);
        bool CheckReceptiveCell(float* data, size_t cellId);

        void LogState(float* data, size_t iter);

        int m_Width, m_Height;

    private:
        void SaveStateToTxt(float* data, const std::string& filename);
        void SaveStateToImg(float* data, const std::string& filename);

        DebugType m_DebugMode = DebugType::None;
};