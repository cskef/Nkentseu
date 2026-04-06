// Tests unitaires NkMath - Semaines 1-4
// Framework de test : Unitest

#include <cstdlib>
#include <memory>
#include <random>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <chrono>

#include <Unitest/TestMacro.h>
#include <Unitest/Unitest.h>

#include "Mat4d.h"
#include "Quat.h"
#include "NKImage.h"
#include "NKMath/NKMath.h"
#include "NKLogger/NkLog.h"

using namespace NkMath;

//=== Global Config ===
const int canvas_width = 512;
const int canvas_height = 512;
NkImage framebuffer(canvas_width, canvas_height);

std::mt19937 randomEngine(42);
std::uniform_real_distribution<double> randomDist(-10.0, 10.0);

// Cube geometry
std::vector<Vec4d> cubeMesh = {
    {-0.5,-0.5,-0.5,1}, {0.5,-0.5,-0.5,1},
    {0.5, 0.5,-0.5,1}, {-0.5, 0.5,-0.5,1},
    {-0.5,-0.5, 0.5,1}, {0.5,-0.5, 0.5,1},
    {0.5, 0.5, 0.5,1}, {-0.5, 0.5, 0.5,1}
};

std::vector<Vec2d> cubeEdges = {
    {0,1},{1,2},{2,3},{3,0},
    {4,5},{5,6},{6,7},{7,4},
    {0,4},{1,5},{2,6},{3,7}
};

// Camera setup
Vec3d cameraPos{0,1,3};
Vec3d cameraTarget{0,0,0};
Vec3d cameraUp{0,1,0};
Mat4d viewMatrix = LookAt(cameraPos, cameraTarget, cameraUp);
Mat4d projMatrix = Perspective(60.0, double(canvas_width)/canvas_height, 0.1, 100.0);

//=== Week 1 Tests ===

TEST_CASE(Semaine1_TP1, BinaryFloatInspection) {
    inspectFloat(0.1f);
    inspectFloat(1.0f);
    inspectFloat(1.0f / 0.0f);
    inspectFloat(std::sqrt(-1.0f));
    inspectFloat(-0.0f);
    inspectFloat(0.0f);
    inspectFloat(std::numeric_limits<float>::min());
}

TEST_CASE(Semaine1_TP2, PrecisionIssues) {
    float kahanResult, naiveResult;
    std::vector<float> dataset;

    dataset = std::vector<float>(1'000'000, 0.1f);
    naiveResult = std::accumulate(dataset.begin(), dataset.end(), 0.0f);
    kahanResult = kahanSum(dataset);
    logger.Info("\nAccum result: {0}\nKahan result: {1}\nExpected: 100000", naiveResult, kahanResult);
    
    dataset = std::vector<float>({1e8f, 1e8f, 1.0f, 2.0f});
    logger.Info("\nNaive var: {0}\nWelford var: {1}", varianceNaive(dataset), varianceWelford(dataset));
    
    logger.Info("\nEpsilon_loop: {0}\nEpsilon_std: {1}", epsilonMachine(), std::numeric_limits<float>::epsilon());
}

