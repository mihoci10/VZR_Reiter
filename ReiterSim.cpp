#include "ReiterSim.h"

#include "lib/FreeImage.h"

#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>

std::shared_ptr<float> ReiterSimulation::CreateGrid(float beta)
{
    auto data = std::shared_ptr<float>((float*)malloc(m_Width * m_Height * sizeof(float)), free);
    
    for (int i = 0; i < m_Height * m_Width; i++)
        data.get()[i] = beta;

    data.get()[(m_Height / 2) * m_Width + (m_Width / 2)] = 1;

    return data;
}

void ReiterSimulation::GetNeighbourCellIds(size_t cellId, size_t* outIdArray)
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

bool ReiterSimulation::CheckReceptiveCell(float* data, size_t cellId)
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

void ReiterSimulation::LogState(float* data, size_t iter)
{
    switch (m_DebugMode)
    {
        case DebugType::None:
            return;
        case DebugType::Txt:
            SaveStateToTxt(data, std::string(typeid(*this).name()) + std::to_string(iter) + std::string(".txt"));
            return;
        case DebugType::Img:
            SaveStateToImg(data, std::string(typeid(*this).name()) + std::to_string(iter) + std::string(".png"));
            return;
        case DebugType::All:
            SaveStateToTxt(data, std::string(typeid(*this).name()) + std::to_string(iter) + std::string(".txt"));
            SaveStateToImg(data, std::string(typeid(*this).name()) + std::to_string(iter) + std::string(".png"));
            return;
    }
}

void ReiterSimulation::SaveStateToTxt(float* data, const std::string& filename)
{
    std::ofstream file(filename);

    for (int i = 0; i < m_Height; i++)
    {
        for (int j = 0; j < m_Width; j++)
        {
            file << std::to_string(data[i * m_Width + j]);
            file << "\t";
        }
        file << "\n";
    }

    file.close();
}

void ReiterSimulation::SaveStateToImg(float* data, const std::string& filename)
{
    int imgHeight = PIX_PER_CELL * 2 * m_Height + PIX_PER_CELL;
    int imgWidth = PIX_PER_CELL * m_Width;
    int imgPitch = ((32 * imgWidth + 31) / 32) * 4;

    unsigned char *imageData = (unsigned char *)calloc(imgHeight * imgWidth * 4, sizeof(unsigned char));

    for (int i = 0; i < m_Height; i++)
    {
        for (int j = 0; j < m_Width; j++)
        {
            int nOff;
            if (j%2 == 0)
                nOff = 0;
            else
                nOff = PIX_PER_CELL;

            int imgI = nOff + (i * PIX_PER_CELL * 2);
            int imgJ = (j * PIX_PER_CELL);

            float val = data[i * m_Width + j];
            unsigned char imgVal = (val / 10) * 255;

            for(int x = 0; x < PIX_PER_CELL; x++){
                for (int y = 0; y < 2 * PIX_PER_CELL; y++){
                    //zapisemo barvo RGBA (v resnici little endian BGRA)
                    imageData[4 * (imgI+y)*imgWidth + 4 * (imgJ+x) + 0] = imgVal; //Blue
                    imageData[4 * (imgI+y)*imgWidth + 4 * (imgJ+x) + 1] = 0; // Green
                    imageData[4 * (imgI+y)*imgWidth + 4 * (imgJ+x) + 2] = 0; // Red
                    imageData[4 * (imgI+y)*imgWidth + 4 * (imgJ+x) + 3] = 255;   // Alpha
                }
            }
        }
    }

    FIBITMAP *dst = FreeImage_ConvertFromRawBits(imageData, imgWidth, imgHeight, imgPitch,
		32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);
	FreeImage_Save(FIF_PNG, dst, filename.c_str(), 0);

    free(imageData);
}
