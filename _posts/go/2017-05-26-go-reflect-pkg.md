---
layout: post
title: golang反射的实现
tags:
- go-language
categories: go-language
description: golang反射的实现
---


包reflect用于实现运行时反射(run-time reflect)，其允许一个程序操作任何类型的object。典型的用法就是获取一个interface{}类型调用reflect.TypeOf()，从而获得该抽象类型的实际的表示Type。

对于某一个变量调用ValueOf()会返回Value，其代表着运行时的数据值。

<!-- more -->


## 1. Type类型的抽象

Type用于表示Go的类型的抽象。但是值得指出的是，并不是go中的所有类型都支持Type接口中定义的所有方法。对于Type接口中方法，假如某些类型不支持，我们会在介绍该方法时对其进行相应的说明。


通常对于一个go的类型，我们首先会调用Kind()方法以找出其切确的类型，然后再会针对该切确类型调用相应的方法来使用。
{% highlight string %}
type Type interface {
	// Methods applicable to all types.

	// Align returns the alignment in bytes of a value of
	// this type when allocated in memory.
	Align() int

	// FieldAlign returns the alignment in bytes of a value of
	// this type when used as a field in a struct.
	FieldAlign() int

	// Method returns the i'th method in the type's method set.
	// It panics if i is not in the range [0, NumMethod()).
	//
	// For a non-interface type T or *T, the returned Method's Type and Func
	// fields describe a function whose first argument is the receiver.
	//
	// For an interface type, the returned Method's Type field gives the
	// method signature, without a receiver, and the Func field is nil.
	Method(int) Method

	// MethodByName returns the method with that name in the type's
	// method set and a boolean indicating if the method was found.
	//
	// For a non-interface type T or *T, the returned Method's Type and Func
	// fields describe a function whose first argument is the receiver.
	//
	// For an interface type, the returned Method's Type field gives the
	// method signature, without a receiver, and the Func field is nil.
	MethodByName(string) (Method, bool)

	// NumMethod returns the number of exported methods in the type's method set.
	NumMethod() int

	// Name returns the type's name within its package for a defined type.
	// For other (non-defined) types it returns the empty string.
	Name() string

	// PkgPath returns a defined type's package path, that is, the import path
	// that uniquely identifies the package, such as "encoding/base64".
	// If the type was predeclared (string, error) or not defined (*T, struct{},
	// []int, or A where A is an alias for a non-defined type), the package path
	// will be the empty string.
	PkgPath() string

	// Size returns the number of bytes needed to store
	// a value of the given type; it is analogous to unsafe.Sizeof.
	Size() uintptr

	// String returns a string representation of the type.
	// The string representation may use shortened package names
	// (e.g., base64 instead of "encoding/base64") and is not
	// guaranteed to be unique among types. To test for type identity,
	// compare the Types directly.
	String() string

	// Kind returns the specific kind of this type.
	Kind() Kind

	// Implements reports whether the type implements the interface type u.
	Implements(u Type) bool

	// AssignableTo reports whether a value of the type is assignable to type u.
	AssignableTo(u Type) bool

	// ConvertibleTo reports whether a value of the type is convertible to type u.
	ConvertibleTo(u Type) bool

	// Comparable reports whether values of this type are comparable.
	Comparable() bool

	// Methods applicable only to some types, depending on Kind.
	// The methods allowed for each kind are:
	//
	//	Int*, Uint*, Float*, Complex*: Bits
	//	Array: Elem, Len
	//	Chan: ChanDir, Elem
	//	Func: In, NumIn, Out, NumOut, IsVariadic.
	//	Map: Key, Elem
	//	Ptr: Elem
	//	Slice: Elem
	//	Struct: Field, FieldByIndex, FieldByName, FieldByNameFunc, NumField

	// Bits returns the size of the type in bits.
	// It panics if the type's Kind is not one of the
	// sized or unsized Int, Uint, Float, or Complex kinds.
	Bits() int

	// ChanDir returns a channel type's direction.
	// It panics if the type's Kind is not Chan.
	ChanDir() ChanDir

	// IsVariadic reports whether a function type's final input parameter
	// is a "..." parameter. If so, t.In(t.NumIn() - 1) returns the parameter's
	// implicit actual type []T.
	//
	// For concreteness, if t represents func(x int, y ... float64), then
	//
	//	t.NumIn() == 2
	//	t.In(0) is the reflect.Type for "int"
	//	t.In(1) is the reflect.Type for "[]float64"
	//	t.IsVariadic() == true
	//
	// IsVariadic panics if the type's Kind is not Func.
	IsVariadic() bool

	// Elem returns a type's element type.
	// It panics if the type's Kind is not Array, Chan, Map, Ptr, or Slice.
	Elem() Type

	// Field returns a struct type's i'th field.
	// It panics if the type's Kind is not Struct.
	// It panics if i is not in the range [0, NumField()).
	Field(i int) StructField

	// FieldByIndex returns the nested field corresponding
	// to the index sequence. It is equivalent to calling Field
	// successively for each index i.
	// It panics if the type's Kind is not Struct.
	FieldByIndex(index []int) StructField

	// FieldByName returns the struct field with the given name
	// and a boolean indicating if the field was found.
	FieldByName(name string) (StructField, bool)

	// FieldByNameFunc returns the struct field with a name
	// that satisfies the match function and a boolean indicating if
	// the field was found.
	//
	// FieldByNameFunc considers the fields in the struct itself
	// and then the fields in any embedded structs, in breadth first order,
	// stopping at the shallowest nesting depth containing one or more
	// fields satisfying the match function. If multiple fields at that depth
	// satisfy the match function, they cancel each other
	// and FieldByNameFunc returns no match.
	// This behavior mirrors Go's handling of name lookup in
	// structs containing embedded fields.
	FieldByNameFunc(match func(string) bool) (StructField, bool)

	// In returns the type of a function type's i'th input parameter.
	// It panics if the type's Kind is not Func.
	// It panics if i is not in the range [0, NumIn()).
	In(i int) Type

	// Key returns a map type's key type.
	// It panics if the type's Kind is not Map.
	Key() Type

	// Len returns an array type's length.
	// It panics if the type's Kind is not Array.
	Len() int

	// NumField returns a struct type's field count.
	// It panics if the type's Kind is not Struct.
	NumField() int

	// NumIn returns a function type's input parameter count.
	// It panics if the type's Kind is not Func.
	NumIn() int

	// NumOut returns a function type's output parameter count.
	// It panics if the type's Kind is not Func.
	NumOut() int

	// Out returns the type of a function type's i'th output parameter.
	// It panics if the type's Kind is not Func.
	// It panics if i is not in the range [0, NumOut()).
	Out(i int) Type

	common() *rtype
	uncommon() *uncommonType
}
{% endhighlight %}

