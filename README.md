# yproto
---

Yet another protobuf.

这是一个protobuf的替代，能生成更简洁的代码，二进制协议更容易人工查阅，
特别适合嵌入式设备，适合各种土制协议。从功能上讲，protobuf更加优秀，
有更好的支持，更多的语言，协议兼容性也很好。yproto 的出发点是嵌入式设备
上的土制协议，这种地方使用protobuf会有一些阻力。

## 使用

获得yproto的代码并编译：

```sh
cd yproto/src
make
```

编写协议定义文件，参考 samples/1.yproto。使用 yproto 解析定义文件，生成代码：

```sh
./yproto samples/1.yproto
```

上述命令将生成 1.yproto.c 和 1.yproto.h，将其连同 yproto.c 和 yproto.h 复制到
自己的程序代码中，编译连接即可使用其中定义的编码和解码逻辑。

每个数据结构生成三个函数：

```C
// 编码到 buf 中，返回值表示使用了多少字节的 buf
int encodeXXX(u8 *buf, struct XXX *v);
// 从 buf 中解码获得数据，返回值表示使用了多少字节的 buf
int decodeXXX(u8 *buf, struct XXX *v);
// C 的数据结构中有 vector，这是malloc的数据，需要使用这个函数清理
void clearXXX(struct XXX *v);
```

## 格式和生成的二进制

1. 原生格式
原生格式包括 i8, u8, i16, u16, i32, u32, i64, u64, 分别表示8, 16, 32, 64
字节的有符号整数和无符号整数。生成的二进制是大端序整数。

2. struct 格式
所有的数据都包含在 struct 中。用户可将 struct 嵌入到另一个struct中。
```
struct A {
    ...
}

struct B {
    A valOfA;
}
```
最后每个 struct 会生成各自的编码和解码函数。编码后的 struct 是其中所有数据的顺序排列。

3. 数组
支持数组，例如：

```C
// 一维数组
[10]i8 arr1;
// 二维数组
[10][20]A arrOfA;
// 三维数组
[10][20][10] vector<i8, A> arrayOfVectorOfA;
```
数组编码后是连续的数据，例如 `[10]i8 arr1` 编码后将是10个字节的连续数据。
`[10][20]A arrOfA` 编码后是 200 个连续的A数据。

4. 可变数组

是可变数组名为 Vector，包括一个长度，和一个数组。

```C
// 4字节表示数据个数n，其后存储n个连续的u8
vector<i32, u8> va;
// 2字节表示数据个数n，气候存储n个A结构的值
vector<u16, A> vb;

// 10个连续的vector<u16, A> vc
[10]vector<u16, A> vc;
```
存储vector的vector和存储数组的vector是不允许的，这会生成奇怪的代码，同时为了方便，
将这两种类型禁止了。可以将vector或数组包含在struct中，再将struct包含在vector中，
生成的二进制数据格式与上述不允许的两种方式生成的格式相同。

