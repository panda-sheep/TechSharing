注：大部分文本复制于维基百科词条`C++11`的中文页面，所以会有很多繁体字。

# 允许sizeof作用在类的成员上，
```cpp
struct SomeType { OtherType member; };

sizeof(SomeType::member); // 直接由SomeType型別取得非靜態成員的大小，C++03不行。C++11允許
```
這會傳回OtherType的大小。C++03並不允許這樣做，所以會引發編譯錯誤。C++11將會允許這種使用。 

# 线程支持

https://zh.cppreference.com/w/cpp/thread

# 正则表达式

https://zh.cppreference.com/w/cpp/regex


# 可扩展的随机数功能

C++11提供了产生伪随机数的新方法，而不是像C那样算法由各类库开发者决定。

C++11的随机数功能分为两部分：第一，一个随机数生成引擎，其中包含生成引擎的状态，用于产生随机数;第二，一个分布，可以指定随机数产生范围，和随机数产生方式。

三种算法：

|模板类 	|整型/浮点型| 品质 	|速度 	|状态数*|
|-          |-      |-      |-      |-  |
|linear_congruential |整形 	|低| 中等| 	1|
|subtract_with_carry |皆可|中等|快| 25|
|mersenne_twister |整型|佳 |快 |624|

C++11将会提供一些标准分布：uniform_int_distribution（离散型均匀分布），bernoulli_distribution(伯努利分布），geometric_distribution（几何分布），poisson_distribution（泊松分布），binomial_distribution（二项分布），uniform_real_distribution（离散型均匀分布)，exponential_distribution（指数分布），normal_distribution（常态分布）和gamma_distribution（伽玛分布）。

底下描述一个随机数生成物件如何由随机数生成引擎和分布构成：

```cpp
std::uniform_int_distribution<int> distribution(0, 99); // 以离散型均匀分布方式产生int随机数，范围落在0到99之间
std::mt19937 engine; // 建立随机数生成引擎
auto generator = std::bind(distribution, engine); // 利用bind将随机数生成引擎和分布组合成一个随机数生成物件
int random = generator();  // 产生随机数
```
# 包装引用
我们可以透过实体化样板类reference_wrapper得到一个包装引用（wrapper reference）。包装引用类似于一般的引用。对于任意物件，我们可以透过模板类ref得到一个包装引用（至于constant reference则可透过cref得到）。

当样板函式需要形参的引用而非其拷贝，这时包装引用就能派上用场：

```cpp
// 此函数将得到形参'r'的引用并对r加一
void f (int &r)  { r++; }

// 样板函式
template<class F, class P> void g (F f, P t)  { f(t); }

int main()
{
    int i = 0 ;
    g (f, i) ;  // 实体化'g<void (int &r), int>' 
                // 'i'不会被修改
    std::cout << i << std::endl;  // 输出0

    g (f, std::ref(i));  // 实体化'g<void(int &r),reference_wrapper<int>>'
                         // 'i'会被修改
    std::cout << i << std::endl;  // 输出1
}
```
这项功能将加入头文件`<utility>`之中，而非透过扩展语言来得到这项功能。 

# 多态函数对象包装器

https://zh.cppreference.com/w/cpp/header/functional

针对函数对象的多态包装器（又称多态函数对象包装器）在语义和语法上和函数指针相似，但不像函式指标那麽狭隘。只要能被调用，且其参数能与包装器相容的都能以多态函数对象包装器称之（函式指标，成员函式指标或仿函式）。

透过以下例子，我们可以了解多态函数对象包装器的特性：

```cpp
std::function<int (int, int)> func;  // 利用样板类'function'
                                     // 建立包装器
std::plus<int> add;  // 'plus'被宣告为'template<class T> T plus( T, T ) ;'
                     //  因此'add'的型别是'int add( int x, int y )'
func = &add;  // 可行。'add'的型参和回返值型别与'func'相符
 
int a = func (1, 2);  // 注意：若包装器'func'没有参考到任何函式
                      // 会丢出'std::bad_function_call'例外

std::function<bool (short, short)> func2 ;
if(!func2) { // 因为尚未赋值与'func2'任何函式，此条件式为真

    bool adjacent(long x, long y);
    func2 = &adjacent ;  // 可行。'adjacent'的型参和回返值型别可透过型别转换进而与'func2'相符
  
    struct Test {
        bool operator()(short x, short y);
    };
    Test car;
    func = std::ref(car);  // 样板类'std::ref'回传一个struct 'car'
                           // 其成员函式'operator()'的包装
}
func = func2;  // 可行。'func2'的型参和回返值型别可透过型别转换进而与'func'相符
```

