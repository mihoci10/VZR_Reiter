#pragma once

#include <string>
#include <memory>

#define MAX_ITER 1000
#define PIX_PER_CELL 2

class ReiterSimulation {

    public:
        ReiterSimulation(int width, int height) : m_Width(width), m_Height(height) {};

        virtual double RunSimulation(float alpha, float beta, float gamma) = 0;

        static bool ParseInputParams(int argc, char** argv, int* width, int* height, float* alpha, float* beta, float* gamma);

    protected:
        
        enum class DebugType{
            None, Txt, Img, All
        };

        enum class DebugFreq{
            None, Last, EveryIter
        };

        std::shared_ptr<float> CreateGrid(float beta);
        void GetNeighbourCellIds(size_t cellId, size_t* outIdArray);
        bool CheckReceptiveCell(float* data, size_t cellId);

        void LogState(float* data, size_t iter);

        int m_Width, m_Height;
        DebugFreq m_DebugFreq = DebugFreq::Last;

    private:
        void SaveStateToTxt(float* data, const std::string& filename);
        void SaveStateToImg(float* data, const std::string& filename);

        DebugType m_DebugMode = DebugType::Img;
};