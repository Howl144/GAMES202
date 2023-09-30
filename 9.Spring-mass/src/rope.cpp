#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.

        Vector2D unitDistance = (end - start) / (num_nodes - 1);
        for (int i = 0; i < num_nodes; i++)
        {
            Mass* mass = new Mass(start + unitDistance * i, node_mass, false);
            masses.push_back(mass);
            //弹簧
            if (i > 0)
            {
                Spring* spring = new Spring(masses[i - 1], masses[i], k);
                springs.push_back(spring);
            }
        }
        //固定质点
        for (auto& i : pinned_nodes)
        {
            masses[i]->pinned = true;
        }
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            Vector2D ab = s->m2->position - s->m1->position;
            //m1指向m2,胡克定律
            Vector2D f = s->k * (ab.unit()) * (ab.norm() - s->rest_length);
            s->m1->forces += f;
            s->m2->forces -= f;
        }

        float kd = 0.05;
        for (auto &m : masses)
        {
            if (!m->pinned)
            {

                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                //F = m * a
                m->forces += m->mass * gravity;
                // TODO (Part 2): Add global damping
                m->forces += -kd * m->velocity;
                Vector2D a = m->forces / m->mass;

                //explicit  Euler
                // m->position += m->velocity * delta_t;
                // m->velocity += a * delta_t;

                //semi-implicit Euler
                m->velocity += a * delta_t;
                //更新位置
                m->position += m->velocity * delta_t;

            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            Vector2D ab = s->m2->position - s->m1->position;
            Vector2D f = s->k * (ab.unit()) * (ab.norm() - s->rest_length);
            s->m1->forces += f;
            s->m2->forces -= f;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                m->forces += gravity * m->mass;
                Vector2D a = m->forces / m->mass;

                Vector2D temp_position = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
                // TODO (Part 4): Add global Verlet damping
                double damping_factor = 0.00005;
                // 核心公式，带阻尼的计算
                m->position = temp_position + (1 - damping_factor) * (m->position - m->last_position) + a * delta_t * delta_t;
                // 用last_position记忆原先的位置
                m->last_position = temp_position;
            }
            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }
}
