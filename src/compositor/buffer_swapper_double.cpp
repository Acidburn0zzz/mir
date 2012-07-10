/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */
#include <mir/compositor/buffer_swapper_double.h>

namespace mc = mir::compositor;

mc::BufferSwapperDouble::BufferSwapperDouble(mc::Buffer* a, mc::Buffer* b )
 :
buf_a(a),
buf_b(b),
invalid0(nullptr),
invalid1(nullptr)
{ 
    atomic_store(&dequeued, &invalid0);
    atomic_store(&grabbed, &invalid1);

}

void mc::BufferSwapperDouble::dequeue_free_buffer(Buffer*& out_buffer )
{
    client_to_dequeued();
    out_buffer = *dequeued.load();
}

void mc::BufferSwapperDouble::queue_finished_buffer()
{
    /* transition dequeued */
    client_to_queued();

    /* toggle grabbed pattern */
    compositor_change_toggle_pattern();
}

void mc::BufferSwapperDouble::grab_last_posted(mc::Buffer*& out_buffer)
{
    compositor_to_grabbed();
    out_buffer = *grabbed.load();
}

void mc::BufferSwapperDouble::ungrab()
{
    compositor_to_ungrabbed();
}


/* class helper functions, mostly compare_and_exchange based state computation */
void mc::BufferSwapperDouble::client_to_dequeued() {
    Buffer **dq_assume;
    Buffer **next_state;

    do {
        dq_assume = dequeued.load();

        if (dq_assume == &invalid0)
        {            
            next_state = &buf_a;
        } 
        else if (dq_assume == &invalid1)
        {
            next_state = &buf_b; 
        }

    } while (!std::atomic_compare_exchange_weak(&dequeued, &dq_assume, next_state )); 
}

void mc::BufferSwapperDouble::client_to_queued() {
    Buffer **dq_assume;
    Buffer **next_state;
    do {
        dq_assume = dequeued.load();

        if (dq_assume == &buf_a)
        {
            next_state = &invalid1; 
        }
        else if (dq_assume == &buf_b)
        {
            next_state = &invalid0; 
        }

    } while (!std::atomic_compare_exchange_weak(&dequeued, &dq_assume, next_state ));
}

void mc::BufferSwapperDouble::compositor_change_toggle_pattern() {
    Buffer **grabbed_assume;
    Buffer **next_state;

    do {
        grabbed_assume = grabbed.load();

        if (grabbed_assume == &invalid0)
        {
            next_state = &invalid1; 
        }
        else if (grabbed_assume == &buf_a)
        {
            next_state = &buf_b; 
        }

        else if (grabbed_assume == &invalid1)
        {
            next_state = &invalid0; 
        }
        else if (grabbed_assume == &buf_b)
        { 
            next_state = &buf_a; 
        }
    } while (!std::atomic_compare_exchange_weak(&grabbed, &grabbed_assume, next_state ));
}

void mc::BufferSwapperDouble::compositor_to_grabbed() {
    Buffer **next_state;
    Buffer **grabbed_assume;

    do {
        grabbed_assume = grabbed.load();
        if (grabbed_assume == &invalid0) /* pattern A */
        { 
            next_state = &buf_a; 
        }
        else if (grabbed_assume == &invalid1) /* pattern B */
        {
            next_state = &buf_b; 
        }
    } while (!std::atomic_compare_exchange_weak(&grabbed, &grabbed_assume, next_state ));
}

void mc::BufferSwapperDouble::compositor_to_ungrabbed() {
    Buffer **grabbed_assume;
    Buffer **next_state;
    do {
        grabbed_assume = grabbed.load();
        if (grabbed_assume == &buf_a) /* pattern A */
        {
            next_state = &invalid0; 
        }
        else if (grabbed_assume == &buf_b) /* pattern B */
        {
            next_state = &invalid1; 
        }
    } while (!std::atomic_compare_exchange_weak(&grabbed, &grabbed_assume, next_state ));
}