TEST_CASE(Semaine1_TP3, FloatUtilTests) {
    // isFiniteValid (5)
    ASSERT_TRUE(!isFiniteValid(std::numeric_limits<float>::quiet_NaN()));
    ASSERT_TRUE(!isFiniteValid(std::numeric_limits<float>::infinity()));
    ASSERT_TRUE(!isFiniteValid(-std::numeric_limits<float>::infinity()));
    ASSERT_TRUE(isFiniteValid(0.0f));
    ASSERT_TRUE(isFiniteValid(1.0f));

    // nearlyZero (8)
    ASSERT_TRUE(nearlyZero(0.0f, 1e-6f));
    ASSERT_TRUE(!nearlyZero(1e-5f, 1e-6f));
    ASSERT_TRUE(nearlyZero(1e-7f, 1e-6f));
    ASSERT_TRUE(nearlyZero(-1e-7f, 1e-6f));
    ASSERT_TRUE(!nearlyZero(-1e-6f, 1e-7f));
    ASSERT_TRUE(nearlyZero(1e-3f, 1e-2f));
    ASSERT_TRUE(!nearlyZero(1e-2f, 1e-3f));
    ASSERT_TRUE(nearlyZero(5e-8f, 1e-7f));

    // approxEq (10)
    ASSERT_TRUE(approxEq(1.0f, 1.0f, 1e-6f));
    ASSERT_TRUE(approxEq(1.0f, 1.0000001f, 1e-5f));
    ASSERT_TRUE(!approxEq(1.0f, 1.1f, 1e-3f));
    ASSERT_TRUE(approxEq(0.0f, 1e-7f, 1e-6f));
    ASSERT_TRUE(!approxEq(0.0f, 1e-4f, 1e-6f));
    ASSERT_TRUE(approxEq(-1.0f, -1.000001f, 1e-5f));
    ASSERT_TRUE(!approxEq(-1.0f, -1.1f, 1e-2f));
    ASSERT_TRUE(approxEq(1000.0f, 1000.0001f, 1e-3f));
    ASSERT_TRUE(approxEq(1000.0f, 1001.0f, 1e-3f));
    ASSERT_TRUE(approxEq(1e-7f, 2e-7f, 1e-6f));

    // kahanSum (10)
    float res1, res2;
    std::vector<float> vals;
  
    vals = std::vector<float>(1000, 0.1f);
    res1 = std::accumulate(vals.begin(), vals.end(), 0.0f);
    res2 = kahanSum(vals);
    ASSERT_TRUE(std::fabs(res2 - 100.0f) < std::fabs(res1 - 100.0f));
    
    vals = std::vector<float>(10000, 0.1f);
    res1 = std::accumulate(vals.begin(), vals.end(), 0.0f);
    res2 = kahanSum(vals);
    ASSERT_TRUE(std::fabs(res2 - 1000.0f) < std::fabs(res1 - 1000.0f));
    
    vals = std::vector<float>({1e8f, 1.0f, -1e8f});
    res1 = std::accumulate(vals.begin(), vals.end(), 0.0f);
    res2 = kahanSum(vals);
    ASSERT_TRUE(std::fabs(res2 - 1.0f) <= std::fabs(res1 - 1.0f));

    vals = std::vector<float>({1.0f, 1e8f, -1e8f});
    res1 = std::accumulate(vals.begin(), vals.end(), 0.0f);
    res2 = kahanSum(vals);
    ASSERT_TRUE(std::fabs(res2 - 1.0f) <= std::fabs(res1 - 1.0f));
    
    vals = std::vector<float>(100000, 0.01f);
    res2 = kahanSum(vals);
    ASSERT_TRUE(approxEq(res2, 1000.0f, 1e-2f));

    vals = std::vector<float>(100000, 1e-5f);
    res2 = kahanSum(vals);
    ASSERT_TRUE(approxEq(res2, 1.0f, 1e-3f));
    
    vals = std::vector<float>({0.1f, 0.2f, 0.3f});
    res2 = kahanSum(vals);
    ASSERT_TRUE(approxEq(res2, 0.6f, 1e-6f));

    vals = std::vector<float>(50000, 0.2f);
    res2 = kahanSum(vals);
    ASSERT_TRUE(approxEq(res2, 10000.0f, 1e-2f));
    
    vals = std::vector<float>({1e7f, 1.0f, 1.0f, -1e7f});
    res2 = kahanSum(vals);
    ASSERT_TRUE(approxEq(res2, 2.0f, 1e-3f));

    vals = std::vector<float>(1000000, 0.1f);
    res2 = kahanSum(vals);
    ASSERT_TRUE(approxEq(res2, 100000.0f, 1e-1f));
}

//=== Week 2 Tests ===

