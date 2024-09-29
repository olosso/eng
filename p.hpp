#include <utility> // Contains std::forward.

template<typename T>
class UniquePointer {

    public:
        // A couple of constructors.
        UniquePointer() : m_Ptr{nullptr} {};
        UniquePointer(T* ptr) : m_Ptr{ptr} {};

        // Delete copy constructors, so that the underlying pointer isn't accidentally copied.
        UniquePointer(const UniquePointer&) = delete;
        UniquePointer &operator=(const UniquePointer&) = delete;

        // Add move constructors.
        UniquePointer(UniquePointer&& other) : m_Ptr{other.release()} {};
        UniquePointer &operator=(UniquePointer&& other) {
            if(this != &other) {
                reset(other.release());
            }
            return *this;
        };

        // Gives up ownership of the underlying pointer.
        T* release() {
            T* temp = m_Ptr;
            m_Ptr = nullptr;
            return temp;
        };

        // Takes up ownership of a new pointer.
        void reset(T* ptr = nullptr) {
            if(m_Ptr) {
                delete m_Ptr;
            };
            m_Ptr = ptr;
        };

        // Make sure that the data is deleted when ~this~ goes out of scope.
        ~UniquePointer() {
            if(m_Ptr) {
                delete m_Ptr;
            }
        };

    private:
        T* m_Ptr;
};

// Helper function for creating UniquePointers.
template<class T, class... Args>
UniquePointer<T> make_unique(Args&&... args) {
    return UniquePointer<T>(new T(std::forward<Args>(args)...));
}
