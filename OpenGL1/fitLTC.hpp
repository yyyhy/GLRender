#pragma once
#include"brdf_ddx.hpp"
#include"ltc.hpp"
#include"nelder_mead.hpp"
#include<iostream>
#include"stb.hpp"


#define MIN_ALPHA 0.00001f

static const unsigned Nsample = 32;

void computeAvgTerms(const BRDF& brdf, const Vector3f& V, const float alpha,
    float& norm, float& fresnel, Vector3f& averageDir) {
    norm = 0.0f;
    fresnel = 0.0f;
    averageDir = Vector3f(0, 0, 0);

    for (int j = 0; j < Nsample; ++j)
        for (int i = 0; i < Nsample; ++i)
        {
            const float U1 = (i + 0.5f) / Nsample;
            const float U2 = (j + 0.5f) / Nsample;

            // sample
            const Vector3f L = brdf.sample(V, alpha, U1, U2);

            // eval
            float pdf;
            float eval = brdf.eval(V, L, alpha, pdf);

            if (pdf > 0)
            {
                float weight = eval / pdf;

                Vector3f H = normalize(V + L);

                // accumulate
                norm += weight;
                fresnel += weight * pow(1.0f - glm::max(dot(V, H), 0.0f), 5.0f);
                averageDir += weight * L;
            }
        }

    norm /= (float)(Nsample * Nsample);
    fresnel /= (float)(Nsample * Nsample);

    // clear y component, which should be zero with isotropic BRDFs
    averageDir.y = 0.0f;

    averageDir = normalize(averageDir);
}


float computeError(const LTC& ltc, const BRDF& brdf, const Vector3f& V, const float alpha)
{
    double error = 0.0;

    for (int j = 0; j < Nsample; ++j)
        for (int i = 0; i < Nsample; ++i)
        {
            const float U1 = (i + 0.5f) / Nsample;
            const float U2 = (j + 0.5f) / Nsample;

            // importance sample LTC
            {
                // sample
                const Vector3f L = ltc.sample(U1, U2);

                float pdf_brdf;
                float eval_brdf = brdf.eval(V, L, alpha, pdf_brdf);
                float eval_ltc = ltc.eval(L);
                float pdf_ltc = eval_ltc / ltc.magnitude;

                // error with MIS weight
                double error_ = fabsf(eval_brdf - eval_ltc);
                error_ = error_ * error_ * error_;
                error += error_ / (pdf_ltc + pdf_brdf);
            }

            // importance sample BRDF
            {
                // sample
                const Vector3f L = brdf.sample(V, alpha, U1, U2);

                float pdf_brdf;
                float eval_brdf = brdf.eval(V, L, alpha, pdf_brdf);
                float eval_ltc = ltc.eval(L);
                float pdf_ltc = eval_ltc / ltc.magnitude;

                // error with MIS weight
                double error_ = fabsf(eval_brdf - eval_ltc);
                error_ = error_ * error_ * error_;
                error += error_ / (pdf_ltc + pdf_brdf);
            }
        }

    return (float)error / (float)(Nsample * Nsample);
}

struct FitLTC
{
    FitLTC(LTC& ltc_, const BRDF& brdf, bool isotropic_, const Vector3f& V_, float alpha_) :
        ltc(ltc_), brdf(brdf), V(V_), alpha(alpha_), isotropic(isotropic_)
    {
    }

    void update(const float* params)
    {
        float m11 = std::max<float>(params[0], 1e-7f);
        float m22 = std::max<float>(params[1], 1e-7f);
        float m13 = params[2];

        if (isotropic)
        {
            ltc.m11 = m11;
            ltc.m22 = m11;
            ltc.m13 = 0.0f;
        }
        else
        {
            ltc.m11 = m11;
            ltc.m22 = m22;
            ltc.m13 = m13;
        }
        ltc.update();
    }

    float operator()(const float* params)
    {
        update(params);
        return computeError(ltc, brdf, V, alpha);
    }

    const BRDF& brdf;
    LTC& ltc;
    bool isotropic;

    const Vector3f& V;
    float alpha;
};


// fit brute force
// refine first guess by exploring parameter space
void fit(LTC& ltc, const BRDF& brdf, const Vector3f& V, const float alpha, const float epsilon = 0.05f, const bool isotropic = false)
{
    float startFit[3] = { ltc.m11, ltc.m22, ltc.m13 };
    float resultFit[3];

    FitLTC fitter(ltc, brdf, isotropic, V, alpha);

    // Find best-fit LTC lobe (scale, alphax, alphay)
    float error = NelderMead<3>(resultFit, startFit, epsilon, 1e-5f, 100, fitter);

    // Update LTC with best fitting values
    fitter.update(resultFit);
}

