#include "ReiterOpenMP.h"

#include <chrono>
#include <omp.h>

double ReiterOpenMP::RunSimulation(float alpha, float beta, float gamma)
{
    auto curData = CreateGrid(beta);
    auto prevData = CreateGrid(beta);

    auto numThreads = omp_get_max_threads();
    size_t idArray[numThreads][6];
    size_t iter = 0;

    auto start = std::chrono::high_resolution_clock::now();
    while(!IsStable(prevData.get()) && iter <= MAX_ITER)
    {
        #pragma omp parallel for
        for (int cellId = 0; cellId < m_Height * m_Width; cellId++)
        {
            auto threadId = omp_get_thread_num();
            int j = cellId % m_Width;
            int i = (cellId - j) / m_Width;

            if(i == 0 || j == 0 || m_Height - i == 1 || m_Width - j == 1)
                continue;
            
            GetNeighbourCellIds(cellId, idArray[threadId]);
            float sum = 0;
            for (int k = 0; k < 6; k++){
                int id = idArray[threadId][k];
                if (!CheckReceptiveCell(prevData.get(), id))
                    sum += prevData.get()[id];
            }
            
            float cellR = (CheckReceptiveCell(prevData.get(), cellId) ? 1.0 : 0.0);
            float cellU = (cellR == 0.0 ? prevData.get()[cellId] : 0.0);

            curData.get()[cellId] = prevData.get()[cellId] +  (alpha / 2.0) * ((sum / 6.0) - cellU) + (gamma * cellR);
        }

        if(m_DebugFreq == DebugFreq::EveryIter)
            LogState(curData.get(), iter);

        curData.swap(prevData);
        iter++;
    }
    if(m_DebugFreq == DebugFreq::Last)
        LogState(prevData.get(), iter);

    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    return (duration.count() * 1e-6);
}

int main(int argc, char** argv){

    int width, height;
    float alpha, beta, gamma;

    if (!ReiterSimulation::ParseInputParams(argc, argv, &width, &height, &alpha, &beta, &gamma))
    {
        printf("Correct usage should be: %s <width> <height> <alpha> <beta> <gamma>\n", argv[0]);
        return -1;
    }

    ReiterOpenMP model(width, height);
    auto dur = model.RunSimulation(alpha, beta, gamma);
    
    printf("{type: \"OpenMP\", n: %d, elapsed: %lf, width: %d, height: %d, alpha: %f, beta: %f, gamma: %f},\n",omp_get_max_threads(), dur, width, height, alpha, beta, gamma);

    return 0;
}