TEST_CASE(Semaine2_TP1, Vec2dOps) {
    // Dot product (6)
    ASSERT_TRUE(Dot({1,0}, {0,1}) == 0.0);
    ASSERT_TRUE(Dot({1,0}, {1,0}) == 1.0);
    ASSERT_TRUE(Dot({3,4}, {3,4}) == 25.0);
    ASSERT_TRUE(Dot({-1,0}, {1,0}) == -1.0);
    ASSERT_TRUE(Dot({2,3}, {4,5}) == 23.0);
    ASSERT_TRUE(Dot({0,0}, {5,7}) == 0.0);
    
    // Cross2D (4)
    ASSERT_TRUE(Cross2D({1,0}, {0,1}) == 1.0);
    ASSERT_TRUE(Cross2D({0,1}, {1,0}) == -1.0);
    ASSERT_TRUE(Cross2D({1,1}, {1,1}) == 0.0);
    ASSERT_TRUE(Cross2D({2,0}, {0,2}) == 4.0);
    
    // Normalize (4)
    Vec2d v = {3,4};
    Vec2d norm_v = v.Normalized();
    ASSERT_TRUE(std::fabs(norm_v.Norm() - 1.0) < kEps);
    ASSERT_TRUE(std::fabs(norm_v.x - 0.6) < kEps);
    ASSERT_TRUE(std::fabs(norm_v.y - 0.8) < kEps);
    
    Vec2d unit = {1,0};
    unit = unit.Normalized();
    ASSERT_TRUE(std::fabs(unit.x - 1.0) < kEps);
    
    // Operator[] (5)
    Vec2d pos = {10, 20};
    ASSERT_TRUE(pos[0] == 10.0);
    ASSERT_TRUE(pos[1] == 20.0);
    pos[0] = 30;
    ASSERT_TRUE(pos.x == 30.0);
    pos[1] = 40;
    ASSERT_TRUE(pos.y == 40.0);
    
    Vec2d tmp = {5, 6};
    ASSERT_TRUE(tmp[0] == 5.0);
    
    // Layout (1)
    static_assert(sizeof(Vec2d) == 16, "Vec2d size error");
}

TEST_CASE(Semaine2_TP2, Vec3dGramSchmidt) {
    Vec3d bx = {1,0,0}, by = {0,1,0}, bz = {0,0,1};

    // Cross3D (6)
    ASSERT_TRUE(ApproxVec(Cross(bx, by), bz));
    ASSERT_TRUE(ApproxVec(Cross(by, bx), {0,0,-1}));
    ASSERT_TRUE(ApproxVec(Cross(by, bz), bx));
    ASSERT_TRUE(ApproxVec(Cross(bz, bx), by));
    ASSERT_TRUE(approxEq(Dot(Cross(bx, by), bx), 0));
    ASSERT_TRUE(approxEq(Dot(Cross(bx, by), by), 0));
    
    // GramSchmidt (6)
    for(int i = 0; i < 10; ++i) {
        Vec3d v1{randomDist(randomEngine), randomDist(randomEngine), randomDist(randomEngine)};
        Vec3d v2{randomDist(randomEngine), randomDist(randomEngine), randomDist(randomEngine)};
        Vec3d v3{randomDist(randomEngine), randomDist(randomEngine), randomDist(randomEngine)};
    
        Vec3d u1 = v1.Normalized();
        Vec3d u2 = (v2 - Project(v2, u1)).Normalized();
        Vec3d u3 = (v3 - Project(v3, u1) - Project(v3, u2)).Normalized();
        
        ASSERT_TRUE(approxEq(u1.Norm(), 1.0));
        ASSERT_TRUE(approxEq(u2.Norm(), 1.0));
        ASSERT_TRUE(approxEq(u3.Norm(), 1.0));
    
        ASSERT_TRUE(approxEq(Dot(u1, u2), 0.0));
        ASSERT_TRUE(approxEq(Dot(u1, u3), 0.0));
        ASSERT_TRUE(approxEq(Dot(u2, u3), 0.0));
    }
    
    // Proj/Reject (1)
    bx = {3,4,0}; by = {1,0,0};
    Vec3d proj = Project(bx, by);
    Vec3d rej = Reject(bx, by);
    ASSERT_TRUE(ApproxVec(proj + rej, bx));
}

