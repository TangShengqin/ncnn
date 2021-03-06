// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2017 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include "bias.h"

namespace ncnn {

DEFINE_LAYER_CREATOR(Bias)

Bias::Bias()
{
    one_blob_only = true;
    support_inplace = true;
}

Bias::~Bias()
{
}

#if NCNN_STDIO
#if NCNN_STRING
int Bias::load_param(FILE* paramfp)
{
    int nscan = fscanf(paramfp, "%d", &bias_data_size);
    if (nscan != 1)
    {
        fprintf(stderr, "Bias load_param failed %d\n", nscan);
        return -1;
    }

    return 0;
}
#endif // NCNN_STRING
int Bias::load_param_bin(FILE* paramfp)
{
    fread(&bias_data_size, sizeof(int), 1, paramfp);

    return 0;
}

int Bias::load_model(FILE* binfp)
{
    int nread;

    bias_data.create(bias_data_size);
    if (bias_data.empty())
        return -100;
    nread = fread(bias_data, bias_data_size * sizeof(float), 1, binfp);
    if (nread != 1)
    {
        fprintf(stderr, "Bias read bias_data failed %d\n", nread);
        return -1;
    }

    return 0;
}
#endif // NCNN_STDIO

int Bias::load_param(const unsigned char*& mem)
{
    bias_data_size = *(int*)(mem);
    mem += 4;

    return 0;
}

int Bias::load_model(const unsigned char*& mem)
{
    bias_data = Mat(bias_data_size, (float*)mem);
    mem += bias_data_size * sizeof(float);

    return 0;
}

int Bias::forward(const Mat& bottom_blob, Mat& top_blob) const
{
    int w = bottom_blob.w;
    int h = bottom_blob.h;
    int channels = bottom_blob.c;
    int size = w * h;

    top_blob.create(w, h, channels);
    if (top_blob.empty())
        return -100;

    const float* bias_ptr = bias_data;
    #pragma omp parallel for
    for (int q=0; q<channels; q++)
    {
        const float* ptr = bottom_blob.channel(q);
        float* outptr = top_blob.channel(q);

        float bias = bias_ptr[q];

        for (int i=0; i<size; i++)
        {
            outptr[i] = ptr[i] + bias;
        }
    }

    return 0;
}

int Bias::forward_inplace(Mat& bottom_top_blob) const
{
    int w = bottom_top_blob.w;
    int h = bottom_top_blob.h;
    int channels = bottom_top_blob.c;
    int size = w * h;

    const float* bias_ptr = bias_data;
    #pragma omp parallel for
    for (int q=0; q<channels; q++)
    {
        float* ptr = bottom_top_blob.channel(q);

        float bias = bias_ptr[q];

        for (int i=0; i<size; i++)
        {
            ptr[i] += bias;
        }
    }

    return 0;
}

} // namespace ncnn
