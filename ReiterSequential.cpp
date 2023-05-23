#include "ReiterSequential.h"

#include <chrono>

double ReiterSequential::RunSimulation(float alpha, float beta, float gamma)
{
    auto curData = CreateGrid(beta);
    auto prevData = CreateGrid(beta);

    auto idArray = std::shared_ptr<size_t>((size_t*)malloc(6 * sizeof(size_t)), free);
    size_t iter = 0;

    auto start = std::chrono::high_resolution_clock::now();
    while(iter <= MAX_ITER)
    {
        for (int i = 0; i < m_Height; i++)
        {
            for (int j = 0; j < m_Width; j++)
            {
                if(i == 0 || j == 0 || m_Height - i == 1 || m_Width - j == 1)
                    continue;
                
                int cellId = m_Width * i + j;
                GetNeighbourCellIds(cellId, idArray.get());
                float sum = 0;
                for (int k = 0; k < 6; k++){
                    int id = idArray.get()[k];
                    if (!CheckReceptiveCell(prevData.get(), id))
                        sum += prevData.get()[id];
                }
                
                float cellR = (CheckReceptiveCell(prevData.get(), cellId) ? 1.0 : 0.0);
                float cellU = (cellR == 0.0 ? prevData.get()[cellId] : 0.0);

                curData.get()[cellId] = prevData.get()[cellId] +  (alpha / 2.0) * ((sum / 6.0) - cellU) + (gamma * cellR);
            }
            
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

    ReiterSequential model(width, height);
    auto dur = model.RunSimulation(alpha, beta, gamma);

    printf("Execution took %lf seconds\n", dur);

    return 0;
}