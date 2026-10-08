#include "Benchmark/Benchmark.h"
#include "Platform/CpuFeatures.h"
#include "Maths/Vec4f.h"
#include "Maths/Vec4d.h"
#include "Maths/Vec3f.h"
#include "Maths/Vec3d.h"
#include "Maths/Base_Matrix3x3.h"
#include "Maths/Matrix3x3.h"
#include "Maths/Base_Matrix4x4.h"
#include "Maths/Matrix4x4.h"
#include "Maths/Base_Vec3.h"
#include "Maths/Base_Vec4.h"

#include <immintrin.h>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#define ANKERL_NANOBENCH_IMPLEMENT
#include <Benchmark/nanobench.h>

#include <chrono>
#include <thread>

int main(const int _argc, char** _argv)
{
    /*
    //Vec4f test;
    //Vec4f a{ 1,2,3,4 };
    //Vec4f b{ 5,6,7,8 };
    //Matrix4x4 ma({ 1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f });
    //Matrix4x4 mb;
    //BMatrix4x4 bma({ 1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f });
    //BMatrix4x4 bmb;
    //BVec4<float> ba{ 1,2,3,4 };
    //BVec4<float> bb{ 5,6,7,8 };

    //Vec4f c{ 3,0,4,0 };
    //BVec4<float> bc{ 3,0,4,0 };


    //float e = 10.f;

    //double d = 1;
    //float cFloat = 0.f;
    //float bcFloat = 0.f;

    //std::cout << "-------- Floats --------" << std::endl;
    //std::cout << c.w << std::endl;
    //std::cout << d.w << std::endl;
    //std::cout << e.w << std::endl;
    //std::cout << "-------- Double --------" << std::endl;
    //std::cout << c1.w << std::endl;
    //std::cout << d1.w << std::endl;
    //std::cout << e1.w << std::endl;

    //bench.run("base_*_scalar", [&] {
    //    mb = mb.ZeroSIMD();
    //    ankerl::nanobench::doNotOptimizeAway(mb);
    //    });
    //cDot = a.Dot(b);
    //c = -c;
    //std::cout << c.reg.m128_f32[0] << " " << c.reg.m128_f32[1] << " " << c.reg.m128_f32[2] << " " << c.reg.m128_f32[3] << std::endl;
    //std::cout << mb.row[0].m128_f32[0] << std::endl;
    //std::cout << c.reg.m128_f32[1];
    */

    using Maths::Vec3f;
    using Maths::Vec4f;
    using Maths::Matrix4x4;
    using Base_Maths::BMatrix4x4;
    using Base_Maths::BVec4;

    const Vec3f a{ 1,2,3 };
    const Vec3f b{ 4,5,6 };
    std::cout << "Dot: " << a.Dot(b) << '\n';
    //const auto matrix = Matrix4x4::Translation({ 10,20,30 }) * Matrix4x4::Scale({ 2,3,4 });
    //const auto point = matrix.TransformPoint(a);
    //std::cout << "Point: " << point.reg.m128_f32[0] << ", " << point.reg.m128_f32[1] << ", " << point.reg.m128_f32[2] << '\n';

#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
    const auto cpu = Platform::CpuFeatures::Detect();
    std::cout << "CPU: " << cpu.Vendor() << " / " << cpu.Brand() << '\n';
    std::cout << std::boolalpha
        << "SSE usable: " << cpu.CanUse(Platform::CpuFeature::SSE) << '\n'
        << "AVX2 usable: " << cpu.CanUse(Platform::CpuFeature::AVX2) << '\n'
        << "FMA usable: " << cpu.CanUse(Platform::CpuFeature::FMA) << '\n';
#else
    std::cout << "CPU detection demo is MSVC/Windows-only.\n";
#endif

    // Runtime inputs. Optional integer seed allows reproducing a run.
    const auto seed = _argc > 1 ? static_cast<unsigned int>(std::strtoul(_argv[1], nullptr, 10))
        : std::random_device{}();
    std::mt19937 engine(seed);
    std::uniform_real_distribution<float> distribution(-10.0f, 10.0f);
    const std::size_t count = 250000;
    std::vector<Vec4f> left(count);
    std::vector<Vec4f> right(count);
    std::vector<Vec4f> input(count);
    std::vector<Vec4f> outputSIMD(count);
    std::vector<BVec4<float>> outputCPP(count);
    std::vector<Vec4f> vecsSIMD(count);
    std::vector<BVec4<float>> vecsCPP(count);
    std::vector<Matrix4x4> Mat4x4SIMD(count);
    std::vector<BMatrix4x4<float>> Mat4x4CPP(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        left[i] = { distribution(engine), distribution(engine), distribution(engine), distribution(engine) };        
        right[i] = { distribution(engine), distribution(engine), distribution(engine), distribution(engine) };
		vecsSIMD[i] = { distribution(engine), distribution(engine), distribution(engine), distribution(engine) };
        vecsCPP[i] = { distribution(engine), distribution(engine), distribution(engine), distribution(engine) };
		Mat4x4CPP[i] = BMatrix4x4<float>({ distribution(engine), distribution(engine), distribution(engine), distribution(engine),
			distribution(engine), distribution(engine), distribution(engine), distribution(engine),
			distribution(engine), distribution(engine), distribution(engine), distribution(engine),
			distribution(engine), distribution(engine), distribution(engine), distribution(engine) });
		Mat4x4SIMD[i] = Matrix4x4({ distribution(engine), distribution(engine), distribution(engine), distribution(engine),
			distribution(engine), distribution(engine), distribution(engine), distribution(engine),
			distribution(engine), distribution(engine), distribution(engine), distribution(engine),
			distribution(engine), distribution(engine), distribution(engine), distribution(engine) });
    }

    const auto result = Benchmark::Run([&]
        {
            for (std::size_t i = 0; i < count; ++i)
            {
                outputSIMD[i] = vecsSIMD[i].Normalize();
            }
            // Included in the measured time. Simple and observable, not a magic barrier.
            double checksum = 0;
            for (const auto& v : outputSIMD)
            {
                checksum += static_cast<double>(v.reg.m128_f32[0]) + v.reg.m128_f32[1] + v.reg.m128_f32[2] + v.reg.m128_f32[3];
            }
            return checksum;
        });

    std::cout << std::fixed << std::setprecision(3)
        << "Seed: " << seed << ", vectors per batch: " << count << '\n'
        << "Batch incl. checksum: min " << result.minimumMs
        << " ms, median " << result.medianMs
        << " ms, max " << result.maximumMs << " ms\n"
        << "Observable checksum: " << result.checksum << '\n';


    ankerl::nanobench::Bench bench;
    bench.epochs(100);
    bench.warmup(10);
    bench.batch(count);

    std::vector<float> dotOut(count);

    bench.run("Normalize", [&] {
        for (std::size_t i = 0; i < count; ++i)
            outputSIMD[i] = vecsSIMD[i].Normalize();
        ankerl::nanobench::doNotOptimizeAway(outputSIMD.data());
        });

    bench.run("CPP_Normalize", [&] {
        for (std::size_t i = 0; i < count; ++i)
            outputCPP[i] = vecsCPP[i].Normalize();
        ankerl::nanobench::doNotOptimizeAway(outputCPP.data());
        });

    bench.run("Dot", [&] {
        for (std::size_t i = 0; i < count; ++i)
            dotOut[i] = vecsSIMD[i].Dot(vecsSIMD[i]);
        ankerl::nanobench::doNotOptimizeAway(dotOut.data());
        });

    bench.run("CPP_Dot", [&] {
        for (std::size_t i = 0; i < count; ++i)
            dotOut[i] = vecsCPP[i].Dot(vecsCPP[i]);
        ankerl::nanobench::doNotOptimizeAway(dotOut.data());
        });

    bench.run("Mat4x4 * Vec4f", [&] {
        for (std::size_t i = 0; i < count; ++i)
            outputSIMD[i] = Mat4x4SIMD[i] * vecsSIMD[i];
        ankerl::nanobench::doNotOptimizeAway(outputSIMD.data());
        });

    bench.run("CPP_Mat4x4 * Vec4f", [&] {
        for (std::size_t i = 0; i < count; ++i)
            outputCPP[i] = Mat4x4CPP[i] * vecsCPP[i];
        ankerl::nanobench::doNotOptimizeAway(outputCPP.data());
        });    
	

    return 0;
}