模板类function将定义在头文件`<functional>`，而不须更动到语言本身。 

# 用于元编程的型别属性

对于那些能自行创建或修改本身或其它程式的程式，我们称之为元编程。这种行为可以发生在编译或执行期。C++标准委员会已经决定引进一组由模板实现的函式库，程式员可利用此一函式库于编译期进行元编程。

底下是一个以元编程来计算指数的例子：

```cpp
template<int B, int N>
struct Pow {
    // recursive call and recombination.
    enum{ value = B*Pow<B, N-1>::value };
};

template< int B > 
struct Pow<B, 0> { 
    // ''N == 0'' condition of termination.
    enum{ value = 1 };
};
int quartic_of_three = Pow<3, 4>::value;
```
许多演算法能作用在不同的资料型别；C++模板支援泛型，这使得代码能更紧凑和有用。然而，演算法经常会需要目前作用的资料型别的资讯。这种资讯可以透过型别属性（type traits）于模板实体化时将该资讯萃取出来。

型别属性能识别一个物件的种类和有关一个型别（class或struct）的特徵。标头档`<type_traits>`描述了我们能识别那些特徵。

底下的例子说明了模板函式‘elaborate’是如何根据给定的资料型别，从而实体化某一特定的演算法（algorithm.do_it）。

```cpp
// 演算法一
template< bool B > struct Algorithm {
    template<class T1, class T2> static int do_it (T1 &, T2 &)  { /*...*/ }
};

// 演算法二
template<> struct Algorithm<true> {
    template<class T1, class T2> static int do_it (T1, T2)  { /*...*/ }
};

// 根据给定的型别，实体化之后的'elaborate'会选择演算法一或二
template<class T1, class T2> 
int elaborate (T1 A, T2 B) 
{
    // 若T1为int且T2为float，选用演算法二
    // 其它情况选用演算法一
    return Algorithm<std::is_integral<T1>::value && std::is_floating_point<T2>::value>::do_it( A, B ) ;
}
```
此种编程技巧能写出优美、简洁的代码；然而除错是此种编程技巧的弱处：编译期的错误讯息让人不知所云，执行期的除错更是困难。 


# 用于计算函数对象返回类型的统一方法

要在编译期决定一个样板仿函式的回返值型别并不容易，特别是当回返值依赖于函式的参数时。举例来说：

```cpp
struct Clear {
    int    operator()(int);     // 参数与回返值的型别相同
    double operator()(double);  // 参数与回返值的型别相同
};

template <class Obj> 
class Calculus {
public:
    template<class Arg> Arg operator()(Arg& a) const
    {
        return member(a);
    }
private:
    Obj member;
};
```

实体化样板类`Calculus<Clear>`，Calculus的仿函式其回返值总是和Clear的仿函式其回返值具有相同的型别。然而，若给定类别Confused:

```cpp
struct Confused {
    double operator()(int);     // 参数与回返值的型别不相同
    int    operator()(double);  // 参数与回返值的型别不相同
};
```

企图实体化样板类Calculus<Confused>将导致Calculus的仿函式其回返值和类别Confused的仿函式其回返值有不同的型别。对于int和double之间的转换，编译器将给出警告。

模板std::result_of被TR1引进且被C++11所採纳，可允许我们决定和使用一个仿函式其回返值的型别。底下，CalculusVer2物件使用std::result_of物件来推导其仿函式的回返值型别：`

```cpp
template< class Obj >
class CalculusVer2 {
public:
    template<class Arg>
    typename std::result_of<Obj(Arg)>::type operator()(Arg& a) const
    { 
        return member(a);
    }
private:
    Obj member;
};
```

如此一来，在实体化CalculusVer2<Confused>其仿函式时，不会有型别转换，警告或是错误发生。

模板std::result_of在TR1和C++11有一点不同。TR1的版本允许实作在特殊情况下，可以无法决定一个函式呼叫其回返值型别。然而，因为C++11支持了decltype，实作被要求在所有情况下，皆能计算出回返值型别。 


# iota 函数
主条目：iota函数

iota 函数可将给定区间的值设定为从某值开始的连续值，例如将连续十个整数设定为从 1 开始的连续整数（即 1、2、3、4、5、6、7、8、9、10）。
```cpp
#include <iostream>
#include <array>
#include <numeric>

std::array<int, 10> ai;
std::iota(ai.begin(), ai.end(), 1);
for(int i: ai){
  std::cout<<i<<" ";//1 2 3 4 5 6 7 8 9 10 
}
```

