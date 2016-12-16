#pragma once

#include <algorithm>
#include <vector>
using namespace std;

class xStream;

/******************************************************************
*
* xArray: like std::vector
*
******************************************************************/
template<class T>
class xArray : public std::vector<T>
{
public:
    const_reference At(int i) const
    {
		
	ASSERT(i >= 0 && i < Count()); 
        return begin()[i]; 
    }

    reference At(int i)        
    {
       
		ASSERT(i >= 0 && i < Count()); 
        return begin()[i]; 
    }

    const_reference operator[](int i) const    
    {  
        return At(i); 
    }

    reference operator[](int i)    
    { 
        return At(i); 
    }

    void Copy(const xArray<T>& src)    
    { 
        *this = src;    
    }

    void Append(const xArray<T>& src)
    { 
        insert(end(), src.begin(), src.end());
    }

    void InsertAt(int i, const T& arg) 
    { 
        insert(begin()+i, arg);
    }

    void RemoveAt(int i)        
    { 
        erase(begin()+i);
    }

    //是否可以添加，比如对于图层类，是不允许名字重复的
    void Add(const T& arg)        
    {
        if(CanAdd(arg))
        {
            push_back(arg);
        }
    }

    virtual bool CanAdd(const T& arg)
    {
        return true;
    }

    void AddRange(const xArray<T>& arg)        
    {
        int count = arg.Count();
        const_iterator p = arg.begin();
        for (int i = 0; i < count; i++)
        {
            Add(*p);
            p++;
        }
    }

    int Count() const
    {
        return (int)size();
    }

    void Clear()
    {
        clear();
    }

    /*
    * reverse
    */
    void Reverse()        
    { 
        /*int count = Count();
        count /= 2;
        reverse_iterator rit = rbegin();
        iterator it = begin();
        for (int i = 0; i < count; i++)
        {
            T& p = rit[i];
            rit[i] = it[i];
            it[i] = p;
        }*/
    }

    /*
    * 查找指定的arg的索引,-1表示未找到
    */
    int Find(const T& arg) const
    {
        int count = Count();
        const_iterator p = begin();
        for (int rtv = 0; rtv < count; rtv++, p++)
        {
            if((*p) == arg)
                return (int)rtv;
        }
        return -1;
    }

    /*
    * return true if p is in PtrArray & delete successfully
    */
    BOOL Remove(const T& arg)
    {
        int count = Count();
        iterator p = begin();
        for (int i = 0; i < count; i++, p++)
        {
            if((*p) == arg)
            {
                erase(p);
                return true;
            }
        }
        return false;
    }

    /*
    * replace all oldVal with newVal
    */
    int Replace(const T& oldVal, const T& newVal)
    {
        int rtv = 0;
        iterator p = begin();
        int count = Count();
        for (int i = 0; i < count; i++, p++)
        {
            if(*p == oldVal)
            {
                *p = newVal;
                rtv++;
            }
        }
        return rtv;
    }

    /*
    *    array serialize function : read/write memory
    */
   /* void ReadDirect(xStream& stm)
    {
        int n = stm.ReadInt32();
        resize(n);
        if (n > 0)
            stm.Read(static_cast<void*>(&*begin()), n*sizeof(T));
    }

    void WriteDirect(xStream& stm)
    {
        int n = (int)Count();
        stm.WriteInt32(n);
        if (n > 0)
            stm.Write(static_cast<void*>(&*begin()), n*sizeof(T));
    }*/

    ///*
    //*    array serialize function : serialize array, call T::read & T::write
    //*/
    //void Read(xStream& stm)
    //{
    //    int n;
    //    stm >> n;
    //    resize(n);
    //    for(iterator p = begin(); n > 0; n--, p++)
    //    {
    //        p->Read(stm);
    //    }
    //}

    //void Write(xStream& stm)
    //{
    //    int n = Count();
    //    stm << n;
    //    for(iterator p = begin(); n > 0; n--, p++)
    //    {
    //        p->Write(stm);
    //    }
    //}
};

/******************************************************************
*
* template PtrArray
* 内部维护指针空间，删除数组元素的时候会自动删除指针的指针指向空间
*
******************************************************************/
template<class T>
class PtrArray : public xArray<T*>
{
public:
    typedef T    ValueType;

    /*
    *    删除指定元素(包括指针内容)
    * @param isSetNullOnly : 从数组中直接删除元素，还是暂时设置为NULL
    *                         如果设置为NULL后，可以统一调用purge()将所有为NULL的元素从数组中删除
    */

    void RemoveAt(int index, BOOL isSetNullOnly = false)
    {
        delete (*this)[index];
        if(isSetNullOnly)
            (*this)[index] = NULL;
        else
            erase(begin() + index);
    }

    void Release()
    {
        int count = Count();
        for (int i = 0; i < count; i++)
        {
            T* ptr = (*this)[i];
            delete ptr;
        }
    }
};

class Ints : public xArray<int>
{
public:
    void Sort()        {  std::sort(begin(), end()); }
    void SortDesc()    {  std::sort(begin(), end(), _greator); }

    static BOOL _greator(int e, int arg)    {  return arg < e; }
};

class Doubles : public xArray<double>
{
public:
    void Sort()        {  std::sort(begin(), end()); }
    void SortDesc()    {  std::sort(begin(), end(), _greator); }

    static BOOL _greator(double e, double arg)    {  return arg < e; }
};