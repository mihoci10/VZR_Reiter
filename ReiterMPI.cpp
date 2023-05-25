#include "ReiterMPI.h"

#include <chrono>
#include <mpi.h>

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

double ReiterMPI::RunSimulation(float alpha, float beta, float gamma)
{
    auto start = std::chrono::high_resolution_clock::now();
    
    Simulation(alpha, beta, gamma);

    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    return (duration.count() * 1e-6);
}

void GetNeighbourCellIdsMPI(size_t global_cell_id, int* outIdArray, int width)
{
    size_t j = global_cell_id % width;

    int nOff;
    if (j%2 == 0)
        nOff = -1;
    else
        nOff = 0;
        
    outIdArray[0] = width * (-1);
    outIdArray[1] = width * (nOff) - 1;
    outIdArray[2] = width * (nOff+1) - 1;
    outIdArray[3] = width * (nOff) + 1;
    outIdArray[4] = width * (nOff+1) + 1;
    outIdArray[5] = width * (1);
}

void ReiterMPI::Simulation(float alpha, float beta, float gamma){

    int rank, n_proc;
	MPI_Comm_size(MPI_COMM_WORLD, &n_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::shared_ptr<float> curData;
    //if (rank == 0) 
        curData = CreateGrid(beta);
    
    int N = m_Width * m_Height;
	int block_size = N / n_proc + 1;

	int* rcv_buf_sizes = new int[n_proc];
	int* rcv_buf_displ = new int[n_proc];

	int* snd_buf_sizes = new int[n_proc];
	int* snd_buf_displ = new int[n_proc];
    
	int snd_sum = 0;
    for (int i = 0; i < n_proc; i++)
	{
		int diff = min(block_size, N - snd_sum);
		snd_buf_sizes[i] = diff;
		snd_buf_displ[i] = snd_sum;
		snd_sum += diff;

        rcv_buf_sizes[i] = snd_buf_sizes[i] + min(m_Width + 1, snd_buf_displ[i]) + min(m_Width + 1, N - snd_buf_displ[i] - snd_buf_sizes[i]);
		rcv_buf_displ[i] = snd_buf_displ[i] - min(m_Width + 1, snd_buf_displ[i]);
	}

    int start_cell_id = snd_buf_displ[rank];
    int stop_cell_id = start_cell_id + snd_buf_sizes[rank];

    int rcv_buf_start_pad = min(m_Width + 1, start_cell_id);
    int rcv_buf_stop_pad = min(m_Width + 1, N - stop_cell_id);

	int rcv_buf_size = snd_buf_sizes[rank];
    rcv_buf_size += rcv_buf_start_pad;
    rcv_buf_size += rcv_buf_stop_pad;
    int snd_buf_size = snd_buf_sizes[rank];

	auto rcv_buf = std::shared_ptr<float>((float*)malloc(rcv_buf_size * sizeof(float)), free);
    auto snd_buf = std::shared_ptr<float>((float*)malloc(snd_buf_size * sizeof(float)), free);

    auto idArray = std::shared_ptr<int>((int*)malloc(6 * sizeof(int)), free);
    auto idArrayS = std::shared_ptr<int>((int*)malloc(6 * sizeof(int)), free);

    size_t iter = 0;

    while(iter <= MAX_ITER){

	    MPI_Scatterv(curData.get(), rcv_buf_sizes, rcv_buf_displ, MPI_FLOAT, rcv_buf.get(), rcv_buf_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

        for(int i=0; i < snd_buf_size; i++){

            int global_cell_id = start_cell_id + i;
            int global_j = global_cell_id % m_Width;
            int global_i = (global_cell_id - global_j) / m_Width;

            if(global_i == 0 || global_j == 0 || m_Height - global_i == 1 || m_Width - global_j == 1){
                snd_buf.get()[i] = beta;
                continue;
            }

            GetNeighbourCellIdsMPI(global_cell_id, idArray.get(), m_Width);
            float sum = 0;
            bool receptive = rcv_buf.get()[rcv_buf_start_pad + i] >= 1;
            for(int k = 0; k < 6; k++){
                int id = idArray.get()[k] + rcv_buf_start_pad + i;
                bool receptiveS = false;
                if(rcv_buf.get()[id] >= 1)
                    receptiveS = true;
                else{
                    GetNeighbourCellIdsMPI(global_cell_id + idArray.get()[k], idArrayS.get(), m_Width);
                    for (int v = 0; v < 6; v++)
                    {
                        if(id + idArrayS.get()[v] >= 0 && id + idArrayS.get()[v] < rcv_buf_size)
                            if(rcv_buf.get()[id + idArrayS.get()[v]] >= 1)
                                receptiveS = true;
                        if(receptiveS)
                            break;
                    }
                }
                
                if(!receptiveS)
                    sum += rcv_buf.get()[id];
                if(rcv_buf.get()[id] >= 1)
                    receptive = true;
            }

            float cellR = (receptive ? 1.0 : 0.0);
            float cellU = (cellR == 0.0 ? rcv_buf.get()[rcv_buf_start_pad + i] : 0.0);

            snd_buf.get()[i] = rcv_buf.get()[rcv_buf_start_pad + i] +  (alpha / 2.0) * ((sum / 6.0) - cellU) + (gamma * cellR);
        }

        MPI_Gatherv(snd_buf.get(), snd_buf_size, MPI_FLOAT, curData.get(), snd_buf_sizes, snd_buf_displ, MPI_FLOAT, 0, MPI_COMM_WORLD);

        if(rank == 0 && m_DebugFreq == DebugFreq::EveryIter)
            LogState(curData.get(), iter);

        iter++;
    }

    if(rank == 0 && m_DebugFreq == DebugFreq::Last)
        LogState(curData.get(), iter);

    delete[] snd_buf_sizes;
    delete[] snd_buf_displ;
}

int main(int argc, char** argv){

    int width, height;
    float alpha, beta, gamma;

    if (!ReiterSimulation::ParseInputParams(argc, argv, &width, &height, &alpha, &beta, &gamma))
    {
        printf("Correct usage should be: %s <width> <height> <alpha> <beta> <gamma>\n", argv[0]);
        return -1;
    }

    MPI_Init(&argc, &argv);

	int rank, n_proc;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &n_proc);

    ReiterMPI model(width, height);

    if(rank == 0){
        auto dur = model.RunSimulation(alpha, beta, gamma);
        printf("Ran on %d nodes\n", n_proc);
        printf("Execution took %lf seconds\n", dur);
    }
    else{
        model.Simulation(alpha, beta, gamma);
    }

    MPI_Finalize();

    return 0;
}