对于Type中的Kind()方法，其返回如下类型中的一种：
{% highlight string %}
// A Kind represents the specific kind of type that a Type represents.
// The zero Kind is not a valid kind.
type Kind uint

const (
	Invalid Kind = iota
	Bool
	Int
	Int8
	Int16
	Int32
	Int64
	Uint
	Uint8
	Uint16
	Uint32
	Uint64
	Uintptr
	Float32
	Float64
	Complex64
	Complex128
	Array
	Chan
	Func
	Interface
	Map
	Ptr
	Slice
	String
	Struct
	UnsafePointer
)
{% endhighlight %}

### 1.1 rtype的实现
前面我们看到reflect中有```Type```抽象接口，其是用于提供接口给外部用户使用的。在refelect包的内部，其有```rtype```来表示一个```类型```:
{% highlight string %}
type tflag uint8

const (
	// tflagUncommon means that there is a pointer, *uncommonType,
	// just beyond the outer type structure.
	//
	// For example, if t.Kind() == Struct and t.tflag&tflagUncommon != 0,
	// then t has uncommonType data and it can be accessed as:
	//
	//	type tUncommon struct {
	//		structType
	//		u uncommonType
	//	}
	//	u := &(*tUncommon)(unsafe.Pointer(t)).u
	tflagUncommon tflag = 1 << 0

	// tflagExtraStar means the name in the str field has an
	// extraneous '*' prefix. This is because for most types T in
	// a program, the type *T also exists and reusing the str data
	// saves binary size.
	tflagExtraStar tflag = 1 << 1

	// tflagNamed means the type has a name.
	tflagNamed tflag = 1 << 2
)