// fit data
void fitTab(glm::mat3* tab, Vector2f* tabMagFresnel, const int N, const BRDF& brdf)
{
    LTC ltc;

    // loop over theta and alpha
    for (int a = N - 1; a >= 0; --a)
        for (int t = 0; t <= N - 1; ++t)
        {
            // parameterised by sqrt(1 - cos(theta))
            float x = t / float(N - 1);
            float ct = 1.0f - x * x;
            float theta = std::min<float>(1.57f, acosf(ct));
            const Vector3f V = Vector3f(sinf(theta), 0, cosf(theta));

            // alpha = roughness^2
            float roughness = a / float(N - 1);
            float alpha = std::max<float>(roughness * roughness, MIN_ALPHA);

            std::cout << "a = " << a << "\t t = " << t << std::endl;
            std::cout << "alpha = " << alpha << "\t theta = " << theta << std::endl;
            std::cout << std::endl;

            Vector3f averageDir;
            computeAvgTerms(brdf, V, alpha, ltc.magnitude, ltc.fresnel, averageDir);

            bool isotropic;

            // 1. first guess for the fit
            // init the hemisphere in which the distribution is fitted
            // if theta == 0 the lobe is rotationally symmetric and aligned with Z = (0 0 1)
            if (t == 0)
            {
                ltc.X = Vector3f(1, 0, 0);
                ltc.Y = Vector3f(0, 1, 0);
                ltc.Z = Vector3f(0, 0, 1);

                if (a == N - 1) // roughness = 1
                {
                    ltc.m11 = 1.0f;
                    ltc.m22 = 1.0f;
                }
                else // init with roughness of previous fit
                {
                    ltc.m11 = tab[a + 1 + t * N][0][0];
                    ltc.m22 = tab[a + 1 + t * N][1][1];
                }

                ltc.m13 = 0;
                ltc.update();

                isotropic = true;
            }
            // otherwise use previous configuration as first guess
            else
            {
                Vector3f L = averageDir;
                Vector3f T1(L.z, 0, -L.x);
                Vector3f T2(0, 1, 0);
                ltc.X = T1;
                ltc.Y = T2;
                ltc.Z = L;

                ltc.update();

                isotropic = false;
            }

            // 2. fit (explore parameter space and refine first guess)
            float epsilon = 0.05f;
            fit(ltc, brdf, V, alpha, epsilon, isotropic);

            // copy data
            tab[a + t * N] = ltc.M;
            tabMagFresnel[a + t * N][0] = ltc.magnitude;
            tabMagFresnel[a + t * N][1] = ltc.fresnel;

            // kill useless coefs in matrix
            tab[a + t * N][0][1] = 0;
            tab[a + t * N][1][0] = 0;
            tab[a + t * N][2][1] = 0;
            tab[a + t * N][1][2] = 0;

            /*std::cout << tab[a + t * N][0][0] << "\t " << tab[a + t * N][1][0] << "\t " << tab[a + t * N][2][0] << std::endl;
            std::cout << tab[a + t * N][0][1] << "\t " << tab[a + t * N][1][1] << "\t " << tab[a + t * N][2][1] << std::endl;
            std::cout << tab[a + t * N][0][2] << "\t " << tab[a + t * N][1][2] << "\t " << tab[a + t * N][2][2] << std::endl;
            std::cout << std::endl;*/
        }
}

void genLTC_Lut(const unsigned N) {
    BRDFGGX brdf;

    glm::mat3* m = new glm::mat3[N * N];
    glm::vec2* f = new glm::vec2[N * N];

    fitTab(m, f, N, brdf);
    unsigned char* tex1 = new unsigned char[N * N * 4];
    for (unsigned i = 0; i < N; i++) {
        for (unsigned j = 0; j < N; j++) {
            auto tmp = m[i*N+j];
            auto inv = glm::inverse(tmp);
            inv /= inv[1][1];
            tex1[4 * (i * N + j)]     = inv[0][0] * 255.f;
            tex1[4 * (i * N + j) + 1] = inv[0][2] * 255.f;
            tex1[4 * (i * N + j) + 2] = inv[2][0] * 255.f;
            tex1[4 * (i * N + j) + 3] = inv[2][2] * 255.f;
            //std::cout << inv[0][0] << " " << inv[0][2] << " " << inv[2][0] << " " << inv[2][2] << "\n";
        }
    }
    stbi_write_png("ltc_lut1.png", N, N, 4, tex1, 0);
    delete[] m;
    delete[] f;
    delete[] tex1;
    std::cout << "\n";
}