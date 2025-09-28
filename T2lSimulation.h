#pragma once

#include <entt/entt.hpp>

struct Position { double x, y, z;};
struct Velocity { double x, y, z;};
struct Gravity  { inline static double g = 9.81; }; //removal means is on solid support
                                                    //for now used z=0
struct Mass     { double m, C, A;
                  inline static double Ro = 1.2;};

struct Target   { double r; }; //TODO

class GraphicsIface { public: virtual void update() = 0; };


class Simulation
//=============================================================================
{
public:
    static Simulation& instance() { static Simulation i; return i; }

//=======================================
    void runStep() {
        updatePosition();
        applyGravity();
        applyDrag();

        if (graphics != nullptr) graphics->update();

        timeNow_ += simStep_;
    }

//=======================================
    entt::entity addProjectile(const Position& position, const Velocity& velocity) {
        entt::entity e = reg.create();
        reg.emplace<Position>(e, position);
        reg.emplace<Velocity>(e, velocity);
        reg.emplace<Gravity>(e);
        reg.emplace<Mass>(e, Mass({1.2, 0.5, 8E-5}));
        return e;
    }

//=======================================
    void updatePosition() {
        auto view = reg.view<Position, Velocity>();
        for (auto entity : view) {
            auto &pos = view.get<Position>(entity);

            if (pos.z <= 0) continue; //idealne intrpolace dopadu, zasahu ...

            const auto &vel = view.get<Velocity>(entity);
            pos.x += vel.x*simStep_;
            pos.y += vel.y*simStep_;
            pos.z += vel.z*simStep_;
        }
    }

//=======================================
    void applyDrag() {
        auto view = reg.view<Velocity, Mass>();
        for (auto e : view) {
            auto &vel  = view.get<Velocity>(e);
            auto &mass = view.get<Mass>(e);

            double v2 = vel.x*vel.x+vel.y*vel.y+vel.z*vel.z;
            double c = 0.5*Mass::Ro*mass.C*mass.A/mass.m; //v2 * 0.5*Ro*C*A/m

            double v = sqrt(v2);
            double k = 1;
            if (v>0) k = 1-v*c*simStep_; //c*v*v/v
            if (k < 0) k = 0;

            vel.x = k*vel.x;
            vel.y = k*vel.y;
            vel.z = k*vel.z;
        }
    }

//=======================================
    void applyGravity() {
        auto view = reg.view<Velocity, Gravity>();
        for (auto e : view) {
            auto &vel = view.get<Velocity>(e);
            double g = Gravity::g;
            vel.z += -Gravity::g * simStep_;
        }
    }

//=======================================
//temporary public
    entt::registry reg;
    GraphicsIface* graphics { nullptr };

protected:
    double timeNow_    = 0;
    double simStep_    = 0.1;
};

