#include "ReiterMPI.h"

#include <chrono>
#include <mpi.h>

double ReiterMPI::RunSimulation(float alpha, float beta, float gamma)
{
    auto start = std::chrono::high_resolution_clock::now();
    
    Simulation(alpha, beta, gamma);

    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    return (duration.count() * 1e-6);
}

void ReiterMPI::Simulation(float alpha, float beta, float gamma){

    // Number of processes and rank
    int n_proc;
	MPI_Comm_size(MPI_COMM_WORLD, &n_proc);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    auto curData = CreateGrid(beta);
    auto prevData = CreateGrid(beta);
    
    // Array describing how many elements to send to each process
	int* sendcounts = (int*)malloc(sizeof(int) * n_proc);

    // Array describing the displacements where each segment begins
	int* displacements = (int*)malloc(sizeof(int) * n_proc);

    // Block size
	int block_size = (m_Width * m_Height) / n_proc;

    // Sum of counts, used to calculate displacements
	int sum = 0;

    // Elements remaining after division among processes
	int remainder = (m_Width * m_Height) % n_proc;

    // Max size of receiving buffer
	int max_receive_buffer_size = 0;

    // Calculate send counts and displacements
	for(int i=0; i<n_proc; i++){
		sendcounts[i] = block_size;
		if(remainder > 0){
			sendcounts[i]++;
			remainder--;
		}

		displacements[i] = sum;
		sum += sendcounts[i];

		if(max_receive_buffer_size < sendcounts[i])
			max_receive_buffer_size = sendcounts[i];
	}

    // Buffer where the received data should be stored
	float* receive_buffer = (float*)malloc(max_receive_buffer_size * sizeof(float));

    size_t iter = 0;
    auto idArray = std::shared_ptr<size_t>((size_t*)malloc(6 * sizeof(size_t)), free);
    float* updatedData = (float*)malloc(max_receive_buffer_size * sizeof(float));

    // Compute
    while(iter <= MAX_ITER){

        // Scatter curData
	    MPI_Scatterv(curData.get(), sendcounts, displacements, MPI_FLOAT, receive_buffer, max_receive_buffer_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

        for(int i=0; i < sendcounts[rank]; i++){

            int cellId = displacements[rank] + i;
            GetNeighbourCellIds(cellId, idArray.get());
            float sum = 0;
            for(int k = 0; k < 6; k++){
                int id = idArray.get()[k];
                if(!CheckReceptiveCell(prevData.get(), id))
                    sum += prevData.get()[id];
            }

            float cellR = (CheckReceptiveCell(prevData.get(), cellId) ? 1.0 : 0.0);
            float cellU = (cellR == 0.0 ? prevData.get()[cellId] : 0.0);

            curData.get()[cellId] = prevData.get()[cellId] +  (alpha / 2.0) * ((sum / 6.0) - cellU) + (gamma * cellR);

            // Store the updated cell data in the updatedData buffer
            updatedData[i] = curData.get()[cellId];

        }

        if(m_DebugFreq == DebugFreq::EveryIter)
            LogState(curData.get(), iter);

        // Gather the updated cell data from all nodes
        MPI_Gatherv(updatedData, sendcounts[rank], MPI_FLOAT, curData.get(), sendcounts, displacements, MPI_FLOAT, 0, MPI_COMM_WORLD);

        curData.swap(prevData);
        iter++;
    }

    if(rank == 0){
        printf("Simulation took %ld iterations\n", iter);
        if(m_DebugFreq == DebugFreq::Last)
        LogState(prevData.get(), iter);
    }

    // Clean up
    free(sendcounts);
    free(displacements);
    free(receive_buffer);

}

int main(int argc, char** argv){

    MPI_Init(&argc, &argv);

    // Rank
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int w = 500;
    int h = 500;
    float alpha = 1;
    float beta = 0.6;
    float gamma = 0.01;

    ReiterMPI model(w, h);

    if(rank == 0){
        auto dur = model.RunSimulation(alpha, beta, gamma);
        printf("Execution took %lf seconds\n", dur);
    }
    else{
        model.Simulation(alpha, beta, gamma);
    }

    MPI_Finalize();

    return 0;
}