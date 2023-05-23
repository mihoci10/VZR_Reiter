#include "ReiterCUDA.h"
#include <cuda.h>
#include <chrono>
#include <stdio.h>

__device__ void GetNeighbourCellIds(size_t cellId, size_t* outIdArray, int m_Width)
{
    size_t j = cellId % m_Width;
    size_t i = (cellId - j) / m_Width;

    int nOff;
    if (j%2 == 0)
        nOff = -1;
    else
        nOff = 0;
        
    outIdArray[0] = m_Width * (i-1) + j;
    outIdArray[1] = m_Width * (nOff + i) + j - 1;
    outIdArray[2] = m_Width * (nOff + i+1) + j - 1;
    outIdArray[3] = m_Width * (nOff + i) + j + 1;
    outIdArray[4] = m_Width * (nOff + i+1) + j + 1;
    outIdArray[5] = m_Width * (i+1) + j;
}

__device__ bool CheckReceptiveCell(float* data, size_t cellId, int m_Width, int m_Height)
{
    if(data[cellId] >= 1)
        return true;

    int j = cellId % m_Width;
    int i = (cellId - j) / m_Width;

    int nOff;
    if (j%2 == 0)
        nOff = -1;
    else
        nOff = 0;
        
    if(i>0 && data[m_Width * (i-1) + j] >= 1)
        return true;
    if(j>0 && (nOff + i) > 0 && data[m_Width * (nOff + i) + j - 1] >= 1)
        return true;
    if(j>0 && (nOff + i+1) < m_Height && data[m_Width * (nOff + i+1) + j - 1] >= 1)
        return true;
    if(j+1 < m_Width && (nOff + i) > 0 && data[m_Width * (nOff + i) + j + 1] >= 1)
        return true;
    if(j+1 < m_Width && (nOff + i+1) < m_Height && data[m_Width * (nOff + i+1) + j + 1] >= 1)
        return true;
    if(i+1 < m_Height && data[m_Width * (i+1) + j] >= 1)
        return true;

    return false;
}

__global__ void simulationKernel(float* curData, float* prevData, int height, int width, size_t* idArray, int maxIter, float alpha, float beta, float gamma)
{
    int cellId = blockIdx.x * blockDim.x + threadIdx.x;
    if (cellId < height * width)
    {
        int j = cellId % width;
        int i = (cellId - j) / width;

        if (i == 0 || j == 0 || height - i == 1 || width - j == 1)
            return;

        GetNeighbourCellIds(cellId, idArray, width);

        float sum = 0;
        for (int k = 0; k < 6; k++) {
            int id = idArray[k];
            if (!CheckReceptiveCell(prevData, id, width, height))
                sum += prevData[id];
        }

        float cellR = (CheckReceptiveCell(prevData, cellId, width, height) ? 1.0 : 0.0);
        float cellU = (cellR == 0.0 ? prevData[cellId] : 0.0);

        curData[cellId] = prevData[cellId] + (alpha / 2.0) * ((sum / 6.0) - cellU) + (gamma * cellR);
    }
}

double ReiterCUDA::RunSimulation(float alpha, float beta, float gamma)
{
    // Host data
    size_t* idArrayHost;

    // Device data
    float* curDataDevice;
    float* prevDataDevice;
    size_t* idArrayDevice;

    // Allocate host memory
    auto curDataHost = CreateGrid(beta);
    auto prevDataHost = CreateGrid(beta);
    idArrayHost = (size_t*)malloc(6 * sizeof(size_t));

    LogState(curDataHost.get(), 0);

    // Allocate device memory
    cudaMalloc((void**)&curDataDevice, m_Height * m_Width * sizeof(float));
    cudaMalloc((void**)&prevDataDevice, m_Height * m_Width * sizeof(float));
    cudaMalloc((void**)&idArrayDevice, 6 * sizeof(size_t));

    auto start = std::chrono::high_resolution_clock::now();

    // Copy initial data to device
    cudaMemcpy(curDataDevice, curDataHost.get(), m_Height * m_Width * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(prevDataDevice, prevDataHost.get(), m_Height * m_Width * sizeof(float), cudaMemcpyHostToDevice);

    bool stable = false;
    size_t iter = 0;

    while (!stable && iter <= MAX_ITER)
    {
        stable = true;

        int blockSize = 256;
        int gridSize = (m_Height * m_Width + blockSize - 1) / blockSize;
        simulationKernel<<<gridSize, blockSize>>>(curDataDevice, prevDataDevice, m_Height, m_Width, idArrayDevice, MAX_ITER, alpha, beta, gamma);

        cudaDeviceSynchronize();

        float* tmp;
        tmp = curDataDevice;
        curDataDevice = prevDataDevice;
        prevDataDevice = tmp;

        iter++;
        stable = false;
    }

    // Get data from device
    float* dataOutput = (float*)malloc(m_Height * m_Width * sizeof(float));
    cudaMemcpy(dataOutput, curDataDevice, m_Height * m_Width * sizeof(float), cudaMemcpyDeviceToHost);
    LogState(dataOutput, 1);

    auto stop = std::chrono::high_resolution_clock::now();
    printf("Simulation took %ld iterations\n", iter);
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    // Free device memory
    cudaFree(curDataDevice);
    cudaFree(prevDataDevice);
    cudaFree(idArrayDevice);

    // Free host memory
    //free(curDataHost);
    //free(prevDataHost);
    free(idArrayHost);

    return (duration.count() * 1e-6);
}

int main(int argc, char** argv)
{
    ReiterCUDA model(100, 100);
    auto dur = model.RunSimulation(1, 0.2, 0.01);

    int deviceCount;
    cudaGetDeviceCount(&deviceCount);
    printf("Ran on %d GPU(s)\n", deviceCount);
    printf("Execution took %lf seconds\n", dur);

    return 0;
}
