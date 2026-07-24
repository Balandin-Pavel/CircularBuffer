#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <memory>
#include <iterator>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <algorithm>
#include <initializer_list>

template<typename T, bool Extendable = true, typename Allocator = std::allocator<T>>
class circular_buffer {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

    template <bool IsConst> class BufferIterator;
    using iterator = BufferIterator<false>;
    using const_iterator = BufferIterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    allocator_type _alloc; 
    pointer _data;
    size_type _capacity;
    size_type _head;
    size_type _tail;
    size_type _size;

    size_type _get_physical_index(size_type logical_index) const noexcept {
        if (_capacity == 0) return 0;
        return (_head + logical_index) % _capacity;
    }

    size_type _next_index(size_type index) const noexcept{
        size_type next = index + 1;
        if (next >= _capacity){
            next = 0;
        }
        return next;
    }

    size_type _prev_index(size_type index) const noexcept{
        if (index == 0) {
            return _capacity > 0 ? _capacity - 1 : 0;
        }
        return index - 1;
    }

    size_type _add_index(size_type index, difference_type step) const noexcept {
        long long target = static_cast<long long>(index) + step;
        target %= static_cast<long long>(_capacity);
        if (target < 0) {
            target += static_cast<long long>(_capacity);
        }
        return static_cast<size_type>(target);
    }

    void _expand_capacity(size_type req_capacity = 0){
        size_type new_capacity = (_capacity == 0) ? 1 : _capacity * 2;
        if (req_capacity > new_capacity) new_capacity = req_capacity;

        pointer new_data = std::allocator_traits<Allocator>::allocate(_alloc, new_capacity);
        try{
            for(size_type i = 0; i< _size; i++){
                size_type old_index = (_head+i) % _capacity;
                std::allocator_traits<Allocator>::construct(_alloc, new_data + i, std::move_if_noexcept(_data[old_index]));
            }
        }
        catch(...){
            std::allocator_traits<Allocator>::deallocate(_alloc, new_data, new_capacity);
            throw;
        }
        for (size_type i = 0; i < _size; ++i) {
            size_type old_index = (_head + i) % _capacity;
            std::allocator_traits<Allocator>::destroy(_alloc, _data + old_index);
        }
        if (_capacity > 0) {
            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
        }
        _data = new_data;
        _capacity = new_capacity;
        _head = 0;
        _tail = _size;
    }

public:
    template <bool IsConst>
    class BufferIterator {
        private:
            using ParentPtr = typename std::conditional<IsConst, const circular_buffer*, circular_buffer*>::type;
            ParentPtr _buffer;
            size_type _index;
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = typename std::conditional<IsConst, const T*, T*>::type;
            using reference = typename std::conditional<IsConst, const T&, T&>::type;
            
            template <bool> friend class BufferIterator;
            
            BufferIterator() : _buffer(nullptr), _index(0) {}
            BufferIterator(ParentPtr buffer, size_type index): _buffer(buffer), _index(index) {}
            
            template <bool WasConst, typename = std::enable_if_t<IsConst && !WasConst>>
            BufferIterator(const BufferIterator<WasConst>& other) : _buffer(other._buffer), _index(other._index){}
            
            reference operator*() const{
                return _buffer->_data[_buffer->_get_physical_index(_index)];
            }
            pointer operator->() const{
                return &this->operator*();
            }
            reference operator[](difference_type n) const{
                return *(*this + n);
            }
            BufferIterator& operator++(){ 
                _index++; 
                return *this; 
            }
            BufferIterator operator++(int){ 
                BufferIterator temp = *this; 
                ++_index; 
                return temp; 
            }
            BufferIterator& operator--(){ 
                _index--; 
                return *this; 
            }
            BufferIterator operator--(int){ 
                BufferIterator temp = *this; 
                --_index; 
                return temp; 
            }
            BufferIterator& operator+=(difference_type n){ 
                _index += n; 
                return *this; 
            }
            BufferIterator& operator-=(difference_type n){ 
                _index -= n; 
                return *this; 
            }
            friend BufferIterator operator+(const BufferIterator& it, difference_type n){ 
                BufferIterator temp = it; 
                temp += n; 
                return temp; 
            }
            friend BufferIterator operator+(difference_type n, const BufferIterator& it){ 
                return it + n; 
            }
            friend BufferIterator operator-(const BufferIterator& it, difference_type n){ 
                BufferIterator temp = it; 
                temp -= n; 
                return temp; 
            }
            friend difference_type operator-(const BufferIterator& lhs, const BufferIterator& rhs){ 
                return static_cast<difference_type>(lhs._index) - static_cast<difference_type>(rhs._index);
            }
            friend bool operator==(const BufferIterator& lhs, const BufferIterator& rhs){ 
                return lhs._index == rhs._index; 
            }
            friend bool operator!=(const BufferIterator& lhs, const BufferIterator& rhs){ 
                return lhs._index != rhs._index; 
            }
            friend bool operator<(const BufferIterator& lhs, const BufferIterator& rhs){ 
                return lhs._index < rhs._index; 
            }
            friend bool operator>(const BufferIterator& lhs, const BufferIterator& rhs){ 
                return lhs._index > rhs._index; 
            }
            friend bool operator<=(const BufferIterator& lhs, const BufferIterator& rhs){ 
                return lhs._index <= rhs._index; 
            }
            friend bool operator>=(const BufferIterator& lhs, const BufferIterator& rhs){ 
                return lhs._index >= rhs._index; 
            }
    };