TEST_CASE(Semaine2_TP3, SimplePerspectiveProj) {
    std::vector<Vec2d> screenCoords;
    
    double camZ = 2.0;
    for(auto& pt : cubeMesh){
        pt.z += camZ;
        screenCoords.push_back(ProjectPoint(pt));
    }
    
    for(const auto& p : screenCoords) {
        int x = (int)p.x, y = (int)p.y;
        for(int dx = -2; dx <= 2; dx++)
            for(int dy = -2; dy <= 2; dy++)
                framebuffer.SetPixel(x+dx, y+dy, 255, 0, 0);
    }
    
    for(auto edge : cubeEdges)
        framebuffer.DrawLine((int)screenCoords[edge.x].x, (int)screenCoords[edge.x].y, 
                            (int)screenCoords[edge.y].x, (int)screenCoords[edge.y].y);
    framebuffer.SavePPM("output_cube_simple.ppm");
}

//=== Week 3 Tests ===

TEST_CASE(Semaine3_TP1, MatrixOps) {
    Mat4d A, B, invB;

    for(int i=0; i<10; i++) {
        for(int x=0; x<4; x++)
            for(int y=0; y<4; y++)
                A(x, y) = randomDist(randomEngine);
        
        B = A * Mat4d::Identity();
        ASSERT_TRUE(ApproxMat(B, A));
    
        if(Inverse(A, invB))
            ASSERT_TRUE(ApproxMat(A * invB, Mat4d::Identity(), 1e-10f));
    }
    
    A = Mat4d::Identity();
    for(int j=0; j<4; j++)
        A(1, j) = A(0, j);
    ASSERT_TRUE(!Inverse(A, invB));
    
    B = Mat4d::RotateAxis({0,1,0}, NKENTSEU_PI_DOUBLE / 2.0f);
    Vec4d before = {1,0,0,1}, after = B * before;

    ASSERT_TRUE(approxEq(after.x, 0.0));
    ASSERT_TRUE(approxEq(after.y, 0.0));
    ASSERT_TRUE(approxEq(after.z, -1.0));
}

TEST_CASE(Semaine3_TP2, CubeRotation) {
    for(int f = 0; f < 10; f++){
        framebuffer = NkImage(canvas_width, canvas_height);
        double rotation = f * 0.3;
        Mat4d rotMatrix = Mat4d::RotateAxis(cameraUp, rotation);
        std::vector<Vec3d> screenPts;
    
        for(auto vertex : cubeMesh){
            Vec4d transformed = projMatrix * (viewMatrix * (rotMatrix * vertex));
            screenPts.push_back(ProjectToScreen(transformed, canvas_width, canvas_height));
        }
    
        for(auto link : cubeEdges)
            framebuffer.DrawLine((int)screenPts[link.x].x, (int)screenPts[link.x].y, 
                                (int)screenPts[link.y].x, (int)screenPts[link.y].y, 255);
        framebuffer.SavePPM("output_rotate_"+std::to_string(f)+".ppm");
    }
}

TEST_CASE(Semaine3_TP3, TRSDecomp) {
    std::uniform_real_distribution<double> randScale(0.5, 5.0);

    for(int i = 0; i < 20; i++){
        Vec3d inputT{randomDist(randomEngine), randomDist(randomEngine), randomDist(randomEngine)};
        Vec3d inputR{randomDist(randomEngine), randomDist(randomEngine), randomDist(randomEngine)};
        Vec3d inputS{randScale(randomEngine), randScale(randomEngine), randScale(randomEngine)};
        
        Mat4d combined = TRS(inputT, inputR, inputS);
        
        Vec3d outT, outR, outS;
        DecomposeTRS(combined, outT, outR, outS);
    
        ASSERT_TRUE(ApproxVec(inputT, outT));
        ASSERT_TRUE(ApproxVec(inputS, outS));
        ASSERT_TRUE(ApproxVec(inputR, outR, 5.0));
    }
}

