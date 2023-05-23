#include "ReiterOpenMP.h"

#include <chrono>
#include <omp.h>

double ReiterOpenMP::RunSimulation(float alpha, float beta, float gamma)
{
    auto curData = CreateGrid(beta);
    auto prevData = CreateGrid(beta);

    auto idArray = std::shared_ptr<size_t>((size_t*)malloc(6 * sizeof(size_t)), free);
    bool stable = false;
    size_t iter = 0;

    LogState(curData.get(), iter);

    auto start = std::chrono::high_resolution_clock::now();
    while(!stable && iter <= MAX_ITER)
    {
        stable = true;
        #pragma omp parallel for
        for (int cellId = 0; cellId < m_Height * m_Width; cellId++)
        {
            int j = cellId % m_Width;
            int i = (cellId - j) / m_Width;

            if(i == 0 || j == 0 || m_Height - i == 1 || m_Width - i == 1)
                continue;
            
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

            if (sum >= 0 && stable)
                stable = false;
        }

        curData.swap(prevData);
        iter++;
        
        LogState(prevData.get(), iter);
    }
    auto stop = std::chrono::high_resolution_clock::now();

    printf("Simulation took %ld iterations\n", iter);

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

    printf("Ran on %d CPU cores\n", omp_get_max_threads());
    printf("Execution took %lf seconds\n", dur);

    return 0;
}