#include "ReiterSim.h"
#include <vector>
#include <cmath>

std::shared_ptr<float> ReiterSimulation::CreateGrid(float beta)
{
    auto data = std::shared_ptr<float>((float*)malloc(m_Width * m_Height * sizeof(float)), free);
    
    for (size_t i = 0; i < m_Height * m_Width; i++)
        data.get()[i] = beta;

    data.get()[(m_Height / 2) * m_Width + (m_Width / 2)] = 1;

    return data;
}

void ReiterSimulation::GetNeighbourCellIds(size_t cellId, const std::shared_ptr<size_t> &outIdArray)
{
    size_t j = cellId % m_Width;
    size_t i = (cellId - j) / m_Width;

    int nOff;
    if (j%2 == 0)
        nOff = -1;
    else
        nOff = 0;
        
    outIdArray.get()[0] = m_Width * (i-1) + j;
    outIdArray.get()[1] = m_Width * (nOff + i) + j - 1;
    outIdArray.get()[2] = m_Width * (nOff + i+1) + j - 1;
    outIdArray.get()[3] = m_Width * (nOff + i) + j + 1;
    outIdArray.get()[4] = m_Width * (nOff + i+1) + j + 1;
    outIdArray.get()[5] = m_Width * (i+1) + j;
}

bool ReiterSimulation::CheckReceptiveCell(const std::shared_ptr<float> &data, size_t cellId)
{
    if(data.get()[cellId] >= 1)
        return true;

    size_t j = cellId % m_Width;
    size_t i = (cellId - j) / m_Width;

    int nOff;
    if (j%2 == 0)
        nOff = -1;
    else
        nOff = 0;
        
    if(data.get()[m_Width * (i-1) + j] >= 1)
        return true;
    if(data.get()[m_Width * (nOff + i) + j - 1] >= 1)
        return true;
    if(data.get()[m_Width * (nOff + i+1) + j - 1] >= 1)
        return true;
    if(data.get()[m_Width * (nOff + i) + j + 1] >= 1)
        return true;
    if(data.get()[m_Width * (nOff + i+1) + j + 1] >= 1)
        return true;
    if(data.get()[m_Width * (i+1) + j] >= 1)
        return true;

    return false;
}