//=== Week 4 Tests ===

TEST_CASE(Semaine4_TP1, QuatBasics) {
    Mat3d m1, m2, m3;
    Quat q1, q2, q3;

    Vec3d axis = {1,0,0};
    q1 = FromAxisAngle({0,1,0}, NKENTSEU_PI_DOUBLE / 2.0f);
    Vec3d rotated = Rotate(q1, axis);

    ASSERT_TRUE(std::fabs(rotated.x - 0.0) < kEps);
    ASSERT_TRUE(std::fabs(rotated.y - 0.0) < kEps);
    ASSERT_TRUE(std::fabs(rotated.z + 1.0) < kEps);
    
    std::uniform_real_distribution<double> randComp(-1.0, 1.0);

    for(int j = 0; j < 50; j++){
        q1 = { randComp(randomEngine), randComp(randomEngine), 
               randComp(randomEngine), randComp(randomEngine) };
        q1 = q1.Normalized();
    
        m1 = ToMat3(q1);
        q2 = FromMat3(m1);
        q2 = q2.Normalized();
    
        ASSERT_TRUE(ApproxQuat(q1, q2, 1e-4f));
    }
    
    for(int j = 0; j < 50; j++){
        q1 = { randComp(randomEngine), randComp(randomEngine), 
               randComp(randomEngine), randComp(randomEngine) };
        q1 = q1.Normalized();
        q2 = q1.Inverse();
        q3 = q1 * q2;
        ASSERT_TRUE(ApproxQuat(q3, Quat::Identity(), 1e-4f));
    }
}

TEST_CASE(Semaine4_TP2, SLERPInterp) {
    Quat start, end, interp;
    
    start = FromAxisAngle({0,1,0}, 0);
    end = FromAxisAngle({0,1,0}, NKENTSEU_PI_DOUBLE);
    
    for(int frame = 0; frame < 60; frame++){
        double ratio = frame / 59.0;
        interp = Slerp(start, end, ratio);
        Mat4d transform = FromRT(ToMat3(interp), {0,0,0});
    
        framebuffer = NkImage(canvas_width, canvas_height);
        std::vector<Vec3d> pts;
        for(auto v : cubeMesh){
            Vec4d p = projMatrix * (viewMatrix * (transform * v));
            pts.push_back(ProjectToScreen(p, canvas_width, canvas_height));
        }
    
        for(auto e : cubeEdges)
            framebuffer.DrawLine((int)pts[e.x].x, (int)pts[e.x].y, (int)pts[e.y].x, (int)pts[e.y].y, 255);
        framebuffer.SavePPM("output_slerp_"+std::to_string(frame)+".ppm");
    }
    
    for(int frame = 0; frame < 60; frame++){
        double ratio = frame / 59.0;
        interp = Lerp(start, end, ratio);
        Mat4d transform = FromRT(ToMat3(interp), {0,0,0});
    
        framebuffer = NkImage(canvas_width, canvas_height);
        std::vector<Vec3d> pts;
        for(auto v : cubeMesh){
            Vec4d p = projMatrix * (viewMatrix * (transform * v));
            pts.push_back(ProjectToScreen(p, canvas_width, canvas_height));
        }
    
        for(auto e : cubeEdges)
            framebuffer.DrawLine((int)pts[e.x].x, (int)pts[e.x].y, (int)pts[e.y].x, (int)pts[e.y].y, 255);
        framebuffer.SavePPM("output_lerp_"+std::to_string(frame)+".ppm");
    }
}
