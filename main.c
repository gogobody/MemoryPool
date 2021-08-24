#include <bits/stdc++.h>

using namespace std;

#define TRACE_METHOD() std::cout << this << " " << __PRETTY_FUNCTION__ << "\n";

template<typename T>
struct minipool {
    // 用联合体因为这两个指针同时只有一个会使用
    union minipool_item {
    private:
        minipool_item *next;
        alignas(alignof(T)) char datum[sizeof(T)];

    public:
        minipool_item *get_next_item() const { return next; }

        void set_next_item(minipool_item *n) { next = n; }

        // Methods for the storage of the item.
        T *get_storage() { return reinterpret_cast<T *>(datum); }

        // Given a T* cast it to a minipool_item*
        static minipool_item *storage_to_item(T *t) {
            auto *current_item = reinterpret_cast<minipool_item *>(t);
            return current_item;
        }
    };

    struct minipool_arena {
    private:
        // Storage of this arena.
        std::unique_ptr<minipool_item[]> storage;
        // Pointer to the next arena.
        std::unique_ptr<minipool_arena> next;
        // Creates an arena with arena_size items.
    public:
        minipool_arena(size_t arena_size) : storage(new minipool_item[arena_size]) {
            for (size_t i = 1; i < arena_size; i++) {
                storage[i - 1].set_next_item(&storage[i]);
            }
            storage[arena_size - 1].set_next_item(nullptr);
        }

        minipool_item *get_storage() const { return storage.get(); }

        void set_next_arena(std::unique_ptr<minipool_arena> &&n) {
            assert(!next);

            next = std::move(n);
        }
    };

    // Size of the arenas created by the pool.
    size_t arena_size;
    // Current arena. Changes when it becomes full and we want to allocate one
    // more object.
    std::unique_ptr<minipool_arena> arena;
    // List of free elements. The list can be threaded between different arenas
    // depending on the deallocation pattern.
    minipool_item *free_list;

    minipool(size_t arena_size)
            : arena_size(arena_size), arena(new minipool_arena(arena_size)),
              free_list(arena->get_storage()) {}

    template<typename... Args>
    T *alloc(Args &&... args) {
        if (free_list == nullptr) {
            // If the current arena is full, create a new one.
            std::unique_ptr<minipool_arena> new_arena(new minipool_arena(arena_size));
            // Link the new arena to the current one.
            new_arena->set_next_arena(std::move(arena));
            // Make the new arena the current one.
            arena = std::move(new_arena); // 转移控制权
            // Update the free_list with the storage of the just created arena.
            free_list = arena->get_storage();
        }

        // Get the first free item.
        minipool_item *current_item = free_list;
        // Update the free list to the next free item.
        free_list = current_item->get_next_item();

        // Get the storage for T.
        T *result = current_item->get_storage();
        // Construct the object in the obtained storage.
        // 在获得的存储中 构建对象
        new(result) T(std::forward<Args>(args)...);

        return result;
    }

    void free(T *t) {
        // Destroy the object.
        t->T::~T();

        // Convert this pointer to T to its enclosing pointer of an item of the
        // arena.
        minipool_item *current_item = minipool_item::storage_to_item(t);

        // Add the item at the beginning of the free list.
        current_item->set_next_item(free_list);
        free_list = current_item;
    }
};

int main(int argc, char *argv[]) {


    minipool<int> mp2(2);
    int *m1= mp2.alloc(1);

//    std::cout << "out: " << sizeof(mp2.free_list->get_storage()[0]) << "\n";
    std::cout << "m1->x=" << m1 <<" "<< *m1<< "\n";

    int *m2= mp2.alloc(2);
    mp2.free(m1);

    int *m3= mp2.alloc(3);
    int *m4= mp2.alloc(4);
    std::cout << "m1->x=" << m1 <<" "<< *m1<< "\n";

    std::cout << "m2->x=" << m2 <<" "<< *m2<< "\n";
    std::cout << "m3->x=" << m3 <<" "<< *m3<< "\n";
    std::cout << "m4->x=" << m4 <<" "<< *m4<< "\n";

    mp2.free(m2);
    mp2.free(m3);
    mp2.free(m4);
    return 0;
}
