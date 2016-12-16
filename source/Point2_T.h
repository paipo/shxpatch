#pragma once

#include "Array.h"
#include "math.h"
template<class T> class Point2_T;
#define RCPoint2_T const Point2_T<T>&

template<class T>
class Point2_T
{
public:
    static int SizeofPoint2;
    T    X, Y;

    Point2_T()
    { 
    }

    Point2_T(RCPoint2_T p) 
    { 
        X = p.X; 
        Y = p.Y;
    }

    Point2_T(T _x, T _y) : X(_x), Y(_y) 
    { 
    }

public:
    /*
    *    operator = 
    */
    RCPoint2_T operator=(RCPoint2_T src);

    void Assign(T _x, T _y)        
    { 
        X = _x; 
        Y = _y;
    }

    void Assign(RCPoint2_T src)    
    { 
        *this = src;    
    }

    void Offset(T dx, T dy)        
    { 
        X += dx;    
        Y += dy;
    }

    void Offset(RCPoint2_T point)    
    { 
        X += point.X;    
        Y += point.Y;    
    }

    void Rotate(RCPoint2_T org, T cosa, T sina);

    void Rotate(T angle);

    void Scale(RCPoint2_T org, T s);

    void Scale(RCPoint2_T basept, RCPoint2_T scalefactors);

    Point2_T<T> Multiply(T scale);

    void Normalize();
	Point2_T<T> PolarGenerate(const T &dist,const T &AzimuthAngle) const {
		T DeltaE,DeltaN;
		DeltaE = dist*sin(AzimuthAngle);
		DeltaN = dist*cos(AzimuthAngle);
		return Point2_T<T>(X+DeltaE,Y+DeltaN);
	};
    //EXT_API void mirror(const Point2D& p0, const Point2D& p1);

    bool operator==(RCPoint2_T point) const        
    { 
        return X == point.X && Y == point.Y; 
    }

    bool operator!=(RCPoint2_T point) const    
    {  
        return X != point.X || Y != point.Y;    
    }

    void operator+=(RCPoint2_T point)    
    { 
        X += point.X;  Y += point.Y;
    }

    void operator-=(RCPoint2_T point)    
    { 
        X -= point.X; 
        Y -= point.Y;    
    }

    void operator*=(RCPoint2_T point)    
    { 
        X *= point.X; 
        Y *= point.Y;    
    }

    Point2_T<T> operator+(RCPoint2_T arg) const;

    Point2_T<T> operator-(RCPoint2_T arg) const;

    Point2_T<T> operator*(const double scalar) const;

    Point2_T<T> operator/(const double scalar) const;

    void Negative()                        
    {
        X = -X;    
        Y = -Y;        
    }

    bool ApproxEqual(RCPoint2_T src, T tolerance) const;

    double Distance(RCPoint2_T pt) const;    

    double DistanceSqr(RCPoint2_T pt) const;    

    double Length() const;

    double Dot(RCPoint2_T v) const;    // 内积

    double Cross(RCPoint2_T dst) const;

    /// <summary>
    /// 点相对点的对称点
    /// </summary>
    /// <param name="origin"></param>
    /// <returns></returns>
    Point2_T<T> Symmetry(RCPoint2_T origin);

    Point2_T<T>& Symmetry(RCPoint2_T  origin, bool collapsed);

    Point2_T<T> Mirror(RCPoint2_T start, RCPoint2_T end);

    Point2_T<T> Mirror(RCPoint2_T start, RCPoint2_T end, bool collapsed);
};

template<class T>
int Point2_T<T>::SizeofPoint2 = sizeof(Point2_T<T>) / sizeof(T);

template<class T>
RCPoint2_T Point2_T<T>::operator=(RCPoint2_T src)
{
    X = src.X;
    Y = src.Y;
    return *this;
}

template<class T>
Point2_T<T> Point2_T<T>::operator+(RCPoint2_T arg) const
{
    Point2_T<T> rtv;
    rtv.X = X + arg.X;
    rtv.Y = Y + arg.Y;
    return rtv;
}

template<class T>
Point2_T<T> Point2_T<T>::operator-(RCPoint2_T arg) const
{
    Point2_T<T> rtv;
    rtv.X = X - arg.X;
    rtv.Y = Y - arg.Y;
    return rtv;
}

template<class T>
Point2_T<T> Point2_T<T>::operator*(const double scalar) const
{
    return Point2_T<T>(X * scalar,Y * scalar);
}

template<class T>
Point2_T<T> Point2_T<T>::operator/(const double scalar) const
{
    return Point2_T<T>(X / scalar,Y / scalar);
}

template<class T>    
bool Point2_T<T>::ApproxEqual(RCPoint2_T src, T tolerance) const
{
    //xApplication::Assert(tolerance> 0);
    T x = X - src.X;
    T y = Y - src.Y;
    return x * x + y * y <= tolerance * tolerance;
}

template<class T>
void Point2_T<T>::Rotate(RCPoint2_T co, T cosa, T sina)
{
    T temp = (X -= co.X);
    Y -= co.Y;
    X = temp * cosa - Y * sina + co.X;
    Y = temp * sina + Y * cosa + co.Y;
}