    circular_buffer() : circular_buffer(0) {}
    
    explicit circular_buffer(const Allocator& alloc) : circular_buffer(0, alloc) {}
    
    explicit circular_buffer(size_type capacity, const Allocator& alloc = Allocator()):
        _alloc(alloc), 
        _data(nullptr), 
        _capacity(capacity), 
        _head(0), 
        _tail(0), 
        _size(0)
    {
        if(_capacity > 0) _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);
    }
    
    circular_buffer(size_type count, const T& value, const Allocator& alloc = Allocator()): circular_buffer(count, alloc) {
        assign(count, value);
    }

    template<class InputIt, typename = std::enable_if_t<!std::is_integral_v<InputIt>>>
    circular_buffer(InputIt first, InputIt last, const Allocator& alloc = Allocator())
        : _alloc(alloc), _data(nullptr), _capacity(0),
        _head(0), _tail(0), _size(0)
    {
        size_type count = 0;
        for (auto it = first; it != last; ++it) {
            ++count;
        }
        if (count > 0) {
            _capacity = count;
            _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);
        }
        for (; first != last; ++first) {
            push_back(*first);
        }
    }

    circular_buffer(std::initializer_list<T> init, const Allocator& alloc = Allocator())
        : circular_buffer(init.size(), alloc)
    {
        assign(init);
    }

    circular_buffer(const circular_buffer& other): 
        _alloc(std::allocator_traits<Allocator>::select_on_container_copy_construction(other._alloc)),
        _data(nullptr), 
        _capacity(other._capacity), 
        _head(0), 
        _tail(other._size), 
        _size(other._size)
    {
        if (_capacity > 0) {
            _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);
            try {
                for (size_type i = 0; i < _size; ++i) {
                    size_type src_index = other._get_physical_index(i);
                    std::allocator_traits<Allocator>::construct(_alloc, _data + i, other._data[src_index]);
                }
            } catch (...) {
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                throw; 
            }
        }
    }
    circular_buffer(circular_buffer&& other) noexcept
        : _alloc(std::move(other._alloc)),
          _data(other._data),
          _capacity(other._capacity),
          _head(other._head),
          _tail(other._tail),
          _size(other._size)
    {
        other._data = nullptr;
        other._capacity = 0;
        other._head = 0;
        other._tail = 0;
        other._size = 0;
    }

    circular_buffer(circular_buffer&& other, const Allocator& alloc)
        : _alloc(alloc), _data(nullptr), _capacity(0), _head(0), _tail(0), _size(0)
    {
        if (_alloc == other._alloc) {
            // Same allocator: cheap pointer steal, no element construction needed.
            _data = other._data;
            _capacity = other._capacity;
            _head = other._head;
            _tail = other._tail;
            _size = other._size;
            other._data = nullptr;
            other._capacity = 0;
            other._head = 0;
            other._tail = 0;
            other._size = 0;
        } else {
            // Different allocators: cannot transfer the raw memory block, so
            // elements are individually move-constructed into freshly
            // allocated storage owned by *this*'s allocator.
            _capacity = other._size;
            if (_capacity > 0) {
                _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);
                try {
                    for (size_type i = 0; i < other._size; ++i) {
                        size_type src_index = other._get_physical_index(i);
                        std::allocator_traits<Allocator>::construct(
                            _alloc, _data + i, std::move_if_noexcept(other._data[src_index]));
                    }
                } catch (...) {
                    std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                    throw;
                }
            }
            _head = 0;
            _size = other._size;
            _tail = _size;
        }
    }

    ~circular_buffer() {
        clear();
        if (_data) std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
    }
    
    circular_buffer& operator=(const circular_buffer& other){
        if(this != &other){
            circular_buffer temp(other);
            swap(temp);
        }
        return *this;
    }
    circular_buffer& operator=(circular_buffer&& other) noexcept {
        if (this == &other) return *this;

        constexpr bool pocma = std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value;
        constexpr bool always_equal = std::allocator_traits<Allocator>::is_always_equal::value;

        if constexpr (pocma || always_equal) {
            // Allowed to (or guaranteed to) share the same allocator: steal the buffer outright.
            clear();
            if (_data) std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            if constexpr (pocma) {
                _alloc = std::move(other._alloc);
            }
            _data = other._data;
            _capacity = other._capacity;
            _head = other._head;
            _tail = other._tail;
            _size = other._size;
            other._data = nullptr;
            other._capacity = 0;
            other._head = 0;
            other._tail = 0;
            other._size = 0;
        } else if (_alloc == other._alloc) {
            clear();
            if (_data) std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _data = other._data;
            _capacity = other._capacity;
            _head = other._head;
            _tail = other._tail;
            _size = other._size;
            other._data = nullptr;
            other._capacity = 0;
            other._head = 0;
            other._tail = 0;
            other._size = 0;
        } else {
            // Non-propagating, unequal allocators: must keep using *this*'s
            // allocator, so elements are moved one by one.
            clear();
            reserve(other._size);
            for (size_type i = 0; i < other._size; ++i) {
                size_type idx = other._get_physical_index(i);
                push_back(std::move_if_noexcept(other._data[idx]));
            }
            other.clear();
        }
        return *this;
    }

    circular_buffer& operator=(std::initializer_list<T> ilist) {
        assign(ilist);
        return *this;
    }
    
    void assign(size_type count, const T& value) {
        clear();
        reserve(count);
        for (size_type i = 0; i < count; ++i) push_back(value);
    }

    template<class InputIt, typename = std::enable_if_t<!std::is_integral_v<InputIt>>>
    void assign(InputIt first, InputIt last) {
        clear();
        for (; first != last; ++first) push_back(*first);
    }

    void assign(std::initializer_list<T> ilist) {
        assign(ilist.begin(), ilist.end());
    }

    reference at(size_type pos) {
        if (pos >= _size) throw std::out_of_range("circular_buffer::at: index out of range");
        return _data[_get_physical_index(pos)];
    }
    
    const_reference at(size_type pos) const{
        if (pos >= _size)throw std::out_of_range("circular_buffer::at: index out of range");  
        return _data[_get_physical_index(pos)];
    }
    
    reference operator[](size_type pos){ 
        return _data[_get_physical_index(pos)]; 
    }
    const_reference operator[](size_type pos) const{ 
        return _data[_get_physical_index(pos)]; 
    }
    
    reference front(){ 
        return _data[_head]; 
    }
    const_reference front() const{ 
        return _data[_head]; 
    }
    
    reference back(){ 
        return _data[_get_physical_index(_size - 1)]; 
    }
    const_reference back() const{ 
        return _data[_get_physical_index(_size - 1)]; 
    }
    
    iterator begin() noexcept { 
        return iterator(this, 0); 
    }
    const_iterator begin() const noexcept { 
        return const_iterator(this, 0); 
    }
    const_iterator cbegin() const noexcept { 
        return const_iterator(this, 0); 
    }
    
    iterator end() noexcept { 
        return iterator(this, _size); 
    }
    const_iterator end() const noexcept { 
        return const_iterator(this, _size); 
    }
    const_iterator cend() const noexcept { 
        return const_iterator(this, _size); 
    }
    
    reverse_iterator rbegin() noexcept { 
        return reverse_iterator(end()); 
    }
    const_reverse_iterator rbegin() const noexcept { 
        return const_reverse_iterator(end()); 
    }
    const_reverse_iterator crbegin() const noexcept { 
        return const_reverse_iterator(end()); 
    }
    
    reverse_iterator rend() noexcept { 
        return reverse_iterator(begin()); 
    }
    const_reverse_iterator rend() const noexcept { 
        return const_reverse_iterator(begin()); 
    }
    const_reverse_iterator crend() const noexcept { 
        return const_reverse_iterator(begin()); 
    }
    
    bool empty() const noexcept{ 
        return _size == 0; 
    }
    bool full() const noexcept{ 
        return _capacity == _size; 
    }
    size_type size() const noexcept{ 
        return _size; 
    }

    size_type max_size() const noexcept{ 
        return std::allocator_traits<Allocator>::max_size(_alloc); 
    }
    size_type capacity() const noexcept { 
        return _capacity; 
    }

    void reserve(size_type new_cap) {
        if (new_cap <= _capacity) return;
        _expand_capacity(new_cap);
    }
    
    void resize(size_type count, const value_type& value) {
        if (count == _size) return;
        
        if (count < _size) {
            while (_size > count) pop_back();
        } else if (count > _size) {
            if (count > _capacity) reserve(count);
            while (_size < count) push_back(value);
        }
    }

    void resize(size_type count) {
        resize(count, value_type());
    }

    void clear() {
        while (!empty()) pop_back();
    }

    void push_back(const T& value) {
        if (full()) {
            if constexpr (Extendable) {
                _expand_capacity();
            } else {
                pop_front(); 
            }
        }
        std::allocator_traits<Allocator>::construct(_alloc, _data + _tail, value);
        _tail = _next_index(_tail);
        _size++;
    }

    void push_back(T&& value){
        push_back(static_cast<const T&>(value));
    }

    template<class... Args>
    reference emplace_back(Args&&... args) {
        if (full()) {
            if constexpr (Extendable) {
                _expand_capacity();
            } else {
                pop_front();
            }
        }
        std::allocator_traits<Allocator>::construct(
            _alloc, _data + _tail, std::forward<Args>(args)...);
        size_type pos = _tail;
        _tail = _next_index(_tail);
        _size++;
        return _data[pos];
    }

    
    void pop_back() {
        if (empty()) throw std::logic_error("circular_buffer::pop_back: empty buffer");
        size_type last_elem_idx = _prev_index(_tail);
        std::allocator_traits<Allocator>::destroy(_alloc, _data + last_elem_idx);
        _tail = last_elem_idx;
        _size--;
    }
    
    void push_front(const T& value) {
        if (full()) {
            if constexpr (Extendable) {
                _expand_capacity();
            } else {
                pop_back();
            }
        }
        size_type new_head = _prev_index(_head);
        std::allocator_traits<Allocator>::construct(_alloc, _data + new_head, value);
        _head = new_head;
        _size++;
    }

    void push_front(T&& value){
        push_front(static_cast<const T&>(value));
    }

    template<class... Args>
    reference emplace_front(Args&&... args) {
        if (full()) {
            if constexpr (Extendable) {
                _expand_capacity();
            } else {
                pop_back();
            }
        }
        size_type new_head = _prev_index(_head);
        std::allocator_traits<Allocator>::construct(
            _alloc, _data + new_head, std::forward<Args>(args)...);
        _head = new_head;
        _size++;
        return _data[_head];
    }

    void pop_front() {
        if (empty()) throw std::logic_error("circular_buffer::pop_front: empty buffer");
        std::allocator_traits<Allocator>::destroy(_alloc, _data + _head);
        _head = _next_index(_head);
        _size--;
    }
    
    iterator insert(const_iterator pos, const T& value) {
        return insert(pos, 1, value);
    }

    iterator insert(const_iterator pos, T&& value){
        return insert(pos, static_cast<const T&>(value));
    }

    iterator insert(const_iterator pos, size_type count, const T& value) {
        difference_type offset = pos - cbegin();
        if (count == 0) return begin() + offset;
        
        if (_size + count > _capacity) {
            size_type needed = _size + count;
            reserve(std::max(needed, _capacity * 2));
        }

        for (difference_type i = static_cast<difference_type>(_size) - 1; i >= offset; --i) {
            size_type dest_idx = _get_physical_index(i + count);
            size_type src_idx = _get_physical_index(i);
            
            if (static_cast<size_type>(i + count) >= _size) {
                std::allocator_traits<Allocator>::construct(_alloc, _data + dest_idx, std::move(_data[src_idx]));
            } else {
                _data[dest_idx] = std::move(_data[src_idx]);
            }
        }
        for (size_type i = 0; i < count; ++i) {
            size_type idx = _get_physical_index(offset + i);
            if (static_cast<size_type>(offset + static_cast<difference_type>(i)) >= _size) {
                std::allocator_traits<Allocator>::construct(_alloc, _data + idx, value);
            } else {
                _data[idx] = value;
            }
        }
        _tail = _add_index(_tail, count);
        _size += count;
        return begin() + offset;
    }
    
    template <class InputIt, typename = std::enable_if_t<!std::is_integral_v<InputIt>>>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        difference_type offset = pos - cbegin();
        size_type count = 0;
        auto temp = first;
        while(temp != last) { ++count; ++temp; }

        if (_size + count > _capacity) {
             reserve(std::max(_size + count, _capacity * 2));
        }

        for (difference_type i = _size - 1; i >= offset; --i) {
            size_type dest_idx = _get_physical_index(i + count);
            size_type src_idx = _get_physical_index(i);
            if (static_cast<size_type>(i + count) >= _size) {
                std::allocator_traits<Allocator>::construct(_alloc, _data + dest_idx, std::move(_data[src_idx]));
            } else {
                _data[dest_idx] = std::move(_data[src_idx]);
            }
        }
        auto it = first;
        for (size_type i = 0; i < count; ++i, ++it) {
            size_type idx = _get_physical_index(offset + i);
            if (static_cast<size_type>(offset + static_cast<difference_type>(i)) >= _size) {
                std::allocator_traits<Allocator>::construct(_alloc, _data + idx, *it);
            } else {
                _data[idx] = *it;
            }
        }
        _tail = _add_index(_tail, count);
        _size += count;
        return begin() + offset;
    }
    
    iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
        return insert(pos, ilist.begin(), ilist.end());
    }

    template<class... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        difference_type offset = pos - cbegin();

        if (_size + 1 > _capacity) {
            if constexpr (Extendable) {
                reserve(std::max(_size + 1, _capacity * 2));
            } else {
                // Mirrors insert()'s existing convention: growth beyond a
                // fixed capacity when inserting in the middle isn't defined
                // by this API (same limitation the provided insert() has).
            }
        }

        for (difference_type i = static_cast<difference_type>(_size) - 1; i >= offset; --i) {
            size_type dest_idx = _get_physical_index(i + 1);
            size_type src_idx = _get_physical_index(i);
            if (static_cast<size_type>(i + 1) >= _size) {
                std::allocator_traits<Allocator>::construct(_alloc, _data + dest_idx, std::move(_data[src_idx]));
            } else {
                _data[dest_idx] = std::move(_data[src_idx]);
            }
        }

        size_type idx = _get_physical_index(offset);
        if (static_cast<size_type>(offset) >= _size) {
            std::allocator_traits<Allocator>::construct(_alloc, _data + idx, std::forward<Args>(args)...);
        } else {
            _data[idx] = T(std::forward<Args>(args)...);
        }

        _tail = _next_index(_tail);
        _size += 1;
        return begin() + offset;
    }

    iterator erase(const_iterator pos) {
        return erase(pos, pos + 1);
    }
    
    iterator erase(const_iterator first, const_iterator last) {
        difference_type start_idx = first - cbegin();
        difference_type end_idx = last - cbegin();
        difference_type count = end_idx - start_idx;
        if (count <= 0) return begin() + start_idx;
        for (size_type i = 0; i < _size - end_idx; ++i) {
            size_type dest = _get_physical_index(start_idx + i);
            size_type src = _get_physical_index(end_idx + i);
            _data[dest] = std::move(_data[src]);
        }
        for (size_type i = 0; i < static_cast<size_type>(count); ++i) {
            pop_back();
        
        }
        return begin() + start_idx;
    }
    
    void swap(circular_buffer& other) noexcept {
        if constexpr (std::allocator_traits<Allocator>::propagate_on_container_swap::value) {
            using std::swap;
            swap(_alloc, other._alloc);
        }
        std::swap(_data, other._data);
        std::swap(_capacity, other._capacity);
        std::swap(_head, other._head);
        std::swap(_tail, other._tail);
        std::swap(_size, other._size);
    }
    
    allocator_type get_allocator() const{ return _alloc; }
    
    friend bool operator==(const circular_buffer& lhs, const circular_buffer& rhs) {
        if (lhs.size() != rhs.size()) return false;
        return std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }
    
    friend bool operator!=(const circular_buffer& lhs, const circular_buffer& rhs) {
        return !(lhs == rhs);
    }

    friend bool operator<(const circular_buffer& lhs, const circular_buffer& rhs) {
        return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    friend bool operator>(const circular_buffer& lhs, const circular_buffer& rhs) {
        return rhs < lhs;
    }

    friend bool operator<=(const circular_buffer& lhs, const circular_buffer& rhs) {
        return !(rhs < lhs);
    }

    friend bool operator>=(const circular_buffer& lhs, const circular_buffer& rhs) {
        return !(lhs < rhs);
    }
};

template<typename T, bool Ext, typename A>
void swap(circular_buffer<T, Ext, A>& lhs, circular_buffer<T, Ext, A>& rhs) noexcept {
    lhs.swap(rhs);
}
#endif