// rtype is the common implementation of most values.
// It is embedded in other struct types.
//
// rtype must be kept in sync with ../runtime/type.go:/^type._type.
type rtype struct {
	size       uintptr
	ptrdata    uintptr  // number of bytes in the type that can contain pointers
	hash       uint32   // hash of type; avoids computation in hash tables
	tflag      tflag    // extra type information flags
	align      uint8    // alignment of variable with this type
	fieldAlign uint8    // alignment of struct field with this type
	kind       uint8    // enumeration for C
	alg        *typeAlg // algorithm table
	gcdata     *byte    // garbage collection data
	str        nameOff  // string form
	ptrToThis  typeOff  // type for pointer to this type, may be zero
}

// a copy of runtime.typeAlg
type typeAlg struct {
	// function for hashing objects of this type
	// (ptr to object, seed) -> hash
	hash func(unsafe.Pointer, uintptr) uintptr
	// function for comparing objects of this type
	// (ptr to object A, ptr to object B) -> ==?
	equal func(unsafe.Pointer, unsafe.Pointer) bool
}

// Method on non-interface type
type method struct {
	name nameOff // name of method
	mtyp typeOff // method type (without receiver)
	ifn  textOff // fn used in interface call (one-word receiver)
	tfn  textOff // fn used for normal method call
}

// uncommonType is present only for defined types or types with methods
// (if T is a defined type, the uncommonTypes for T and *T have methods).
// Using a pointer to this struct reduces the overall size required
// to describe a non-defined type with no methods.
type uncommonType struct {
	pkgPath nameOff // import path; empty for built-in types like int, string
	mcount  uint16  // number of methods
	xcount  uint16  // number of exported methods
	moff    uint32  // offset from this uncommontype to [mcount]method
	_       uint32  // unused
}
{% endhighlight %}

之后是针对某一些具体类型，例如array类型，chan类型需要为其提供特定的内部表示。

### 1.2 Method的抽象
{% highlight string %}
// Method represents a single method.
type Method struct {
	// Name is the method name.
	// PkgPath is the package path that qualifies a lower case (unexported)
	// method name. It is empty for upper case (exported) method names.
	// The combination of PkgPath and Name uniquely identifies a method
	// in a method set.
	// See https://golang.org/ref/spec#Uniqueness_of_identifiers
	Name    string
	PkgPath string

	Type  Type  // method type
	Func  Value // func with receiver as first argument
	Index int   // index for Type.Method
}
{% endhighlight %}


## 2. Value的抽象
对于go中的一个变量，我们可以用Value来对其进行抽象。值得注意的是，对于任意一个go类型的变量，其可能并不是支持Value的全部方法。
{% highlight string %}
type Value struct {
	// typ holds the type of the value represented by a Value.
	typ *rtype

	// Pointer-valued data or, if flagIndir is set, pointer to data.
	// Valid when either flagIndir is set or typ.pointers() is true.
	ptr unsafe.Pointer

	// flag holds metadata about the value.
	// The lowest bits are flag bits:
	//	- flagStickyRO: obtained via unexported not embedded field, so read-only
	//	- flagEmbedRO: obtained via unexported embedded field, so read-only
	//	- flagIndir: val holds a pointer to the data
	//	- flagAddr: v.CanAddr is true (implies flagIndir)
	//	- flagMethod: v is a method value.
	// The next five bits give the Kind of the value.
	// This repeats typ.Kind() except for method values.
	// The remaining 23+ bits give a method number for method values.
	// If flag.kind() != Func, code can assume that flagMethod is unset.
	// If ifaceIndir(typ), code can assume that flagIndir is set.
	flag

	// A method value represents a curried method invocation
	// like r.Read for some receiver r. The typ+val+flag bits describe
	// the receiver r, but the flag's Kind bits say Func (methods are
	// functions), and the top bits of the flag give the method number
	// in r's type's method table.
}
{% endhighlight %}
通过上面的定义我们可以看到，对于golang中的一个变量，其抽象后的Value类型其实是对变量的一个封装：

![go-reflect-value](https://ivanzz1001.github.io/records/assets/img/go/go_reflect_value.jpg)



<br />
<br />
**[参看]：**



<br />
<br />
<br />