template<class T>
void Point2_T<T>::Rotate(T angle)
{   
    T cosa = cos(angle);
    T sina = sin(angle);

    T tx = X;
    T ty = Y;
    X = tx * cosa - ty * sina;
    Y = tx * sina + ty * cosa;
}

template<class T>
void Point2_T<T>::Scale(RCPoint2_T org, T s)
{ 
    X = org.X+s*(X-org.X); 
    Y = org.Y+s*(Y-org.Y);    
}

template<class T>
void Point2_T<T>::Scale(RCPoint2_T basept, RCPoint2_T scalefactors)
{
    double dx = scalefactors.X * (X - basept.X);
    double dy = scalefactors.Y * (Y - basept.Y);
    X = basept.X + dx;
    Y = basept.Y + dy;
}

template<class T>
Point2_T<T> Point2_T<T>::Multiply(T scale)
{
    X *= scale;
    Y *= scale;
    return *this;
}

template<class T>
double Point2_T<T>::Distance(RCPoint2_T pt) const
{
    return sqrt(DistanceSqr(pt));
}

template<class T>
double Point2_T<T>::DistanceSqr(RCPoint2_T pt) const
{
    T x = X - pt.X;
    T y = Y - pt.Y;
    return (x * x + y * y);
}

template<class T>
double Point2_T<T>::Length() const
{
    return (double)sqrt(X * X + Y * Y);
} 

template<class T>
double Point2_T<T>::Dot(RCPoint2_T v) const
{
    return X * v.X + Y * v.Y;
}

template<class T>
double Point2_T<T>::Cross(RCPoint2_T dst) const
{
    return X * dst.Y - Y * dst.X;
}

template<class T>
void Point2_T<T>::Normalize()
{
    double dis = sqrt(X * X + Y * Y);
    if (dis > 1.0e-11)
    {
        X /= dis;
        Y /= dis;
    }
}

template<class T>
Point2_T<T> Point2_T<T>::Symmetry(RCPoint2_T origin)
{
    return Symmetry(origin, true);
}

template<class T>
Point2_T<T>& Point2_T<T>::Symmetry(RCPoint2_T  origin, bool collapsed)
{
    if (collapsed)
    {
        X = 2 * origin.X - X;
        Y = 2 * origin.Y - Y;
        return *this;
    }
    else
    {
        // ?
        //Point2D pt = Clone() as Point2D;
        return Point2_T<T>(2 * origin.X - X, 2 * origin.Y - Y);
    }
}

template<class T>
Point2_T<T> Point2_T<T>::Mirror(RCPoint2_T start, RCPoint2_T end)
{
    return Mirror(start, end, true);
}

template<class T>
Point2_T<T> Point2_T<T>::Mirror(RCPoint2_T start, RCPoint2_T end, bool collapsed)
{
    if (collapsed)
    {
        double x0 = start.X;
        double y0 = start.Y;
        double x1 = end.X;
        double y1 = end.Y;
        double deltax = x1 - x0;
        double deltay = y1 - y0;
        double xp = X;
        double yp = Y;

        double u = 0;
        if (deltay == 0 && deltax == 0)
            u = 0;
        else
            u = ((xp - x0) * deltax + (yp - y0) * deltay) / (deltax * deltax + deltay * deltay);

        //垂足
        double x = x0 + u * deltax;
        double y = y0 + u * deltay;

        x = x * 2 - X;
        y = y * 2 - Y;
        X = x;
        Y = y;
        return *this;
    }
    else
    {
        Point2_T<T> pt;
        return pt.Mirror(start, end, true);
    }
} 

typedef Point2_T<double> Point2D;
typedef Point2_T<float> Point2F;
typedef Point2D Vector2D;
typedef Point2F Vector2F;
typedef Point2_T<int> Point2I;

class Point2Ds : public xArray<Point2D>
{
public:
	Point2Ds(){}
	Point2Ds(const Point2Ds& src)
	{
		resize(src.Count());
		for(int i=0;i <src.Count();i++)
			(*this)[i]=src[i];
	}

    double GetDistance(bool isClosed = false) const
    {
        double dis = 0.0;
        int count = this->Count();
        if (count > 1)
        {
            for (int i = 0; i < count - 1; i++)
            {
                Point2D pt1 = this->At(i);
                Point2D pt2 = this->At(i + 1);
                dis += pt1.Distance(pt2);
            }
            if (isClosed)
            {
                Point2D pt1 = this->At(count - 1);
                Point2D pt2 = this->At(0);
                dis += pt1.Distance(pt2);
            }
        }
        return dis;
    }
};

class Part2D : public Point2Ds
{
    int count;

public:
    using Point2Ds::Add;

    Part2D() { }
	Part2D(const Point2Ds& src) : Point2Ds(src) {}

public:
    void Add(double x, double y)
    {
        Point2D t(x, y);
        Add(t);
    }
};

class Part2Ds: public xArray<Part2D>
{
public:
    Part2Ds()
    { 
    }
};


class Point2Is : public xArray<Point2I>
{
};