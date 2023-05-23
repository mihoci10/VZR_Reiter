#include "ReiterMPI.h"

#include <chrono>
#include <mpi.h>

double ReiterMPI::RunSimulation(float alpha, float beta, float gamma)
{
    auto curData = CreateGrid(beta);
    auto prevData = CreateGrid(beta);

    auto idArray = std::shared_ptr<size_t>((size_t*)malloc(6 * sizeof(size_t)), free);
    bool stable = false;
    size_t iter = 0;

    //LogState(curData.get(), iter);

    auto start = std::chrono::high_resolution_clock::now();
    while(!stable && iter <= MAX_ITER)
    {
        stable = true;
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
        
    }
    LogState(curData.get(), 0);
    auto stop = std::chrono::high_resolution_clock::now();

    printf("Simulation took %ld iterations\n", iter);

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    return (duration.count() * 1e-6);
}

double ReiterMPI::Simulation(float alpha, float beta, float gamma, int argc, char** argv){
    
    double duration;

    MPI_Init(&argc, &argv);

    int num_p;
	MPI_Comm_size(MPI_COMM_WORLD, &num_p);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    float * curDataPtr = NULL;

    // Divide work
    int mystart = m_Height / num_p * rank;
	int myend = m_Height / num_p * (rank + 1);
	int myrows = m_Height / num_p;

    // Initialize my structures
    auto mycurData = std::shared_ptr<float>((float*)malloc(m_Width * myrows * sizeof(float)), free);
    auto myrowtop = std::shared_ptr<float>((float*)malloc(m_Width * sizeof(float)), free);
    auto myrowbot = std::shared_ptr<float>((float*)malloc(m_Width * sizeof(float)), free);

    if(rank == 0){
        auto curData = CreateGrid(beta);
        curDataPtr = curData.get();
        LogState(curData.get(), 0);
        mycurData = curData;
    }

    // Scatter initial data
    MPI_Scatter(curDataPtr, m_Width * myrows, MPI_FLOAT, mycurData.get(), m_Width * myrows, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // Main loop
    bool stable = false;
    size_t iter = 0;
    auto idArray = std::shared_ptr<size_t>((size_t*)malloc(6 * sizeof(size_t)), free);
    
    // Timer - Start
	MPI_Barrier(MPI_COMM_WORLD);
	double start = MPI_Wtime();

    while(!stable && iter <= MAX_ITER)
    {
        // Exchange borders with neigbouring processes
        MPI_Sendrecv(mycurData.get(), m_Width, MPI_FLOAT, (rank + num_p - 1) % num_p, 0, myrowbot.get(), m_Width, MPI_FLOAT, (rank + 1) % num_p, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

        MPI_Sendrecv(mycurData.get(), m_Width, MPI_FLOAT, (rank + 1) % num_p, 1, myrowtop.get(), m_Width, MPI_FLOAT, (rank + num_p - 1) % num_p, 1, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

        // TODO: Do calculations for my part

        /*
        printf("%d: ", rank);
        for(int i=0; i<m_Width; i++){
            printf("%f ", myrowtop.get()[i]);
        }
        printf("\n");
        */

        iter++;
        stable = true;
    }

    printf("%d: %d - %d  %d\n", rank, mystart, myend, myrows);

    // Timer - Stop
	duration = MPI_Wtime() - start;

    // Gather results
    MPI_Gather(mycurData.get(), m_Width * myrows, MPI_FLOAT, curDataPtr, m_Width * myrows, MPI_FLOAT, 0, MPI_COMM_WORLD);

    MPI_Finalize();

    return duration;
}

int main(int argc, char** argv){

    int w = 10;
    int h = 10;

    ReiterMPI model(w, h);

    auto dur = model.Simulation(1, 0.5, 0.01, argc, argv);
    printf("Execution took %lf seconds\n", dur);

    return 0;
}