#include "ReiterCUDA.h"
#include <cuda.h>
#include <chrono>
#include <stdio.h>

__device__ void GetNeighbourCellIds(size_t cellId, size_t* outIdArray, int width)
{
    size_t j = cellId % width;
    size_t i = (cellId - j) / width;

    int nOff;
    if (j%2 == 0)
        nOff = -1;
    else
        nOff = 0;
        
    outIdArray[0] = width * (i-1) + j;
    outIdArray[1] = width * (nOff + i) + j - 1;
    outIdArray[2] = width * (nOff + i+1) + j - 1;
    outIdArray[3] = width * (nOff + i) + j + 1;
    outIdArray[4] = width * (nOff + i+1) + j + 1;
    outIdArray[5] = width * (i+1) + j;
}

__device__ bool CheckReceptiveCell(float* data, size_t cellId, int width, int height)
{
    if(data[cellId] >= 1)
        return true;

    int j = cellId % width;
    int i = (cellId - j) / width;

    int nOff;
    if (j%2 == 0)
        nOff = -1;
    else
        nOff = 0;
        
    if(i>0 && data[width * (i-1) + j] >= 1)
        return true;
    if(j>0 && (nOff + i) > 0 && data[width * (nOff + i) + j - 1] >= 1)
        return true;
    if(j>0 && (nOff + i+1) < height && data[width * (nOff + i+1) + j - 1] >= 1)
        return true;
    if(j+1 < width && (nOff + i) > 0 && data[width * (nOff + i) + j + 1] >= 1)
        return true;
    if(j+1 < width && (nOff + i+1) < height && data[width * (nOff + i+1) + j + 1] >= 1)
        return true;
    if(i+1 < height && data[width * (i+1) + j] >= 1)
        return true;

    return false;
}

__global__ void simulationKernel(float* curData, float* prevData, int height, int width, float alpha, float beta, float gamma)
{
    int cellId = blockIdx.x * blockDim.x + threadIdx.x;

    if (cellId >= height * width)
        return;

    size_t idArray[6];
    
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

double ReiterCUDA::RunSimulation(float alpha, float beta, float gamma)
{
    // Device data
    float* curDataDevice;
    float* prevDataDevice;

    // Allocate host memory
    auto hostGrid = CreateGrid(beta);

    // Allocate device memory
    cudaMalloc((void**)&curDataDevice, m_Height * m_Width * sizeof(float));
    cudaMalloc((void**)&prevDataDevice, m_Height * m_Width * sizeof(float));

    auto start = std::chrono::high_resolution_clock::now();

    // Copy initial data to device
    cudaMemcpy(curDataDevice, hostGrid.get(), m_Height * m_Width * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(prevDataDevice, hostGrid.get(), m_Height * m_Width * sizeof(float), cudaMemcpyHostToDevice);

    bool stable = false;
    size_t iter = 0;

    LogState(hostGrid.get(), 0);
    while (!stable && iter <= MAX_ITER)
    {
        stable = true;

        int blockSize = 256;
        int gridSize = (m_Height * m_Width + blockSize - 1) / blockSize;
        simulationKernel<<<gridSize, blockSize>>>(curDataDevice, prevDataDevice, m_Height, m_Width, alpha, beta, gamma);

        cudaDeviceSynchronize();

        auto tmp = curDataDevice;
        curDataDevice = prevDataDevice;
        prevDataDevice = tmp;

        iter++;
        stable = false;
    }

    // Get data from device
    cudaMemcpy(hostGrid.get(), curDataDevice, m_Height * m_Width * sizeof(float), cudaMemcpyDeviceToHost);
    LogState(hostGrid.get(), iter);

    auto stop = std::chrono::high_resolution_clock::now();
    printf("Simulation took %ld iterations\n", iter);
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    // Free device memory
    cudaFree(curDataDevice);
    cudaFree(prevDataDevice);

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
