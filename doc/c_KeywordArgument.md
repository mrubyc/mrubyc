# About keyword argument written by C.

## case 1
  `def func1( k1:, k2: )`

  * k1, k2 引数が指定されていないとエラー
  * k1, k2 以外が指定されているとエラー

```
static void c_func1(struct VM *vm, mrbc_value v[], int argc)
{
  MRBC_KW_ARG(k1, k2);
  if( !MRBC_KW_MANDATORY(k1, k2) ) goto RETURN;
  if( !MRBC_KW_END() ) goto RETURN;

  mrbc_p(&k1);
  mrbc_p(&k2);

 RETURN:
  MRBC_KW_DELETE(k1, k2);
}
```


## case 2
  `def func2( k1:11, k2:22 )`

  * キーワード引数はあっても無くても良い
  * k1, k2 以外が指定されているとエラー

```
static void c_func2(struct VM *vm, mrbc_value v[], int argc)
{
  MRBC_KW_ARG(k1, k2);
  if( !MRBC_KW_END() ) goto RETURN;

  if( !MRBC_KW_ISVALID(k1) ) k1 = mrbc_integer_value(11);
  if( !MRBC_KW_ISVALID(k2) ) k2 = mrbc_integer_value(22);

  mrbc_p(&k1);
  mrbc_p(&k2);

 RETURN:
  MRBC_KW_DELETE(k1, k2);
}
```


## case 3
  `def func3( k1:, k2:"ABC" )`

  * k1 引数が指定されていないとエラー
  * k1, k2 以外が指定されているとエラー

```
static void c_func3(struct VM *vm, mrbc_value v[], int argc)
{
  MRBC_KW_ARG(k1, k2);
  if( !MRBC_KW_MANDATORY(k1) ) goto RETURN;
  if( !MRBC_KW_END() ) goto RETURN;

  if( !MRBC_KW_ISVALID(k2) ) k2 = mrbc_string_new_cstr(vm, "ABC");

  mrbc_p(&k1);
  mrbc_p(&k2);

 RETURN:
  MRBC_KW_DELETE(k1, k2);
}
```


## case 4
  `def func4( k1:, k2:, **dict )`

  * k1, k2 引数が指定されていないとエラー

```
static void c_func4(struct VM *vm, mrbc_value v[], int argc)
{
  MRBC_KW_ARG(k1, k2);
  MRBC_KW_DICT(dict);
  if( !MRBC_KW_MANDATORY(k1, k2) ) goto RETURN;

  mrbc_p(&k1);
  mrbc_p(&k2);
  mrbc_p(&dict);

 RETURN:
  MRBC_KW_DELETE(k1, k2, dict);
}
```


## case 5
  `def func5( k1:11, k2:22, **dict )`

  * キーワード引数はあっても無くても良い

```
static void c_func5(struct VM *vm, mrbc_value v[], int argc)
{
  MRBC_KW_ARG(k1, k2);
  MRBC_KW_DICT(dict);

  if( !MRBC_KW_ISVALID(k1) ) k1 = mrbc_integer_value(11);
  if( !MRBC_KW_ISVALID(k2) ) k2 = mrbc_integer_value(22);

  mrbc_p(&k1);
  mrbc_p(&k2);
  mrbc_p(&dict);

 RETURN:
  MRBC_KW_DELETE(k1, k2, dict);
}
```


## case 6
  `def func6( **dict )`

  * キーワード引数はあっても無くても良い

```
static void c_func6(struct VM *vm, mrbc_value v[], int argc)
{
  MRBC_KW_DICT(dict);

  mrbc_p(&dict);

 RETURN:
  MRBC_KW_DELETE(dict);
}
```
