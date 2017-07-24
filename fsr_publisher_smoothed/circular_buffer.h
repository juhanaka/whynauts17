#ifndef __MBLIB_COMMON_CIRCULAR_BUFFER_H_
#define __MBLIB_COMMON_CIRCULAR_BUFFER_H_


//#include <cstdint>
//#include <cstring>


/// circular buffer class
/// @param T type of data to be stored in this buffer
template <typename T>
class CircularBuffer {
  public:
  
  ~CircularBuffer() {}
  
  /// Push data into the circular buffer
  /// @param[in] data pointer to data to be pushed
  /// @param[in] n number of data elements to be pushed
  void Push(const T* data, size_t n);
  
  /// Pop data from the circular buffer
  /// @param[out] data pointer to which data will be popped
  /// @param[in] n number of data elements to be popped
  void Pop(T* data, size_t n);

  // Returns the last element in the buffer without effecting the contents
  T PeekLast();
  
  void PopPeek(T* data, size_t n);

  /// Check number of elements available for push
  /// @return number of elements available for push
  size_t PushAvailable();
  
  /// Check the number of elements stored in the buffer
  /// @return number of elements available for pop
  size_t PopAvailable();
  
  /// Get a pointer for a push DMA operation
  T* PushDMA();
  
  /// Indicate a DMA push has completed with a number of elements
  void PushDMA(size_t n);
  
  /// Get a pointer for a pop DMA operation
  T* PopDMA();
  
  /// Indicate a DMA pop has completed with a number of elements
  void PopDMA(size_t n);
  
  /// Get the number of elements available for pushing with DMA
  size_t PushDMAAvailable();
  
  /// Get the nubmer of elements available for popping with DMA
  size_t PopDMAAvailable();

  T* const data_;
  
  private:
  size_t head_ = 0;
  size_t tail_ = 0;
  size_t count_ = 0;
  
  size_t const N_;
  

  protected:
  /// Constructor, should only be called by the instantiation
  /// class
  CircularBuffer(T* data, size_t N) : data_(data), N_(N) {};
};


/// Instantiation class for circular buffers
/// @param T type of data to be stored in this buffer
/// @param N maximum number of data elements that can be stored
template<typename T, size_t N>
class CircularBufferI : public CircularBuffer<T> {
  public:
  CircularBufferI() : CircularBuffer<T>((T*)&data_, N) {}
  
  ~CircularBufferI() {}

  T data_[N];

//  private:
//  T data_[N];
};


template<typename T>
void CircularBuffer<T>::Push(const T* data, size_t n) {
  if ((count_ + n) > N_){
      return; //reject any operation that will take this over the maximum allocation
    }
  if (n + head_ >= N_) {
    size_t end = N_ - head_;
    memcpy(&data_[head_], data, sizeof(T) * end);
    n -= end;
    count_ += end;
    head_ = 0;
    data = &data[end];
  }
  memcpy(&data_[head_], data, sizeof(T) * n);
  head_ += n;
  count_ += n;
}


template<typename T>
void CircularBuffer<T>::Pop(T* data, size_t n) {
  if ((count_ - n) > count_){//size_t is unsigned, therefore < 0 is going to actually be > original value
    return;//reject any operation that will take this under 0
  }

  if (n + tail_ >= N_) {
    size_t end = N_ - tail_;
    memcpy(data, &data_[tail_], sizeof(T) * end);
    n -= end;
    count_ -= end;
    tail_ = 0;
    data = &data[end];
  }
  memcpy(data, &data_[tail_], sizeof(T) * n);
  tail_ += n;
  count_ -= n;
}

template<typename T>
T CircularBuffer<T>::PeekLast() {
  T retVal;
  memcpy(&retVal, &data_[tail_], sizeof(T));
  return retVal;
}

template<typename T>
void CircularBuffer<T>::PopPeek(T* data, size_t n) {
  if (n + tail_ >= N_) {
    size_t end = N_ - tail_;
    memcpy(data, &data_[tail_], sizeof(T) * end);
    n -= end;
    data = &data[end];
    memcpy(data, &data_[0], sizeof(T) * n);
  }
  else
  {
  memcpy(data, &data_[tail_], sizeof(T) * n);
  }
}


template<typename T>
size_t CircularBuffer<T>::PushAvailable() {
  return (N_ - count_);
}


template<typename T>
size_t CircularBuffer<T>::PopAvailable() {
  return count_;
}


template<typename T>
T* CircularBuffer<T>::PushDMA() {
  return &data_[head_];
}


template<typename T>
void CircularBuffer<T>::PushDMA(size_t n) {
  head_ += n;
  if (head_ >= N_) head_ -= N_;
}


template <typename T>
size_t CircularBuffer<T>::PushDMAAvailable() {
  if (head_ >= tail_) {
    return N_ - head_;
  } else {
    return tail_ - head_;
  }
}


template<typename T>
T* CircularBuffer<T>::PopDMA() {
  return &data_[tail_];
}


template<typename T>
void CircularBuffer<T>::PopDMA(size_t n) {
  tail_ += n;
  if (tail_ >= N_) tail_ -= N_;
}


template <typename T>
size_t CircularBuffer<T>::PopDMAAvailable() {
  if (tail_ > head_) {
    return N_ - tail_;
  } else {
    return head_ - tail_;
  }
}




#endif //__MBLIB_COMMON_CIRCULAR_BUFFER_H__

