#include "ReiterSequential.h"

#include <chrono>

double ReiterSequential::RunSimulation(float alpha, float beta, float gamma)
{
    auto curData = CreateGrid(beta);
    auto prevData = CreateGrid(beta);

    auto idArray = std::shared_ptr<size_t>((size_t*)malloc(6 * sizeof(size_t)), free);
    bool stable = false;
    size_t iter = 0;

    auto start = std::chrono::high_resolution_clock::now();
    while(!stable && iter <= MAX_ITER)
    {
        stable = true;
        for (size_t i = 0; i < m_Height; i++)
        {
            for (size_t j = 0; j < m_Width; j++)
            {
                if(i == 0 || j == 0 || m_Height - i == 1 || m_Width - i == 1)
                    continue;
                
                size_t cellId = m_Width * i + j;
                GetNeighbourCellIds(cellId, idArray);
                float sum = 0;
                for (size_t k = 0; k < 6; k++)
                    if (!CheckReceptiveCell(prevData, idArray.get()[k]))
                        sum += prevData.get()[idArray.get()[k]];
                
                float cellR = (CheckReceptiveCell(prevData, cellId) ? 1.0 : 0.0);
                float cellU = (cellR == 0.0 ? prevData.get()[cellId] : 0.0);

                curData.get()[cellId] = prevData.get()[cellId] +  (alpha / 2.0) * ((sum / 6.0) - cellU) + (gamma * cellR);

                stable &= (curData.get()[cellId] < 1.0 || prevData.get()[cellId] >= 1.0);
            }
            
        }

        curData.swap(prevData);
        iter++;
    }
    auto stop = std::chrono::high_resolution_clock::now();

    printf("Simulation took %ld iterations\n", iter);

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    return (duration.count() * 1e-6);
}

int main(int argc, char** argv){

    ReiterSequential model(50, 50);
    auto dur = model.RunSimulation(0.5, 0.5, 0.5);

    printf("Execution took %lf seconds\n", dur);

    return 0;
}